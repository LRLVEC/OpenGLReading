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
#include <glm/gtc/random.hpp>

namespace
{
	char const * VERT_SHADER_SOURCE("gl-320/primitive-point-quad.vert");
	char const * GEOM_SHADER_SOURCE("gl-320/primitive-point-quad.geom");
	char const * FRAG_SHADER_SOURCE("gl-320/primitive-point-quad.frag");
}//namespace

class gl_320_primitive_point_quad : public test
{
public:
	gl_320_primitive_point_quad(int argc, char* argv[]) :
		test(argc, argv, "gl-320-primitive-point-quad", test::CORE, 3, 2),
		VertexArrayName(0),
		ProgramName(0),
		BufferName(0),
		UniformMVP(-1),
		UniformMV(-1),
		UniformCameraPosition(-1)
	{}

private:
	GLsizei VertexCount;
	GLuint VertexArrayName;
	GLuint ProgramName;
	GLuint BufferName;
	GLint UniformMVP;
	GLint UniformMV;
	GLint UniformCameraPosition;

	bool initProgram()
	{
		bool Validated = true;
	
		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 150 --profile core");
			GLuint GeomShaderName = Compiler.create(GL_GEOMETRY_SHADER, getDataDirectory() + GEOM_SHADER_SOURCE, "--version 150 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 150 --profile core");

			ProgramName = glCreateProgram();
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, GeomShaderName);
			glAttachShader(ProgramName, FragShaderName);
			
			glBindAttribLocation(ProgramName, semantic::attr::POSITION, "Position");
			glBindAttribLocation(ProgramName, semantic::attr::COLOR, "Color");
			glBindFragDataLocation(ProgramName, semantic::frag::COLOR, "Color");
			glLinkProgram(ProgramName);

			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.checkProgram(ProgramName);
		}

		// Get variables locations
		if(Validated)
		{
			UniformMVP = glGetUniformLocation(ProgramName, "MVP");
			UniformMV = glGetUniformLocation(ProgramName, "MV");
			UniformCameraPosition = glGetUniformLocation(ProgramName, "CameraPosition");
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

		this->VertexCount = 5;

		// Reserve buffer memory but don't copy the values
		glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(sizeof(glf::vertex_v4fc4f) * this->VertexCount), nullptr, GL_STATIC_DRAW);

		glf::vertex_v4fc4f* Data = (glf::vertex_v4fc4f*)glMapBufferRange(GL_ARRAY_BUFFER, 
			0, // Offset
			GLsizeiptr(sizeof(glf::vertex_v4fc4f) * this->VertexCount), // Size,
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		Data[0].Position = glm::vec4(0.0, 0.0,-0.5, 1.0);
		Data[0].Color = glm::vec4(1.0, 0.0, 0.0, 1.0);
		Data[1].Position = glm::vec4(0.2, 0.0, 0.5, 1.0);
		Data[1].Color = glm::vec4(1.0, 0.5, 0.0, 1.0);
		Data[2].Position = glm::vec4(0.4, 0.0, 1.5, 1.0);
		Data[2].Color = glm::vec4(1.0, 1.0, 0.0, 1.0);
		Data[3].Position = glm::vec4(0.6, 0.0, 2.5, 1.0);
		Data[3].Color = glm::vec4(0.0, 1.0, 0.0, 1.0);
		Data[4].Position = glm::vec4(0.8, 0.0, 3.5, 1.0);
		Data[4].Color = glm::vec4(0.0, 0.0, 1.0, 1.0);

		glUnmapBuffer(GL_ARRAY_BUFFER);

		// Unbind the buffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return this->checkError("initArrayBuffer");
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName);
			glVertexAttribPointer(semantic::attr::POSITION, 4, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v4fc4f), BUFFER_OFFSET(0));
			glVertexAttribPointer(semantic::attr::COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v4fc4f), BUFFER_OFFSET(sizeof(glm::vec4)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
			glEnableVertexAttribArray(semantic::attr::COLOR);
		glBindVertexArray(0);

		return this->checkError("initVertexArray");
	}

	bool begin()
	{
		//caps Caps(caps::CORE);

		bool Validated = true;

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_PROGRAM_POINT_SIZE);
		//glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
		glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_UPPER_LEFT);

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
		glm::mat4 View = this->view();
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * View * Model;
		glm::mat4 MV = View * Model;

		float Depth(1.0f);
		glViewport(0, 0, WindowSize.x, WindowSize.y);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f)[0]);
		glClearBufferfv(GL_DEPTH, 0, &Depth);

		glDisable(GL_SCISSOR_TEST);

		glm::vec3 CameraPosition = -this->cameraPosition();
		//glm::vec3 CameraPosition(glm::vec4(glm::normalize(glm::vec3(1.0)), 1.0) * this->view());

		glUseProgram(ProgramName);
		glUniform3fv(UniformCameraPosition, 1, &CameraPosition[0]);
		glUniformMatrix4fv(UniformMV, 1, GL_FALSE, &MV[0][0]);
		glUniformMatrix4fv(UniformMVP, 1, GL_FALSE, &MVP[0][0]);

		glBindVertexArray(VertexArrayName);

		glDrawArraysInstanced(GL_POINTS, 0, this->VertexCount, 1);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_320_primitive_point_quad Test(argc, argv);
	Error += Test();

	return Error;
}

