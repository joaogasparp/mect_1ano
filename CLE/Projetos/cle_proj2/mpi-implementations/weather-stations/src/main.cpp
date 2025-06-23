#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <mpi.h>
#include <cstdio>
#include <cstring>
#include <limits>

struct WSData {
    float min = std::numeric_limits<float>::max();
    float max = std::numeric_limits<float>::lowest();
    float total = 0.0f;
    int count = 0;

    void update(float value) {
        total += value;
        count++;
        if (value < min) min = value;
        if (value > max) max = value;
    }

    void update(const WSData& other) {
        total += other.total;
        count += other.count;
        if (other.min < min) min = other.min;
        if (other.max > max) max = other.max;
    }
    
    float avg() const {
        return count == 0 ? 0.0f : total / count;
    }
};

struct StationData {
    char name[128];
    WSData data;

    StationData() {
        std::memset(name, 0, sizeof(name));
    }
};

bool compare_by_max(const StationData &a, const StationData &b) {
    return a.data.max < b.data.max;
}

void process_line(const std::string &line, std::map<std::string, WSData> &stationData)
{
    std::string station, temp_str;
    size_t i = 0;

    while (i < line.size() && line[i] != ';')
    {
        station += line[i++];
    }
    if (i >= line.size())
        return;

    i++;
    while (i < line.size())
    {
        temp_str += line[i++];
    }
    try
    {
        float temp = std::stof(temp_str);
        WSData &data = stationData[station];
        data.total += temp;
        data.count++;
        if (data.count == 1)
        {
            data.min = temp;
            data.max = temp;
        }
        else
        {
            if (temp < data.min)
                data.min = temp;
            if (temp > data.max)
                data.max = temp;
        }
    }
    catch (const std::invalid_argument &)
    {
        std::cerr << "WARNING: Invalid temperature value in line: " << line << std::endl;
    }
}

std::string serialize_data(const std::map<std::string, WSData> &data)
{
    std::ostringstream oss;
    for (const auto &entry : data)
    {
        oss << entry.first << ";" << entry.second.total << ";" << entry.second.min
            << ";" << entry.second.max << ";" << entry.second.count << "\n";
    }
    return oss.str();
}

