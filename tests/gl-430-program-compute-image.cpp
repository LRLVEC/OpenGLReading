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
	char const * VS_SOURCE("gl-430/program-compute-image.vert");
	char const * FS_SOURCE("gl-430/program-compute-image.frag");
	char const * CS_SOURCE("gl-430/program-compute-image.comp");
	char const * TEXTURE_DIFFUSE("kueken1-bgr8.dds");

	GLsizei const VertexCount(8);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v4fv4fv4f);
	glf::vertex_v4fv4fv4f const VertexData[VertexCount] =
	{
		glf::vertex_v4fv4fv4f(glm::vec4(-1.0f,-1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, glm::vec2(0.0f)), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)),
		glf::vertex_v4fv4fv4f(glm::vec4( 1.0f,-1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, glm::vec2(0.0f)), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)),
		glf::vertex_v4fv4fv4f(glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, glm::vec2(0.0f)), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)),
		glf::vertex_v4fv4fv4f(glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, glm::vec2(0.0f)), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)),
		glf::vertex_v4fv4fv4f(glm::vec4(-1.0f,-1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, glm::vec2(0.0f)), glm::vec4(1.0f, 0.5f, 0.5f, 1.0f)),
		glf::vertex_v4fv4fv4f(glm::vec4( 1.0f,-1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, glm::vec2(0.0f)), glm::vec4(1.0f, 1.0f, 0.5f, 1.0f)),
		glf::vertex_v4fv4fv4f(glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, glm::vec2(0.0f)), glm::vec4(0.5f, 1.0f, 0.0f, 1.0f)),
		glf::vertex_v4fv4fv4f(glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, glm::vec2(0.0f)), glm::vec4(0.5f, 0.5f, 1.0f, 1.0f)),
	};

	GLsizeiptr const PositionSize = VertexCount * sizeof(glm::vec4);
	glm::vec4 const PositionData[VertexCount] = 
	{
		glm::vec4(-1.0f,-1.0f, 0.0f, 1.0f),
		glm::vec4( 1.0f,-1.0f, 0.0f, 1.0f),
		glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(-1.0f,-1.0f, 0.0f, 1.0f),
		glm::vec4( 1.0f,-1.0f, 0.0f, 1.0f),
		glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f)
	};

	GLsizeiptr const TexcoordSize = VertexCount * sizeof(glm::vec4);
	glm::vec4 const TexcoordData[VertexCount] = 
	{
		glm::vec4(0.0f, 1.0f, glm::vec2(0.0f)),
		glm::vec4(1.0f, 1.0f, glm::vec2(0.0f)),
		glm::vec4(1.0f, 0.0f, glm::vec2(0.0f)),
		glm::vec4(0.0f, 0.0f, glm::vec2(0.0f)),
		glm::vec4(0.0f, 1.0f, glm::vec2(0.0f)),
		glm::vec4(1.0f, 1.0f, glm::vec2(0.0f)),
		glm::vec4(1.0f, 0.0f, glm::vec2(0.0f)),
		glm::vec4(0.0f, 0.0f, glm::vec2(0.0f))
	};

	GLsizeiptr const ColorSize = VertexCount * sizeof(glm::vec4);
	glm::vec4 const ColorData[VertexCount] = 
	{
		glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
		glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
		glm::vec4(1.0f, 0.5f, 0.5f, 1.0f),
		glm::vec4(1.0f, 1.0f, 0.5f, 1.0f),
		glm::vec4(0.5f, 1.0f, 0.0f, 1.0f),
		glm::vec4(0.5f, 0.5f, 1.0f, 1.0f)
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2, 
		2, 3, 0
	};

	namespace program
	{
		enum type
		{
			GRAPHICS,
			COMPUTE,
			MAX
		};
	}//namespace program

	namespace buffer
	{
		enum type
		{
			ELEMENT,
			TRANSFORM,
			POSITION_INPUT,
			TEXCOORD_INPUT,
			COLOR_INPUT,
			POSITION_OUTPUT,
			TEXCOORD_OUTPUT,
			COLOR_OUTPUT,
			MAX
		};
	}//namespace buffer

	namespace texture
	{
		enum type
		{
			DIFFUSE,
			POSITION_INPUT,
			TEXCOORD_INPUT,
			COLOR_INPUT,
			POSITION_OUTPUT,
			TEXCOORD_OUTPUT,
			COLOR_OUTPUT,
			MAX
		};
	}//namespace texture

	namespace image
	{
		enum type
		{
			POSITION_INPUT		= 0,
			TEXCOORD_INPUT		= 1,
			COLOR_INPUT			= 2,
			POSITION_OUTPUT		= 3,
			TEXCOORD_OUTPUT		= 4,
			COLOR_OUTPUT		= 5
		};
	}//namespace image

	namespace semantics
	{
		enum type
		{
			INPUT,
			OUTPUT
		};
	}//namespace semantics
}//namespace

