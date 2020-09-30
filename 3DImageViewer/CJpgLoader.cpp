#include "CJpgLoader.h"
#include "CApp.h"

glm::vec2 CJpgLoader::GetImageSize()
{
    if (decodeSuccess)
        return glm::vec2(cinfo.image_width, cinfo.image_height);
    return glm::vec2();
}

BYTE* CJpgLoader::GetAllImageData()
{
    if (decodeSuccess) {
        SetFullDataSize(width * height * depth);

        unsigned char* src_buff = (unsigned char*)malloc(fullDataSize);
        memset(src_buff, 0, fullDataSize);

        JSAMPARRAY buffer;//用于存取一行数据
        buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, width * depth, 1);//分配一行数据空间

        jpeg_start_decompress(&cinfo);
        unsigned char* point = src_buff;
        while (cinfo.output_scanline < height)//逐行读取位图数据
        {
            jpeg_read_scanlines(&cinfo, buffer, 1); //读取一行jpg图像数据到buffer
            memcpy(point, *buffer, width * depth); //将buffer中的数据逐行给src_buff
            point += width * depth; //指针偏移一行
        }

        jpeg_finish_decompress(&cinfo);
        return src_buff;
    }
    return nullptr;
}

BYTE* CJpgLoader::GetImageChunkData(int x, int y, int chunkW, int chunkH)
{
    SetChunkDataSize(chunkW * chunkH * depth);

    unsigned char* src_buff = (unsigned char*)malloc(chunkDataSize);
    memset(src_buff, 0, chunkDataSize);

    JSAMPARRAY buffer;//用于存取一行数据
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, chunkW * depth, 1);//分配一行数据空间
    
    JDIMENSION ux = x, uw = chunkW, end = y + chunkH;

    jpeg_start_decompress(&cinfo);
    jpeg_skip_scanlines(&cinfo, y);
    jpeg_crop_scanline(&cinfo, &ux, &uw);
    unsigned char* point = src_buff;

    while (cinfo.output_scanline < end)//逐行读取位图数据
    {
        jpeg_read_scanlines(&cinfo, buffer, 1); //读取一行jpg图像数据到buffer
        memcpy(point, *buffer, uw * depth); //将buffer中的数据逐行给src_buff
        point += uw * depth; //指针偏移一行
    }

    jpeg_skip_scanlines(&cinfo, height - chunkH - y);
    jpeg_finish_decompress(&cinfo);
    jpeg_abort_decompress(&cinfo);
    return src_buff;
}

char jpeg_last_err[JMSG_LENGTH_MAX];

void jpeg_error_exit(j_common_ptr cinfo) {
    (*cinfo->err->output_message) (cinfo);
    jpeg_destroy(cinfo);
}
void jpeg_output_message(j_common_ptr cinfo) {
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message) (cinfo, buffer);
    CApp::Instance->GetLogger()->Log2(L"%hs", buffer);
    strcpy_s(jpeg_last_err, buffer);
}

bool CJpgLoader::Load(const wchar_t* path)
{
    decodeSuccess = false;
    this->path = path;
    if (file == nullptr) {
        _wfopen_s(&file, path, L"rb");
        if (file) {
            fseek(file, 0, SEEK_SET);
            jpeg_create_decompress(&cinfo);
            cinfo.err = jpeg_std_error(&jerr);
            jerr.error_exit = jpeg_error_exit;
            jerr.output_message = jpeg_output_message;
            jpeg_stdio_src(&cinfo, file);
            if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
                SetLastError(jpeg_last_err);
                return false;
            }

            width = cinfo.image_width;//图像宽度
            height = cinfo.image_height;//图像高度
            depth = cinfo.num_components;//图像深度

            if (width <= 0 || height <= 0)
            {
                SetLastError(L"Bad image size");
                return false;
            }

            decodeSuccess = true;
            return true;
        }
    }
    return false;
}

const wchar_t* CJpgLoader::GetPath()
{
    return path.c_str();
}
void CJpgLoader::Destroy()
{   
    jpeg_destroy_decompress(&cinfo);
    if (file) {
        fclose(file);
        file = nullptr;
    }
}
bool CJpgLoader::IsOpened()
{
    return file != nullptr;
}

USHORT CJpgLoader::GetImageDepth()
{
    return depth;
}
