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
	char const * VERT_SHADER_SOURCE("gl-320/texture-buffer.vert");
	char const * FRAG_SHADER_SOURCE("gl-320/texture-buffer.frag");

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
			DISPLACEMENT,
			DIFFUSE,
			MAX
		};
	}//namespace buffer

	GLuint VertexArrayName = 0;
	GLuint ProgramName = 0;
	GLuint BufferName[buffer::MAX] = {0, 0, 0};
	GLuint DisplacementTextureName = 0;
	GLuint DiffuseTextureName = 0;
	GLint UniformMVP = 0;
	GLint UniformDiffuse = 0;
	GLint UniformDisplacement = 0;
}//namespace

class gl_320_texture_buffer : public test
{
public:
	gl_320_texture_buffer(int argc, char* argv[]) :
		test(argc, argv, "gl-320-texture-buffer", test::CORE, 3, 2, glm::vec2(glm::pi<float>() * 0.2f))
	{}

private:
	bool initTest()
	{
		bool Validated = true;
		glEnable(GL_DEPTH_TEST);

		return Validated && this->checkError("initTest");
	}

	bool initProgram()
	{
		bool Validated = true;
	
		// Create program
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
			Validated = Validated && Compiler.checkProgram(ProgramName);
		}

		// Get variables locations
		if(Validated)
		{
			UniformMVP = glGetUniformLocation(ProgramName, "MVP");
			UniformDiffuse = glGetUniformLocation(ProgramName, "Diffuse");
			UniformDisplacement = glGetUniformLocation(ProgramName, "Displacement");
		}

		return Validated && this->checkError("initProgram");
	}

	bool initBuffer()
	{
		glGenBuffers(buffer::MAX, BufferName);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, PositionSize, PositionData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glm::vec4 Position[5] = 
		{
			glm::vec4( 0.1f, 0.3f,-1.0f, 1.0f), 
			glm::vec4(-0.5f, 0.0f,-0.5f, 1.0f),
			glm::vec4(-0.2f,-0.2f, 0.0f, 1.0f),
			glm::vec4( 0.3f, 0.2f, 0.5f, 1.0f),
			glm::vec4( 0.1f,-0.3f, 1.0f, 1.0f)
		};

		glBindBuffer(GL_TEXTURE_BUFFER, BufferName[buffer::DISPLACEMENT]);
		glBufferData(GL_TEXTURE_BUFFER, sizeof(Position), Position, GL_STATIC_DRAW);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		glm::u8vec4 Diffuse[5] = 
		{
			glm::u8vec4(255,   0,   0, 255),
			glm::u8vec4(255, 127,   0, 255),
			glm::u8vec4(255, 255,   0, 255),
			glm::u8vec4(  0, 255,   0, 255),
			glm::u8vec4(  0,   0, 255, 255)
		};	

		GLint MaxTextureBufferSize(0);
		glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &MaxTextureBufferSize);

		glBindBuffer(GL_TEXTURE_BUFFER, BufferName[buffer::DIFFUSE]);
		glBufferData(GL_TEXTURE_BUFFER, 500000, NULL, GL_STATIC_DRAW);
		//glBufferData(GL_TEXTURE_BUFFER, sizeof(Diffuse), Diffuse, GL_STATIC_DRAW);
		glBufferSubData(GL_TEXTURE_BUFFER, 0, sizeof(Diffuse), Diffuse);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		return this->checkError("initBuffer");
	}

	bool initTexture()
	{
		glGenTextures(1, &DisplacementTextureName);
		glBindTexture(GL_TEXTURE_BUFFER, DisplacementTextureName);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, BufferName[buffer::DISPLACEMENT]);
		glBindTexture(GL_TEXTURE_BUFFER, 0);

		glGenTextures(1, &DiffuseTextureName);
		glBindTexture(GL_TEXTURE_BUFFER, DiffuseTextureName);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8, BufferName[buffer::DIFFUSE]);
		glBindTexture(GL_TEXTURE_BUFFER, 0);	

		return this->checkError("initTexture");
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

		if(Validated)
			Validated = initTest();
		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initVertexArray();

		return Validated && this->checkError("begin");
	}

	bool end()
	{
		glDeleteTextures(1, &DiffuseTextureName);
		glDeleteTextures(1, &DisplacementTextureName);
		glDeleteBuffers(buffer::MAX, BufferName);
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

		// Set the display viewport
		glViewport(0, 0, WindowSize.x, WindowSize.y);

		// Clear color buffer with black
		float Depth(1.0f);
		glClearBufferfv(GL_DEPTH, 0, &Depth);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

		// Bind program
		glUseProgram(ProgramName);
		glUniformMatrix4fv(UniformMVP, 1, GL_FALSE, &MVP[0][0]);
		glUniform1i(UniformDisplacement, 0);
		glUniform1i(UniformDiffuse, 1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER, DisplacementTextureName);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, DiffuseTextureName);

		glBindVertexArray(VertexArrayName);
		glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 5);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_320_texture_buffer Test(argc, argv);
	Error += Test();

	return Error;
}

