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

#include "test.hpp"

namespace
{
	char const * VERT_SHADER_SOURCE("gl-320/texture-format.vert");
	char const * FRAG_SHADER_SOURCE[3] = 
	{
		"gl-320/texture-format-normalized.frag", 
		"gl-320/texture-format-uint.frag"
	};
	char const * TEXTURE_DIFFUSE("kueken2-bgra8.dds");

	// With DDS textures, v texture coordinate are reversed, from top to bottom
	GLsizei const VertexCount(6);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f))
	};

	namespace texture
	{
		enum type
		{
			RGBA8, // GL_RGBA8
			RGBA8UI, // GL_RGBA8UI
			RGBA16F, // GL_RGBA16F
			RGBA32F, // GL_RGBA32F
			RGBA8_SNORM, 
			//RGB10_A2UI, // GL_RGB10_A2UI
/*
			RGBA8I,
			RGBA16I, // GL_RGBA16_SNORM
			RGBA32I, // GL_RGBA8I
			RGBA8U, // GL_RGBA8
			RGBA16U,
			RGBA32U,
			RGBA16F, // GL_RGBA16F
			RGBA32F, // GL_RGBA32F
			RGBA8_SNORM, // GL_RGBA8_SNORM
*/
			MAX
		};
	}//namespace texture

	namespace program
	{
		enum type
		{
			NORMALIZED, 
			UINT, 
			MAX
		};
	}//namespace program

	GLenum TextureInternalFormat[texture::MAX] = 
	{
		GL_RGBA8, 
		GL_RGBA8UI, 
		GL_RGBA16F,
		GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
		GL_RGBA8_SNORM
	};

	GLenum TextureFormat[texture::MAX] = 
	{
		GL_BGRA, 
		GL_BGRA_INTEGER, 
		GL_BGRA,
		GL_BGRA,
		GL_BGRA
	};

	GLuint VertexArrayName(0);
	GLuint ProgramName[program::MAX] = {0, 0};

	GLuint BufferName(0);
	GLuint TextureName[texture::MAX] = {0, 0};

	GLint UniformMVP[program::MAX] = {0, 0};
	GLint UniformDiffuse[program::MAX] = {0, 0};

	glm::ivec4 Viewport[texture::MAX] = 
	{
		glm::ivec4(  0,   0, 320, 240),
		glm::ivec4(320,   0, 320, 240),
		glm::ivec4(320, 240, 320, 240),
		glm::ivec4(  0, 240, 320, 240)
	};
	
	namespace shader
	{
		enum type
		{
			VERT,
			FRAG_NORMALIZED,
			FRAG_UINT,
			MAX
		};
	}//namespace shader
}//namespace

class gl_320_texture_format : public test
{
public:
	gl_320_texture_format(int argc, char* argv[]) :
		test(argc, argv, "gl-320-texture-format", test::CORE, 3, 2)
	{}

private:
	bool initProgram()
	{
		bool Validated(true);

		std::array<GLuint, shader::MAX> ShaderName;
		
		compiler Compiler;
		ShaderName[shader::VERT] = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 150 --profile core");

		for(std::size_t i = 0; i < program::MAX; ++i)
			ShaderName[shader::FRAG_NORMALIZED + i] = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE[i], "--version 150 --profile core");
		Validated = Validated && Compiler.check();

		for(std::size_t i = 0; (i < program::MAX) && Validated; ++i)
		{
			ProgramName[i] = glCreateProgram();
			glAttachShader(ProgramName[i], ShaderName[shader::VERT]);
			glAttachShader(ProgramName[i], ShaderName[shader::FRAG_NORMALIZED + i]);

			glBindAttribLocation(ProgramName[i], semantic::attr::POSITION, "Position");
			glBindAttribLocation(ProgramName[i], semantic::attr::TEXCOORD, "Texcoord");
			glBindFragDataLocation(ProgramName[i], semantic::frag::COLOR, "Color");
			glLinkProgram(ProgramName[i]);
			Validated = Validated && Compiler.checkProgram(ProgramName[i]);

			UniformMVP[i] = glGetUniformLocation(ProgramName[i], "MVP");
			Validated = Validated && (UniformMVP[i] != -1);
			UniformDiffuse[i] = glGetUniformLocation(ProgramName[i], "Diffuse");
			Validated = Validated && (UniformDiffuse[i] != -1);
		}

