#include "dcf.hpp"
#include <iostream>
#include <fstream>



int main() {
    std::ifstream fileStream("test/simple.dcf");
    if(!fileStream.is_open()) {
        throw std::runtime_error("Unable to open file");
    }

    // Read the full source file
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    std::string fullFile = buffer.str();
    fileStream.close();



    dcf::Section config = dcf::DCF::parse(fullFile);
    config.set("hey", config.get("hey").asDouble() * 30);
    std::cout << config.toString() << std::endl;

    return 0;
}
