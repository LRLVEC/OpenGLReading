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
	char const * VERT_SHADER_SOURCE_TRANSFORM("gl-320/transform-feedback-transform.vert");
	char const * VERT_SHADER_SOURCE_FEEDBACK("gl-320/transform-feedback-feedback.vert");
	char const * FRAG_SHADER_SOURCE_FEEDBACK("gl-320/transform-feedback-feedback.frag");

	GLsizei const VertexCount(6);
	GLsizeiptr const PositionSize = VertexCount * sizeof(glm::vec4);
	glm::vec4 const PositionData[VertexCount] =
	{
		glm::vec4(-1.0f,-1.0f, 0.0f, 1.0f),
		glm::vec4( 1.0f,-1.0f, 0.0f, 1.0f),
		glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(-1.0f,-1.0f, 0.0f, 1.0f)
	};

	namespace program
	{
		enum type
		{
			TRANSFORM,
			FEEDBACK,
			MAX
		};
	}//namespace program

	namespace buffer
	{
		enum type
		{
			VERTEX,
			POSITION,
			COLOR,
			MAX
		};
	}//namespace buffer

	namespace shader
	{
		enum type
		{
			VERT_TRANSFORM,
			VERT_FEEDBACK,
			FRAG_FEEDBACK,
			MAX
		};
	}//namespace shader

	std::vector<GLuint> BufferName(buffer::MAX);
	std::vector<GLuint> VertexArrayName(program::MAX);
	std::vector<GLuint> ProgramName(program::MAX);
	GLint UniformMVP(0);
	GLint UniformDiffuse(0);
}//namespace

class gl_320_transform_feedback_separated : public test
{
public:
	gl_320_transform_feedback_separated(int argc, char* argv[]) :
		test(argc, argv, "gl-320-transform-feedback-separated", test::CORE, 3, 2),
		QueryName(0)
	{}

private:
	GLuint QueryName;

