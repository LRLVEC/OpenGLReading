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
		glf::vertex_v2fv2f(glm::vec2(-4.0f,-3.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 4.0f,-3.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 4.0f, 3.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2( 4.0f, 3.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-4.0f, 3.0f), glm::vec2(0.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-4.0f,-3.0f), glm::vec2(0.0f, 1.0f))
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

	namespace texture
	{
		enum type
		{
			COLORBUFFER,
			MULTISAMPLE_DEPTHBUFFER,
			MULTISAMPLE_COLORBUFFER,
			DIFFUSE,
			MAX
		};
	}//namespace texture

	namespace shader
	{
		enum type
		{
			VERT,
			FRAG_TEXTURE,
			FRAG_BOX,
			FRAG_NEAR,
			MAX
		};
	}//namespace shader

	namespace framebuffer
	{
		enum type
		{
			RENDER,
			RESOLVE,
			MAX
		};
	}//namespace framebuffer

	namespace buffer
	{
		enum type
		{
			VERTEX,
			TRANSFORM,
			MAX
		};
	}//namespace buffer

	char const * VERT_SHADER_SOURCE("gl-450/fbo-multisample-explicit.vert");
	char const * FRAG_SHADER_SOURCE[program::MAX] = 
	{
		"gl-450/fbo-multisample-explicit-texture.frag",
		"gl-450/fbo-multisample-explicit-box.frag",
		"gl-450/fbo-multisample-explicit-near.frag",
	};
}//namespace

class gl_450_fbo_multisample_explicit : public test
{
public:
	gl_450_fbo_multisample_explicit(int argc, char* argv[]) :
		test(argc, argv, "gl-450-fbo-multisample-explicit", test::CORE, 4, 4, glm::vec2(glm::pi<float>() * 0.2f)),
		VertexArrayName(0),
		UniformBlockSize(0),
		UniformPointer(nullptr)
	{}

private:
	std::array<GLuint, framebuffer::MAX> FramebufferName;
	std::array<GLuint, program::MAX> PipelineName;
	std::array<GLuint, program::MAX> ProgramName;
	std::array<GLuint, texture::MAX> TextureName;
	std::array<GLuint, buffer::MAX> BufferName;
	GLuint VertexArrayName;
	GLint UniformBlockSize;
	glm::uint8* UniformPointer;

	bool initProgram()
	{
		bool Validated = true;

		std::array<GLuint, shader::MAX> ShaderName;

		compiler Compiler;
		ShaderName[shader::VERT] = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 440 --profile core");

		for(int i = 0; i < program::MAX; ++i)
		{
			ShaderName[shader::FRAG_TEXTURE + i] = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE[i], "--version 440 --profile core");

			ProgramName[i] = glCreateProgram();
			glProgramParameteri(ProgramName[i], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[i], ShaderName[shader::VERT]);
			glAttachShader(ProgramName[i], ShaderName[shader::FRAG_TEXTURE + i]);
			glLinkProgram(ProgramName[i]);
		}
		
		if(Validated)
		{
			Validated = Validated && Compiler.check();
			for(int i = 0; i < program::MAX; ++i)
				Validated = Validated && Compiler.checkProgram(ProgramName[i]);
		}

		if (Validated)
		{
			glGenProgramPipelines(program::MAX, &PipelineName[0]);
			for (std::size_t i = 0; i < PipelineName.size(); ++i)
				glUseProgramStages(PipelineName[i], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName[i]);
		}

