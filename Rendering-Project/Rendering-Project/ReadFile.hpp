#ifndef READ_FILE_HPP
#define READ_FILE_HPP

#include <fstream>
#include <iostream>
#include <string>

namespace CM {
/**
 * @brief Reads the content of a file into a string.
 *
 * This function opens a file in binary mode, reads its entire content, and stores it in the provided string.
 *
 * @param filePath The path to the file to be read.
 * @param fileData A reference to a string where the file content will be stored.
 * @return true if the file was successfully read, false otherwise.
 */
static bool ReadFile(const std::string& filePath, std::string& fileData) {
    std::ifstream reader(filePath, std::ios::binary | std::ios::ate);
    if (!reader.is_open()) {
        std::cerr << "Could not open file: " << filePath << "\n";
        return false;
    }

    std::streamsize size = reader.tellg();
    reader.seekg(0, std::ios::beg);

    fileData.resize(size);
    if (!reader.read(fileData.data(), size)) {
        std::cerr << "Failed to read file: " << filePath << "\n";
        return false;
    }

    return true;
}
} // namespace CM

#endif