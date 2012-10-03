// ***************************************************************** -*- C++ -*-
// path-test.cpp, $Rev: 1703 $

#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int main(int argc, char* const argv[])
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << *argv[1] << ": Failed to open file for reading\n";
        return 1;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::string path, dir, base;
        std::istringstream is(line);
        is >> path >> dir >> base;
        std::string d = Util::dirname(path);
        std::string b = Util::basename(path);

        if (d != dir || b != base) {
            std::cout << path << "\t'" << d << "'\t '" << b
                      << "'\t ==> Testcase failed\n";
        }
    }

    return 0;
}
