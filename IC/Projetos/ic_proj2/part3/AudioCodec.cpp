#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <ctime>
#include <cmath>
#include <sndfile.hh>
#include "BitStream.h"
#include "Golomb.h"

const std::vector<std::string> AUDIO_FILES = {"sample01.wav", "sample02.wav", "sample03.wav"};
const std::vector<int> FIXED_M_VALUES = {128, 256, 512, 1024, 2048};
const std::vector<int> BS_VALUES = {128, 512, 1024, 2048, 16384};
const std::vector<int> QUANT_VALUES = {2, 4, 6, 8};

double calculateSNR(const std::string &original, const std::string &decoded)
{
    std::cerr << "Calculating SNR for: " << original << " vs " << decoded << "\n";
    SndfileHandle sfOrig(original);
    SndfileHandle sfDec(decoded);

    if (sfOrig.error() || sfDec.error())
    {
        std::cerr << "Error: Could not open one of the files for SNR calculation.\n";
        return -1.0;
    }

    if (sfOrig.frames() != sfDec.frames() || sfOrig.channels() != sfDec.channels())
    {
        std::cerr << "Error: File dimensions do not match for SNR calculation.\n";
        return -1.0;
    }

    std::vector<short> origSamples(sfOrig.frames() * sfOrig.channels());
    std::vector<short> decSamples(sfDec.frames() * sfDec.channels());
    sfOrig.readf(origSamples.data(), sfOrig.frames());
    sfDec.readf(decSamples.data(), sfDec.frames());

    double signalPower = 0.0;
    double noisePower = 0.0;

    for (size_t i = 0; i < origSamples.size(); ++i)
    {
        signalPower += origSamples[i] * origSamples[i];
        double noise = origSamples[i] - decSamples[i];
        noisePower += noise * noise;
    }

    if (noisePower == 0)
    {
        return std::numeric_limits<double>::infinity();
    }

    return 10.0 * log10(signalPower / noisePower);
}

void dump_stats(std::ofstream &file, const std::vector<double> &times,
                const std::vector<long> &sizes, const std::vector<double> &snrs = {})
{
    file << "Times:\n";
    for (const auto &t : times)
        file << "\t" << t << "\n";

    file << "Sizes:\n";
    for (const auto &s : sizes)
        file << "\t" << s << "\n";

    if (!snrs.empty())
    {
        file << "SNRs:\n";
        for (const auto &s : snrs)
            file << "\t" << s << " dB\n";
    }
}

double extract_time(const std::string &output)
{
    std::istringstream iss(output);
    std::string token;
    while (iss >> token)
    {
        if (token == "Execution")
        {
            iss >> token; // skip "time:"
            iss >> token; // get the time value
            return std::stod(token);
        }
    }
    throw std::invalid_argument("Time not found in output");
}

long extract_size(const std::string &output)
{
    std::istringstream iss(output);
    std::string token;
    while (iss >> token)
    {
        if (token.find_first_of("0123456789") != std::string::npos)
        {
            return std::stol(token);
        }
    }
    throw std::invalid_argument("Size not found in output");
}

int lossy_encoder(std::ofstream &loe, int idx)
{
    std::vector<double> times;
    std::vector<long> sizes;
    std::vector<double> snrs;

    loe << "Testing with auto M and block size 2048\n";
    loe << "Testing quantization values (2, 4, 6, 8)\n";

    for (const auto &audio : AUDIO_FILES)
    {
        loe << "Audio File: " << audio << "\n";
        for (const auto &q : QUANT_VALUES)
        {
            std::stringstream cmd_time, cmd_size;
            cmd_time << "./GolombEncoder " << audio << " " << idx << " 2048 auto " << q;
            cmd_size << "stat -c %s " << idx;

            char buffer[128];
            FILE *pipe = popen(cmd_time.str().c_str(), "r");
            fgets(buffer, sizeof(buffer), pipe);
            std::string time_output(buffer);
            times.push_back(extract_time(time_output));
            pclose(pipe);

            pipe = popen(cmd_size.str().c_str(), "r");
            fgets(buffer, sizeof(buffer), pipe);
            std::string size_output(buffer);
            sizes.push_back(extract_size(size_output));
            pclose(pipe);

            std::stringstream decodeCmd;
            decodeCmd << "./GolombDecoder " << idx << " " << idx << ".wav";
            system(decodeCmd.str().c_str());

            std::string decodedFile = std::to_string(idx) + ".wav";
            double snr = calculateSNR(audio, decodedFile);
            if (snr == -1.0)
            {
                std::cerr << "Error: Could not calculate SNR for file: " << decodedFile << "\n";
                snrs.push_back(-1.0);
            }
            else
            {
                snrs.push_back(snr);
            }

            std::stringstream rmCmd;
            rmCmd << "rm -f " << decodedFile;
            system(rmCmd.str().c_str());

            idx++;
        }
        dump_stats(loe, times, sizes, snrs);
        times.clear();
        sizes.clear();
        snrs.clear();
    }
    return idx;
}