		return Validated && this->checkError("initProgram");
	}

	bool initBuffer()
	{
		glGenBuffers(1, &BufferName);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName);
		glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return this->checkError("initBuffer");;
	}

	bool initTexture()
	{
		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(texture::MAX, TextureName);
		glActiveTexture(GL_TEXTURE0);

		for(std::size_t i = 0; i < texture::MAX; ++i)
		{
			glBindTexture(GL_TEXTURE_2D, TextureName[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

			glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(0),
				TextureInternalFormat[i],
				static_cast<GLsizei>(Texture.dimensions().x), static_cast<GLsizei>(Texture.dimensions().y),
				0,
				TextureFormat[i], GL_UNSIGNED_BYTE,
				Texture.data());
		}
	
		glBindTexture(GL_TEXTURE_2D, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		return this->checkError("initTexture");
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(0));
			glVertexAttribPointer(semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(sizeof(glm::vec2)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
			glEnableVertexAttribArray(semantic::attr::TEXCOORD);
		glBindVertexArray(0);

		return this->checkError("initVertexArray");
	}

	bool begin()
	{
		bool Validated = true;

		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();

		return Validated && this->checkError("begin");
	}

	bool end()
	{
		glDeleteBuffers(1, &BufferName);
		for(std::size_t i = 0; i < program::MAX; ++i)
			glDeleteProgram(ProgramName[i]);
		glDeleteTextures(texture::MAX, TextureName);
		glDeleteVertexArrays(1, &VertexArrayName);

		return this->checkError("end");
	}

	bool render()
	{
		glm::ivec2 WindowSize(this->getWindowSize());

		glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * this->view() * Model;

		glViewport(0, 0, WindowSize.x, WindowSize.y);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);
		glBindVertexArray(VertexArrayName);
		glActiveTexture(GL_TEXTURE0);

		{
			std::size_t ViewportIndex(0); 
			glViewport(Viewport[ViewportIndex].x, Viewport[ViewportIndex].y, Viewport[ViewportIndex].z, Viewport[ViewportIndex].w);

			glUseProgram(ProgramName[program::UINT]);
			glUniform1i(UniformDiffuse[program::UINT], 0);
			glUniformMatrix4fv(UniformMVP[program::UINT], 1, GL_FALSE, &MVP[0][0]);

			glBindTexture(GL_TEXTURE_2D, TextureName[texture::RGBA8UI]);

			glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
		}

		{
			std::size_t ViewportIndex(1); 
			glViewport(Viewport[ViewportIndex].x, Viewport[ViewportIndex].y, Viewport[ViewportIndex].z, Viewport[ViewportIndex].w);

			glUseProgram(ProgramName[program::NORMALIZED]);
			glUniform1i(UniformDiffuse[program::NORMALIZED], 0);
			glUniformMatrix4fv(UniformMVP[program::NORMALIZED], 1, GL_FALSE, &MVP[0][0]);

			glBindTexture(GL_TEXTURE_2D, TextureName[texture::RGBA16F]);

			glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
		}

		{
			std::size_t ViewportIndex(2); 
			glViewport(Viewport[ViewportIndex].x, Viewport[ViewportIndex].y, Viewport[ViewportIndex].z, Viewport[ViewportIndex].w);

			glUseProgram(ProgramName[program::NORMALIZED]);
			glUniform1i(UniformDiffuse[program::NORMALIZED], 0);
			glUniformMatrix4fv(UniformMVP[program::NORMALIZED], 1, GL_FALSE, &MVP[0][0]);

			glBindTexture(GL_TEXTURE_2D, TextureName[texture::RGBA8]);

			glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
		}

		{
			std::size_t ViewportIndex(3); 
			glViewport(Viewport[ViewportIndex].x, Viewport[ViewportIndex].y, Viewport[ViewportIndex].z, Viewport[ViewportIndex].w);

			glUseProgram(ProgramName[program::NORMALIZED]);
			glUniform1i(UniformDiffuse[program::NORMALIZED], 0);
			glUniformMatrix4fv(UniformMVP[program::NORMALIZED], 1, GL_FALSE, &MVP[0][0]);

			glBindTexture(GL_TEXTURE_2D, TextureName[texture::RGBA8_SNORM]);

			glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
		}

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_320_texture_format Test(argc, argv);
	Error += Test();

	return Error;
}