class gl_430_program_compute_image : public test
{
public:
	gl_430_program_compute_image(int argc, char* argv[]) :
		test(argc, argv, "gl-430-program-compute-image", test::CORE, 4, 2),
		VertexArrayName(0)
	{}

private:
	std::array<GLuint, program::MAX> PipelineName;
	std::array<GLuint, program::MAX> ProgramName;
	std::array<GLuint, buffer::MAX> BufferName;
	std::array<GLuint, texture::MAX> TextureName;
	GLuint VertexArrayName;

	bool initProgram()
	{
		bool Validated(true);
	
		glGenProgramPipelines(program::MAX, &PipelineName[0]);

		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VS_SOURCE, "--version 420 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FS_SOURCE, "--version 420 --profile core");
			GLuint CompShaderName = Compiler.create(GL_COMPUTE_SHADER, getDataDirectory() + CS_SOURCE, "--version 420 --profile core");
			Validated = Validated && Compiler.check();

			if(Validated)
			{
				ProgramName[program::GRAPHICS] = glCreateProgram();
				glProgramParameteri(ProgramName[program::GRAPHICS], GL_PROGRAM_SEPARABLE, GL_TRUE);
				glAttachShader(ProgramName[program::GRAPHICS], VertShaderName);
				glAttachShader(ProgramName[program::GRAPHICS], FragShaderName);
				glLinkProgram(ProgramName[program::GRAPHICS]);
			}

			if(Validated)
			{
				ProgramName[program::COMPUTE] = glCreateProgram();
				glProgramParameteri(ProgramName[program::COMPUTE], GL_PROGRAM_SEPARABLE, GL_TRUE);
				glAttachShader(ProgramName[program::COMPUTE], CompShaderName);
				glLinkProgram(ProgramName[program::COMPUTE]);
			}

			Validated = Validated && Compiler.checkProgram(ProgramName[program::GRAPHICS]);
			Validated = Validated && Compiler.checkProgram(ProgramName[program::COMPUTE]);
		}

		if(Validated)
		{
			glUseProgramStages(PipelineName[program::GRAPHICS], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName[program::GRAPHICS]);
			glUseProgramStages(PipelineName[program::COMPUTE], GL_COMPUTE_SHADER_BIT, ProgramName[program::COMPUTE]);
		}

