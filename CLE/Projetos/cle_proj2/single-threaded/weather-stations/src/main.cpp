#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <chrono>

struct WSData
{
    float sum = 0.0;
    float min;
    float max;
    int count = 0;
};

void process_line(const std::string &line, std::map<std::string, WSData> &stationData)
{
    std::string station, temp_str;
    size_t i = 0;

    while (i < line.size() && line[i] != ';')
    {
        station += line[i];
        i++;
    }

    if (i >= line.size())
        return;

    i++;

    while (i < line.size() && line[i] != '\0')
    {
        temp_str += line[i];
        i++;
    }

    try
    {
        float temp = std::stof(temp_str);
        WSData &data = stationData[station];
        data.sum += temp;
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

int main(int argc, char *argv[])
{
    auto start_time = std::chrono::high_resolution_clock::now();

    const char *file = "measurements.txt";
    if (argc > 1)
    {
        file = argv[1];
    }

    std::ifstream fh(file);
    if (!fh.is_open())
    {
        std::cerr << "ERROR: Unable to open file '" << file << "'" << std::endl;
        return 1;
    }

    std::map<std::string, WSData> stationData;
    std::string line;

    while (std::getline(fh, line))
    {
        process_line(line, stationData);
    }
    fh.close();

    for (const auto &entry : stationData)
    {
        const std::string &station = entry.first;
        const WSData &data = entry.second;
        std::cout << station << ": avg=" << data.sum / data.count
                  << " min=" << data.min
                  << " max=" << data.max << std::endl;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time = end_time - start_time;
    std::cout << "Execution time: " << elapsed_time.count() << " seconds" << std::endl;

    return 0;
}
