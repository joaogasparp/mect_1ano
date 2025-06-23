#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <execution>
#include <queue>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <limits>

#define BLOCK_SIZE 100000

struct WSData {
    float sum = 0.0;
    float min = std::numeric_limits<float>::max();
    float max = std::numeric_limits<float>::lowest();
    int count = 0;
};

class ThreadPool {
public:
    ThreadPool(size_t num_workers);
    ~ThreadPool();
    void enqueue(std::function<void()> task);
    void wait_all();

private:
    void worker_thread();
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<int> active_tasks;
    bool stop;
};

ThreadPool::ThreadPool(size_t num_workers) : active_tasks(0), stop(false) {
    for (size_t i = 0; i < num_workers; ++i) {
        workers.emplace_back(&ThreadPool::worker_thread, this);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push(std::move(task));
        ++active_tasks;
    }
    condition.notify_one();
}

void ThreadPool::wait_all() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    condition.wait(lock, [this] { return active_tasks == 0 && tasks.empty(); });
}

void ThreadPool::worker_thread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) return;
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
        --active_tasks;
        condition.notify_all();
    }
}

std::unordered_map<std::string, WSData> stationData;
std::mutex data_mutex;

// Função para processar um bloco do ficheiro
void process_chunk(const char *file_data, size_t start, size_t end, int thread_id) {
    (void) thread_id;
    std::unordered_map<std::string, WSData> localData;

    while (start > 0 && file_data[start - 1] != '\n') {
        start++;
    }

    size_t pos = start;
    std::string line;

    while (pos < end) {
        while (pos < end && file_data[pos] != '\n') {
            line += file_data[pos++];
        }
        pos++;

        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string station, temp_str;
        if (std::getline(iss, station, ';') && std::getline(iss, temp_str)) {
            if (station.empty()) continue;

            try {
                float temp = std::stof(temp_str);
                WSData &data = localData[station];
                data.sum += temp;
                data.count++;
                data.min = std::min(data.min, temp);
                data.max = std::max(data.max, temp);
            } catch (...) {
                std::cerr << "[Erro] Falha ao converter temperatura: " << temp_str << std::endl;
            }
        }
        line.clear();
    }

    // Atualizar `stationData` com os dados locais de cada thread
    std::lock_guard<std::mutex> lock(data_mutex);
    for (const auto &[station, data] : localData) {
        WSData &globalData = stationData[station];
        globalData.sum += data.sum;
        globalData.count += data.count;
        globalData.min = std::min(globalData.min, data.min);
        globalData.max = std::max(globalData.max, data.max);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <file> <num_threads>" << std::endl;
        return 1;
    }

    std::string file = argv[1];
    int num_threads = std::stoi(argv[2]);

    auto start_time = std::chrono::high_resolution_clock::now();

    // Abrir o arquivo usando mmap
    int fd = open(file.c_str(), O_RDONLY);
    if (fd == -1) {
        std::cerr << "Erro ao abrir o arquivo." << std::endl;
        return 1;
    }

    size_t file_size = lseek(fd, 0, SEEK_END);
    const char *file_data = static_cast<const char *>(mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0));
    close(fd);

    if (file_data == MAP_FAILED) {
        std::cerr << "Erro ao mapear o arquivo para memória." << std::endl;
        return 1;
    }

    ThreadPool pool(num_threads);

    size_t chunk_size = file_size / num_threads;
    for (int i = 0; i < num_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == num_threads - 1) ? file_size : (i + 1) * chunk_size;
        pool.enqueue([=] { process_chunk(file_data, start, end, i); });
    }

    pool.wait_all();

    munmap((void *)file_data, file_size);

    std::vector<std::pair<std::string, WSData>> sortedData(stationData.begin(), stationData.end());
    std::sort(std::execution::par_unseq, sortedData.begin(), sortedData.end(), 
          [](const auto &a, const auto &b) { return a.second.max > b.second.max; });

    for (const auto &entry : sortedData) {
        std::cout << entry.first << ": avg=" << entry.second.sum / entry.second.count
                  << " min=" << entry.second.min 
                  << " max=" << entry.second.max << std::endl;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time = end_time - start_time;
    std::cout << "Execution time: " << elapsed_time.count() << " seconds" << std::endl;    

    return 0;
}
