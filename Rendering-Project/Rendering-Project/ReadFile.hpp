#ifndef READ_FILE_HPP
#define READ_FILE_HPP

#include <fstream>
#include <iostream>

// Function to read file content into a string
static bool ReadFile(const std::string& filePath, std::string& fileData) {
    std::ifstream reader(filePath, std::ios::binary | std::ios::ate);
    if (!reader.is_open()) {
        std::cerr << "Could not open file: " << filePath << std::endl;
        return false;
    }

    std::streamsize size = reader.tellg();
    reader.seekg(0, std::ios::beg);

    fileData.resize(size);
    if (!reader.read(&fileData[0], size)) {
        std::cerr << "Failed to read file: " << filePath << std::endl;
        reader.close();
        return false;
    }

    reader.close();
    return true;
}

#endif
