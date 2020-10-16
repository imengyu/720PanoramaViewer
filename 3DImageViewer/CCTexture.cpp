#include "CCTexture.h"
#include "StringHlp.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

CCTexture::CCTexture()
{
}
CCTexture::~CCTexture()
{
	Destroy();
}

bool CCTexture::Load(LPWSTR path)
{
	char* pathAnsi = StringHlp::UnicodeToAnsi(path);
	bool result = Load(pathAnsi);
	StringHlp::FreeStringPtr(pathAnsi);
}
bool CCTexture::Load(char* path)
{
	int w, h, nrChannels;
	BYTE*data = stbi_load(path, &w, &h, &nrChannels, 0);
	if (data) {
		if(nrChannels == 3)
			LoadRGB(data, w, h);
		else if(nrChannels == 4)
			LoadRGBA(data, w, h);
		return true;
	}
	return false;
}
void CCTexture::LoadRGB(BYTE* data, int width, int height)
{
	LoadBytes(data, width, height, GL_RGB);
}
void CCTexture::LoadRGBA(BYTE* data, int width, int height)
{
	LoadBytes(data, width, height, GL_RGBA);
}
void CCTexture::LoadBytes(BYTE* data, int width, int height, GLenum type) {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)width, (GLsizei)height, 0, type, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	this->width = width;
	this->height = height;
}

void CCTexture::Destroy()
{
	if (texture > 0)
		glDeleteTextures(1, &texture);
}
void CCTexture::Use()
{
	glBindTexture(GL_TEXTURE_2D, texture);
}
void CCTexture::UnUse()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}