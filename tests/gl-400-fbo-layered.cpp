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
	char const * VERT_SHADER_SOURCE1("gl-400/layer.vert");
	char const * GEOM_SHADER_SOURCE1("gl-400/layer.geom");
	char const * FRAG_SHADER_SOURCE1("gl-400/layer.frag");
	char const * VERT_SHADER_SOURCE2("gl-400/rtt-array.vert");
	char const * FRAG_SHADER_SOURCE2("gl-400/rtt-array.frag");
	int const FRAMEBUFFER_SIZE(2);

	GLsizei const VertexCount = 4;
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 1.0f))
	};

	GLsizei const ElementCount = 6;
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2, 
		2, 3, 0
	};

	enum buffer_type
	{
		BUFFER_VERTEX,
		BUFFER_ELEMENT,
		BUFFER_MAX
	};

	enum program 
	{
		LAYERING,
		IMAGE_2D,
		PROGRAM_MAX
	};

	GLuint FramebufferName = 0;
	GLuint VertexArrayName[PROGRAM_MAX];
	GLuint ProgramName[PROGRAM_MAX];
	GLint UniformMVP[PROGRAM_MAX];
	GLint UniformDiffuse = 0;
	GLint UniformLayer = 0;
	GLuint BufferName[BUFFER_MAX];
	GLuint TextureColorbufferName = 0;
	GLuint SamplerName = 0;
	glm::ivec4 Viewport[4];
}//namespace

class gl_400_fbo_layered : public test
{
public:
	gl_400_fbo_layered(int argc, char* argv[]) :
		test(argc, argv, "gl-400-fbo-layered", test::CORE, 4, 0)
	{}

private:
	bool initProgram()
	{
		bool Validated = true;

		compiler Compiler;

		if(Validated)
		{
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE1);
			GLuint GeomShaderName = Compiler.create(GL_GEOMETRY_SHADER, getDataDirectory() + GEOM_SHADER_SOURCE1);
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE1);
		
