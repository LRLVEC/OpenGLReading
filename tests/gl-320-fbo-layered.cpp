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
	char const * VERT_SHADER_SOURCE1("gl-320/fbo-layered.vert");
	char const * GEOM_SHADER_SOURCE1("gl-320/fbo-layered.geom");
	char const * FRAG_SHADER_SOURCE1("gl-320/fbo-layered.frag");
	char const * VERT_SHADER_SOURCE2("gl-320/fbo-layered-rtt-array.vert");
	char const * FRAG_SHADER_SOURCE2("gl-320/fbo-layered-rtt-array.frag");
	glm::ivec2 const FRAMEBUFFER_SIZE(320, 240);

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 1.0f))
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
			VERTEX,
			ELEMENT,
			MAX
		};
	}//namespace buffer

	namespace program
	{
		enum type
		{
			LAYERING,
			SPLASH,
			MAX
		};
	}//namespace program

	namespace shader
	{
		enum type
		{
			VERT1,
			GEOM1,
			FRAG1,
			VERT2,
			FRAG2,
			MAX
		};
	}//namespace shader

	GLuint FramebufferName(0);
	std::vector<GLuint> VertexArrayName(program::MAX);
	std::vector<GLuint> ProgramName(program::MAX);
	std::vector<GLuint> UniformMVP(program::MAX);
	GLint UniformDiffuse(0);
	GLint UniformLayer(0);

	std::vector<GLuint> BufferName(buffer::MAX);
	GLuint TextureColorbufferName(0);
	glm::ivec4 Viewport[4];
}//namespace

class gl_320_fbo_layered : public test
{
public:
	gl_320_fbo_layered(int argc, char* argv[]) :
		test(argc, argv, "gl-320-fbo-layered", test::CORE, 3, 2)
	{}

private:
	bool initProgram()
	{
		bool Validated = true;

		std::vector<GLuint> ShaderName(shader::MAX);
		compiler Compiler;

		if(Validated)
		{
			ShaderName[shader::VERT1] = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE1, "--version 150 --profile core");
			ShaderName[shader::GEOM1] = Compiler.create(GL_GEOMETRY_SHADER, getDataDirectory() + GEOM_SHADER_SOURCE1, "--version 150 --profile core");
			ShaderName[shader::FRAG1] = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE1, "--version 150 --profile core");

			ProgramName[program::LAYERING] = glCreateProgram();
			glAttachShader(ProgramName[program::LAYERING], ShaderName[shader::VERT1]);
			glAttachShader(ProgramName[program::LAYERING], ShaderName[shader::GEOM1]);
			glAttachShader(ProgramName[program::LAYERING], ShaderName[shader::FRAG1]);

