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
	char const * VERT_SHADER_SOURCE1("gl-320/fbo-multisample-integer.vert");
	char const * FRAG_SHADER_SOURCE1("gl-320/fbo-multisample-integer.frag");
	char const * VERT_SHADER_SOURCE2("gl-320/texture-integer.vert");
	char const * FRAG_SHADER_SOURCE2("gl-320/texture-integer.frag");
	char const * TEXTURE_DIFFUSE("kueken3-bgr8.dds");
	int const FRAMEBUFFER_SIZE(4);

	// With DDS textures, v texture coordinate are reversed, from top to bottom
	GLsizei const VertexCount(6);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-4.0f,-3.0f) * 0.5f, glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 4.0f,-3.0f) * 0.5f, glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 4.0f, 3.0f) * 0.5f, glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 4.0f, 3.0f) * 0.5f, glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-4.0f, 3.0f) * 0.5f, glm::vec2(0.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-4.0f,-3.0f) * 0.5f, glm::vec2(0.0f, 1.0f))
	};

	namespace texture
	{
		enum type
		{
			DIFFUSE,
			COLORBUFFER,
			MULTISAMPLE,
			MAX
		};
	}//namespace texture

	namespace program
	{
		enum type
		{
			RENDER,
			SPLASH,
			MAX
		};
	}//namespace buffer

	namespace framebuffer
	{
		enum type
		{
			RENDER,
			RESOLVE,
			MAX
		};
	}//namespace framebuffer

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

	GLuint VertexArrayName(0);
	GLuint BufferName(0);
	std::vector<GLuint> ShaderName(shader::MAX);
	std::vector<GLuint> TextureName(texture::MAX);
	std::vector<GLuint> FramebufferName(framebuffer::MAX);
	std::vector<GLuint> ProgramName(program::MAX);
	std::vector<GLuint> UniformMVP(program::MAX);
	std::vector<GLuint> UniformDiffuse(program::MAX);
}//namespace

class gl_320_fbo_multisample_integer : public test
{
public:
	gl_320_fbo_multisample_integer(int argc, char* argv[]) :
		test(argc, argv, "gl-320-fbo-multisample-integer", test::CORE, 3, 2)
	{}

private:
	bool initProgram()
	{
		bool Validated = true;
	
		compiler Compiler;

		if(Validated)
		{
			ShaderName[shader::VERT1] = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE1, "--version 150 --profile core");
			ShaderName[shader::FRAG1] = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE1, "--version 150 --profile core");

			ProgramName[program::RENDER] = glCreateProgram();
			glAttachShader(ProgramName[program::RENDER], ShaderName[shader::VERT1]);
			glAttachShader(ProgramName[program::RENDER], ShaderName[shader::FRAG1]);

