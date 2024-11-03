#pragma once
#include <map>
#include <string>
#include <SFML/Graphics.hpp>

void createHistogramFromCharacterFrequencies(const std::map<wchar_t, int>& freqMap, const std::string& filename);
std::string convertWStringToString(const std::wstring& wstr);
void createHistogramFromWordFrequencies(const std::map<std::wstring, int>& freqMap, const std::string& filename);
void writeGnuplotScriptForWords(const std::string& dataFile, const std::string& title, const std::string& outputFile);
void writeGnuplotScriptForCharacters(const std::string& dataFile, const std::string& title, const std::string& outputFile);