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
	char const * VERT_SHADER_SOURCE("gl-440/sampler-wrap.vert");
	char const * FRAG_SHADER_SOURCE("gl-440/sampler-wrap.frag");
	char const * TEXTURE_DIFFUSE_DXT5("kueken1-dxt5.dds");

	struct vertex
	{
		vertex
		(
			glm::vec2 const & Position,
			glm::vec2 const & Texcoord
		) :
			Position(Position),
			Texcoord(Texcoord)
		{}

		glm::vec2 Position;
		glm::vec2 Texcoord;
	};

	// With DDS textures, v texture coordinate are reversed, from top to bottom
	GLsizei const VertexCount(6);
	GLsizeiptr const VertexSize(VertexCount * sizeof(vertex));
	vertex const VertexData[VertexCount] =
	{
		vertex(glm::vec2(-1.0f,-1.0f), glm::vec2(-2.0f, 2.0f)),
		vertex(glm::vec2( 1.0f,-1.0f), glm::vec2( 2.0f, 2.0f)),
		vertex(glm::vec2( 1.0f, 1.0f), glm::vec2( 2.0f,-2.0f)),
		vertex(glm::vec2( 1.0f, 1.0f), glm::vec2( 2.0f,-2.0f)),
		vertex(glm::vec2(-1.0f, 1.0f), glm::vec2(-2.0f,-2.0f)),
		vertex(glm::vec2(-1.0f,-1.0f), glm::vec2(-2.0f, 2.0f))
	};

	namespace viewport
	{
		enum type
		{
			VIEWPORT0,
			VIEWPORT1,
			VIEWPORT2,
			VIEWPORT3,
			VIEWPORT4,
			VIEWPORT5,
			MAX
		};
	}//namespace viewport

	namespace buffer
	{
		enum type
		{
			VERTEX,
			TRANSFORM,
			MAX
		};
	}//namespace buffer
}//namespace

class gl_440_sampler_wrap : public test
{
public:
	gl_440_sampler_wrap(int argc, char* argv[]) :
		test(argc, argv, "gl-440-sampler-wrap", test::CORE, 4, 4),
		PipelineName(0),
		ProgramName(0),
		VertexArrayName(0),
		TextureName(0),
		UniformPointer(nullptr)
	{}

private:
	GLuint PipelineName;
	GLuint ProgramName;
	GLuint VertexArrayName;
	GLuint TextureName;
	glm::mat4* UniformPointer;
	std::array<GLuint, buffer::MAX> BufferName;
	std::array<GLuint, viewport::MAX> SamplerName;
	std::array<glm::vec4, viewport::MAX> Viewport;

	bool initProgram()
	{
		bool Validated(true);
	
		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 440 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 440 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glProgramParameteri(ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);
			glLinkProgram(ProgramName);

			Validated = Validated && Compiler.checkProgram(ProgramName);
		}

