#include <iostream>
#include <fstream>

void compareFiles(const std::string& file1, const std::string& file2) {
    std::ifstream f1(file1, std::ios::binary);
    std::ifstream f2(file2, std::ios::binary);

    if (!f1.is_open() || !f2.is_open()) {
        std::cerr << "Error: Could not open one of the files.\n";
        return;
    }

    char c1, c2;
    int pos = 0;
    while (f1.get(c1) && f2.get(c2)) {
        if (c1 != c2) {
            std::cout << "Files differ at position: " << pos << "\n";
            return;
        }
        pos++;
    }

    if (f1.eof() && f2.eof()) {
        std::cout << "Files are identical.\n";
    } else {
        std::cout << "Files differ in length.\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: compare <file1> <file2>\n";
        return 1;
    }
    compareFiles(argv[1], argv[2]);
    return 0;
}
