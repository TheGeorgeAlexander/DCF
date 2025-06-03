#include "dcf.hpp"
#include <iostream>
#include <fstream>



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

    try {
        dcf::DCF::parse(fullFile);
    } catch(dcf::parse_error err) {
        std::cout << "{\"message\":\"" << err.message() << "\",";
        std::cout << "\"line\":" << err.line() << ",";
        std::cout << "\"column\":" << err.column() << "}" << std::endl << std::endl;

        std::cout << err.what() << std::endl;
    }
    return 0;
}
