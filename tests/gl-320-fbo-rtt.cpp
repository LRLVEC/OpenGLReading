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
	char const * VERT_SHADER_SOURCE1("gl-320/fbo-rtt-multiple-output.vert");
	char const * FRAG_SHADER_SOURCE1("gl-320/fbo-rtt-multiple-output.frag");
	char const * VERT_SHADER_SOURCE2("gl-320/fbo-rtt-single-output.vert");
	char const * FRAG_SHADER_SOURCE2("gl-320/fbo-rtt-single-output.frag");
	char const * TEXTURE_DIFFUSE("kueken1-bgr8.dds");
	glm::ivec2 const FRAMEBUFFER_SIZE(320, 240);

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-4.0f,-3.0f), glm::vec2(0.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 4.0f,-3.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 4.0f, 3.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2(-4.0f, 3.0f), glm::vec2(0.0f, 1.0f))
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

	namespace texture
	{
		enum type
		{
			TEXTURE_R,
			TEXTURE_G,
			TEXTURE_B,
			MAX
		};
	}//namespace texture

	namespace program
	{
		enum type
		{
			SINGLE,
			MULTIPLE,
			MAX
		};
	}//namespace buffer

	namespace shader
	{
		enum type
		{
			VERT1,
			FRAG1,
			VERT2,
			FRAG2,
			MAX
		};
	}//namespace shader

	GLuint FramebufferName(0);
	std::vector<GLuint> ProgramName(program::MAX);
	std::vector<GLuint> UniformMVP(program::MAX);
	std::vector<GLuint> UniformDiffuse(program::MAX);
	std::vector<GLuint> VertexArrayName(program::MAX);
	std::vector<GLuint> BufferName(buffer::MAX);
	std::vector<GLuint> TextureName(texture::MAX);
	std::vector<glm::ivec4> Viewport(texture::MAX);
}//namespace

