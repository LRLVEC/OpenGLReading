///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Samples Pack (ogl-samples.g-truc.net)
///
/// Copyright (c) 2004 - 2014 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////

#include "png.hpp"
#include <FreeImage.h>
#include <cstdlib>

namespace
{
	static void FreeImageFree()
	{
		FreeImage_DeInitialise();
	}


	static void FreeImageInit()
	{
		static bool Init = false;

		if(!Init)
		{
			Init = true;
			FreeImage_Initialise(false);
			atexit(FreeImageFree);
		}
	}
}//namespace

/// Loading a PNG file
gli::texture2D load_png(char const * Filename)
{
	FreeImageInit();

	FIBITMAP * Bitmap = FreeImage_Load(FIF_PNG, Filename, 0);
	if(!Bitmap)
		return gli::texture2D();

	glm::uint BPP = FreeImage_GetBPP(Bitmap);
	glm::uint Width = FreeImage_GetWidth(Bitmap);
	glm::uint Height = FreeImage_GetHeight(Bitmap);

	gli::texture2D Texture(1, BPP == 24 ? gli::RGB8_UNORM : gli::RGBA8_UNORM, gli::texture2D::dim_type(Width, Height));
	memcpy(Texture.data(), FreeImage_GetBits(Bitmap), Texture.size());
	FreeImage_Unload(Bitmap);

	switch(gli::component_count(Texture.format()))
	{
	default:
		assert(0);
		break;
	case 3:
		for(std::size_t Offset = 0; Offset < Texture.size() / 3; ++Offset)
		{
			glm::u8vec3 Src = *(reinterpret_cast<glm::u8vec3 const *>(Texture.data()) + Offset);
			*(reinterpret_cast<glm::u8vec3*>(Texture.data()) + Offset) = glm::u8vec3(Src.z, Src.y, Src.x);
		}
		break;
	case 4:
		for(std::size_t Offset = 0; Offset < Texture.size() / 4; ++Offset)
		{
			glm::u8vec4 Src = *(reinterpret_cast<glm::u8vec4 const *>(Texture.data()) + Offset);
			*(reinterpret_cast<glm::u8vec4*>(Texture.data()) + Offset) = glm::u8vec4(Src.z, Src.y, Src.x, Src.w);
		}
		break;
	}

	return Texture;
}

void save_png(gli::texture2D const & Texture, char const * Filename)
{
	gli::texture2D Copy = gli::copy(Texture);

	switch(gli::component_count(Copy.format()))
	{
	default:
		assert(0);
		break;
	case 3:
		for(std::size_t Offset = 0; Offset < Copy.size() / 3; ++Offset)
		{
			glm::u8vec3 Src = *(reinterpret_cast<glm::u8vec3 const *>(Copy.data()) + Offset);
			*(reinterpret_cast<glm::u8vec3*>(Copy.data()) + Offset) = glm::u8vec3(Src.z, Src.y, Src.x);
		}
		break;
	case 4:
		for(std::size_t Offset = 0; Offset < Copy.size() / 4; ++Offset)
		{
			glm::u8vec4 Src = *(reinterpret_cast<glm::u8vec4 const *>(Copy.data()) + Offset);
			*(reinterpret_cast<glm::u8vec4*>(Copy.data()) + Offset) = glm::u8vec4(Src.z, Src.y, Src.x, Src.w);
		}
		break;
	}

	FreeImageInit();

	FIBITMAP* Bitmap = FreeImage_ConvertFromRawBits(
		Copy.data<BYTE>(),
		Copy.dimensions().x, Copy.dimensions().y,
		static_cast<int>(Copy.dimensions().x * gli::component_count(Copy.format())),
		static_cast<unsigned int>(gli::component_count(Copy.format()) * 8), 0x0000FF, 0x00FF00, 0xFF0000, false);

	BOOL Result = FreeImage_Save(FIF_PNG, Bitmap, Filename, 0);
	assert(Result);

	FreeImage_Unload(Bitmap);
}