int lossless_encoder(std::ofstream &lle, int idx)
{
    std::vector<double> times;
    std::vector<long> sizes;
    std::vector<double> snrs;

    lle << "Testing with fixed M values (128, 256, 512, 1024, 2048)\n";
    for (const auto &audio : AUDIO_FILES)
    {
        lle << "Audio File: " << audio << "\n";
        for (const auto &m : FIXED_M_VALUES)
        {
            std::stringstream cmd_time, cmd_size;
            cmd_time << "./GolombEncoder " << audio << " " << idx << " " << m;
            cmd_size << "stat -c %s " << idx;

            char buffer[128];
            FILE *pipe = popen(cmd_time.str().c_str(), "r");
            fgets(buffer, sizeof(buffer), pipe);
            std::string time_output(buffer);
            times.push_back(extract_time(time_output));
            pclose(pipe);

            pipe = popen(cmd_size.str().c_str(), "r");
            fgets(buffer, sizeof(buffer), pipe);
            std::string size_output(buffer);
            sizes.push_back(extract_size(size_output));
            pclose(pipe);

            std::stringstream decodeCmd;
            decodeCmd << "./GolombDecoder " << idx << " " << idx << ".wav";
            system(decodeCmd.str().c_str());

            std::string decodedFile = std::to_string(idx) + ".wav";
            double snr = calculateSNR(audio, decodedFile);
            if (snr == -1.0)
            {
                std::cerr << "Error: Could not calculate SNR for file: " << decodedFile << "\n";
                snrs.push_back(-1.0);
            }
            else
            {
                snrs.push_back(snr);
            }

            std::stringstream rmCmd;
            rmCmd << "rm -f " << decodedFile;
            system(rmCmd.str().c_str());

            idx++;
        }
        dump_stats(lle, times, sizes, snrs);
        times.clear();
        sizes.clear();
        snrs.clear();
    }

    lle << "\nTesting with auto M and different block sizes (128, 512, 1024, 2048, 16384)\n";
    for (const auto &audio : AUDIO_FILES)
    {
        lle << "Audio File: " << audio << "\n";
        for (const auto &bs : BS_VALUES)
        {
            std::stringstream cmd_time, cmd_size;
            cmd_time << "./GolombEncoder " << audio << " " << idx << " " << bs << " auto";
            cmd_size << "stat -c %s " << idx;

            char buffer[128];
            FILE *pipe = popen(cmd_time.str().c_str(), "r");
            fgets(buffer, sizeof(buffer), pipe);
            std::string time_output(buffer);
            times.push_back(extract_time(time_output));
            pclose(pipe);

            pipe = popen(cmd_size.str().c_str(), "r");
            fgets(buffer, sizeof(buffer), pipe);
            std::string size_output(buffer);
            sizes.push_back(extract_size(size_output));
            pclose(pipe);

            std::stringstream decodeCmd;
            decodeCmd << "./GolombDecoder " << idx << " " << idx << ".wav";
            system(decodeCmd.str().c_str());

            std::string decodedFile = std::to_string(idx) + ".wav";
            double snr = calculateSNR(audio, decodedFile);
            if (snr == -1.0)
            {
                std::cerr << "Error: Could not calculate SNR for file: " << decodedFile << "\n";
                snrs.push_back(-1.0);
            }
            else
            {
                snrs.push_back(snr);
            }

            std::stringstream rmCmd;
            rmCmd << "rm -f " << decodedFile;
            system(rmCmd.str().c_str());

            idx++;
        }
        dump_stats(lle, times, sizes, snrs);
        times.clear();
        sizes.clear();
        snrs.clear();
    }
    return idx;
}

int decoder(std::ofstream &d, int idx)
{
    for (int i = 0; i < idx; ++i)
    {
        std::stringstream cmd_decode;
        cmd_decode << "./GolombDecoder " << i << " " << i << ".wav";

        char buffer[128];
        FILE *pipe = popen(cmd_decode.str().c_str(), "r");
        fgets(buffer, sizeof(buffer), pipe);
        d << std::string(buffer) << "\n";
        pclose(pipe);

        std::stringstream cmd_rm1, cmd_rm2;
        cmd_rm1 << "rm -f " << i;
        cmd_rm2 << "rm " << i << ".wav";
        system(cmd_rm1.str().c_str());
        system(cmd_rm2.str().c_str());
    }
    return idx;
}

int main()
{
    std::ofstream lle("lossless_encoder.txt");
    std::ofstream loe("lossy_encoder.txt");
    std::ofstream d("decoder.txt");

    time_t start = time(0);

    lle << "[Lossless Encoder]\n\n";
    std::cout << "Testing Lossless Encoder\n";
    int idx = lossless_encoder(lle, 0);

    loe << "[Lossy Encoder]\n\n";
    std::cout << "Testing Lossy Encoder\n";
    idx = lossy_encoder(loe, idx);

    d << "[Testing Decoder]\n\n";
    std::cout << "Testing Decoder\n";
    idx = decoder(d, idx);

    time_t end = time(0);

    std::cout << "Total execution time: " << difftime(end, start) << " seconds\n";

    lle.close();
    loe.close();
    d.close();

    return 0;
}