			glBindAttribLocation(ProgramName[program::LAYERING], semantic::attr::POSITION, "Position");
			glBindFragDataLocation(ProgramName[program::LAYERING], semantic::frag::COLOR, "FragColor");
			glLinkProgram(ProgramName[program::LAYERING]);
		}

		if(Validated)
		{
			ShaderName[shader::VERT2] = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE2, "--version 150 --profile core");
			ShaderName[shader::FRAG2] = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE2, "--version 150 --profile core");

			ProgramName[program::SPLASH] = glCreateProgram();
			glAttachShader(ProgramName[program::SPLASH], ShaderName[shader::VERT2]);
			glAttachShader(ProgramName[program::SPLASH], ShaderName[shader::FRAG2]);

			glBindAttribLocation(ProgramName[program::SPLASH], semantic::attr::POSITION, "Position");
			glBindAttribLocation(ProgramName[program::SPLASH], semantic::attr::TEXCOORD, "Texcoord");
			glBindFragDataLocation(ProgramName[program::SPLASH], semantic::frag::COLOR, "Color");
			glLinkProgram(ProgramName[program::SPLASH]);
		}

		if(Validated)
		{
			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.checkProgram(ProgramName[program::LAYERING]);
			Validated = Validated && Compiler.checkProgram(ProgramName[program::SPLASH]);
		}

		if(Validated)
		{
			for(std::size_t i = 0; i < program::MAX; ++i)
				UniformMVP[i] = glGetUniformLocation(ProgramName[i], "MVP");

			UniformDiffuse = glGetUniformLocation(ProgramName[program::SPLASH], "Diffuse");
			UniformLayer = glGetUniformLocation(ProgramName[program::SPLASH], "Layer");
		}

		return Validated && this->checkError("initProgram");
	}

	bool initBuffer()
	{
		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return this->checkError("initBuffer");
	}

	bool initTexture()
	{
		glGenTextures(1, &TextureColorbufferName);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, TextureColorbufferName);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage3D(
			GL_TEXTURE_2D_ARRAY,
			0,
			GL_RGB8,
			GLsizei(FRAMEBUFFER_SIZE.x),
			GLsizei(FRAMEBUFFER_SIZE.y),
			GLsizei(4), //depth
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			NULL);

		return this->checkError("initTexture");
	}

	bool initFramebuffer()
	{
		glGenFramebuffers(1, &FramebufferName);
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureColorbufferName, 0);

		if(!this->checkFramebuffer(FramebufferName))
			return false;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return true;
	}

	bool initVertexArray()
	{
		glGenVertexArrays(program::MAX, &VertexArrayName[0]);

		glBindVertexArray(VertexArrayName[program::SPLASH]);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(0));
			glVertexAttribPointer(semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(sizeof(glm::vec2)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
			glEnableVertexAttribArray(semantic::attr::TEXCOORD);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBindVertexArray(0);

		glBindVertexArray(VertexArrayName[program::LAYERING]);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(0));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBindVertexArray(0);

		return this->checkError("initVertexArray");
	}

	bool begin()
	{
		glm::ivec2 WindowSize(this->getWindowSize());

		int Border = 2;
		Viewport[0] = glm::ivec4(Border, Border, FRAMEBUFFER_SIZE - 2 * Border);
		Viewport[1] = glm::ivec4((WindowSize.x >> 1) + Border, 1, FRAMEBUFFER_SIZE - 2 * Border);
		Viewport[2] = glm::ivec4((WindowSize.x >> 1) + Border, (WindowSize.y >> 1) + Border, FRAMEBUFFER_SIZE - 2 * Border);
		Viewport[3] = glm::ivec4(Border, (WindowSize.y >> 1) + Border, FRAMEBUFFER_SIZE - 2 * Border);

		bool Validated(true);

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initFramebuffer();

		return Validated && this->checkError("begin");
	}

	bool end()
	{
		for(std::size_t i = 0; i < program::MAX; ++i)
			glDeleteProgram(ProgramName[i]);
		glDeleteVertexArrays(program::MAX, &VertexArrayName[0]);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteTextures(1, &TextureColorbufferName);
		glDeleteFramebuffers(1, &FramebufferName);

		return this->checkError("end");
	}

	bool render()
	{
		glm::ivec2 WindowSize(this->getWindowSize());

		glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, 1.0f,-1.0f, 1.0f, -1.0f);
		glm::mat4 View = glm::mat4(1.0f);
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * View * Model;

		// Pass 1
		{
			glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
			glViewport(0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y);

			glUseProgram(ProgramName[program::LAYERING]);
			glUniformMatrix4fv(UniformMVP[program::LAYERING], 1, GL_FALSE, &MVP[0][0]);

			glBindVertexArray(VertexArrayName[program::LAYERING]);
			glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, NULL, 1, 0);
		}

		// Pass 2
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)[0]);
			
			glUseProgram(ProgramName[program::SPLASH]);
			glUniformMatrix4fv(UniformMVP[program::SPLASH], 1, GL_FALSE, &MVP[0][0]);
			glUniform1i(UniformDiffuse, 0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, TextureColorbufferName);

			glBindVertexArray(VertexArrayName[program::SPLASH]);

			for(int i = 0; i < 4; ++i)
			{
				glUniform1i(UniformLayer, i);
				glViewport(Viewport[i].x, Viewport[i].y, Viewport[i].z, Viewport[i].w);
				glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, NULL, 1, 0);
			}
		}

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_320_fbo_layered Test(argc, argv);
	Error += Test();

	return Error;
}