class gl_320_fbo_rtt : public test
{
public:
	gl_320_fbo_rtt(int argc, char* argv[]) :
		test(argc, argv, "gl-320-fbo-rtt", test::CORE, 3, 2)
	{}

private:
	bool initProgram()
	{
		bool Validated = true;

		compiler Compiler;

		if(Validated)
		{
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE1, "--version 150 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE1, "--version 150 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName[program::MULTIPLE] = glCreateProgram();
			glAttachShader(ProgramName[program::MULTIPLE], VertShaderName);
			glAttachShader(ProgramName[program::MULTIPLE], FragShaderName);

			glBindAttribLocation(ProgramName[program::MULTIPLE], semantic::attr::POSITION, "Position");
			glBindFragDataLocation(ProgramName[program::MULTIPLE], semantic::frag::RED, "Red");
			glBindFragDataLocation(ProgramName[program::MULTIPLE], semantic::frag::GREEN, "Green");
			glBindFragDataLocation(ProgramName[program::MULTIPLE], semantic::frag::BLUE, "Blue");
			glLinkProgram(ProgramName[program::MULTIPLE]);
			Validated = Validated && Compiler.checkProgram(ProgramName[program::MULTIPLE]);
		}

		if(Validated)
		{
			UniformMVP[program::MULTIPLE] = glGetUniformLocation(ProgramName[program::MULTIPLE], "MVP");
		}

		if(Validated)
		{
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE2, "--version 150 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE2, "--version 150 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName[program::SINGLE] = glCreateProgram();
			glAttachShader(ProgramName[program::SINGLE], VertShaderName);
			glAttachShader(ProgramName[program::SINGLE], FragShaderName);
			
			glBindAttribLocation(ProgramName[program::SINGLE], semantic::attr::POSITION, "Position");
			glBindAttribLocation(ProgramName[program::SINGLE], semantic::attr::TEXCOORD, "Texcoord");
			glBindFragDataLocation(ProgramName[program::SINGLE], semantic::frag::COLOR, "Color");
			glLinkProgram(ProgramName[program::SINGLE]);
			Validated = Validated && Compiler.checkProgram(ProgramName[program::SINGLE]);
		}

		if(Validated)
		{
			UniformMVP[program::SINGLE] = glGetUniformLocation(ProgramName[program::SINGLE], "MVP");
			UniformDiffuse[program::SINGLE] = glGetUniformLocation(ProgramName[program::SINGLE], "Diffuse");
		}

		return this->checkError("initProgram");
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
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(texture::MAX, &TextureName[0]);

		for(int i = texture::TEXTURE_R; i <= texture::TEXTURE_B; ++i)
		{
			glBindTexture(GL_TEXTURE_2D, TextureName[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTexImage2D(
				GL_TEXTURE_2D,
				0, 
				GL_RGB8,
				GLsizei(FRAMEBUFFER_SIZE.x),
				GLsizei(FRAMEBUFFER_SIZE.y),
				0,
				GL_BGR,
				GL_UNSIGNED_BYTE,
				0);
		}

		return this->checkError("initTexture");
	}

	bool initFramebuffer()
	{
		glGenFramebuffers(1, &FramebufferName);
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureName[0], 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, TextureName[1], 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, TextureName[2], 0);
		GLenum DrawBuffers[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
		glDrawBuffers(3, DrawBuffers);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLenum const Buffers = GL_BACK;
		glDrawBuffers(1, &Buffers);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;

		return this->checkError("initFramebuffer");
	}

	bool initVertexArray()
	{
		glGenVertexArrays(program::MAX, &VertexArrayName[0]);

		glBindVertexArray(VertexArrayName[program::MULTIPLE]);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(0));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBindVertexArray(0);

		glBindVertexArray(VertexArrayName[program::SINGLE]);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(0));
			glVertexAttribPointer(semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(sizeof(glm::vec2)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
			glEnableVertexAttribArray(semantic::attr::TEXCOORD);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBindVertexArray(0);

		return this->checkError("initVertexArray");
	}

	bool begin()
	{
		glm::ivec2 WindowSize(this->getWindowSize());
		Viewport[texture::TEXTURE_R] = glm::ivec4(WindowSize.x >> 1, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y);
		Viewport[texture::TEXTURE_G] = glm::ivec4(WindowSize.x >> 1, WindowSize.y >> 1, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y);
		Viewport[texture::TEXTURE_B] = glm::ivec4(0, WindowSize.y >> 1, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y);

		bool Validated = true;

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
		
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteTextures(texture::MAX, &TextureName[0]);
		glDeleteFramebuffers(1, &FramebufferName);

		return this->checkError("end");
	}

	bool render()
	{
		glm::ivec2 WindowSize(this->getWindowSize());

		GLint const Border = 16;

		// Pass 1
		{
			// Compute the MVP (Model View Projection matrix)
			glm::mat4 Projection = glm::ortho(-1.0f, 1.0f,-1.0f, 1.0f, -1.0f, 1.0f);
			glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
			glm::mat4 View = ViewTranslate;
			glm::mat4 Model = glm::mat4(1.0f);
			glm::mat4 MVP = Projection * View * Model;

			glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
			glViewport(0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y);
			glScissor(Border, Border, FRAMEBUFFER_SIZE.x - Border * 2, FRAMEBUFFER_SIZE.y - Border * 2);
			glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

			glUseProgram(ProgramName[program::MULTIPLE]);
			glUniformMatrix4fv(UniformMVP[program::MULTIPLE], 1, GL_FALSE, &MVP[0][0]);

			glBindVertexArray(VertexArrayName[program::MULTIPLE]);
			glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, NULL, 1, 0);
		}

		// Pass 2
		{
			glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, 1.0f,-1.0f, -1.0f, 1.0f);
			glm::mat4 View = glm::mat4(1.0f);
			glm::mat4 Model = glm::mat4(1.0f);
			glm::mat4 MVP = Projection * View * Model;

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, WindowSize.x, WindowSize.y);
			glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

			glUseProgram(ProgramName[program::SINGLE]);
			glUniformMatrix4fv(UniformMVP[program::SINGLE], 1, GL_FALSE, &MVP[0][0]);
			glUniform1i(UniformDiffuse[program::SINGLE], 0);
		}

		for(std::size_t i = 0; i < texture::MAX; ++i)
		{
			glViewport(Viewport[i].x, Viewport[i].y, Viewport[i].z, Viewport[i].w);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, TextureName[i]);

			glBindVertexArray(VertexArrayName[program::SINGLE]);
			glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, NULL, 1, 0);
		}

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_320_fbo_rtt Test(argc, argv);
	Error += Test();

	return Error;
}

