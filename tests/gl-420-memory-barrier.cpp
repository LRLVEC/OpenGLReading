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
	char const * SHADER_VERT_SOURCE_UPDATE("gl-420/memory-barrier-update.vert");
	char const * SHADER_FRAG_SOURCE_UPDATE("gl-420/memory-barrier-update.frag");
	char const * SHADER_VERT_SOURCE_BLIT("gl-420/memory-barrier-blit.vert");
	char const * SHADER_FRAG_SOURCE_BLIT("gl-420/memory-barrier-blit.frag");
	char const * TEXTURE_DIFFUSE("kueken4-dxt1.dds");

	GLsizei const VertexCount(3);

	namespace program
	{
		enum type
		{
			UPDATE,
			BLIT,
			MAX
		};
	}//namespace program
	
	namespace pipeline
	{
		enum type
		{
			UPDATE,
			BLIT,
			MAX
		};
	}//namespace pipeline

	namespace texture
	{
		enum type
		{
			DIFFUSE,
			COLORBUFFER,
			MAX
		};
	}//namespace texture
}//namespace

class gl_420_memory_barrier : public test
{
public:
	gl_420_memory_barrier(int argc, char* argv[]) :
		test(argc, argv, "gl-420-memory-barrier", test::CORE, 4, 2),
		VertexArrayName(0),
		FramebufferName(0),
		SamplerName(0),
		FrameBufferSize(0)
	{}

private:
	std::array<GLuint, pipeline::MAX> PipelineName;
	std::array<GLuint, program::MAX> ProgramName;
	std::array<GLuint, texture::MAX> TextureName;
	GLuint VertexArrayName;
	GLuint FramebufferName;
	GLuint SamplerName;
	glm::ivec2 FrameBufferSize;

	bool initProgram()
	{
		bool Validated(true);
	
		compiler Compiler;

		if(Validated)
		{
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + SHADER_VERT_SOURCE_UPDATE);
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + SHADER_FRAG_SOURCE_UPDATE);

			ProgramName[program::UPDATE] = glCreateProgram();
			glProgramParameteri(ProgramName[program::UPDATE], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[program::UPDATE], VertShaderName);
			glAttachShader(ProgramName[program::UPDATE], FragShaderName);
			glLinkProgram(ProgramName[program::UPDATE]);
		}

		if(Validated)
		{
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + SHADER_VERT_SOURCE_BLIT);
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + SHADER_FRAG_SOURCE_BLIT);

			ProgramName[program::BLIT] = glCreateProgram();
			glProgramParameteri(ProgramName[program::BLIT], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[program::BLIT], VertShaderName);
			glAttachShader(ProgramName[program::BLIT], FragShaderName);
			glLinkProgram(ProgramName[program::BLIT]);
		}

		if(Validated)
		{
			Validated = Validated && Compiler.checkProgram(ProgramName[program::UPDATE]);
			Validated = Validated && Compiler.checkProgram(ProgramName[program::BLIT]);
		}

		if(Validated)
		{
			glGenProgramPipelines(pipeline::MAX, &PipelineName[0]);
			glUseProgramStages(PipelineName[pipeline::UPDATE], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName[program::UPDATE]);
			glUseProgramStages(PipelineName[pipeline::BLIT], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName[program::BLIT]);
		}

		return Validated && this->checkError("initProgram - stage");
	}

	bool initSampler()
	{
		bool Validated(true);

		glGenSamplers(1, &SamplerName);
		glSamplerParameteri(SamplerName, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri(SamplerName, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		return Validated;
	}

	bool initTexture()
	{
		bool Validated(true);

		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));
		this->FrameBufferSize = Texture.dimensions();

		glGenTextures(texture::MAX, &TextureName[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::DIFFUSE]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
		glTexStorage2D(GL_TEXTURE_2D, GLint(Texture.levels()), GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 
			GLsizei(Texture.dimensions().x), GLsizei(Texture.dimensions().y));

		for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
		{
			glCompressedTexSubImage2D(GL_TEXTURE_2D,
				GLint(Level),
				0, 0,
				GLsizei(Texture[Level].dimensions().x), 
				GLsizei(Texture[Level].dimensions().y), 
				GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 
				GLsizei(Texture[Level].size()), 
				Texture[Level].data());
		}

		glBindTexture(GL_TEXTURE_2D, TextureName[texture::COLORBUFFER]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, this->FrameBufferSize.x, this->FrameBufferSize.y);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);

		return Validated;
	}

	bool initFramebuffer()
	{
		bool Validated(true);

		glGenFramebuffers(1, &FramebufferName);
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureName[texture::COLORBUFFER], 0);

		Validated = Validated && (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return Validated;
	}

	bool initVertexArray()
	{
		bool Validated(true);

		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
		glBindVertexArray(0);

		return Validated;
	}

	bool begin()
	{
		bool Validated(true);

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initSampler();
		if(Validated)
			Validated = initFramebuffer();

		return Validated;
	}

	bool end()
	{
		glDeleteTextures(texture::MAX, &TextureName[0]);
		glDeleteFramebuffers(1, &FramebufferName);
		glDeleteProgram(ProgramName[program::BLIT]);
		glDeleteProgram(ProgramName[program::UPDATE]);
		glDeleteVertexArrays(1, &VertexArrayName);
		glDeleteSamplers(1, &SamplerName);
		glDeleteProgramPipelines(pipeline::MAX, &PipelineName[0]);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		static int FrameIndex = 0;

		// Bind shared objects
		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);
		glBindSampler(0, SamplerName);
		glBindVertexArray(VertexArrayName);

		// Update a colorbuffer bound as a framebuffer attachement and as a texture
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glBindProgramPipeline(PipelineName[pipeline::UPDATE]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, FrameIndex ? TextureName[texture::COLORBUFFER] : TextureName[texture::DIFFUSE]);

		glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
		glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, VertexCount, 1, 0);

		// Blit to framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindProgramPipeline(PipelineName[pipeline::BLIT]);
	
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::COLORBUFFER]);

		glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
		glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, VertexCount, 1, 0);

		FrameIndex = (FrameIndex + 1) % 256;

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_420_memory_barrier Test(argc, argv);
	Error += Test();

	return Error;
}

