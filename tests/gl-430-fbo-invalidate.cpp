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
	char const * VERT_SHADER_SOURCE_AA("gl-430/fbo-invalidate-render.vert");
	char const * FRAG_SHADER_SOURCE_AA("gl-430/fbo-invalidate-render.frag");
	char const * VERT_SHADER_SOURCE_SPLASH("gl-430/fbo-invalidate-splash.vert");
	char const * FRAG_SHADER_SOURCE_SPLASH("gl-430/fbo-invalidate-splash.frag");

	glm::uvec2 const FRAMEBUFFER_SIZE(80, 60);
	std::vector<glm::vec2> VertexData;

	namespace buffer
	{
		enum type
		{
			VERTEX,
			TRANSFORM,
			MAX
		};
	}//namespace buffer
	
	namespace texture
	{
		enum type
		{
			MULTISAMPLE,
			COLOR,
			MAX
		};
	}//namespace texture

	namespace framebuffer
	{
		enum type
		{
			MULTISAMPLE,
			COLOR,
			MAX
		};
	}//namespace framebuffer

	namespace pipeline
	{
		enum type
		{
			MULTISAMPLE,
			SPLASH,
			MAX
		};
	}//namespace pipeline
}//namespace

class gl_430_fbo_invalidate : public test
{
public:
	gl_430_fbo_invalidate(int argc, char* argv[]) :
		test(argc, argv, "gl-430-fbo-invalidate", test::CORE, 4, 2)
	{}

private:
	std::array<GLuint, pipeline::MAX> PipelineName;
	std::array<GLuint, pipeline::MAX> ProgramName;
	std::array<GLuint, pipeline::MAX> VertexArrayName;
	std::array<GLuint, framebuffer::MAX> FramebufferName;
	std::array<GLuint, buffer::MAX> BufferName;
	std::array<GLuint, texture::MAX> TextureName;

	bool initProgram()
	{
		bool Validated(true);
	
		compiler Compiler;

		if(Validated)
		{
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE_AA);
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE_AA);

			ProgramName[pipeline::MULTISAMPLE] = glCreateProgram();
			glProgramParameteri(ProgramName[pipeline::MULTISAMPLE], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[pipeline::MULTISAMPLE], VertShaderName);
			glAttachShader(ProgramName[pipeline::MULTISAMPLE], FragShaderName);
			glLinkProgram(ProgramName[pipeline::MULTISAMPLE]);

			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.checkProgram(ProgramName[pipeline::MULTISAMPLE]);
		}

		if(Validated)
		{
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE_SPLASH);
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE_SPLASH);

			ProgramName[pipeline::SPLASH] = glCreateProgram();
			glProgramParameteri(ProgramName[pipeline::SPLASH], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[pipeline::SPLASH], VertShaderName);
			glAttachShader(ProgramName[pipeline::SPLASH], FragShaderName);
			glLinkProgram(ProgramName[pipeline::SPLASH]);

			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.checkProgram(ProgramName[pipeline::SPLASH]);
		}

		if(Validated)
		{
			glGenProgramPipelines(pipeline::MAX, &PipelineName[0]);
			glUseProgramStages(PipelineName[pipeline::MULTISAMPLE], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName[pipeline::MULTISAMPLE]);
			glUseProgramStages(PipelineName[pipeline::SPLASH], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName[pipeline::SPLASH]);
		}

		return Validated;
	}

	bool initBuffer()
	{
		std::size_t const Count(360);
		float const Step(360.f / float(Count));

		VertexData.resize(Count);
		for(std::size_t i = 0; i < Count; ++i)
			VertexData[i] = glm::vec2(glm::sin(glm::radians(Step * float(i))), glm::cos(glm::radians(Step * float(i))));

		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, VertexData.size() * sizeof(glm::vec2), &VertexData[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return true;
	}

	bool initVertexArray()
	{
		glGenVertexArrays(pipeline::MAX, &VertexArrayName[0]);

		glBindVertexArray(VertexArrayName[pipeline::MULTISAMPLE]);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), BUFFER_OFFSET(0));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
		glBindVertexArray(0);

		glBindVertexArray(VertexArrayName[pipeline::SPLASH]);
		glBindVertexArray(0);

		return true;
	}

	bool initFramebuffer()
	{
		glGenFramebuffers(framebuffer::MAX, &FramebufferName[0]);

		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName[framebuffer::MULTISAMPLE]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureName[texture::MULTISAMPLE], 0);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName[framebuffer::COLOR]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureName[texture::COLOR], 0);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return true;
	}

	bool initTexture()
	{
		glGenTextures(texture::MAX, &TextureName[0]);

		{
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, TextureName[texture::MULTISAMPLE]);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGBA8, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, GL_TRUE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		}

		{
			glBindTexture(GL_TEXTURE_2D, TextureName[texture::COLOR]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	
		return true;
	}

	bool initState()
	{
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

		return true;
	}

	bool begin()
	{
		bool Validated(true);
		Validated = Validated && this->checkExtension("GL_ARB_invalidate_subdata");

		if(Validated)
			Validated = initState();
		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initFramebuffer();
		if(Validated)
			Validated = initVertexArray();

		return Validated;
	}

	bool end()
	{
		glDeleteBuffers(texture::MAX, &TextureName[0]);
		glDeleteProgramPipelines(pipeline::MAX, &PipelineName[0]);
		glDeleteProgram(ProgramName[pipeline::MULTISAMPLE]);
		glDeleteProgram(ProgramName[pipeline::SPLASH]);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteVertexArrays(pipeline::MAX, &VertexArrayName[0]);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		{
			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(
				GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

			glm::mat4 Projection = glm::perspectiveFov(glm::pi<float>() * 0.25f, WindowSize.x, WindowSize.y, 0.1f, 100.0f);
			glm::mat4 Model = glm::mat4(1.0f);
		
			*Pointer = Projection * this->view() * Model;

			// Make sure the uniform buffer is uploaded
			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}

		//////////////////////////////
		// Render multisampled texture

		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName[framebuffer::MULTISAMPLE]);

		glViewportIndexedf(0, 0, 0, GLfloat(FRAMEBUFFER_SIZE.x), GLfloat(FRAMEBUFFER_SIZE.y));
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

		glBindProgramPipeline(PipelineName[pipeline::MULTISAMPLE]);
		glBindVertexArray(VertexArrayName[pipeline::MULTISAMPLE]);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

		glEnable(GL_MULTISAMPLE);
			glDrawArraysInstancedBaseInstance(GL_LINE_LOOP, 0, GLsizei(VertexData.size()), 3, 0);
		glDisable(GL_MULTISAMPLE);

		//////////////////////////
		// Resolving multisampling
		glBindFramebuffer(GL_READ_FRAMEBUFFER, FramebufferName[framebuffer::MULTISAMPLE]);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FramebufferName[framebuffer::COLOR]);
		glBlitFramebuffer(
			0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, 
			0, 0, FRAMEBUFFER_SIZE.x, FRAMEBUFFER_SIZE.y, 
			GL_COLOR_BUFFER_BIT, GL_NEAREST);

		GLenum attachments[1] = {GL_COLOR_ATTACHMENT0};
		glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, attachments);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//////////////////////////////////////
		// Render resolved multisample texture

		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glDisable(GL_MULTISAMPLE);
		glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, TextureName[texture::MULTISAMPLE]);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::COLOR]);
		glBindVertexArray(VertexArrayName[pipeline::SPLASH]);
		glBindProgramPipeline(PipelineName[pipeline::SPLASH]);

		glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_430_fbo_invalidate Test(argc, argv);
	Error += Test();

	return Error;
}

