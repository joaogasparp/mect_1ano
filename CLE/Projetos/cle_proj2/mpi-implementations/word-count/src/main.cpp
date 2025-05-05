#include <getopt.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <mpi.h>
#include "word_count.h"

static option long_options[] = {
    {"lines", no_argument, 0, 0},
    {"words", no_argument, 0, 0},
    {"chars", no_argument, 0, 0},
    {0, 0, 0, 0}};

struct CountResult
{
    int chars = 0;
    int lines = 0;
    int words = 0;
};

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    double start_time = MPI_Wtime();

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    WordCountFlags flags;
    while (1)
    {
        int option_index = 0;
        int c = getopt_long(argc, argv, "lwc",
                            long_options, &option_index);
        if (c == -1)
            break;

        switch (c)
        {
        case 'l':
            flags.lines = true;
            break;
        case 'w':
            flags.words = true;
            break;
        case 'c':
            flags.chars = true;
            break;
        case '?':
            break;
        }
    }

    if (!flags.lines && !flags.words && !flags.chars)
        flags.lines = flags.words = flags.chars = true;

    std::vector<std::string> files;

    if (rank == 0)
    {
        if (optind < argc)
        {
            std::string fname = argv[optind];
            std::ifstream file(fname);
            if (file.is_open())
            {
                std::string line;
                while (std::getline(file, line))
                {
                    files.push_back(line);
                }
                file.close();
            }
            else
            {
                std::cerr << "ERROR: Unable to open file: " << fname << std::endl;
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
        else
        {
            std::cerr << "No files provided" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    int num_files = files.size();
    MPI_Bcast(&num_files, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0)
    {
        files.resize(num_files);
    }
    for (int i = 0; i < num_files; ++i)
    {
        int length = (rank == 0) ? files[i].size() : 0;
        MPI_Bcast(&length, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (rank != 0)
        {
            files[i].resize(length);
        }
        MPI_Bcast(files[i].data(), length, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    int files_per_proc = num_files / size;
    int extra_files = num_files % size;
    int start_file = rank * files_per_proc + std::min(rank, extra_files);
    int end_file = start_file + files_per_proc + (rank < extra_files ? 1 : 0);

    std::map<std::string, CountResult> local_results;
    for (int i = start_file; i < end_file; ++i)
    {
        int chars = 0, lines = 0, words = 0;
        word_count(flags, {files[i]}, chars, lines, words);
        local_results[files[i]] = {chars, lines, words};
    }

    std::string serialized_results;
    for (const auto &[file, count] : local_results)
    {
        serialized_results += file + ";" + std::to_string(count.chars) + ";" +
                              std::to_string(count.lines) + ";" + std::to_string(count.words) + "\n";
    }

    int local_size = serialized_results.size();
    std::vector<int> sizes(size);
    MPI_Gather(&local_size, 1, MPI_INT, sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<int> displs(size);
    int total_size = 0;
    if (rank == 0)
    {
        for (int i = 0; i < size; ++i)
        {
            displs[i] = total_size;
            total_size += sizes[i];
        }
    }

    std::vector<char> recv_buffer(total_size);
    MPI_Gatherv(serialized_results.data(), local_size, MPI_CHAR,
                recv_buffer.data(), sizes.data(), displs.data(), MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        std::istringstream iss(std::string(recv_buffer.begin(), recv_buffer.end()));
        std::map<std::string, CountResult> global_results;
        std::string line;
        while (std::getline(iss, line))
        {
            std::istringstream line_stream(line);
            std::string file;
            CountResult count;
            char delim;
            std::getline(line_stream, file, ';');
            line_stream >> count.chars >> delim >> count.lines >> delim >> count.words;
            global_results[file] = count;
        }

        int total_chars = 0;
        int total_lines = 0;
        int total_words = 0;
        for (const auto &[file, count] : global_results)
        {
            std::cout << file << ": " << count.chars << " " << count.lines << " " << count.words << std::endl;
            total_chars += count.chars;
            total_lines += count.lines;
            total_words += count.words;
        }

        std::cout << "total: " << total_chars << " " << total_lines << " " << total_words << std::endl;
    }

    double end_time = MPI_Wtime();

    if (rank == 0)
    {
        std::cout << "Execution time: " << (end_time - start_time) << " seconds" << std::endl;
    }

    MPI_Finalize();
    return 0;
}