		if(Validated)
		{
			glGenProgramPipelines(1, &PipelineName);
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);
		}

		return Validated && this->checkError("initProgram");
	}

	bool initBuffer()
	{
		// Generate a buffer object
		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferStorage(GL_ARRAY_BUFFER, VertexSize, VertexData, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLint UniformBufferOffset(0);

		glGetIntegerv(
			GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
			&UniformBufferOffset);

		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferStorage(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return this->checkError("initBuffer");;
	}

	bool initSampler()
	{
		glGenSamplers(viewport::MAX, &SamplerName[0]);

		for(std::size_t i = 0; i < viewport::MAX; ++i)
		{
			glSamplerParameteri(SamplerName[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glSamplerParameteri(SamplerName[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glm::vec4 BorderColor(0.0f, 0.5f, 1.0f, 1.0f);
			glSamplerParameterfv(SamplerName[i], GL_TEXTURE_BORDER_COLOR, &BorderColor[0]);
		}

		glSamplerParameteri(SamplerName[viewport::VIEWPORT0], GL_TEXTURE_WRAP_S, GL_REPEAT);
		glSamplerParameteri(SamplerName[viewport::VIEWPORT1], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(SamplerName[viewport::VIEWPORT2], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glSamplerParameteri(SamplerName[viewport::VIEWPORT3], GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glSamplerParameteri(SamplerName[viewport::VIEWPORT4], GL_TEXTURE_WRAP_S, GL_MIRROR_CLAMP_TO_EDGE);
		if(this->checkExtension("GL_EXT_texture_mirror_clamp"))
			glSamplerParameteri(SamplerName[viewport::VIEWPORT5], GL_TEXTURE_WRAP_S, GL_MIRROR_CLAMP_TO_BORDER_EXT);
		else
			glSamplerParameteri(SamplerName[viewport::VIEWPORT5], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);

		glSamplerParameteri(SamplerName[viewport::VIEWPORT0], GL_TEXTURE_WRAP_T, GL_REPEAT);
		glSamplerParameteri(SamplerName[viewport::VIEWPORT1], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(SamplerName[viewport::VIEWPORT2], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glSamplerParameteri(SamplerName[viewport::VIEWPORT3], GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glSamplerParameteri(SamplerName[viewport::VIEWPORT4], GL_TEXTURE_WRAP_T, GL_MIRROR_CLAMP_TO_EDGE);
		if(this->checkExtension("GL_EXT_texture_mirror_clamp"))
			glSamplerParameteri(SamplerName[viewport::VIEWPORT5], GL_TEXTURE_WRAP_T, GL_MIRROR_CLAMP_TO_BORDER_EXT);
		else
			glSamplerParameteri(SamplerName[viewport::VIEWPORT5], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		return this->checkError("initSampler");
	}

	bool initTexture()
	{
		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE_DXT5).c_str()));

		glGenTextures(1, &TextureName);

		glBindTexture(GL_TEXTURE_2D, TextureName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));

		for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
		{
			glCompressedTexImage2D(
				GL_TEXTURE_2D,
				GLint(Level),
				GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
				GLsizei(Texture[Level].dimensions().x), 
				GLsizei(Texture[Level].dimensions().y), 
				0, 
				GLsizei(Texture[Level].size()), 
				Texture[Level].data());
		}

		return this->checkError("initTexture");
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glVertexAttribFormat(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, 0);
			glVertexAttribBinding(semantic::attr::POSITION, semantic::buffer::STATIC);
			glEnableVertexAttribArray(semantic::attr::POSITION);

			glVertexAttribFormat(semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2));
			glVertexAttribBinding(semantic::attr::TEXCOORD, semantic::buffer::STATIC);
			glEnableVertexAttribArray(semantic::attr::TEXCOORD);
		glBindVertexArray(0);

		return this->checkError("initVertexArray");
	}

	bool begin()
	{
		glm::uvec2 WindowSize = this->getWindowSize();

		glm::vec2 ViewportSize(WindowSize.x * 0.33f, WindowSize.y * 0.50f);

		Viewport[viewport::VIEWPORT0] = glm::vec4(ViewportSize.x * 0.0f, ViewportSize.y * 0.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f);
		Viewport[viewport::VIEWPORT1] = glm::vec4(ViewportSize.x * 1.0f, ViewportSize.y * 0.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f);
		Viewport[viewport::VIEWPORT2] = glm::vec4(ViewportSize.x * 2.0f, ViewportSize.y * 0.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f);
		Viewport[viewport::VIEWPORT3] = glm::vec4(ViewportSize.x * 0.0f, ViewportSize.y * 1.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f);
		Viewport[viewport::VIEWPORT4] = glm::vec4(ViewportSize.x * 1.0f, ViewportSize.y * 1.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f);
		Viewport[viewport::VIEWPORT5] = glm::vec4(ViewportSize.x * 2.0f, ViewportSize.y * 1.0f, ViewportSize.x * 1.0f, ViewportSize.y * 1.0f);

		bool Validated = true;

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initSampler();
		if(Validated)
			Validated = initVertexArray();

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		UniformPointer = (glm::mat4*)glMapBufferRange(
			GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

		return Validated;
	}

	bool end()
	{
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteProgram(ProgramName);
		glDeleteProgramPipelines(1, &PipelineName);
		glDeleteTextures(1, &TextureName);
		glDeleteVertexArrays(1, &VertexArrayName);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		{
			float Aspect = (WindowSize.x * 0.33f) / (WindowSize.y * 0.50f);
			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, Aspect, 0.1f, 100.0f);
			glm::mat4 MVP = Projection * this->view() * glm::mat4(1.0f);

			*UniformPointer = MVP;
		}

		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

		glBindProgramPipeline(PipelineName);
		glBindBuffersBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, 1, &BufferName[buffer::TRANSFORM]);
		glBindTextures(semantic::sampler::DIFFUSE, 1, &TextureName);
		glBindVertexArray(VertexArrayName);
		glBindVertexBuffer(semantic::buffer::STATIC, BufferName[buffer::VERTEX], 0, GLsizei(sizeof(vertex)));

		for(std::size_t Index = 0; Index < viewport::MAX; ++Index)
		{
			glViewportIndexedf(0, 
				Viewport[Index].x, 
				Viewport[Index].y, 
				Viewport[Index].z, 
				Viewport[Index].w);

			glBindSamplers(0, 1, &SamplerName[Index]);
			glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
		}

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_440_sampler_wrap Test(argc, argv);
	Error += Test();

	return Error;
}


