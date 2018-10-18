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
	char const * VERT_SHADER_SOURCE("gl-320/query-conditional.vert");
	char const * FRAG_SHADER_SOURCE("gl-320/query-conditional.frag");

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
			VERTEX,
			TRANSFORM,
			MATERIAL,
			MAX
		};
	}//namespace buffer
}//namespace

class gl_320_query_conditional : public test
{
public:
	gl_320_query_conditional(int argc, char* argv[]) :
		test(argc, argv, "gl-320-query-conditional", test::CORE, 3, 2),
		VertexArrayName(0),
		ProgramName(0),
		QueryName(0),
		UniformMaterialOffset(0)
	{}

private:
	std::array<GLuint, buffer::MAX> BufferName;
	GLuint VertexArrayName;
	GLuint ProgramName;
	GLuint QueryName;
	GLuint UniformMaterialOffset;

	bool initQuery()
	{
		glGenQueries(1, &QueryName);

		int QueryBits(0);
		glGetQueryiv(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &QueryBits);

		bool Validated = QueryBits >= 32;

		return Validated && this->checkError("initQuery");
	}

	bool initProgram()
	{
		bool Validated = true;
	
		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 150 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 150 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);
			
			glBindAttribLocation(ProgramName, semantic::attr::POSITION, "Position");
			glBindFragDataLocation(ProgramName, semantic::frag::COLOR, "Color");
			glLinkProgram(ProgramName);
			Validated = Compiler.checkProgram(ProgramName);
		}

		// Get variables locations
		if(Validated)
		{
			glUniformBlockBinding(ProgramName, glGetUniformBlockIndex(ProgramName, "transform"), semantic::uniform::TRANSFORM0);
			glUniformBlockBinding(ProgramName, glGetUniformBlockIndex(ProgramName, "material"), semantic::uniform::MATERIAL);
		}

		return Validated && this->checkError("initProgram");
	}

	bool initBuffer()
	{
		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);

		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, PositionSize, &PositionData[0][0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLint UniformTransformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferData(GL_UNIFORM_BUFFER, UniformTransformBlockSize, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		this->UniformMaterialOffset = glm::max(GLint(sizeof(glm::vec4)), UniformBufferOffset);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::MATERIAL]);
		glBufferData(GL_UNIFORM_BUFFER, this->UniformMaterialOffset * 2, nullptr, GL_STATIC_DRAW);

			glm::byte* Pointer = reinterpret_cast<glm::byte*>(
				glMapBufferRange(GL_UNIFORM_BUFFER, 0, this->UniformMaterialOffset * 2, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

			*reinterpret_cast<glm::vec4*>(Pointer + 0) = glm::vec4(0.0f, 0.5f, 1.0f, 1.0f);
			*reinterpret_cast<glm::vec4*>(Pointer + this->UniformMaterialOffset) = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);

			glUnmapBuffer(GL_UNIFORM_BUFFER);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return this->checkError("initBuffer");
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
		glBindVertexArray(0);

		return this->checkError("initVertexArray");
	}

	bool begin()
	{
		bool Validated = true;

		GLint QueryCounter(0);
		glGetQueryiv(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &QueryCounter);

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initQuery();

		return Validated && this->checkError("begin");
	}

	bool end()
	{
		glDeleteProgram(ProgramName);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteVertexArrays(1, &VertexArrayName);

		return this->checkError("end");
	}

	bool render()
	{
		glm::ivec2 WindowSize(this->getWindowSize());

		{
			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glm::mat4* Pointer = reinterpret_cast<glm::mat4*>(glMapBufferRange(GL_UNIFORM_BUFFER,
				0, sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, static_cast<float>(WindowSize.x) / static_cast<float>(WindowSize.y), 0.1f, 100.0f);
			glm::mat4 Model = glm::mat4(1.0f);
		
			*Pointer = Projection * this->view() * Model;

			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}

		// Set the display viewport
		glViewport(0, 0, WindowSize.x, WindowSize.y);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

		// Clear color buffer with black
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

		// Bind program
		glUseProgram(ProgramName);
		glBindVertexArray(VertexArrayName);

		glBindBufferRange(GL_UNIFORM_BUFFER, semantic::uniform::MATERIAL, BufferName[buffer::MATERIAL], 0, sizeof(glm::vec4));
	
		// The first orange quad is not written in the framebuffer.
		glColorMaski(0, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		// Beginning of the samples count query
		glBeginQuery(GL_SAMPLES_PASSED, QueryName);
			// To test the condional rendering, comment this line, the next draw call won't happen.
			glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
		// End of the samples count query
		glEndQuery(GL_SAMPLES_PASSED);

		// The second blue quad is written in the framebuffer only if a sample pass the occlusion query.
		glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	
		glBindBufferRange(GL_UNIFORM_BUFFER, semantic::uniform::MATERIAL, BufferName[buffer::MATERIAL], this->UniformMaterialOffset, sizeof(glm::vec4));

		// Draw only if one sample went through the tests, 
		// we don't need to get the query result which prevent the rendering pipeline to stall.
		glBeginConditionalRender(QueryName, GL_QUERY_WAIT);

			// Clear color buffer with white
			glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

			glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
		glEndConditionalRender();

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_320_query_conditional Test(argc, argv);
	Error += Test();

	return Error;
}

