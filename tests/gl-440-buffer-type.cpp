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
#include <glm/gtc/packing.hpp>

namespace
{
	char const * VERT_SHADER_SOURCE("gl-440/buffer-type.vert");
	char const * FRAG_SHADER_SOURCE("gl-440/buffer-type.frag");

	GLsizei const VertexCount(6);
	GLsizeiptr const PositionSizeF16 = VertexCount * sizeof(glm::uint16) * 2;
	glm::uint16 const PositionDataF16[VertexCount * 2] =
	{
		glm::packHalf1x16(0.0f), glm::packHalf1x16(0.0f),
		glm::packHalf1x16(1.0f), glm::packHalf1x16(0.0f),
		glm::packHalf1x16(1.0f), glm::packHalf1x16(1.0f),
		glm::packHalf1x16(1.0f), glm::packHalf1x16(1.0f),
		glm::packHalf1x16(0.0f), glm::packHalf1x16(1.0f),
		glm::packHalf1x16(0.0f), glm::packHalf1x16(0.0f)
	};

	GLsizeiptr const PositionSizeF32 = VertexCount * sizeof(glm::vec2);
	glm::vec2 const PositionDataF32[VertexCount] =
	{
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(0.0f, 1.0f),
		glm::vec2(0.0f, 0.0f)
	};

	GLsizeiptr const PositionSizeI8 = VertexCount * sizeof(glm::i8vec2);
	glm::i8vec2 const PositionDataI8[VertexCount] =
	{
		glm::i8vec2(0, 0),
		glm::i8vec2(1, 0),
		glm::i8vec2(1, 1),
		glm::i8vec2(1, 1),
		glm::i8vec2(0, 1),
		glm::i8vec2(0, 0)
	};

	GLsizeiptr const PositionSizeRGB10A2 = VertexCount * sizeof(glm::uint32);
	glm::uint32 const PositionDataRGB10A2[VertexCount] =
	{
		glm::packSnorm3x10_1x2(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)),
		glm::packSnorm3x10_1x2(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)),
		glm::packSnorm3x10_1x2(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)),
		glm::packSnorm3x10_1x2(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)),
		glm::packSnorm3x10_1x2(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)),
		glm::packSnorm3x10_1x2(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f))
	};

	GLsizeiptr const PositionSizeI32 = VertexCount * sizeof(glm::i32vec2);
	glm::i32vec2 const PositionDataI32[VertexCount] =
	{
		glm::i32vec2(0, 0),
		glm::i32vec2(1, 0),
		glm::i32vec2(1, 1),
		glm::i32vec2(1, 1),
		glm::i32vec2(0, 1),
		glm::i32vec2(0, 0)
	};

	GLsizeiptr const PositionSizeRG11FB10F = VertexCount * sizeof(glm::uint32);
	glm::uint32 const PositionDataRG11FB10F[VertexCount] =
	{
		glm::packF2x11_1x10(glm::vec3( 0.0f, 0.0f, 0.0f)),
		glm::packF2x11_1x10(glm::vec3( 1.0f, 0.0f, 0.0f)),
		glm::packF2x11_1x10(glm::vec3( 1.0f, 1.0f, 0.0f)),
		glm::packF2x11_1x10(glm::vec3( 1.0f, 1.0f, 0.0f)),
		glm::packF2x11_1x10(glm::vec3( 0.0f, 1.0f, 0.0f)),
		glm::packF2x11_1x10(glm::vec3( 0.0f, 0.0f, 0.0f))
	};

	namespace vertex_format
	{
		enum type
		{
			F32,
			I8,
			I32,
			RGB10A2,
			F16,
			RG11B10F,
			MAX
		};
	}

	namespace buffer
	{
		enum type
		{
			VERTEX,
			TRANSFORM,
			MAX
		};
	}//namespace buffer

	namespace viewport
	{
		enum type
		{
			VIEWPORT0,
			VIEWPORT1,
			VIEWPORT2,
			VIEWPORT3,
			VIEWPORT4,
			VIEWPORT5,
			MAX
		};
	}//namespace viewport
}//namespace

