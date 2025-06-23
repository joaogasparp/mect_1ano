#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <chrono>
#include <sstream>
#include <queue>
#include <condition_variable>
#include <functional>
#include "word_count.h"

struct CountResult {
    int chars;
    int lines;
    int words;
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

// Adicionar tarefas à fila
void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push(std::move(task));
        ++active_tasks;
    }
    condition.notify_one();
}

// Esperar todas as tarefas terminarem
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

std::mutex result_mutex;

void process_file(const std::string& file, std::unordered_map<std::string, CountResult>& global_result) {
    WordCountResult local_result = word_count(file); // Contagem de caracteres, linhas e palavras

    std::lock_guard<std::mutex> lock(result_mutex);
    global_result[file] = {local_result.chars, local_result.lines, local_result.words};
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <file_list> <num_threads>" << std::endl;
        return 1;
    }

    std::ifstream file_list(argv[1]);
    if (!file_list.is_open()) {
        std::cerr << "Error opening file list: " << argv[1] << std::endl;
        return 1;
    }

    int num_threads = std::stoi(argv[2]);
    ThreadPool pool(num_threads);

    std::vector<std::string> files;
    std::string file;
    while (std::getline(file_list, file)) {
        files.push_back(file);
    }
    file_list.close();

    std::unordered_map<std::string, CountResult> global_result;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (const auto& f : files) {
        pool.enqueue([&, f] { process_file(f, global_result); });
    }

    pool.wait_all();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time = end_time - start_time;

    int total_chars = 0;
    int total_lines = 0;
    int total_words = 0;
    for (const auto& [file, count] : global_result) {
        std::cout << file << ": " << count.chars << " " << count.lines << " " << count.words << std::endl;
        total_chars += count.chars;
        total_lines += count.lines;
        total_words += count.words;
    }

    std::cout << "total: " << total_chars << " " << total_lines << " " << total_words << std::endl;
    std::cout << "Execution time: " << elapsed_time.count() << " seconds" << std::endl;

    return 0;
}