		return Validated;
	}

	bool initBuffer()
	{
		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		this->UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glCreateBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferStorage(GL_ARRAY_BUFFER, VertexSize, VertexData, 0);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferStorage(GL_UNIFORM_BUFFER, this->UniformBlockSize * 2, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

		this->UniformPointer = reinterpret_cast<glm::uint8*>(glMapBufferRange(GL_UNIFORM_BUFFER,
			0, this->UniformBlockSize * 2, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return true;
	}

	bool initTexture()
	{
		glGenTextures(texture::MAX, &TextureName[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::DIFFUSE]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));
		for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexImage2D(GL_TEXTURE_2D, 
				GLint(Level),
				GL_RGB8,
				GLsizei(Texture[Level].dimensions().x), GLsizei(Texture[Level].dimensions().y),
				0,
				GL_BGR, GL_UNSIGNED_BYTE,
				Texture[Level].data());
		}

		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, TextureName[texture::MULTISAMPLE_COLORBUFFER]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, TextureName[texture::MULTISAMPLE_DEPTHBUFFER]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH_COMPONENT24, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		glBindTexture(GL_TEXTURE_2D, TextureName[texture::COLORBUFFER]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		return true;
	}

	bool initFramebuffer()
	{
		glGenFramebuffers(framebuffer::MAX, &this->FramebufferName[0]);

		glBindFramebuffer(GL_FRAMEBUFFER, this->FramebufferName[framebuffer::RENDER]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureName[texture::MULTISAMPLE_COLORBUFFER], 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, TextureName[texture::MULTISAMPLE_DEPTHBUFFER], 0);

		glBindFramebuffer(GL_FRAMEBUFFER, this->FramebufferName[framebuffer::RESOLVE]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureName[texture::COLORBUFFER], 0);

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return true;
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(0));
			glVertexAttribPointer(semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(sizeof(glm::vec2)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
			glEnableVertexAttribArray(semantic::attr::TEXCOORD);
		glBindVertexArray(0);

		return true;
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

		return Validated;
	}

	bool end()
	{
		for(int i = 0; i < program::MAX; ++i)
			glDeleteProgram(ProgramName[i]);
		
		glDeleteProgramPipelines(program::MAX, &PipelineName[0]);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteTextures(texture::MAX, &TextureName[0]);
		glDeleteFramebuffers(framebuffer::MAX, &FramebufferName[0]);
		glDeleteVertexArrays(1, &VertexArrayName);

		return true;
	}

	void renderFBO(GLuint Framebuffer)
	{
		glEnable(GL_DEPTH_TEST);

		glBindProgramPipeline(PipelineName[program::THROUGH]);
		glBindBufferRange(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM], 0, this->UniformBlockSize);

		glViewport(0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y);

		glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer);
		float Depth(1.0f);
		glClearBufferfv(GL_DEPTH, 0, &Depth);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

		glBindTextures(0, 1, &TextureName[texture::DIFFUSE]);
		glBindVertexArray(VertexArrayName);

		glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 5);

		glDisable(GL_DEPTH_TEST);
	}

	void resolveMultisampling()
	{
		glm::ivec2 WindowSize(this->getWindowSize());

		glViewport(0, 0, WindowSize.x, WindowSize.y);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindTextures(0, 1, &TextureName[texture::MULTISAMPLE_COLORBUFFER]);
		glBindVertexArray(VertexArrayName);
		glBindBufferRange(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM], this->UniformBlockSize, this->UniformBlockSize);

		glEnable(GL_SCISSOR_TEST);

		// Box
		{
			glScissor(1, 1, WindowSize.x  / 2 - 2, WindowSize.y - 2);

			glBindProgramPipeline(PipelineName[program::RESOLVE_BOX]);
			glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 5);
		}

		// Near
		{
			glScissor(WindowSize.x / 2 + 1, 1, WindowSize.x / 2 - 2, WindowSize.y - 2);

			glBindProgramPipeline(PipelineName[program::RESOLVE_NEAR]);
			glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 5);
		}

		glDisable(GL_SCISSOR_TEST);
	}

	bool render()
	{
		glm::ivec2 WindowSize(this->getWindowSize());

		{
			glm::mat4 Perspective = glm::perspective(glm::pi<float>() * 0.25f, float(FRAMEBUFFER_SIZE.x) / FRAMEBUFFER_SIZE.y, 0.1f, 100.0f);
			glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f));
			glm::mat4 MVP = Perspective * this->view() * Model;
			*reinterpret_cast<glm::mat4*>(this->UniformPointer + this->UniformBlockSize * 0) = MVP;
		}

		{
			glm::mat4 Perspective = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f);
			glm::mat4 ViewFlip = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));
			glm::mat4 View = glm::translate(ViewFlip, glm::vec3(0.0f, 0.0f, -this->cameraDistance() * 2.0));
			glm::mat4 Model = glm::mat4(1.0f);
			glm::mat4 MVP = Perspective * View * Model;
			*reinterpret_cast<glm::mat4*>(this->UniformPointer + this->UniformBlockSize * 1) = MVP;
		}

		// Clear the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		GLenum Buffer = GL_BACK;
		glDrawBuffers(1, &Buffer);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.5f, 1.0f, 1.0f)[0]);

		// Pass 1
		// Render the scene in a multisampled framebuffer
		glEnable(GL_MULTISAMPLE);
		renderFBO(FramebufferName[framebuffer::RENDER]);
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

	gl_450_fbo_multisample_explicit Test(argc, argv);
	Error += Test();

	return Error;
}