	bool initProgram()
	{
		bool Validated = true;
	
		std::array<GLuint, shader::MAX> ShaderName;
		
		compiler Compiler;
		ShaderName[shader::VERT_TRANSFORM] = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE_TRANSFORM, "--version 150 --profile core");
		ShaderName[shader::VERT_FEEDBACK] = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE_FEEDBACK, "--version 150 --profile core");
		ShaderName[shader::FRAG_FEEDBACK] = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE_FEEDBACK, "--version 150 --profile core");
		Validated = Validated && Compiler.check();

		// Create program
		if(Validated)
		{
			ProgramName[program::TRANSFORM] = glCreateProgram();
			glAttachShader(ProgramName[program::TRANSFORM], ShaderName[shader::VERT_TRANSFORM]);
			glBindAttribLocation(ProgramName[program::TRANSFORM], semantic::attr::POSITION, "Position");

	// These two approaches behave the same way
	#if 1
			GLchar const * Strings[] = {"gl_Position", "block.Color"}; 
			glTransformFeedbackVaryings(ProgramName[program::TRANSFORM], 2, Strings, GL_SEPARATE_ATTRIBS); 
	#else // OpenGL 4.0
			GLchar const * Strings[] = {"gl_Position", "gl_NextBuffer", "VertColor"}; 
			glTransformFeedbackVaryings(ProgramName[program::TRANSFORM], 3, Strings, GL_INTERLEAVED_ATTRIBS); 
	#endif
			glLinkProgram(ProgramName[program::TRANSFORM]);

			Validated = Validated && Compiler.checkProgram(ProgramName[program::TRANSFORM]);

			char Name[64];
			memset(Name, 0, 64);
			GLsizei Length(0);
			GLsizei Size(0);
			GLenum Type(0);

			glGetTransformFeedbackVarying(
				ProgramName[program::TRANSFORM],
				0,
				64,
				&Length,
				&Size,
				&Type,
				Name);

			Validated = Validated && (Size == 1) && (Type == GL_FLOAT_VEC4);
		}

		// Get variables locations
		if(Validated)
		{
			UniformMVP = glGetUniformLocation(ProgramName[program::TRANSFORM], "MVP");
			Validated = Validated && (UniformMVP >= 0);
		}

		// Create program
		if(Validated)
		{
			ProgramName[program::FEEDBACK] = glCreateProgram();
			glAttachShader(ProgramName[program::FEEDBACK], ShaderName[shader::VERT_FEEDBACK]);
			glAttachShader(ProgramName[program::FEEDBACK], ShaderName[shader::FRAG_FEEDBACK]);

			glBindAttribLocation(ProgramName[program::FEEDBACK], semantic::attr::POSITION, "Position");
			glBindAttribLocation(ProgramName[program::FEEDBACK], semantic::attr::COLOR, "Color");
			glBindFragDataLocation(ProgramName[program::FEEDBACK], semantic::frag::COLOR, "Color");
			glLinkProgram(ProgramName[program::FEEDBACK]);
			Validated = Validated && Compiler.checkProgram(ProgramName[program::FEEDBACK]);
		}

		return Validated && this->checkError("initProgram");
	}

	bool initVertexArray()
	{
		glGenVertexArrays(program::MAX, &VertexArrayName[0]);

		// Build a vertex array object
		glBindVertexArray(VertexArrayName[program::TRANSFORM]);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 4, GL_FLOAT, GL_FALSE, 0, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
		glBindVertexArray(0);

		// Build a vertex array object
		glBindVertexArray(VertexArrayName[program::FEEDBACK]);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::POSITION]);
			glVertexAttribPointer(semantic::attr::POSITION, 4, GL_FLOAT, GL_FALSE, 0, 0);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::COLOR]);
			glVertexAttribPointer(semantic::attr::COLOR, 4, GL_FLOAT, GL_FALSE, 0, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
			glEnableVertexAttribArray(semantic::attr::COLOR);
		glBindVertexArray(0);

		return this->checkError("initVertexArray");
	}

	bool initBuffer()
	{
		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, PositionSize, PositionData, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::POSITION]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * VertexCount, NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::COLOR]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * VertexCount, NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return this->checkError("initBuffer");
	}

	bool initQuery()
	{
		glGenQueries(1, &QueryName);

		int QueryBits(0);
		glGetQueryiv(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, GL_QUERY_COUNTER_BITS, &QueryBits);

		bool Validated = QueryBits >= 1;

		return Validated && this->checkError("initQuery");
	}

	bool begin()
	{
		bool Validated = true;

		if(Validated)
			Validated = initQuery();
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
		for(std::size_t i = 0; i < program::MAX; ++i)
			glDeleteProgram(ProgramName[i]);
		glDeleteVertexArrays(program::MAX, &VertexArrayName[0]);
		glDeleteBuffers(program::MAX, &BufferName[0]);
		glDeleteQueries(1, &QueryName);

		return this->checkError("end");
	}

	bool render()
	{
		glm::ivec2 WindowSize(this->getWindowSize());

		glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * this->view() * Model;

		// Set the display viewport
		glViewport(0, 0, WindowSize.x, WindowSize.y);

		// Clear color buffer with black
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// First draw, capture the attributes
		{
			// Disable rasterisation, vertices processing only!
			glEnable(GL_RASTERIZER_DISCARD);

			glUseProgram(ProgramName[program::TRANSFORM]);
			glUniformMatrix4fv(UniformMVP, 1, GL_FALSE, &MVP[0][0]);

			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, BufferName[buffer::POSITION]); 
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, BufferName[buffer::COLOR]); 

			glBindVertexArray(VertexArrayName[program::TRANSFORM]);

			glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, QueryName); 
			glBeginTransformFeedback(GL_TRIANGLES);
				glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
			glEndTransformFeedback();
			glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN); 

			glDisable(GL_RASTERIZER_DISCARD);
		}

		// Second draw, reuse the captured attributes
		{
			glUseProgram(ProgramName[program::FEEDBACK]);

			GLuint PrimitivesWritten = 0;
			glGetQueryObjectuiv(QueryName, GL_QUERY_RESULT, &PrimitivesWritten);

			glBindVertexArray(VertexArrayName[program::FEEDBACK]);
			glDrawArraysInstanced(GL_TRIANGLES, 0, PrimitivesWritten * 3, 1);
		}

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_320_transform_feedback_separated Test(argc, argv);
	Error += Test();

	return Error;
}

