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
	char const * TEXTURE_DIFFUSE("kueken3-bgr8.dds");
	glm::ivec2 const FRAMEBUFFER_SIZE(160, 120);

	// With DDS textures, v texture coordinate are reversed, from top to bottom
	GLsizei const VertexCount(6);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-4.0f,-3.0f) * 0.3f, glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 4.0f,-3.0f) * 0.3f, glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 4.0f, 3.0f) * 0.3f, glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 4.0f, 3.0f) * 0.3f, glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-4.0f, 3.0f) * 0.3f, glm::vec2(0.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-4.0f,-3.0f) * 0.3f, glm::vec2(0.0f, 1.0f))
	};

	namespace program
	{
		enum type
		{
			THROUGH,
			RESOLVE_BOX,
			RESOLVE_NEAR,
			MAX
		};
	}//namespace program

	namespace renderbuffer
	{
		enum type
		{
			DEPTH,
			COLOR,
			MAX
		};
	}//namespace renderbuffer

	namespace texture
	{
		enum type
		{
			DIFFUSE,
			COLOR,
			MAX
		};
	}//namespace texture

	char const * VERT_SHADER_SOURCE("gl-330/multisample-explicit-texture-nv.vert");
	char const * FRAG_SHADER_SOURCE[program::MAX] = 
	{
		"gl-330/multisample-explicit-texture-nv.frag",
		"gl-330/multisample-explicit-box-nv.frag",
		"gl-330/multisample-explicit-near-nv.frag",
	};

	namespace shader
	{
		enum type
		{
			VERT,
			FRAG_THROUGH,
			FRAG_RESOLVE_BOX,
			FRAG_RESOLVE_NEAR,
			MAX
		};
	}//namespace shader

	GLuint VertexArrayName(0);
	GLuint ProgramName[program::MAX];
	GLuint BufferName(0);
	GLuint SamplerName(0);
	GLuint TextureName[texture::MAX] = {0, 0};
	GLuint RenderbufferName[renderbuffer::MAX] = {0, 0};
	GLuint FramebufferName(0);
	GLint UniformMVP[program::MAX];
	GLint UniformDiffuse[program::MAX];
}//namespace

class gl_330_fbo_multisample_explicit_nv : public test
{
public:
	gl_330_fbo_multisample_explicit_nv(int argc, char* argv[]) :
		test(argc, argv, "gl-330-fbo-multisample-explicit-nv", test::CORE, 3, 3, glm::vec2(glm::pi<float>() * 0.2f))
	{}

private:
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

	bool initProgram()
	{
		bool Validated = true;

		std::array<GLuint, shader::MAX> ShaderName;

		compiler Compiler;
		ShaderName[shader::VERT] = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 330 --profile core");
		for(int i = 0; i < program::MAX; ++i)
			ShaderName[shader::FRAG_THROUGH + i] = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE[i], "--version 330 --profile core");
		Validated = Validated && Compiler.check();

		for(int i = 0; i < program::MAX; ++i)
		{
			ProgramName[i] = glCreateProgram();
			glAttachShader(ProgramName[i], ShaderName[shader::VERT]);
			glAttachShader(ProgramName[i], ShaderName[shader::FRAG_THROUGH + i]);
			glLinkProgram(ProgramName[i]);
			Validated = Validated && Compiler.checkProgram(ProgramName[i]);

			UniformMVP[i] = glGetUniformLocation(ProgramName[i], "MVP");
			UniformDiffuse[i] = glGetUniformLocation(ProgramName[i], "Diffuse");
		}

