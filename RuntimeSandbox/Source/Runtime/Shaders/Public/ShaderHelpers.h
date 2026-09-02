#pragma once

#include <fstream>
#include <vector>
#include <cstdint>

namespace Sora::Shader
{
    inline std::vector<uint32_t> LoadSPIRV(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            return {};
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();

        return buffer;
    }
}
