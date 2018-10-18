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
	char const * VERT_SHADER_SOURCE("gl-500/buffer-sparse.vert");
	char const * FRAG_SHADER_SOURCE("gl-500/buffer-sparse.frag");
	char const * TEXTURE_DIFFUSE("kueken7_srgba8_unorm.dds");

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
			COPY,
			VERTEX,
			ELEMENT,
			TRANSFORM,
			MAX
		};
	}//namespace buffer
}//namespace

class gl_500_buffer_storage_arb : public test
{
public:
	gl_500_buffer_storage_arb(int argc, char* argv[]) :
		test(argc, argv, "gl-500-buffer-storage-arb", test::CORE, 4, 5),
		PipelineName(0),
		VertexArrayName(0),
		TextureName(0),
		ProgramName(0),
		UniformPointer(nullptr)
	{}

private:
	GLuint PipelineName;
	GLuint VertexArrayName;
	GLuint TextureName;
	GLuint ProgramName;
	std::array<GLuint, buffer::MAX> BufferName;
	glm::mat4* UniformPointer;

	bool initProgram()
	{
		bool Validated(true);
	
		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 450 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 450 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glProgramParameteri(ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);
			glLinkProgram(ProgramName);
			Validated = Validated && Compiler.checkProgram(ProgramName);
		}

		if(Validated)
		{
			glCreateProgramPipelines(1, &PipelineName);
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);
		}

		return Validated;
	}

	bool initBuffer()
	{
		GLint const Alignement = 256;
		GLint BufferPageSize = 0;
		glGetIntegerv(GL_SPARSE_BUFFER_PAGE_SIZE_ARB, &BufferPageSize);

		bool Validated(true);

		GLintptr CopyBufferSize = glm::ceilMultiple<GLint>(VertexSize, Alignement) + glm::ceilMultiple<GLint>(ElementSize, Alignement);

		glCreateBuffers(buffer::MAX, &BufferName[0]);

		glNamedBufferStorage(BufferName[buffer::COPY], CopyBufferSize, nullptr, GL_MAP_WRITE_BIT);
		glm::byte* CopyBufferPointer = reinterpret_cast<glm::byte*>(glMapNamedBufferRange(BufferName[buffer::COPY], 0, CopyBufferSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
		memcpy(CopyBufferPointer + 0, VertexData, VertexSize);
		memcpy(CopyBufferPointer + glm::ceilMultiple<GLint>(VertexSize, Alignement), ElementData, ElementSize);
		glUnmapNamedBuffer(BufferName[buffer::COPY]);

		glBindBuffer(GL_COPY_READ_BUFFER, BufferName[buffer::COPY]);

		glBindBuffer(GL_COPY_WRITE_BUFFER, BufferName[buffer::ELEMENT]);
		glBufferStorage(GL_COPY_WRITE_BUFFER, glm::ceilMultiple<GLint>(ElementSize, BufferPageSize), nullptr, GL_SPARSE_STORAGE_BIT_ARB);
		glBufferPageCommitmentARB(GL_COPY_WRITE_BUFFER, 0, BufferPageSize, GL_TRUE);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, glm::ceilMultiple<GLint>(VertexSize, Alignement), 0, glm::ceilMultiple<GLint>(ElementSize, Alignement));

		glBindBuffer(GL_COPY_WRITE_BUFFER, BufferName[buffer::VERTEX]);
		glBufferStorage(GL_COPY_WRITE_BUFFER, glm::ceilMultiple<GLint>(VertexSize, BufferPageSize), nullptr, GL_SPARSE_STORAGE_BIT_ARB);
		glBufferPageCommitmentARB(GL_COPY_WRITE_BUFFER, 0, BufferPageSize, GL_TRUE);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, glm::ceilMultiple<GLint>(VertexSize, Alignement));

		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glNamedBufferStorage(BufferName[buffer::TRANSFORM], UniformBlockSize, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

		return Validated;
	}

	bool initTexture()
	{
		bool Validated(true);

		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &TextureName);
		glTextureParameteri(TextureName, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTextureParameteri(TextureName, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTextureParameteri(TextureName, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTextureParameteri(TextureName, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
		glTextureParameteri(TextureName, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(TextureName, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
		glTextureParameteri(TextureName, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(TextureName, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureStorage3D(TextureName, static_cast<GLint>(Texture.levels()),
			gli::internal_format(Texture.format()),
			static_cast<GLsizei>(Texture[0].dimensions().x), static_cast<GLsizei>(Texture[0].dimensions().y), static_cast<GLsizei>(1));

		for(gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glTextureSubImage3D(TextureName, static_cast<GLint>(Level),
				0, 0, 0,
				static_cast<GLsizei>(Texture[Level].dimensions().x), static_cast<GLsizei>(Texture[Level].dimensions().y), static_cast<GLsizei>(1),
				gli::external_format(Texture.format()),
				gli::type_format(Texture.format()),
				Texture[Level].data());
		}
	
		return Validated;
	}

	bool initVertexArray()
	{
		bool Validated(true);

		glCreateVertexArrays(1, &VertexArrayName);
		glVertexArrayElementBuffer(VertexArrayName, BufferName[buffer::ELEMENT]);

		return Validated;
	}

	bool begin()
	{
		bool Validated(true);
		Validated = Validated && this->checkExtension("GL_ARB_buffer_storage");
		Validated = Validated && this->checkExtension("GL_ARB_sparse_buffer");

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initTexture();
		if(Validated)
		{
			UniformPointer = (glm::mat4*)glMapNamedBufferRange(
				BufferName[buffer::TRANSFORM], 0, sizeof(glm::mat4),
				GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		}

		return Validated;
	}

	bool end()
	{
		if(!UniformPointer)
		{
			glUnmapNamedBuffer(BufferName[buffer::TRANSFORM]);
			UniformPointer = nullptr;
		}

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
			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.1f, 100.0f);
			glm::mat4 MVP = Projection * this->test::view() * glm::mat4(1.0f);

			*UniformPointer = MVP;
		}

		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

		glBindProgramPipeline(PipelineName);
		glBindTextureUnit(semantic::sampler::DIFFUSE, TextureName);
		glBindVertexArray(VertexArrayName);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, semantic::storage::VERTEX, BufferName[buffer::VERTEX]);

		glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 1, 0, 0);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_500_buffer_storage_arb Test(argc, argv);
	Error += Test();

	return Error;
}
