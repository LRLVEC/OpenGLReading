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
	char const * VERT_SHADER_SOURCE1("gl-410/layer.vert");
	char const * GEOM_SHADER_SOURCE1("gl-410/layer.geom");
	char const * FRAG_SHADER_SOURCE1("gl-410/layer.frag");
	char const * VERT_SHADER_SOURCE2("gl-410/viewport.vert");
	char const * GEOM_SHADER_SOURCE2("gl-410/viewport.geom");
	char const * FRAG_SHADER_SOURCE2("gl-410/viewport.frag");
	glm::ivec2 const FRAMEBUFFER_SIZE(640, 480);

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

	enum buffer_type
	{
		BUFFER_VERTEX,
		BUFFER_ELEMENT,
		BUFFER_MAX
	};

	enum program 
	{
		LAYERING,
		VIEWPORT,
		PROGRAM_MAX
	};

	GLuint FramebufferName(0);
	GLuint VertexArrayName[PROGRAM_MAX] = {0, 0};
	GLuint ProgramName[PROGRAM_MAX] = {0, 0};
	GLint UniformMVP[PROGRAM_MAX] = {0, 0};
	GLint UniformDiffuse(0);
	GLuint SamplerName(0);
	GLuint BufferName[BUFFER_MAX] = {0, 0};
	GLuint TextureColorbufferName(0);
}//namespace

class gl_410_fbo_layered : public test
{
public:
	gl_410_fbo_layered(int argc, char* argv[]) :
		test(argc, argv, "gl-410-fbo-layered", test::CORE, 4, 1)
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
			GLuint GeomShaderName = Compiler.create(GL_GEOMETRY_SHADER, getDataDirectory() + GEOM_SHADER_SOURCE2);
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE2);

			ProgramName[VIEWPORT] = glCreateProgram();
			glAttachShader(ProgramName[VIEWPORT], VertShaderName);
			glAttachShader(ProgramName[VIEWPORT], GeomShaderName);
			glAttachShader(ProgramName[VIEWPORT], FragShaderName);
			glLinkProgram(ProgramName[VIEWPORT]);
		}

		if(Validated)
		{
			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.checkProgram(ProgramName[LAYERING]);
			Validated = Validated && Compiler.checkProgram(ProgramName[VIEWPORT]);
		}

		if(Validated)
		{
			for(std::size_t i = 0; i < PROGRAM_MAX; ++i)
				UniformMVP[i] = glGetUniformLocation(ProgramName[i], "MVP");
			UniformDiffuse = glGetUniformLocation(ProgramName[VIEWPORT], "Diffuse");
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
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 1000);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);

		glTexImage3D(
			GL_TEXTURE_2D_ARRAY, 
			0, 
			GL_RGB, 
			GLsizei(FRAMEBUFFER_SIZE.x), 
			GLsizei(FRAMEBUFFER_SIZE.y), 
			GLsizei(4), //depth
			0,  
			GL_RGB, 
			GL_UNSIGNED_BYTE, 
			NULL);

		return this->checkError("initTexture");
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

	bool initFramebuffer()
	{
		GLenum const BuffersRender = GL_COLOR_ATTACHMENT0;
		glGenFramebuffers(1, &FramebufferName);
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureColorbufferName, 0);
		glDrawBuffers(1, &BuffersRender);
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

		glBindVertexArray(VertexArrayName[VIEWPORT]);
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
			Validated = initTexture();
		if(Validated)
			Validated = initFramebuffer();
		if(Validated)
			Validated = initSampler();
	
		return Validated;
	}

	bool end()
	{
		glDeleteVertexArrays(PROGRAM_MAX, VertexArrayName);
		glDeleteBuffers(BUFFER_MAX, BufferName);
		glDeleteTextures(1, &TextureColorbufferName);
		glDeleteFramebuffers(1, &FramebufferName);
		glDeleteProgram(ProgramName[VIEWPORT]);
		glDeleteProgram(ProgramName[LAYERING]);
		glDeleteSamplers(1, &SamplerName);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, 1.0f,-1.0f, 1.0f, -1.0f);
		glm::mat4 View = glm::mat4(1.0f);
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * View * Model;

		glProgramUniformMatrix4fv(ProgramName[LAYERING], UniformMVP[LAYERING], 1, GL_FALSE, &MVP[0][0]);
		glProgramUniformMatrix4fv(ProgramName[VIEWPORT], UniformMVP[VIEWPORT], 1, GL_FALSE, &MVP[0][0]);
		glProgramUniform1i(ProgramName[VIEWPORT], UniformDiffuse, 0);

		// Pass 1
		{
			glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
			glViewportIndexedfv(0, &glm::vec4(0, 0, FRAMEBUFFER_SIZE)[0]);

			glUseProgram(ProgramName[LAYERING]);

			glBindVertexArray(VertexArrayName[LAYERING]);
			glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, NULL, 1, 0);
		}

		// Pass 2
		{
			GLint Border = 2;

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
			glClearBufferfv(GL_COLOR, 0, &glm::vec4(0)[0]);
		
			glViewportIndexedfv(0, &glm::vec4(Border, Border, WindowSize * 0.5f - 2.0f * Border)[0]);
			glViewportIndexedfv(1, &glm::vec4((WindowSize.x * 0.5f) + Border, Border, WindowSize * 0.5f - 2.0f * Border)[0]);
			glViewportIndexedfv(2, &glm::vec4((WindowSize.x * 0.5f) + Border, (WindowSize.y * 0.5f) + 1, WindowSize * 0.5f - 2.0f * Border)[0]);
			glViewportIndexedfv(3, &glm::vec4(Border, (WindowSize.y * 0.5f) + Border, WindowSize * 0.5f - 2.0f * Border)[0]);

			glUseProgram(ProgramName[VIEWPORT]);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, TextureColorbufferName);
			glBindSampler(0, SamplerName);

			glBindVertexArray(VertexArrayName[VIEWPORT]);
			glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, nullptr, 1, 0);
		}

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_410_fbo_layered Test(argc, argv);
	Error += Test();

	return Error;
}

