#pragma once

//From Beginning OpenGL Game Programming by Astle and Hawkins, 
//see Project report for full reference
enum TGATypes
{
	TGA_NODATA = 0,
	TGA_INDEXED = 1,
	TGA_RGB = 2,
	TGA_GRAYSCALE = 3,
	TGA_INDEXED_RLE = 9,
	TGA_RGB_RLE = 10,
	TGA_GRAYSCALE_RLE = 11
};

#define IMAGE_RGB		0
#define IMAGE_RGBA		1
#define IMAGE_LUMINANCE 2

#define IMAGE_DATA_UNSIGNED_BYTE	0

#define BOTTOM_LEFT  0x00
#define BOTTOM_RIGHT 0x10
#define TOP_LEFT	 0x20
#define TOP_RIGHT	 0x30

struct tgaheader_t
{
	unsigned char	idLength;
	unsigned char	colorMapType;
	unsigned char	imageTypeCode;
	unsigned char	colorMapSpec[5];
	unsigned short	xOrigin;
	unsigned short	yOrigin;
	unsigned short	width;
	unsigned short  height;
	unsigned char   bpp;
	unsigned char	imageDesc;
};

struct rgba_t
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

struct rgb_t
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

class Texture2D
{
public:
	Texture2D(void);
	~Texture2D(void);
	bool Load(const char* fileName);
	void Release();

	bool FlipVertical();

	unsigned short GetWidth() { return m_width; }
	unsigned short GetHeight() { return m_height; }
	unsigned char GetImageFormat() { return m_imageDataFormat; }

	bool ConvertRGBAToRGB();
	bool ConvertRGBToRGBA(unsigned char alphaValue);

	unsigned char *GetImage() { return m_pImageData; }

private:
	unsigned char m_colorDepth;
	unsigned char m_imageDataType;
	unsigned char m_imageDataFormat;
	unsigned char *m_pImageData;
	unsigned short m_width;
	unsigned short m_height;
	unsigned long m_imageSize;

	unsigned int textureIndex;

	// Swap the red and blue components in the image data
	void SwapRedBlue();

	friend class ContentManager;
	friend class SpriteBatch;
};

