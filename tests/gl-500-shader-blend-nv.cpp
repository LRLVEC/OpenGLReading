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
	char const * VERT_SHADER_SOURCE_RENDER("gl-500/shader-blend-render-nv.vert");
	char const * FRAG_SHADER_SOURCE_RENDER("gl-500/shader-blend-render-nv.frag");
	char const * VERT_SHADER_SOURCE_SPLASH("gl-500/shader-blend-blit-nv.vert");
	char const * FRAG_SHADER_SOURCE_SPLASH("gl-500/shader-blend-blit-nv.frag");
	char const * TEXTURE_DIFFUSE("kueken1-bgr8.dds");

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f))
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2, 
		2, 3, 0
	};

	namespace texture
	{
		enum type
		{
			DIFFUSE,
			COLORBUFFER,
			MAX
		};
	}//namespace texture
	
	namespace pipeline
	{
		enum type
		{
			RENDER,
			SPLASH,
			MAX
		};
	}//namespace pipeline

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			TRANSFORM,
			MAX
		};
	}//namespace buffer
}//namespace

class gl_500_shader_blend_nv : public test
{
public:
	gl_500_shader_blend_nv(int argc, char* argv[]) :
		test(argc, argv, "gl-500-shader-blend-nv", test::CORE, 4, 5),
		FramebufferName(0),
		Supersampling(2)
	{}

private:
	std::array<GLuint, pipeline::MAX> PipelineName;
	std::array<GLuint, pipeline::MAX> ProgramName;
	std::array<GLuint, texture::MAX> TextureName;
	std::array<GLuint, pipeline::MAX> SamplerName;
	std::array<GLuint, buffer::MAX> BufferName;
	std::array<GLuint, pipeline::MAX> VertexArrayName;
	GLuint FramebufferName;
	GLuint const Supersampling;
	std::vector<glm::vec4> Viewports;

	bool initProgram()
	{
		bool Validated(true);
	
		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE_RENDER, "--version 450 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE_RENDER, "--version 450 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName[pipeline::RENDER] = glCreateProgram();
			glProgramParameteri(ProgramName[pipeline::RENDER], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[pipeline::RENDER], VertShaderName);
			glAttachShader(ProgramName[pipeline::RENDER], FragShaderName);
			glLinkProgram(ProgramName[pipeline::RENDER]);

			Validated = Validated && Compiler.checkProgram(ProgramName[pipeline::RENDER]);
		}

		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE_SPLASH, "--version 450 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE_SPLASH, "--version 450 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName[pipeline::SPLASH] = glCreateProgram();
			glProgramParameteri(ProgramName[pipeline::SPLASH], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[pipeline::SPLASH], VertShaderName);
			glAttachShader(ProgramName[pipeline::SPLASH], FragShaderName);
			glLinkProgram(ProgramName[pipeline::SPLASH]);