void deserialize_data(const std::string &serialized, std::map<std::string, WSData> &data)
{
    std::istringstream iss(serialized);
    std::string line;
    while (std::getline(iss, line))
    {
        std::istringstream line_stream(line);
        std::string station;
        WSData ws_data;
        char delim;
        if (std::getline(line_stream, station, ';'))
        {
            line_stream >> ws_data.total >> delim >> ws_data.min >> delim >> ws_data.max >> delim >> ws_data.count;
            data[station] = ws_data;
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    double start_time = MPI_Wtime();

    if (argc < 2) {
        if (rank == 0) std::cerr << "Usage: " << argv[0] << " <input_file>\n";
        MPI_Finalize();
        return 1;
    }

    const char* filename = argv[1];
    MPI_File file;
    MPI_Offset file_size;

    MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    MPI_File_get_size(file, &file_size);
    MPI_File_close(&file);

    MPI_Offset chunk_size = file_size / size;
    MPI_Offset start = rank * chunk_size;
    MPI_Offset end = (rank == size - 1) ? file_size : start + chunk_size;

    std::ifstream infile(filename);
    infile.seekg(start);

    std::unordered_map<std::string, WSData> local_map;
    std::string line;

    if (rank != 0) std::getline(infile, line);

    MPI_Offset current_pos = infile.tellg();
    while (current_pos < end && std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string name;
        float temp;
        if (std::getline(iss, name, ';') && iss >> temp) {
            local_map[name].update(temp);
        }
        current_pos = infile.tellg();
    }

    std::vector<StationData> local_vector;
    for (const auto& [name, data] : local_map) {
        StationData sd;
        std::strncpy(sd.name, name.c_str(), sizeof(sd.name) - 1);
        sd.data = data;
        local_vector.push_back(sd);
    }

    std::sort(local_vector.begin(), local_vector.end(), compare_by_max);

    int samples_per_proc = size - 1;
    std::vector<StationData> local_pivots;
    int stride = local_vector.size() / size;
    for (int i = 1; i < size; ++i) {
        if (i * stride < local_vector.size()) {
            local_pivots.push_back(local_vector[i * stride]);
        }
    }

    std::vector<StationData> all_pivots(size * samples_per_proc);
    MPI_Datatype MPI_STATIONDATA;
    {
        StationData dummy;
        int blocklengths[2] = {128, sizeof(WSData) / sizeof(float)};
        MPI_Aint displacements[2];
        MPI_Datatype types[2] = {MPI_CHAR, MPI_FLOAT};

        MPI_Aint base_address;
        MPI_Get_address(&dummy, &base_address);
        MPI_Get_address(&dummy.name, &displacements[0]);
        MPI_Get_address(&dummy.data, &displacements[1]);

        displacements[0] -= base_address;
        displacements[1] -= base_address;

        MPI_Type_create_struct(2, blocklengths, displacements, types, &MPI_STATIONDATA);
        MPI_Type_commit(&MPI_STATIONDATA);
    }

    MPI_Gather(local_pivots.data(), samples_per_proc, MPI_STATIONDATA,
               all_pivots.data(), samples_per_proc, MPI_STATIONDATA, 0, MPI_COMM_WORLD);

    std::vector<StationData> pivots;
    if (rank == 0) {
        std::sort(all_pivots.begin(), all_pivots.end(), compare_by_max);
        for (int i = 1; i < size; ++i) {
            pivots.push_back(all_pivots[i * samples_per_proc]);
        }
    } else {
        pivots.resize(size - 1);
    }

    MPI_Bcast(pivots.data(), size - 1, MPI_STATIONDATA, 0, MPI_COMM_WORLD);

    std::vector<std::vector<StationData>> buckets(size);
    for (const auto& entry : local_vector) {
        int i = 0;
        while (i < pivots.size() && compare_by_max(entry, pivots[i])) ++i;
        buckets[i].push_back(entry);
    }

    std::vector<int> send_counts(size), send_displs(size);
    int total_send = 0;
    for (int i = 0; i < size; ++i) {
        send_counts[i] = buckets[i].size();
        send_displs[i] = total_send;
        total_send += send_counts[i];
    }

    std::vector<StationData> send_buffer(total_send);
    for (int i = 0; i < size; ++i) {
        std::copy(buckets[i].begin(), buckets[i].end(), send_buffer.begin() + send_displs[i]);
    }

    std::vector<int> recv_counts(size), recv_displs(size);
    MPI_Alltoall(send_counts.data(), 1, MPI_INT, recv_counts.data(), 1, MPI_INT, MPI_COMM_WORLD);

    int total_recv = 0;
    for (int i = 0; i < size; ++i) {
        recv_displs[i] = total_recv;
        total_recv += recv_counts[i];
    }

    std::vector<StationData> recv_buffer(total_recv);
    MPI_Alltoallv(send_buffer.data(), send_counts.data(), send_displs.data(), MPI_STATIONDATA,
                  recv_buffer.data(), recv_counts.data(), recv_displs.data(), MPI_STATIONDATA,
                  MPI_COMM_WORLD);

    
    std::unordered_map<std::string, WSData> final_map;
    for (const auto& entry : recv_buffer) {
        final_map[entry.name].update(entry.data);
    }
    
    std::vector<StationData> final_vector;
    for (const auto& [name, data] : final_map) {
        StationData sd;
        std::strncpy(sd.name, name.c_str(), sizeof(sd.name) - 1);
        sd.data = data;
        final_vector.push_back(sd);
    }
    
    std::sort(final_vector.begin(), final_vector.end(), compare_by_max);
              
    if (rank == 0) {
        for (const auto& entry : final_vector) {
            std::cout << entry.name << ": avg=" << entry.data.avg()
                      << " min=" << entry.data.min
                      << " max=" << entry.data.max << "\n";
        }    
    }

    MPI_Type_free(&MPI_STATIONDATA);
    double end_time = MPI_Wtime();
    if (rank == 0) {
        std::cout << "\nExecution Time: " << (end_time - start_time) << " seconds" << std::endl;
    }
    MPI_Finalize();
    return 0;
} 
