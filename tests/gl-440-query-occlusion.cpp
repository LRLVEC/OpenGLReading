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
	char const * VERT_SHADER_SOURCE("gl-440/query-occlusion.vert");
	char const * FRAG_SHADER_SOURCE("gl-440/query-occlusion.frag");

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
			QUERY,
			MAX
		};
	}//namespace buffer
}//namespace

class gl_440_query_occlusion : public test
{
public:
	gl_440_query_occlusion(int argc, char* argv[]) :
		test(argc, argv, "gl-440-query-occlusion", test::CORE, 4, 2),
		VertexArrayName(0),
		PipelineName(0),
		ProgramName(0)
	{}

private:
	std::array<GLuint, buffer::MAX> BufferName;
	std::array<glm::vec4, 4> Viewports;
	GLuint VertexArrayName;
	GLuint PipelineName;
	GLuint ProgramName;
	std::array<GLuint, 4> QueryName;

	bool initQuery()
	{
		glGenQueries(static_cast<GLsizei>(QueryName.size()), &QueryName[0]);

		return this->checkError("initQuery");
	}

	bool initProgram()
	{
		bool Validated = true;
	
		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 420 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 420 --profile core");
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

		return Validated && this->checkError("initProgram");
	}

	bool initBuffer()
	{
		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, PositionSize, &PositionData[0][0], GL_STATIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBuffer(GL_QUERY_BUFFER, BufferName[buffer::QUERY]);
		glBufferData(GL_QUERY_BUFFER, sizeof(GLuint) * QueryName.size(), nullptr, GL_DYNAMIC_COPY);
		glBindBuffer(GL_QUERY_BUFFER, 0);

		return this->checkError("initBuffer");
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
		glBindVertexArray(0);

		return this->checkError("initVertexArray");
	}

	bool begin()
	{
		bool Validated = true;
		Validated = Validated && this->checkExtension("GL_ARB_query_buffer_object");

		GLint QueryCounter(0);
		glGetQueryiv(GL_ANY_SAMPLES_PASSED_CONSERVATIVE, GL_QUERY_COUNTER_BITS, &QueryCounter);
		assert(QueryCounter > 0);

		glm::vec2 WindowSize(this->getWindowSize());
		this->Viewports[0] = glm::vec4(WindowSize * -0.5f, WindowSize * 0.5f);
		this->Viewports[1] = glm::vec4(0, 0, WindowSize * 0.5f);
		this->Viewports[2] = glm::vec4(WindowSize * 0.5f, WindowSize * 0.5f);
		this->Viewports[3] = glm::vec4(WindowSize * 1.0f, WindowSize * 0.5f);

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
		glDeleteQueries(static_cast<GLsizei>(QueryName.size()), &QueryName[0]);
		glDeleteProgramPipelines(1, &PipelineName);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteProgram(ProgramName);
		glDeleteVertexArrays(1, &VertexArrayName);

		return this->checkError("end");
	}

	#define BUFFER_UINT_OFFSET(i) ((GLuint *)NULL + (i))

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		{
			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glm::mat4* Pointer = reinterpret_cast<glm::mat4*>(glMapBufferRange(GL_UNIFORM_BUFFER,
				0, sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.1f, 100.0f);
			glm::mat4 Model = glm::mat4(1.0f);
		
			*Pointer = Projection * this->view() * Model;

			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}

		// Clear color buffer with black
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

		glBindProgramPipeline(PipelineName);
		glBindVertexArray(VertexArrayName);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, semantic::storage::VERTEX, BufferName[buffer::VERTEX]);

		// Samples count query
		for(std::size_t i = 0; i < this->Viewports.size(); ++i)
		{
			glViewportArrayv(0, 1, &this->Viewports[i][0]);

			glBeginQuery(GL_ANY_SAMPLES_PASSED, this->QueryName[i]);
				glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
			glEndQuery(GL_ANY_SAMPLES_PASSED);
		}

		glBindBuffer(GL_QUERY_BUFFER, BufferName[buffer::QUERY]);
		for(std::size_t i = 0; i < this->Viewports.size(); ++i)
			glGetQueryObjectuiv(this->QueryName[i], GL_QUERY_RESULT, BUFFER_UINT_OFFSET(i));

		GLuint* Pointer = reinterpret_cast<GLuint*>(glMapBufferRange(GL_QUERY_BUFFER, 0, sizeof(GLuint) * 4, GL_MAP_READ_BIT));

		fprintf(stdout, "Samples count: %d, %d, %d, %d\r", Pointer[0], Pointer[1], Pointer[2], Pointer[3]);

		glUnmapBuffer(GL_QUERY_BUFFER);
		glBindBuffer(GL_QUERY_BUFFER, 0);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_440_query_occlusion Test(argc, argv);
	Error += Test();

	return Error;
}