		return Validated;
	}

	bool initBuffer()
	{
		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::POSITION_INPUT]);
		glBufferData(GL_ARRAY_BUFFER, PositionSize * 2, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, PositionSize, PositionData);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::TEXCOORD_INPUT]);
		glBufferData(GL_ARRAY_BUFFER, TexcoordSize * 2, TexcoordData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::COLOR_INPUT]);
		glBufferData(GL_ARRAY_BUFFER, ColorSize * 2, ColorData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::POSITION_OUTPUT]);
		glBufferData(GL_ARRAY_BUFFER, PositionSize * 2, NULL, GL_STATIC_COPY);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::TEXCOORD_OUTPUT]);
		glBufferData(GL_ARRAY_BUFFER, TexcoordSize * 2, NULL, GL_STATIC_COPY);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::COLOR_OUTPUT]);
		glBufferData(GL_ARRAY_BUFFER, ColorSize * 2, NULL, GL_STATIC_COPY);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLint UniformBufferOffset(0);
		glGetIntegerv(
			GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
			&UniformBufferOffset);
		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return true;
	}

	bool initTexture()
	{
		glGenTextures(texture::MAX, &TextureName[0]);

		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));
		assert(!Texture.empty());

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::DIFFUSE]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexStorage2D(GL_TEXTURE_2D, GLint(Texture.levels()), GL_RGBA8, GLsizei(Texture[0].dimensions().x), GLsizei(Texture[0].dimensions().y));
		for(gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexSubImage2D(
				GL_TEXTURE_2D, 
				GLint(Level), 
				0, 0, 
				GLsizei(Texture[Level].dimensions().x), 
				GLsizei(Texture[Level].dimensions().y), 
				GL_BGR, GL_UNSIGNED_BYTE, 
				Texture[Level].data());
		}
	
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		GLint TextureBufferOffset(0);
		glGetIntegerv(
			GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT,
			&TextureBufferOffset);
		GLint TextureBufferRange = glm::max(GLint(PositionSize), TextureBufferOffset);

		glBindTexture(GL_TEXTURE_BUFFER, TextureName[texture::POSITION_INPUT]);
		glTexBufferRange(GL_TEXTURE_BUFFER, GL_RGBA32F, BufferName[buffer::POSITION_INPUT], 0, TextureBufferRange);
		glBindTexture(GL_TEXTURE_BUFFER, TextureName[texture::TEXCOORD_INPUT]);
		glTexBufferRange(GL_TEXTURE_BUFFER, GL_RGBA32F, BufferName[buffer::TEXCOORD_INPUT], 0, TextureBufferRange);
		glBindTexture(GL_TEXTURE_BUFFER, TextureName[texture::COLOR_INPUT]);
		glTexBufferRange(GL_TEXTURE_BUFFER, GL_RGBA32F, BufferName[buffer::COLOR_INPUT], 0, TextureBufferRange);
		glBindTexture(GL_TEXTURE_BUFFER, TextureName[texture::POSITION_OUTPUT]);
		glTexBufferRange(GL_TEXTURE_BUFFER, GL_RGBA32F, BufferName[buffer::POSITION_OUTPUT], 0, TextureBufferRange);
		glBindTexture(GL_TEXTURE_BUFFER, TextureName[texture::TEXCOORD_OUTPUT]);
		glTexBufferRange(GL_TEXTURE_BUFFER, GL_RGBA32F, BufferName[buffer::TEXCOORD_OUTPUT], 0, TextureBufferRange);
		glBindTexture(GL_TEXTURE_BUFFER, TextureName[texture::COLOR_OUTPUT]);
		glTexBufferRange(GL_TEXTURE_BUFFER, GL_RGBA32F, BufferName[buffer::COLOR_OUTPUT], 0, TextureBufferRange);
		glBindTexture(GL_TEXTURE_BUFFER, 0);

		return true;
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBindVertexArray(0);

		return true;
	}

	bool begin()
	{
		this->logImplementationDependentLimit(GL_MAX_COMPUTE_UNIFORM_BLOCKS, "GL_MAX_COMPUTE_UNIFORM_BLOCKS");
		this->logImplementationDependentLimit(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, "GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS");
		this->logImplementationDependentLimit(GL_MAX_COMPUTE_IMAGE_UNIFORMS, "GL_MAX_COMPUTE_IMAGE_UNIFORMS");
		this->logImplementationDependentLimit(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, "GL_MAX_COMPUTE_SHARED_MEMORY_SIZE");
		this->logImplementationDependentLimit(GL_MAX_COMPUTE_UNIFORM_COMPONENTS, "GL_MAX_COMPUTE_UNIFORM_COMPONENTS");
		this->logImplementationDependentLimit(GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS, "GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS");
		this->logImplementationDependentLimit(GL_MAX_COMPUTE_ATOMIC_COUNTERS, "GL_MAX_COMPUTE_ATOMIC_COUNTERS");
		this->logImplementationDependentLimit(GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS, "GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS");
		this->logImplementationDependentLimit(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, "GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS");
	
		this->logImplementationDependentLimit(GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT, "GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT");

		bool Validated(true);
		Validated = Validated && this->checkExtension("GL_ARB_compute_shader");

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initVertexArray();

		return Validated;
	}

	bool end()
	{
		glDeleteProgramPipelines(program::MAX, &PipelineName[0]);
		glDeleteProgram(ProgramName[program::GRAPHICS]);
		glDeleteProgram(ProgramName[program::COMPUTE]);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteTextures(texture::MAX, &TextureName[0]);
		glDeleteVertexArrays(1, &VertexArrayName);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		{
			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(
				GL_UNIFORM_BUFFER, 0,	sizeof(glm::mat4),
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

			glm::mat4 Projection = glm::perspectiveFov(glm::pi<float>() * 0.25f, WindowSize.x, WindowSize.y, 0.1f, 100.0f);
			//glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);
			glm::mat4 Model = glm::mat4(1.0f);
		
			*Pointer = Projection * this->view() * Model;

			// Make sure the uniform buffer is uploaded
			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}

		glBindProgramPipeline(PipelineName[program::COMPUTE]);
		glBindImageTexture(image::POSITION_INPUT, TextureName[texture::POSITION_INPUT], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(image::TEXCOORD_INPUT, TextureName[texture::TEXCOORD_INPUT], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(image::COLOR_INPUT, TextureName[texture::COLOR_INPUT], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(image::POSITION_OUTPUT, TextureName[texture::POSITION_OUTPUT], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindImageTexture(image::TEXCOORD_OUTPUT, TextureName[texture::TEXCOORD_OUTPUT], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindImageTexture(image::COLOR_OUTPUT, TextureName[texture::COLOR_OUTPUT], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);
		glDispatchCompute(GLuint(VertexCount), 1, 1);

		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

		glBindProgramPipeline(PipelineName[program::GRAPHICS]);
		glActiveTexture(GL_TEXTURE0 + semantic::sampler::DIFFUSE);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::DIFFUSE]);
		glActiveTexture(GL_TEXTURE0 + semantic::sampler::POSITION);
		glBindTexture(GL_TEXTURE_BUFFER, TextureName[texture::POSITION_OUTPUT]);
		glActiveTexture(GL_TEXTURE0 + semantic::sampler::TEXCOORD);
		glBindTexture(GL_TEXTURE_BUFFER, TextureName[texture::TEXCOORD_OUTPUT]);
		glActiveTexture(GL_TEXTURE0 + semantic::sampler::COLOR);
		glBindTexture(GL_TEXTURE_BUFFER, TextureName[texture::COLOR_OUTPUT]);

		glBindVertexArray(VertexArrayName);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

		glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0), 1, 0, 0);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_430_program_compute_image Test(argc, argv);
	Error += Test();

	return Error;
}