			ProgramName[LAYERING] = glCreateProgram();
			glAttachShader(ProgramName[LAYERING], VertShaderName);
			glAttachShader(ProgramName[LAYERING], GeomShaderName);
			glAttachShader(ProgramName[LAYERING], FragShaderName);
			glLinkProgram(ProgramName[LAYERING]);
		}

		if(Validated)
		{
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE2);
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE2);

			ProgramName[IMAGE_2D] = glCreateProgram();
			glAttachShader(ProgramName[IMAGE_2D], VertShaderName);
			glAttachShader(ProgramName[IMAGE_2D], FragShaderName);
			glLinkProgram(ProgramName[IMAGE_2D]);
		}

		if(Validated)
		{
			Validated = Validated && Compiler.checkProgram(ProgramName[LAYERING]);
			Validated = Validated && Compiler.checkProgram(ProgramName[IMAGE_2D]);
		}

		if(Validated)
		{
			for(std::size_t i = 0; i < PROGRAM_MAX; ++i)
				UniformMVP[i] = glGetUniformLocation(ProgramName[i], "MVP");

			UniformDiffuse = glGetUniformLocation(ProgramName[IMAGE_2D], "Diffuse");
			UniformLayer = glGetUniformLocation(ProgramName[IMAGE_2D], "Layer");
		}

		return Validated && this->checkError("initProgram");
	}

	bool initBuffer()
	{
		glGenBuffers(BUFFER_MAX, BufferName);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[BUFFER_ELEMENT]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[BUFFER_VERTEX]);
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
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);

		glm::ivec2 WindowSize(this->getWindowSize());

		glTexImage3D(
			GL_TEXTURE_2D_ARRAY, 
			0, 
			GL_RGB, 
			GLsizei(WindowSize.x / FRAMEBUFFER_SIZE),
			GLsizei(WindowSize.y / FRAMEBUFFER_SIZE),
			GLsizei(4), //depth
			0,  
			GL_RGB, 
			GL_UNSIGNED_BYTE, 
			NULL);

		return this->checkError("initTexture");
	}

	bool initFramebuffer()
	{
		GLenum const Buffers = GL_COLOR_ATTACHMENT0;
		glGenFramebuffers(1, &FramebufferName);
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureColorbufferName, 0);
		glDrawBuffers(1, &Buffers);
		if(!this->checkFramebuffer(FramebufferName))
			return false;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDrawBuffer(GL_BACK);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;

		return this->checkError("initFramebuffer");
	}

	bool initVertexArray()
	{
		glGenVertexArrays(PROGRAM_MAX, VertexArrayName);

		glBindVertexArray(VertexArrayName[IMAGE_2D]);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[BUFFER_VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(0));
			glVertexAttribPointer(semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(sizeof(glm::vec2)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
			glEnableVertexAttribArray(semantic::attr::TEXCOORD);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[BUFFER_ELEMENT]);
		glBindVertexArray(0);

		glBindVertexArray(VertexArrayName[LAYERING]);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[BUFFER_VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(0));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[BUFFER_ELEMENT]);
		glBindVertexArray(0);

		return this->checkError("initVertexArray");
	}

	bool initSampler()
	{
		glGenSamplers(1, &SamplerName);
		glSamplerParameteri(SamplerName, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri(SamplerName, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(SamplerName, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glSamplerParameterfv(SamplerName, GL_TEXTURE_BORDER_COLOR, &glm::vec4(0.0f)[0]);
		glSamplerParameterf(SamplerName, GL_TEXTURE_MIN_LOD, -1000.f);
		glSamplerParameterf(SamplerName, GL_TEXTURE_MAX_LOD, 1000.f);
		glSamplerParameterf(SamplerName, GL_TEXTURE_LOD_BIAS, 0.0f);
		glSamplerParameteri(SamplerName, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glSamplerParameteri(SamplerName, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

		return this->checkError("initSampler");
	}

	bool begin()
	{
		glm::ivec2 WindowSize(this->getWindowSize());
		glm::ivec2 FramebufferSize = WindowSize / FRAMEBUFFER_SIZE;
	
		int Border = 2;
		Viewport[0] = glm::ivec4(Border, Border, FramebufferSize - 2 * Border);
		Viewport[1] = glm::ivec4((WindowSize.x >> 1) + Border, 1, FramebufferSize - 2 * Border);
		Viewport[2] = glm::ivec4((WindowSize.x >> 1) + Border, (WindowSize.y >> 1) + Border, FramebufferSize - 2 * Border);
		Viewport[3] = glm::ivec4(Border, (WindowSize.y >> 1) + Border, FramebufferSize - 2 * Border);

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
		if(Validated)
			Validated = initSampler();

		return Validated && this->checkError("begin");
	}

	bool end()
	{
		glDeleteVertexArrays(PROGRAM_MAX, VertexArrayName);
		glDeleteBuffers(BUFFER_MAX, BufferName);
		glDeleteTextures(1, &TextureColorbufferName);
		glDeleteFramebuffers(1, &FramebufferName);
		glDeleteProgram(ProgramName[IMAGE_2D]);
		glDeleteProgram(ProgramName[LAYERING]);

		return this->checkError("end");
	}

	bool render()
	{
		glm::ivec2 WindowSize(this->getWindowSize());

		glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, 1.0f,-1.0f, 1.0f, -1.0f);
		glm::mat4 View = glm::mat4(1.0f);
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * View * Model;

		glViewport(0, 0, WindowSize.x, WindowSize.y);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0)[0]);
	
		// Pass 1
		{
			glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
			glViewport(0, 0, WindowSize.x / FRAMEBUFFER_SIZE, WindowSize.y / FRAMEBUFFER_SIZE);

			glUseProgram(ProgramName[LAYERING]);
			glUniformMatrix4fv(UniformMVP[LAYERING], 1, GL_FALSE, &MVP[0][0]);

			glBindVertexArray(VertexArrayName[LAYERING]);
			glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, NULL, 1, 0);
		}

		// Pass 2
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glUseProgram(ProgramName[IMAGE_2D]);
			glUniformMatrix4fv(UniformMVP[IMAGE_2D], 1, GL_FALSE, &MVP[0][0]);
			glUniform1i(UniformDiffuse, 0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, TextureColorbufferName);
			glBindSampler(0, SamplerName);

			glBindVertexArray(VertexArrayName[IMAGE_2D]);

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

	gl_400_fbo_layered Test(argc, argv);
	Error += Test();

	return Error;
}