			glBindAttribLocation(ProgramName[program::RENDER], semantic::attr::POSITION, "Position");
			glBindAttribLocation(ProgramName[program::RENDER], semantic::attr::TEXCOORD, "Texcoord");
			glBindFragDataLocation(ProgramName[program::RENDER], semantic::frag::COLOR, "Color");
			glLinkProgram(ProgramName[program::RENDER]);
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
			Validated = Validated && Compiler.checkProgram(ProgramName[program::RENDER]);
			Validated = Validated && Compiler.checkProgram(ProgramName[program::SPLASH]);
		}

		if(Validated)
		{
			UniformMVP[program::RENDER] = glGetUniformLocation(ProgramName[program::RENDER], "MVP");
			UniformDiffuse[program::RENDER] = glGetUniformLocation(ProgramName[program::RENDER], "Diffuse");

			UniformMVP[program::SPLASH] = glGetUniformLocation(ProgramName[program::SPLASH], "MVP");
			UniformDiffuse[program::SPLASH] = glGetUniformLocation(ProgramName[program::SPLASH], "Diffuse");
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
		GLint MaxSampleMaskWords(0);
		GLint MaxColorTextureSamples(0);
		GLint MaxDepthTextureSamples(0);
		GLint MaxIntegerSamples(0);

		glGetIntegerv(GL_MAX_SAMPLE_MASK_WORDS, &MaxSampleMaskWords); 
		glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &MaxColorTextureSamples); 
		glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &MaxDepthTextureSamples); 
		glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &MaxIntegerSamples); 

		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));

		glGenTextures(texture::MAX, &TextureName[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::DIFFUSE]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexImage2D(
				GL_TEXTURE_2D, 
				GLint(Level), 
				GL_RGBA8UI, 
				GLsizei(Texture[Level].dimensions().x), 
				GLsizei(Texture[Level].dimensions().y), 
				0,
				GL_BGR_INTEGER, GL_UNSIGNED_BYTE,
				Texture[Level].data());
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		glm::ivec2 WindowSize(this->getWindowSize());

		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, TextureName[texture::MULTISAMPLE]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MaxIntegerSamples, GL_RGBA8UI,
			WindowSize.x / FRAMEBUFFER_SIZE,
			WindowSize.y / FRAMEBUFFER_SIZE,
			GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		glBindTexture(GL_TEXTURE_2D, TextureName[texture::COLORBUFFER]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI,
			WindowSize.x / FRAMEBUFFER_SIZE,
			WindowSize.y / FRAMEBUFFER_SIZE,
			0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		glBindTexture(GL_TEXTURE_2D, 0);

		return this->checkError("initTexture");
	}

	bool initFramebuffer()
	{
		glGenFramebuffers(framebuffer::MAX, &FramebufferName[0]);

		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName[framebuffer::RENDER]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureName[texture::MULTISAMPLE], 0);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName[framebuffer::RESOLVE]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureName[texture::COLORBUFFER], 0);
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
		caps Caps(caps::CORE);

		// Multisample integer texture is optional
		bool Validated = Caps.Limits.MAX_INTEGER_SAMPLES > 1;

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
		glDeleteBuffers(1, &BufferName);
		glDeleteProgram(ProgramName[program::RENDER]);
		glDeleteProgram(ProgramName[program::SPLASH]);
		glDeleteTextures(texture::MAX, &TextureName[0]);
		glDeleteFramebuffers(framebuffer::MAX, &FramebufferName[0]);
		glDeleteVertexArrays(1, &VertexArrayName);

		return this->checkError("end");
	}

	void renderFBO(GLuint Framebuffer)
	{
		glm::ivec2 WindowSize(this->getWindowSize());

		glm::mat4 Perspective = glm::perspective(glm::pi<float>() * 0.25f, float(WindowSize.x) / WindowSize.y, 0.1f, 100.0f);
		glm::mat4 ViewFlip = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		glm::mat4 ViewTranslate = glm::translate(ViewFlip, glm::vec3(0.0f, 0.0f, -this->cameraDistance() * 2.0f));
		glm::mat4 View = glm::rotate(ViewTranslate,-15.f, glm::vec3(0.f, 0.f, 1.f));
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Perspective * View * Model;

		glUseProgram(ProgramName[program::RENDER]);
		glUniform1i(UniformDiffuse[program::RENDER], 0);
		glUniformMatrix4fv(UniformMVP[program::RENDER], 1, GL_FALSE, &MVP[0][0]);

		glViewport(0, 0,
			WindowSize.x / FRAMEBUFFER_SIZE,
			WindowSize.y / FRAMEBUFFER_SIZE);
		glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer);
		glClearBufferuiv(GL_COLOR, 0, &glm::uvec4(0, 128, 255, 255)[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::DIFFUSE]);
		glBindVertexArray(VertexArrayName);

		glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);

		this->checkError("renderFBO");
	}

	void renderFB(GLuint TextureName)
	{
		glm::ivec2 WindowSize(this->getWindowSize());

		glm::mat4 Perspective = glm::perspective(glm::pi<float>() * 0.25f, float(WindowSize.x) / WindowSize.y, 0.1f, 100.0f);
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Perspective * this->view() * Model;

		glUseProgram(ProgramName[program::SPLASH]);
		glUniform1i(UniformDiffuse[program::SPLASH], 0);
		glUniformMatrix4fv(UniformMVP[program::SPLASH], 1, GL_FALSE, &MVP[0][0]);

		glViewport(0, 0, WindowSize.x, WindowSize.y);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName);
		glBindVertexArray(VertexArrayName);

		glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
		
		this->checkError("renderFB");
	}

	bool render()
	{
		glm::ivec2 WindowSize(this->getWindowSize());

		// Clear the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

		// Pass 1
		// Render the scene in a multisampled framebuffer
		glEnable(GL_MULTISAMPLE);
		renderFBO(FramebufferName[framebuffer::RENDER]);
		glDisable(GL_MULTISAMPLE);

		// Resolved multisampling
		glBindFramebuffer(GL_READ_FRAMEBUFFER, FramebufferName[framebuffer::RENDER]);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FramebufferName[framebuffer::RESOLVE]);
		glBlitFramebuffer(
			0, 0,
			WindowSize.x / FRAMEBUFFER_SIZE,
			WindowSize.y / FRAMEBUFFER_SIZE,
			0, 0,
			WindowSize.x / FRAMEBUFFER_SIZE,
			WindowSize.y / FRAMEBUFFER_SIZE,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Pass 2
		// Render the colorbuffer from the multisampled framebuffer
		renderFB(TextureName[texture::COLORBUFFER]);

		return this->checkError("render");;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_320_fbo_multisample_integer Test(argc, argv);
	Error += Test();

	return Error;
}

