#include <iostream>

#include <cstring>

#include <fstream>

using namespace std;

static std::vector<char> loadbin(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Error:Failed to open file");
    }
	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

void readImage(unsigned char* pixels, int &resolutionx, int &resolutiony, const char* path){ //ppm ascii
    fstream readimage;
    readimage.open(path);
    int i1, i2, i3;
    string trash;
    readimage >> trash;
    readimage >> resolutionx >> resolutiony;
    readimage >> i1;
    for(int i = 0; readimage >> i1 >> i2 >> i3; i+=4){
        pixels[i] = i1;
        pixels[i+1] = i2;
        pixels[i+2] = i3;
        pixels[i+3] = 255;
    }
}
