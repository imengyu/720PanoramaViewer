#include "stdafx.h"
#include "ImageUtils.h"
#include "720Core.h"
#include <jpeglib.h>

bool ImageUtils::CheckSizeIsNormalPanorama(glm::vec2 imageSize) {
    return ImageUtils::CheckSizeIs320Panorama(imageSize) || ImageUtils::CheckSizeIs720Panorama(imageSize);
}
bool ImageUtils::CheckSizeIs720Panorama(glm::vec2 imageSize) {
    float w = imageSize.x, h = imageSize.y;
    return abs(2.0 - (double)w / h) < 0.15;
}
bool ImageUtils::CheckSizeIs320Panorama(glm::vec2 imageSize) {
    float w = imageSize.x, h = imageSize.y;
    return h > 0 && (double)w / h > 5;
}

void imageUtils_jpeg_error_exit(j_common_ptr cinfo) {
    (*cinfo->err->output_message) (cinfo);
    jpeg_destroy(cinfo);
}
void imageUtils_jpeg_output_message(j_common_ptr cinfo) {
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message) (cinfo, buffer);
    Logger::GetStaticInstance()->Log2(L"%hs", buffer);
}

bool ImageUtils::SaveJPEGToFile(glm::vec2 imageSize, unsigned char* data, std::wstring path) {

    jpeg_compress_struct cinfo;
    jpeg_error_mgr jerr;
    FILE* outfile;                 //target file 

    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = imageUtils_jpeg_error_exit;
    jerr.output_message = imageUtils_jpeg_output_message;
    jpeg_create_compress(&cinfo);

    if (0 != _wfopen_s(&outfile, path.c_str(), L"wb")) {
        Logger::GetStaticInstance()->Log2(L"Cant open file %s", path.c_str());
        return false;
    }

    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = (int)imageSize.x;      //image width and height, in pixels 
    cinfo.image_height = (int)imageSize.y;
    cinfo.input_components = 3;         //# of color components per pixel 
    cinfo.in_color_space = JCS_EXT_RGB;         //colorspace of input image 

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 100, TRUE);//limit to baseline-JPEG values 
    jpeg_start_compress(&cinfo, TRUE);

    UINT bfferSize = cinfo.image_width * cinfo.input_components;
    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, bfferSize, 1);
    UCHAR* point = data + bfferSize * (cinfo.image_height - 1);

    while (cinfo.next_scanline < cinfo.image_height)
    {
        memcpy(*buffer, point, bfferSize); //将buffer中的数据逐行给src_buff
        point -= bfferSize; //指针偏移一行

        jpeg_write_scanlines(&cinfo, buffer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    fclose(outfile);

    return true;
}