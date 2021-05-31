#pragma once
class ImageUtils
{
public:
    static bool CheckSizeIsNormalPanorama(glm::vec2 imageSize);
    static bool CheckSizeIs720Panorama(glm::vec2 imageSize);
    static bool CheckSizeIs320Panorama(glm::vec2 imageSize);
    static bool SaveJPEGToFile(glm::vec2 imageSize, unsigned char* data, std::wstring path);
};

