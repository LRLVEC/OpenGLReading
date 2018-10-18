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
	char const * VERT_SHADER_SOURCE("gl-410/double.vert");
	char const * FRAG_SHADER_SOURCE("gl-410/double.frag");

	GLsizei const VertexCount(4);
	GLsizeiptr const PositionSize = VertexCount * sizeof(glm::dvec3);
	glm::dvec3 const PositionData[VertexCount] =
	{
		glm::dvec3(-1.0f,-1.0f, 0.0f),
		glm::dvec3( 1.0f,-1.0f, 0.0f),
		glm::dvec3( 1.0f, 1.0f, 0.0f),
		glm::dvec3(-1.0f, 1.0f, 0.0f)
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(glm::uint16);
	glm::uint16 const ElementData[ElementCount] =
	{
		0, 1, 2,
		0, 2, 3
	};

	namespace buffer
	{
		enum type
		{
			F64,
			ELEMENT,
			MAX
		};
	}//namespace buffer

	namespace program
	{
		enum type
		{
			VERT,
			FRAG,
			MAX
		};
	}//namespace program

	GLuint PipelineName(0);
	GLuint ProgramName[program::MAX] = {0, 0};
	GLuint BufferName[buffer::MAX] = {0, 0};
	GLuint VertexArrayName(0);
	GLint UniformMVP(0);
	GLint UniformDiffuse(0);
}//namespace

class gl_410_program_64 : public test
{
public:
	gl_410_program_64(int argc, char* argv[]) :
		test(argc, argv, "gl-410-program-64", test::CORE, 4, 1)
	{}

private:
	bool initProgram()
	{
		bool Validated = true;

		if(Validated)
		{
			std::string VertexSourceContent = this->loadFile(getDataDirectory() + VERT_SHADER_SOURCE);
			char const * VertexSourcePointer = VertexSourceContent.c_str();
			ProgramName[program::VERT] = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &VertexSourcePointer);
		}

		if(Validated)
		{
			std::string FragmentSourceContent = this->loadFile(getDataDirectory() + FRAG_SHADER_SOURCE);
			char const * FragmentSourcePointer = FragmentSourceContent.c_str();
			ProgramName[program::FRAG] = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &FragmentSourcePointer);
		}

		if(Validated)
		{
			compiler Compiler;
			Validated = Validated && Compiler.checkProgram(ProgramName[program::VERT]);
			Validated = Validated && Compiler.checkProgram(ProgramName[program::FRAG]);
		}

		if(Validated)
		{
			UniformMVP = glGetUniformLocation(ProgramName[program::VERT], "MVP");
			UniformDiffuse = glGetUniformLocation(ProgramName[program::FRAG], "Diffuse");
		}

		if(Validated)
		{
			glGenProgramPipelines(1, &PipelineName);
			glBindProgramPipeline(PipelineName);
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT, ProgramName[program::VERT]);
			glUseProgramStages(PipelineName, GL_FRAGMENT_SHADER_BIT, ProgramName[program::FRAG]);
		}

		return Validated && this->checkError("initProgram");
	}

	bool initVertexBuffer()
	{
		glGenBuffers(buffer::MAX, BufferName);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::F64]);
		glBufferData(GL_ARRAY_BUFFER, PositionSize, PositionData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return this->checkError("initArrayBuffer");
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);

		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::F64]);
			glVertexAttribLPointer(semantic::attr::POSITION, 3, GL_DOUBLE, sizeof(glm::dvec3), BUFFER_OFFSET(0));
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
			Validated = initVertexBuffer();
		if(Validated)
			Validated = initVertexArray();

		return Validated;
	}

	bool end()
	{
		glDeleteBuffers(buffer::MAX, BufferName);
		glDeleteVertexArrays(1, &VertexArrayName);
		glDeleteProgram(ProgramName[program::VERT]);
		glDeleteProgram(ProgramName[program::FRAG]);
		glBindProgramPipeline(0);
		glDeleteProgramPipelines(1, &PipelineName);

		return true;
	}

	bool render()
	{
		glm::dvec2 WindowSize(this->getWindowSize());

		glm::dmat4 Projection = glm::perspective(glm::pi<double>() * 0.25, WindowSize.x / WindowSize.y, 0.1, 100.0);
		glm::dmat4 View(this->view());
		glm::dmat4 Model = glm::dmat4(1.0f);
		glm::dmat4 MVP = Projection * View * Model;

		glProgramUniformMatrix4dv(ProgramName[program::VERT], UniformMVP, 1, GL_FALSE, &MVP[0][0]);
		glProgramUniform4dv(ProgramName[program::FRAG], UniformDiffuse, 1, &glm::dvec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

		glViewportIndexedfv(0, &glm::vec4(0, 0, WindowSize)[0]);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f)[0]);

		glBindProgramPipeline(PipelineName);

		glBindVertexArray(VertexArrayName);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, nullptr, 1, 0);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_410_program_64 Test(argc, argv);
	Error += Test();

	return Error;
}

