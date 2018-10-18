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
	char const * VERT_SHADER_SOURCE("gl-320/buffer-update.vert");
	char const * FRAG_SHADER_SOURCE("gl-320/buffer-update.frag");

	GLsizei const VertexCount(6);
	GLsizeiptr const PositionSize = VertexCount * sizeof(glm::vec2);
	glm::vec2 const PositionData[VertexCount] =
	{
		glm::vec2(-1.0f,-1.0f),
		glm::vec2( 1.0f,-1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2(-1.0f, 1.0f),
		glm::vec2(-1.0f,-1.0f)
	};

	namespace buffer
	{
		enum type
		{
			ARRAY,
			COPY,
			MATERIAL,
			TRANSFORM,
			MAX
		};
	}//namespace program
}//namespace

class gl_320_buffer_update : public test
{
public:
	gl_320_buffer_update(int argc, char* argv[]) :
		test(argc, argv, "gl-320-buffer-update", test::CORE, 3, 2),
		VertexArrayName(0),
		ProgramName(0),
		UniformTransform(0),
		UniformMaterial(0)
	{}

private:
	std::array<GLuint, buffer::MAX> BufferName;
	GLuint ProgramName;
	GLuint VertexArrayName;
	GLint UniformTransform;
	GLint UniformMaterial;

	bool initProgram()
	{
		bool Validated = true;
	
		compiler Compiler;

		// Create program
		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 150 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 150 --profile core");

			ProgramName = glCreateProgram();
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);

			glBindAttribLocation(ProgramName, semantic::attr::POSITION, "Position");
			glBindFragDataLocation(ProgramName, semantic::frag::COLOR, "Color");
			glLinkProgram(ProgramName);

			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.checkProgram(ProgramName);
		}

		// Get variables locations
		if(Validated)
		{
			UniformTransform = glGetUniformBlockIndex(ProgramName, "transform");
			UniformMaterial = glGetUniformBlockIndex(ProgramName, "material");

			glUniformBlockBinding(ProgramName, UniformTransform, semantic::uniform::TRANSFORM0);
			glUniformBlockBinding(ProgramName, UniformMaterial, semantic::uniform::MATERIAL);
		}

		return Validated && this->checkError("initProgram");
	}

	// Buffer update using glMapBufferRange
	bool initBuffer()
	{
		// Generate a buffer object
		glGenBuffers(buffer::MAX, &BufferName[0]);

		// Bind the buffer for use
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::ARRAY]);

		// Reserve buffer memory but don't copy the values
		glBufferData(
			GL_ARRAY_BUFFER, 
			PositionSize, 
			0, 
			GL_STATIC_DRAW);

		// Copy the vertex data in the buffer, in this sample for the whole range of data.
		// It doesn't required to be the buffer size but pointers require no memory overlapping.
		GLvoid* Data = glMapBufferRange(
			GL_ARRAY_BUFFER, 
			0,				// Offset
			PositionSize,	// Size,
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
		memcpy(Data, PositionData, PositionSize);

		// Explicitly send the data to the graphic card.
		glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, PositionSize);

		glUnmapBuffer(GL_ARRAY_BUFFER);

		// Unbind the buffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Copy buffer
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::COPY]);
		glBufferData(GL_ARRAY_BUFFER, PositionSize, 0, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_COPY_READ_BUFFER, BufferName[buffer::ARRAY]);
		glBindBuffer(GL_COPY_WRITE_BUFFER, BufferName[buffer::COPY]);

		glCopyBufferSubData(
			GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
			0, 0,
			PositionSize);

		glBindBuffer(GL_COPY_READ_BUFFER, 0);
		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

		GLint UniformBlockSize = 0;

		{
			glGetActiveUniformBlockiv(
				ProgramName, 
				UniformTransform,
				GL_UNIFORM_BLOCK_DATA_SIZE,
				&UniformBlockSize);

			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, 0, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		{
			glm::vec4 Diffuse(1.0f, 0.5f, 0.0f, 1.0f);

			glGetActiveUniformBlockiv(
				ProgramName, 
				UniformMaterial,
				GL_UNIFORM_BLOCK_DATA_SIZE,
				&UniformBlockSize);

			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::MATERIAL]);
			glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, &Diffuse[0], GL_DYNAMIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		return this->checkError("initBuffer");
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::COPY]);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), BUFFER_OFFSET(0));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
		glBindVertexArray(0);

		return this->checkError("initVertexArray");
	}

	bool begin()
	{
		bool Validated = true;

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
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteProgram(ProgramName);
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

			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.1f, 100.0f);
			glm::mat4 Model = glm::mat4(1.0f);
			glm::mat4 MVP = Projection * this->view() * Model;
		
			*Pointer = MVP;

			// Make sure the uniform buffer is uploaded
			glUnmapBuffer(GL_UNIFORM_BUFFER);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		glViewport(0, 0, static_cast<GLsizei>(WindowSize.x), static_cast<GLsizei>(WindowSize.y));
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

		glUseProgram(ProgramName);

		// Attach the buffer to UBO binding point semantic::uniform::TRANSFORM0
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);
		// Attach the buffer to UBO binding point semantic::uniform::MATERIAL
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::MATERIAL, BufferName[buffer::MATERIAL]);

		glBindVertexArray(VertexArrayName);
		glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_320_buffer_update Test(argc, argv);
	Error += Test();

	return Error;
}

