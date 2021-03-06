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
	char const * VERT_SHADER_SOURCE("gl-500/texture-sparse.vert");
	char const * FRAG_SHADER_SOURCE("gl-500/texture-sparse.frag");
	char const * TEXTURE_DIFFUSE("kueken1-bgr8.dds");

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f))
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2, 
		2, 3, 0
	};

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			TRANSFORM,
			MAX
		};
	}//namespace buffer
}//namespace

class gl_500_texture_sparse_arb : public test
{
public:
	gl_500_texture_sparse_arb(int argc, char* argv[]) :
		test(argc, argv, "gl-500-texture-sparse-arb", test::CORE, 4, 4, glm::uvec2(640, 480), glm::vec2(0.0f, -glm::pi<float>() * 0.4f)),
		PipelineName(0),
		ProgramName(0),
		VertexArrayName(0),
		TextureName(0)
	{}

private:
	std::array<GLuint, buffer::MAX> BufferName;
	GLuint PipelineName;
	GLuint ProgramName;
	GLuint TextureName;
	GLuint VertexArrayName;

	bool initProgram()
	{
		bool Validated(true);
	
		glGenProgramPipelines(1, &PipelineName);

		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 440 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 440 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glProgramParameteri(ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);
			glLinkProgram(ProgramName);

			Validated = Validated && Compiler.checkProgram(ProgramName);
		}

		if(Validated)
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);

		return Validated;
	}

	bool initBuffer()
	{
		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);

		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return true;
	}

	bool initTexture()
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		GLsizei const Size(16384);
		std::size_t const Levels = gli::levels(Size);
		std::size_t const MaxLevels = 4;

		glGenTextures(1, &TextureName);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, TextureName);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, MaxLevels - 1);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SPARSE_ARB, GL_TRUE);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, static_cast<GLsizei>(gli::levels(Size)), GL_RGBA8, GLsizei(Size), GLsizei(Size), 1);

		glm::ivec3 PageSize;
		glGetInternalformativ(GL_TEXTURE_2D_ARRAY, GL_RGBA8, GL_VIRTUAL_PAGE_SIZE_X_ARB, 1, &PageSize.x);
		glGetInternalformativ(GL_TEXTURE_2D_ARRAY, GL_RGBA8, GL_VIRTUAL_PAGE_SIZE_Y_ARB, 1, &PageSize.y);
		glGetInternalformativ(GL_TEXTURE_2D_ARRAY, GL_RGBA8, GL_VIRTUAL_PAGE_SIZE_Z_ARB, 1, &PageSize.z);

		std::vector<glm::u8vec4> Page;
		Page.resize(static_cast<std::size_t>(PageSize.x * PageSize.y * PageSize.z));

		GLint Page3DSizeX(0);
		GLint Page3DSizeY(0);
		GLint Page3DSizeZ(0);
		glGetInternalformativ(GL_TEXTURE_3D, GL_RGBA32F, GL_VIRTUAL_PAGE_SIZE_X_ARB, 1, &Page3DSizeX);
		glGetInternalformativ(GL_TEXTURE_3D, GL_RGBA32F, GL_VIRTUAL_PAGE_SIZE_Y_ARB, 1, &Page3DSizeY);
		glGetInternalformativ(GL_TEXTURE_3D, GL_RGBA32F, GL_VIRTUAL_PAGE_SIZE_Z_ARB, 1, &Page3DSizeZ);

		for(std::size_t Level = 0; Level < MaxLevels; ++Level)
		{
			GLsizei LevelSize = (Size >> Level);
			GLsizei TileCountY = LevelSize / PageSize.y;
			GLsizei TileCountX = LevelSize / PageSize.x;

			for(GLsizei j = 0; j < TileCountY; ++j)
			for(GLsizei i = 0; i < TileCountX; ++i)
			{
				if(glm::abs(glm::length(glm::vec2(i, j) / glm::vec2(TileCountX, TileCountY) * 2.0f - 1.0f)) > 1.0f)
					continue;

				std::fill(Page.begin(), Page.end(), glm::u8vec4(
					static_cast<unsigned char>(float(i) / float(LevelSize / PageSize.x) * 255),
					static_cast<unsigned char>(float(j) / float(LevelSize / PageSize.y) * 255),
					static_cast<unsigned char>(float(Level) / float(MaxLevels) * 255), 255));

				glTexPageCommitmentARB(GL_TEXTURE_2D_ARRAY, static_cast<GLint>(Level),
					static_cast<GLsizei>(PageSize.x) * i, static_cast<GLsizei>(PageSize.y) * j, 0,
					static_cast<GLsizei>(PageSize.x), static_cast<GLsizei>(PageSize.y), 1,
					GL_TRUE);

				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, static_cast<GLint>(Level),
					static_cast<GLsizei>(PageSize.x) * i, static_cast<GLsizei>(PageSize.y) * j, 0,
					static_cast<GLsizei>(PageSize.x), static_cast<GLsizei>(PageSize.y), 1,
					GL_RGBA, GL_UNSIGNED_BYTE,
					&Page[0][0]);
			}
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

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
		bool Validated(true);
		Validated = Validated && this->checkExtension("GL_ARB_sparse_texture");

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initTexture();

		return Validated;
	}

	bool end()
	{
		glDeleteProgramPipelines(1, &PipelineName);
		glDeleteProgram(ProgramName);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteTextures(1, &TextureName);
		glDeleteVertexArrays(1, &VertexArrayName);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		{
			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glm::mat4* Pointer = reinterpret_cast<glm::mat4*>(glMapBufferRange(
				GL_UNIFORM_BUFFER, 0,	sizeof(glm::mat4),
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.001f, 100.0f);
			glm::mat4 Model = glm::mat4(1.0f);
		
			*Pointer = Projection * this->view() * Model;

			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}

		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

		glBindProgramPipeline(PipelineName);
		glBindVertexArray(VertexArrayName);
		glBindTextures(semantic::sampler::DIFFUSE, 1, &TextureName);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, semantic::storage::VERTEX, BufferName[buffer::VERTEX]);

		glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 1, 0, 0);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_500_texture_sparse_arb Test(argc, argv);
	Error += Test();

	return Error;
}