class gl_440_buffer_type : public test
{
public:
	gl_440_buffer_type(int argc, char* argv[]) :
		test(argc, argv, "gl-440-buffer-type", test::CORE, 4, 4),
		PipelineName(0),
		ProgramName(0),
		UniformPointer(nullptr)
	{}

private:
	struct view
	{
		view(){}

		view(
			glm::vec4 const & Viewport, 
			vertex_format::type const & VertexFormat) :
			Viewport(Viewport),
			VertexFormat(VertexFormat)
		{}

		glm::vec4 Viewport;
		vertex_format::type VertexFormat;
	};

	std::array<view, viewport::MAX> Viewport;
	std::array<GLuint, buffer::MAX> BufferName;
	std::array<GLuint, vertex_format::MAX> VertexArrayName;
	GLuint PipelineName;
	GLuint ProgramName;
	glm::mat4* UniformPointer;

	bool initProgram()
	{
		bool Validated(true);
		
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
		{
			glGenProgramPipelines(1, &PipelineName);
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);
		}

		return Validated;
	}

	bool initBuffer()
	{
		// Generate a buffer object
		glGenBuffers(buffer::MAX, &BufferName[0]);

		// Allocate and copy buffers memory
		std::vector<glm::byte> Data(PositionSizeF32 + PositionSizeI8 + PositionSizeI32 + PositionSizeRGB10A2 + PositionSizeF16 + PositionSizeRG11FB10F);
		
		std::size_t CurrentOffset = 0;
		memcpy(&Data[0] + CurrentOffset, PositionDataF32, PositionSizeF32);
		CurrentOffset += PositionSizeF32;
		memcpy(&Data[0] + CurrentOffset, PositionDataI8, PositionSizeI8);
		CurrentOffset += PositionSizeI8;
		memcpy(&Data[0] + CurrentOffset, PositionDataI32, PositionSizeI32);
		CurrentOffset += PositionSizeI32;
		memcpy(&Data[0] + CurrentOffset, PositionDataRGB10A2, PositionSizeRGB10A2);
		CurrentOffset += PositionSizeRGB10A2;
		memcpy(&Data[0] + CurrentOffset, PositionDataF16, PositionSizeF16);
		CurrentOffset += PositionSizeF16;
		memcpy(&Data[0] + CurrentOffset, PositionDataRG11FB10F, PositionSizeRG11FB10F);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferStorage(GL_ARRAY_BUFFER, GLsizeiptr(Data.size()), &Data[0], 0);

		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferStorage(GL_UNIFORM_BUFFER, UniformBlockSize, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return this->checkError("initBuffer");
	}

	bool initVertexArray()
	{
		glGenVertexArrays(vertex_format::MAX, &VertexArrayName[0]);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);

		std::size_t CurrentOffset(0);
		glBindVertexArray(VertexArrayName[vertex_format::F32]);
		glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), BUFFER_OFFSET(CurrentOffset));
		glEnableVertexAttribArray(semantic::attr::POSITION);
		
		CurrentOffset += PositionSizeF32;
		glBindVertexArray(VertexArrayName[vertex_format::I8]);
		glVertexAttribPointer(semantic::attr::POSITION, 2, GL_BYTE, GL_FALSE, sizeof(glm::u8vec2), BUFFER_OFFSET(CurrentOffset));
		glEnableVertexAttribArray(semantic::attr::POSITION);

		CurrentOffset += PositionSizeI8;
		glBindVertexArray(VertexArrayName[vertex_format::I32]);
		glVertexAttribPointer(semantic::attr::POSITION, 2, GL_INT, GL_FALSE, sizeof(glm::i32vec2), BUFFER_OFFSET(CurrentOffset));
		glEnableVertexAttribArray(semantic::attr::POSITION);

		CurrentOffset += PositionSizeI32;
		glBindVertexArray(VertexArrayName[vertex_format::RGB10A2]);
		glVertexAttribPointer(semantic::attr::POSITION, 4, GL_INT_2_10_10_10_REV, GL_TRUE, sizeof(glm::uint), BUFFER_OFFSET(CurrentOffset));
		glEnableVertexAttribArray(semantic::attr::POSITION);

		CurrentOffset += PositionSizeRGB10A2;
		glBindVertexArray(VertexArrayName[vertex_format::F16]);
		glVertexAttribPointer(semantic::attr::POSITION, 2, GL_HALF_FLOAT, GL_FALSE, sizeof(glm::uint16) * 2, BUFFER_OFFSET(CurrentOffset));
		glEnableVertexAttribArray(semantic::attr::POSITION);

		CurrentOffset += PositionSizeRG11FB10F;
		glBindVertexArray(VertexArrayName[vertex_format::RG11B10F]);
		glVertexAttribPointer(semantic::attr::POSITION, 3, GL_UNSIGNED_INT_10F_11F_11F_REV, GL_FALSE, sizeof(glm::uint), BUFFER_OFFSET(CurrentOffset));
		glEnableVertexAttribArray(semantic::attr::POSITION);
		
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return this->checkError("initVertexArray");
	}

	bool begin()
	{
		glm::vec2 ViewportSize = glm::vec2(this->getWindowSize()) * glm::vec2(0.33f, 0.50f);

		Viewport[viewport::VIEWPORT0] = view(glm::vec4(ViewportSize.x * 0.0f, ViewportSize.y * 0.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f), vertex_format::F16);
		Viewport[viewport::VIEWPORT1] = view(glm::vec4(ViewportSize.x * 1.0f, ViewportSize.y * 0.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f), vertex_format::I32);
		Viewport[viewport::VIEWPORT2] = view(glm::vec4(ViewportSize.x * 2.0f, ViewportSize.y * 0.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f), vertex_format::RGB10A2);
		Viewport[viewport::VIEWPORT3] = view(glm::vec4(ViewportSize.x * 0.0f, ViewportSize.y * 1.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f), vertex_format::I8);
		Viewport[viewport::VIEWPORT4] = view(glm::vec4(ViewportSize.x * 1.0f, ViewportSize.y * 1.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f), vertex_format::F32);
		Viewport[viewport::VIEWPORT5] = view(glm::vec4(ViewportSize.x * 2.0f, ViewportSize.y * 1.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f), vertex_format::RG11B10F);

		bool Validated = true;

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		UniformPointer = (glm::mat4*)glMapBufferRange(
			GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		return Validated;
	}

	bool end()
	{
		if(!UniformPointer)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glUnmapBuffer(GL_UNIFORM_BUFFER);
			UniformPointer = nullptr;
		}

		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteVertexArrays(vertex_format::MAX, &VertexArrayName[0]);
		glDeleteProgramPipelines(1, &PipelineName);
		glDeleteProgram(ProgramName);

		return true;
	}

	bool render()
	{
		glm::uvec2 WindowSize = this->getWindowSize();

		{
			// Compute the MVP (Model View Projection matrix)
			float Aspect = (WindowSize.x * 0.33f) / (WindowSize.y * 0.50f);
			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, Aspect, 0.1f, 100.0f);
			glm::mat4 MVP = Projection * this->test::view() * glm::mat4(1.0f);

			*UniformPointer = MVP;
		}

		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

		glBindProgramPipeline(PipelineName);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

		for(std::size_t Index = 0; Index < viewport::MAX; ++Index)
		{
			glViewportIndexedf(0, 
				Viewport[Index].Viewport.x, 
				Viewport[Index].Viewport.y, 
				Viewport[Index].Viewport.z, 
				Viewport[Index].Viewport.w);

			glBindVertexArray(VertexArrayName[Viewport[Index].VertexFormat]);
			glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
		}

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_440_buffer_type Test(argc, argv);
	Error += Test();

	return Error;
}
