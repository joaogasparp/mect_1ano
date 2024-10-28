#include <iostream>
#include <vector>
#include <string>
#include <map>

using namespace std;

// Structure to hold the interval range for each symbol
struct SymbolInterval {
    double low;
    double high;
};

// Function to compute the interval ranges based on probabilities
map<char, SymbolInterval> compute_intervals(const map<char, double>& probabilities) {
    map<char, SymbolInterval> intervals;
    double cumulative_prob = 0.0;

    // Compute intervals for each symbol
    for (auto& symbol_prob : probabilities) {
        SymbolInterval interval;
        interval.low = cumulative_prob;
        interval.high = cumulative_prob + symbol_prob.second;
        intervals[symbol_prob.first] = interval;
        cumulative_prob += symbol_prob.second;
    }

    return intervals;
}

// Arithmetic encoding function
double arithmetic_encode(const string& input, const map<char, SymbolInterval>& intervals) {
    double low = 0.0;
    double high = 1.0;

    // Narrow down the interval for each symbol in the input string
    for (char symbol : input) {
        double range = high - low;
        high = low + range * intervals.at(symbol).high;
        low = low + range * intervals.at(symbol).low;
    }

    // Return the final encoding value (any value in the final range is valid)
    return (low + high) / 2.0;
}

// Arithmetic decoding function
string arithmetic_decode(double encoded_value, const map<char, SymbolInterval>& intervals, int message_length) {
    string decoded_message;
    for (int i = 0; i < message_length; i++) {
        for (const auto& symbol_interval : intervals) {
            if (encoded_value >= symbol_interval.second.low && encoded_value < symbol_interval.second.high) {
                decoded_message += symbol_interval.first;
                double range = symbol_interval.second.high - symbol_interval.second.low;
                encoded_value = (encoded_value - symbol_interval.second.low) / range;
                break;
            }
        }
    }
    return decoded_message;
}

int main() {
    // Example binary string probabilities
    map<char, double> probabilities = {
            {'0', 0.7}, // Probability of '0'
            {'1', 0.3}  // Probability of '1'
    };

    // Example binary string to encode
    string input = "0101011";
    cout << "Input: " << input << endl;

    // Compute intervals based on the symbol probabilities
    map<char, SymbolInterval> intervals = compute_intervals(probabilities);

    // Encode the input string
    double encoded_value = arithmetic_encode(input, intervals);
    cout << "Encoded value: " << encoded_value << endl;

    // Decode the encoded value
    string decoded_message = arithmetic_decode(encoded_value, intervals, input.length());
    cout << "Decoded message: " << decoded_message << endl;

    return 0;
}