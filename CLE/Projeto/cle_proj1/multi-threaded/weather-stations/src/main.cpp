#include <iostream>
#include <fstream>
#include <map>
#include <ctime>
#include <vector>
#include "thread_pool.h"

struct WSData
{
    float sum = 0.0;
    float min;
    float max;
    int count = 0;
};

void processLine(const std::string &line, std::map<std::string, WSData> &stationData, std::mutex &data_mutex)
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
        return;
    i++; // skip the ';'
    while (i < line.size() && line[i] != '\0')
    {
        temp_str += line[i];
        i++;
    }
    float temp = std::stof(temp_str);
    std::lock_guard<std::mutex> lock(data_mutex);
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

int main(int argc, char *argv[])
{
    clock_t clock_start = clock();
    const char *file = "measurements.txt";
    if (argc > 1)
    {
        file = argv[1];
    }
    std::ifstream fh(file);
    if (not fh.is_open())
    {
        std::cerr << "Unable to open '" << file << "'" << std::endl;
        return 1;
    }

    std::map<std::string, WSData> stationData;
    std::mutex data_mutex;
    ThreadPool pool(std::thread::hardware_concurrency());

    std::string line;
    std::vector<std::future<void>> results;

    while (getline(fh, line))
    {
        results.emplace_back(pool.enqueue(processLine, line, std::ref(stationData), std::ref(data_mutex)));
    }
    fh.close();

    for (auto &&result : results)
        result.get();

    std::vector<std::pair<std::string, WSData>> sortedData(stationData.begin(), stationData.end());
    std::sort(sortedData.begin(), sortedData.end(), [](const auto &a, const auto &b)
              { return a.second.max > b.second.max; });

    for (const auto &entry : sortedData)
    {
        const std::string &station = entry.first;
        const WSData &data = entry.second;
        std::cout << station << ": avg=" << data.sum / data.count << " min=" << data.min << " max=" << data.max << std::endl;
    }

    clock_t now = clock();
    double elapsed_time = (double)(now - clock_start) / CLOCKS_PER_SEC;
    std::cout << "Time to run: " << elapsed_time << "s" << std::endl;
    return 0;
}
