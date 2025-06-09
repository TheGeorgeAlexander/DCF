#include "dcf.hpp"
#include <iostream>
#include <fstream>



void readFile(const std::string &file, std::string &content) {
    std::ifstream fileStream(file);
    if(!fileStream.is_open()) {
        throw std::runtime_error("Unable to open file");
    }

    // Read the full source file
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    content = buffer.str();
    fileStream.close();
}



int main() {
    std::string fullFile;
    readFile("test/simple.dcf", fullFile);

    dcf::Section config = dcf::parse(fullFile);
    std::cout << config.toString() << std::endl;

    return 0;
}