			Validated = Validated && Compiler.checkProgram(ProgramName[pipeline::SPLASH]);
		}

		if(Validated)
		{
			glGenProgramPipelines(pipeline::MAX, &PipelineName[0]);
			glUseProgramStages(PipelineName[pipeline::RENDER], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName[pipeline::RENDER]);
			glUseProgramStages(PipelineName[pipeline::SPLASH], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName[pipeline::SPLASH]);
		}

		return Validated;
	}

	bool initSampler()
	{
		glCreateSamplers(pipeline::MAX, &SamplerName[0]);

		glSamplerParameteri(SamplerName[pipeline::RENDER], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glSamplerParameteri(SamplerName[pipeline::RENDER], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(SamplerName[pipeline::RENDER], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(SamplerName[pipeline::RENDER], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(SamplerName[pipeline::RENDER], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glSamplerParameterfv(SamplerName[pipeline::RENDER], GL_TEXTURE_BORDER_COLOR, &glm::vec4(0.0f)[0]);
		glSamplerParameterf(SamplerName[pipeline::RENDER], GL_TEXTURE_MIN_LOD, -1000.f);
		glSamplerParameterf(SamplerName[pipeline::RENDER], GL_TEXTURE_MAX_LOD, 1000.f);
		glSamplerParameterf(SamplerName[pipeline::RENDER], GL_TEXTURE_LOD_BIAS, 0.0f);
		glSamplerParameteri(SamplerName[pipeline::RENDER], GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glSamplerParameteri(SamplerName[pipeline::RENDER], GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

		glSamplerParameteri(SamplerName[pipeline::SPLASH], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glSamplerParameteri(SamplerName[pipeline::SPLASH], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(SamplerName[pipeline::SPLASH], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(SamplerName[pipeline::SPLASH], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(SamplerName[pipeline::SPLASH], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glSamplerParameterfv(SamplerName[pipeline::SPLASH], GL_TEXTURE_BORDER_COLOR, &glm::vec4(0.0f)[0]);
		glSamplerParameterf(SamplerName[pipeline::SPLASH], GL_TEXTURE_MIN_LOD, -1000.f);
		glSamplerParameterf(SamplerName[pipeline::SPLASH], GL_TEXTURE_MAX_LOD, 1000.f);
		glSamplerParameterf(SamplerName[pipeline::SPLASH], GL_TEXTURE_LOD_BIAS, 0.0f);
		glSamplerParameteri(SamplerName[pipeline::SPLASH], GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glSamplerParameteri(SamplerName[pipeline::SPLASH], GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

		return true;
	}

	bool initBuffer()
	{
		bool Validated(true);

		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glCreateBuffers(buffer::MAX, &BufferName[0]);

		glNamedBufferData(BufferName[buffer::ELEMENT], ElementSize, ElementData, GL_STATIC_DRAW);
		glNamedBufferData(BufferName[buffer::VERTEX], VertexSize, VertexData, GL_STATIC_DRAW);
		glNamedBufferData(BufferName[buffer::TRANSFORM], UniformBlockSize, nullptr, GL_DYNAMIC_DRAW);

		return Validated;
	}

	bool initTexture()
	{
		bool Validated(true);

		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));
		assert(!Texture.empty());

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glCreateTextures(GL_TEXTURE_2D, texture::MAX, &TextureName[0]);
		glTextureParameteri(TextureName[texture::DIFFUSE], GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(TextureName[texture::DIFFUSE], GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTextureParameteri(TextureName[texture::DIFFUSE], GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTextureParameteri(TextureName[texture::DIFFUSE], GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTextureParameteri(TextureName[texture::DIFFUSE], GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTextureParameteri(TextureName[texture::DIFFUSE], GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
		glTextureParameteri(TextureName[texture::DIFFUSE], GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTextureParameteri(TextureName[texture::DIFFUSE], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureStorage2D(TextureName[texture::DIFFUSE], static_cast<GLint>(Texture.levels()), gli::internal_format(Texture.format()),
			static_cast<GLsizei>(Texture.dimensions().x), static_cast<GLsizei>(Texture.dimensions().y));

		for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
		{
			glTextureSubImage2D(TextureName[texture::DIFFUSE], static_cast<GLint>(Level),
				0, 0,
				static_cast<GLsizei>(Texture[Level].dimensions().x), static_cast<GLsizei>(Texture[Level].dimensions().y),
				GL_BGR, GL_UNSIGNED_BYTE,
				Texture[Level].data());
		}
	
		glm::ivec2 WindowSize(this->getWindowSize());

		glTextureParameteri(TextureName[texture::COLORBUFFER], GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(TextureName[texture::COLORBUFFER], GL_TEXTURE_MAX_LEVEL, 0);
		glTextureParameteri(TextureName[texture::COLORBUFFER], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(TextureName[texture::COLORBUFFER], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureStorage2D(TextureName[texture::COLORBUFFER], static_cast<GLint>(1), GL_RGBA8,
			static_cast<GLsizei>(WindowSize.x * this->Supersampling), static_cast<GLsizei>(WindowSize.y * this->Supersampling));

		return Validated;
	}

	bool initVertexArray()
	{
		bool Validated(true);

		glCreateVertexArrays(pipeline::MAX, &VertexArrayName[0]);
		glVertexArrayElementBuffer(VertexArrayName[pipeline::RENDER], BufferName[buffer::ELEMENT]);

		return Validated;
	}

	bool initFramebuffer()
	{
		bool Validated(true);

		glm::ivec2 WindowSize(this->getWindowSize());

		glCreateFramebuffers(1, &FramebufferName);
		glNamedFramebufferParameteri(FramebufferName, GL_FRAMEBUFFER_DEFAULT_WIDTH, WindowSize.x * this->Supersampling);
		glNamedFramebufferParameteri(FramebufferName, GL_FRAMEBUFFER_DEFAULT_HEIGHT, WindowSize.y * this->Supersampling);
		glNamedFramebufferParameteri(FramebufferName, GL_FRAMEBUFFER_DEFAULT_LAYERS, 1);
		glNamedFramebufferParameteri(FramebufferName, GL_FRAMEBUFFER_DEFAULT_SAMPLES, 1);
		glNamedFramebufferParameteri(FramebufferName, GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS, GL_TRUE);

		return this->checkFramebuffer(FramebufferName);
	}

	bool begin()
	{
		bool Validated = this->checkExtension("GL_NV_fragment_shader_interlock");

		glm::vec2 WindowSize(this->getWindowSize());
		this->Viewports.resize(1000);
		for (std::size_t i = 0; i < this->Viewports.size(); ++i)
		{
			this->Viewports[i] = glm::vec4(
				glm::linearRand(-WindowSize.x, WindowSize.x * 2.0f), 
				glm::linearRand(-WindowSize.y, WindowSize.y * 2.0f),
				WindowSize * glm::linearRand(0.0f, 1.0f));
		}

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initSampler();
		if(Validated)
			Validated = initFramebuffer();

		return Validated;
	}

	bool end()
	{
		glDeleteSamplers(pipeline::MAX, &SamplerName[0]);
		glDeleteProgramPipelines(pipeline::MAX, &PipelineName[0]);
		glDeleteProgram(ProgramName[pipeline::SPLASH]);
		glDeleteProgram(ProgramName[pipeline::RENDER]);
		glDeleteFramebuffers(1, &FramebufferName);
		glDeleteTextures(texture::MAX, &TextureName[0]);
		glDeleteVertexArrays(pipeline::MAX, &VertexArrayName[0]);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		{
			glm::mat4* Pointer = reinterpret_cast<glm::mat4*>(glMapNamedBufferRange(BufferName[buffer::TRANSFORM], 0, sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

			glm::mat4 Projection = glm::perspectiveFov(glm::pi<float>() * 0.25f, WindowSize.x, WindowSize.y, 0.1f, 100.0f);
			glm::mat4 Model = glm::mat4(1.0f);
			*Pointer = Projection * this->view() * Model;

			glUnmapNamedBuffer(BufferName[buffer::TRANSFORM]);
		}

		// Render
		glClearTexImage(TextureName[texture::COLORBUFFER], 0, GL_RGBA, GL_FLOAT, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

		glViewportIndexedf(0, 0, 0, WindowSize.x * this->Supersampling, WindowSize.y * this->Supersampling);
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glBindVertexArray(this->VertexArrayName[pipeline::RENDER]);

		glBindProgramPipeline(PipelineName[pipeline::RENDER]);
		glBindTextureUnit(0, TextureName[texture::DIFFUSE]);
		glBindImageTexture(semantic::image::DIFFUSE, TextureName[texture::COLORBUFFER], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, semantic::storage::VERTEX, BufferName[buffer::VERTEX]);

		for (std::size_t i = 0; i < this->Viewports.size(); ++i)
		{
			glViewportIndexedfv(0, &this->Viewports[i][0]);
			glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 1, 0, 0);
		}

		// Splash
		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindVertexArray(this->VertexArrayName[pipeline::SPLASH]);

		glBindProgramPipeline(PipelineName[pipeline::SPLASH]);
		glBindTextureUnit(0, TextureName[texture::COLORBUFFER]);

		glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 3, 1, 0);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_500_shader_blend_nv Test(argc, argv);
	Error += Test();

	return Error;
}
