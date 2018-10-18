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
	char const * VERT_SHADER_SOURCE("gl-330/query-counter.vert");
	char const * FRAG_SHADER_SOURCE("gl-330/query-counter.frag");

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

	namespace query
	{
		enum type
		{
			BEGIN,
			END,
			MAX
		};
	}//namespace query

	GLuint VertexArrayName = 0;
	GLuint ProgramName = 0;
	GLuint BufferName = 0;
	GLint UniformMVP = 0;
}//namespace

class gl_330_query_timer : public test
{
public:
	gl_330_query_timer(int argc, char* argv[]) :
		test(argc, argv, "gl-330-query-counter", test::CORE, 3, 3)
	{}

private:
	std::array<GLuint, query::MAX> QueryName;

	bool initQuery()
	{
		int QueryBits(0);
		glGetQueryiv(GL_TIMESTAMP, GL_QUERY_COUNTER_BITS, &QueryBits);

		bool Validated = QueryBits >= 30;
		if(Validated)
			glGenQueries(static_cast<GLsizei>(QueryName.size()), &QueryName[0]);

		return Validated && this->checkError("initQuery");
	}

	bool initProgram()
	{
		bool Validated = true;
	
		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 330 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 330 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);
			glLinkProgram(ProgramName);
			Validated = Validated && Compiler.checkProgram(ProgramName);
		}

		// Get variables locations
		if(Validated)
		{
			UniformMVP = glGetUniformLocation(ProgramName, "MVP");
			glUseProgram(ProgramName);
			glUniform4fv(glGetUniformLocation(ProgramName, "Diffuse"), 1, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);
			glUseProgram(0);
		}

		return Validated && this->checkError("initProgram");
	}

	// Buffer update using glBufferSubData
	bool initBuffer()
	{
		// Generate a buffer object
		glGenBuffers(1, &BufferName);

		// Bind the buffer for use
		glBindBuffer(GL_ARRAY_BUFFER, BufferName);

		// Reserve buffer memory but and copy the values
		glBufferData(GL_ARRAY_BUFFER, PositionSize, &PositionData[0][0], GL_STATIC_DRAW);

		// Unbind the buffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return this->checkError("initBuffer");
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, 0, 0);
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
		if(Validated)
			Validated = initQuery();

		return Validated && this->checkError("begin");
	}

	bool end()
	{
		glDeleteBuffers(1, &BufferName);
		glDeleteProgram(ProgramName);
		glDeleteVertexArrays(1, &VertexArrayName);

		return this->checkError("end");
	}

	bool render()
	{
		glm::ivec2 WindowSize(this->getWindowSize());

		glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * this->view() * Model;

		glQueryCounter(QueryName[query::BEGIN], GL_TIMESTAMP);

		glViewport(0, 0, WindowSize.x, WindowSize.y);

		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

		glUseProgram(ProgramName);
		glUniformMatrix4fv(UniformMVP, 1, GL_FALSE, &MVP[0][0]);

		glBindVertexArray(VertexArrayName);
		glDrawArrays(GL_TRIANGLES, 0, VertexCount);

		glQueryCounter(QueryName[query::END], GL_TIMESTAMP);

		GLint AvailableBegin = GL_FALSE;
		glGetQueryObjectiv(QueryName[query::BEGIN], GL_QUERY_RESULT_AVAILABLE, &AvailableBegin);

		GLint AvailableEnd = GL_FALSE;
		glGetQueryObjectiv(QueryName[query::END], GL_QUERY_RESULT_AVAILABLE, &AvailableEnd);

		// The OpenGL implementations will wait for the query if it's not available
		GLint64 TimeBegin = 0, TimeEnd = 0;
		glGetQueryObjecti64v(QueryName[query::BEGIN], GL_QUERY_RESULT, &TimeBegin);
		glGetQueryObjecti64v(QueryName[query::END], GL_QUERY_RESULT, &TimeEnd);

		//glGetInteger64v(GL_TIMESTAMP, &TimeBegin);
		//glGetInteger64v(GL_TIMESTAMP, &TimeEnd);

		fprintf(stdout, "%d, %d / Time stamp: %f ms   \r", AvailableBegin, AvailableEnd, (TimeEnd - TimeBegin) / 1000.f / 1000.f);

		return TimeEnd - TimeBegin > 0;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_330_query_timer Test(argc, argv);
	Error += Test();

	return Error;
}
