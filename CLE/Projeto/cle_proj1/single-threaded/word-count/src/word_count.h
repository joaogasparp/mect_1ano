
#pragma once

#include <vector>
#include <string>

struct WordCountFlags {
    bool lines = false; // count the number of new lines
    bool words = false; // count the total number of words
    bool chars = false; // count the number of total characters
};

void word_count(const WordCountFlags& flags, const std::vector<std::string>& files);