		return Validated && this->checkError("initProgram");
	}

	bool initBuffer()
	{
		glGenBuffers(1, &BufferName);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName);
		glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return this->checkError("initBuffer");;
	}

	bool initTexture()
	{
		glGenTextures(texture::MAX, TextureName);

		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::DIFFUSE]);
		for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexImage2D(
				GL_TEXTURE_2D, 
				GLint(Level), 
				GL_RGB8, 
				GLsizei(Texture[Level].dimensions().x), 
				GLsizei(Texture[Level].dimensions().y), 
				0,
				GL_BGR, 
				GL_UNSIGNED_BYTE, 
				Texture[Level].data());
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindTexture(GL_TEXTURE_RENDERBUFFER_NV, TextureName[texture::COLOR]);
		glTexRenderbufferNV(GL_TEXTURE_RENDERBUFFER_NV, RenderbufferName[renderbuffer::COLOR]);
		glBindTexture(GL_TEXTURE_RENDERBUFFER_NV, 0);

		return this->checkError("initTexture");
	}

	bool initRenderbuffer()
	{
		glGenRenderbuffers(renderbuffer::MAX, RenderbufferName);
		glBindRenderbuffer(GL_RENDERBUFFER, RenderbufferName[renderbuffer::COLOR]);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y);
		glBindRenderbuffer(GL_RENDERBUFFER, RenderbufferName[renderbuffer::DEPTH]);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT24, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		return this->checkError("initRenderbuffer");
	}

	bool initFramebuffer()
	{
		glGenFramebuffers(1, &FramebufferName);
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, RenderbufferName[renderbuffer::COLOR]);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RenderbufferName[renderbuffer::DEPTH]);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return this->checkError("initFramebuffer");
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(0));
			glVertexAttribPointer(semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(sizeof(glm::vec2)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
			glEnableVertexAttribArray(semantic::attr::TEXCOORD);
		glBindVertexArray(0);

		return this->checkError("initVertexArray");
	}

	bool begin()
	{
		bool Validated = true;
		Validated = Validated && this->checkExtension("GL_NV_explicit_multisample");

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initSampler();
		if(Validated)
			Validated = initRenderbuffer();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initFramebuffer();

		return Validated && this->checkError("begin");
	}

	bool end()
	{
		glDeleteBuffers(1, &BufferName);
		for(int i = 0; i < program::MAX; ++i)
			glDeleteProgram(ProgramName[i]);
		glDeleteSamplers(1, &SamplerName);
		glDeleteTextures(renderbuffer::MAX, RenderbufferName);
		glDeleteTextures(texture::MAX, TextureName);
		glDeleteFramebuffers(1, &FramebufferName);
		glDeleteVertexArrays(1, &VertexArrayName);

		return this->checkError("end");
	}

	void renderFBO()
	{
		glm::mat4 Perspective = glm::perspective(glm::pi<float>() * 0.25f, float(FRAMEBUFFER_SIZE.x) / FRAMEBUFFER_SIZE.y, 0.1f, 100.0f);
		glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(1, -1, 1));
		glm::mat4 MVP = Perspective * this->view() * Model;

		glEnable(GL_DEPTH_TEST);

		glUseProgram(ProgramName[program::THROUGH]);
		glUniform1i(UniformDiffuse[program::THROUGH], 0);
		glUniformMatrix4fv(UniformMVP[program::THROUGH], 1, GL_FALSE, &MVP[0][0]);

		glViewport(0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y);

		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		float Depth(1.0f);
		glClearBufferfv(GL_DEPTH, 0, &Depth);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::DIFFUSE]);
		glBindSampler(0, SamplerName);
		glBindVertexArray(VertexArrayName);

		glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 5);

		glDisable(GL_DEPTH_TEST);

		this->checkError("renderFBO");
	}

	void resolveMultisampling()
	{
		glm::ivec2 WindowSize(this->getWindowSize());

		glm::mat4 Perspective = glm::perspective(glm::pi<float>() * 0.25f, float(WindowSize.x) / WindowSize.y, 0.1f, 100.0f);
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Perspective * this->view() * Model;

		glViewport(0, 0, WindowSize.x, WindowSize.y);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_RENDERBUFFER_NV, TextureName[texture::COLOR]);
		glBindSampler(0, SamplerName);

		glBindVertexArray(VertexArrayName);

		glEnable(GL_SCISSOR_TEST);

		// Box
		{
			glScissor(1, 1, WindowSize.x  / 2 - 2, WindowSize.y - 2);
			glUseProgram(ProgramName[program::RESOLVE_BOX]);
			glUniform1i(UniformDiffuse[program::RESOLVE_BOX], 0);
			glUniformMatrix4fv(UniformMVP[program::RESOLVE_BOX], 1, GL_FALSE, &MVP[0][0]);
			glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 5);
		}

		// Near
		{
			glScissor(WindowSize.x / 2 + 1, 1, WindowSize.x / 2 - 2, WindowSize.y - 2);
			glUseProgram(ProgramName[program::RESOLVE_NEAR]);
			glUniform1i(UniformDiffuse[program::RESOLVE_NEAR], 0);
			glUniformMatrix4fv(UniformMVP[program::RESOLVE_NEAR], 1, GL_FALSE, &MVP[0][0]);
			glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 5);
		}

		glDisable(GL_SCISSOR_TEST);
	}

	bool render()
	{
		// Clear the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.5f, 1.0f, 1.0f)[0]);

		// Pass 1
		// Render the scene in a multisampled framebuffer
		glEnable(GL_MULTISAMPLE);
		renderFBO();
		glDisable(GL_MULTISAMPLE);

		// Pass 2
		// Resolved and render the colorbuffer from the multisampled framebuffer
		resolveMultisampling();

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_330_fbo_multisample_explicit_nv Test(argc, argv);
	Error += Test();

	return Error;
}
