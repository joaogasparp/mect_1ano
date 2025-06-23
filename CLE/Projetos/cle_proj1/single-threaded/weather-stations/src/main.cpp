#include <iostream>
#include <fstream>
#include <map>
#include <ctime>

struct WSData
{
    float sum = 0.0;
    float min;
    float max;
    int count = 0;
};

int main(int argc, char *argv[])
{
    clock_t clock_start = clock();

    // Use default file ...
    const char *file = "measurements.txt";
    if (argc > 1)
    {
        // ... or the first argument.
        file = argv[1];
    }
    std::ifstream fh(file);
    if (not fh.is_open())
    {
        std::cerr << "Unable to open '" << file << "'" << std::endl;
        return 1;
    }

    // initialize the data structure
    // string <-> data
    std::map<std::string, WSData> stationData;

    // Variable to store each line from the file
    std::string line;

    // Read each line from the file and print it
    while (getline(fh, line))
    {
        std::string temp_str;
        std::string station;

        std::string::size_type i = 0;
        while (i < line.size() && line[i] != ';')
        {
            station += line[i];
            i++;
        }

        if (i >= line.size())
            continue;

        i++; // skip the ';'

        while (i < line.size() && line[i] != '\0')
        {
            temp_str += line[i];
            i++;
        }

        // Process each line as needed
        WSData &data = stationData[station];
        float temp = std::stof(temp_str);
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
    fh.close();

    // Output sorted results
    for (const auto &entry : stationData)
    {
        const std::string &station = entry.first;
        const WSData &data = entry.second;
        std::cout << station << ": avg=" << data.sum / data.count << " min=" << data.min << " max=" << data.max << std::endl;
    }

    clock_t now = clock();
    double elapsed_time = (double)(now - clock_start) / CLOCKS_PER_SEC;
    std::cout << "Execution time: " << elapsed_time << "s" << std::endl;
    return 0;
}
