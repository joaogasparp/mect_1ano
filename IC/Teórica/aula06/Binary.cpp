#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>

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

// Function to handle outputting bits and deferred bits
void output_bit(vector<int>& output, int bit, int& deferred_bits) {
    output.push_back(bit);
    // Output deferred bits, which are the opposite of the current bit
    while (deferred_bits > 0) {
        output.push_back(1 - bit);
        deferred_bits--;
    }
}

// Arithmetic encoding function with bit-by-bit output
vector<int> arithmetic_encode(const string& input, const map<char, SymbolInterval>& intervals) {
    double low = 0.0;
    double high = 1.0;
    int deferred_bits = 0;
    vector<int> output;

    // Narrow down the interval for each symbol in the input string
    for (char symbol : input) {
        double range = high - low;
        high = low + range * intervals.at(symbol).high;
        low = low + range * intervals.at(symbol).low;

        // Bit output handling
        while (true) {
            if (high <= 0.5) {
                // Output '0' and shift the interval
                output_bit(output, 0, deferred_bits);
                low *= 2;
                high *= 2;
            } else if (low >= 0.5) {
                // Output '1' and shift the interval
                output_bit(output, 1, deferred_bits);
                low = 2 * (low - 0.5);
                high = 2 * (high - 0.5);
            } else if (low >= 0.25 && high <= 0.75) {
                // Middle case: Defer output and shift the interval
                deferred_bits++;
                low = 2 * (low - 0.25);
                high = 2 * (high - 0.25);
            } else {
                break;  // Interval is in the middle zone, continue encoding
            }
        }
    }

    // Final bit resolution after all symbols are processed
    deferred_bits++;
    if (low < 0.25) {
        output_bit(output, 0, deferred_bits);
    } else {
        output_bit(output, 1, deferred_bits);
    }

    return output;
}

// Decoding function to convert a binary string back into the original message
string arithmetic_decode(const vector<int>& encoded_bits, const map<char, SymbolInterval>& intervals, int message_length) {
    double low = 0.0;
    double high = 1.0;
    double value = 0.0;

    // Convert the encoded bits into a value
    for (int i = 0; i < encoded_bits.size(); i++) {
        value += encoded_bits[i] * pow(0.5, i + 1);
    }

    string decoded_message;

    for (int i = 0; i < message_length; i++) {
        double range = high - low;
        double scaled_value = (value - low) / range;

        // Find the corresponding symbol by comparing against the intervals
        for (const auto& symbol_interval : intervals) {
            if (scaled_value >= symbol_interval.second.low && scaled_value < symbol_interval.second.high) {
                decoded_message += symbol_interval.first;
                high = low + range * symbol_interval.second.high;
                low = low + range * symbol_interval.second.low;
                break;
            }
        }
    }

    return decoded_message;
}

int main() {
    // Example binary string probabilities
    map<char, double> probabilities = {
            {'0', 0.25}, // Probability of '0'
            {'1', 0.75}  // Probability of '1'
    };

    // Example binary string to encode
    string input = "111111010101111111011111110111110111011111101111111111";
    input = "11111111111111111111111111111111111111111111111111";

    // Compute intervals based on the symbol probabilities
    map<char, SymbolInterval> intervals = compute_intervals(probabilities);

    // Encode the input string
    vector<int> encoded_bits = arithmetic_encode(input, intervals);
    cout << "Encoded bits:    ";
    for (int bit : encoded_bits) {
        cout << bit;
    }
    cout << endl;

    // Decode the encoded bits back to the original message
    string decoded_message = arithmetic_decode(encoded_bits, intervals, input.length());

    cout << "Input:           " << input << endl;
    cout << "Decoded message: " << decoded_message << endl;

    return 0;
}