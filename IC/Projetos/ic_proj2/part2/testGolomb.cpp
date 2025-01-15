#include "Golomb.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

int main()
{
    // Parameter M for Golomb
    int M = 10; // Can adjust to another value if necessary

    // Create a Golomb object
    Golomb golomb;

    // Vectors to store values
    std::vector<int> values;
    std::vector<std::string> encoded;
    std::vector<int> decoded;

    // Read values from 'input.txt'
    std::ifstream inputFile("input.txt");
    if (!inputFile)
    {
        std::cerr << "Error opening input.txt!" << std::endl;
        return 1;
    }

    int num;
    while (inputFile >> num)
    {
        values.push_back(num); // Add value to vector
    }
    inputFile.close(); // Close file

    // Encode the values
    std::cout << "Encoding values:" << std::endl;
    for (int val : values)
    {
        std::string code = golomb.encode(val, M); // Encode with M=10
        encoded.push_back(code);
        std::cout << "Value: " << val << "\t-> Code: " << code << std::endl;
    }

    // Decode the codes
    std::cout << "\nDecoding values:" << std::endl;
    for (const std::string &code : encoded)
    {
        std::vector<int> dec = golomb.decode(code, M); // Decode with M=10
        int val = dec[0];                              // Get the first value from the returned vector
        decoded.push_back(val);
        std::cout << "Code:" << code << "\t-> Value: " << val << std::endl;
    }

    // Validate if the decoded values match the original ones
    if (values == decoded)
    {
        std::cout << "\nTest successful! The decoded values match the original ones." << std::endl;
    }
    else
    {
        std::cout << "\nTest failed! The decoded values do not match." << std::endl;
    }

    return 0;
}
