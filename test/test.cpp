#include "dcf.hpp"
#include <iostream>



int main() {
    std::ifstream fileStream("test/test.dcf");
    if(!fileStream.is_open()) {
        throw std::runtime_error("Unable to open file");
    }

    // Read the full source file
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    std::string fullFile = buffer.str();
    fileStream.close();



    // Parse the file 25 times
    for(int i = 0; i < 25; i++) {
        dcf::DCF::parse(fullFile);
    } 

    return 0;
}
