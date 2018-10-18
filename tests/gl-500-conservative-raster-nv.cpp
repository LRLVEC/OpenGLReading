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
	char const * VERT_SHADER_SOURCE_TEXTURE("gl-500/conservative-raster.vert");
	char const * FRAG_SHADER_SOURCE_TEXTURE("gl-500/conservative-raster.frag");
	char const * VERT_SHADER_SOURCE_SPLASH("gl-500/conservative-raster-blit.vert");
	char const * FRAG_SHADER_SOURCE_SPLASH("gl-500/conservative-raster-blit.frag");
	char const * TEXTURE_DIFFUSE("kueken1-dxt1.dds");

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

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			TRANSFORM,
			MATERIAL0,
			MATERIAL1,
			MAX
		};
	}//namespace buffer

	namespace texture
	{
		enum type
		{
			COLORBUFFER,
			MAX
		};
	}//namespace texture
	
	namespace pipeline
	{
		enum type
		{
			TEXTURE,
			SPLASH,
			MAX
		};
	}//namespace pipeline

	GLuint FramebufferName(0);
	std::vector<GLuint> ProgramName(pipeline::MAX);
	std::vector<GLuint> VertexArrayName(pipeline::MAX);
	std::vector<GLuint> BufferName(buffer::MAX);
	std::vector<GLuint> TextureName(texture::MAX);
	std::vector<GLuint> PipelineName(pipeline::MAX);
}//namespace

class gl_500_conservative_raster_nv : public test
{
public:
	gl_500_conservative_raster_nv(int argc, char* argv[]) :
		test(argc, argv, "gl-500-conservative-raster-nv", test::CORE, 4, 5, glm::vec2(glm::pi<float>() * 0.25f))
	{}

private:
	bool initProgram()
	{
		bool Validated(true);

		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE_TEXTURE, "--version 450 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE_TEXTURE, "--version 450 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName[pipeline::TEXTURE] = glCreateProgram();
			glProgramParameteri(ProgramName[pipeline::TEXTURE], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[pipeline::TEXTURE], VertShaderName);
			glAttachShader(ProgramName[pipeline::TEXTURE], FragShaderName);
			glLinkProgram(ProgramName[pipeline::TEXTURE]);

			Validated = Validated && Compiler.checkProgram(ProgramName[pipeline::TEXTURE]);
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
			glCreateProgramPipelines(pipeline::MAX, &PipelineName[0]);
			glUseProgramStages(PipelineName[pipeline::TEXTURE], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName[pipeline::TEXTURE]);
			glUseProgramStages(PipelineName[pipeline::SPLASH], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName[pipeline::SPLASH]);
		}

		return Validated;
	}

	bool initBuffer()
	{
		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glm::vec4 Material0(1.0f, 0.5f, 0.0f, 1.0f);
		glm::vec4 Material1(0.0f, 0.5f, 1.0f, 1.0f);

		glCreateBuffers(buffer::MAX, &BufferName[0]);
		glNamedBufferData(BufferName[buffer::ELEMENT], ElementSize, ElementData, GL_STATIC_DRAW);
		glNamedBufferData(BufferName[buffer::VERTEX], VertexSize, VertexData, GL_STATIC_DRAW);
		glNamedBufferData(BufferName[buffer::TRANSFORM], UniformBlockSize, nullptr, GL_DYNAMIC_DRAW);
		glNamedBufferData(BufferName[buffer::MATERIAL0], sizeof(glm::vec4), &Material0[0], GL_STATIC_DRAW);
		glNamedBufferData(BufferName[buffer::MATERIAL1], sizeof(glm::vec4), &Material1[0], GL_STATIC_DRAW);

		return true;
	}

	bool initTexture()
	{
		bool Validated(true);

		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));
		assert(!Texture.empty());

		glm::uvec2 FramebufferSize(this->getWindowSize() / 16u);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glCreateTextures(GL_TEXTURE_2D, texture::MAX, &TextureName[0]);
		glTextureParameteri(TextureName[0], GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(TextureName[0], GL_TEXTURE_MAX_LEVEL, 0);
		glTextureParameteri(TextureName[0], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(TextureName[0], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureStorage2D(TextureName[0], GLint(1), GL_RGBA8, GLsizei(FramebufferSize.x), GLsizei(FramebufferSize.y));

		return Validated;
	}

	bool initVertexArray()
	{
		bool Validated(true);

		glCreateVertexArrays(pipeline::MAX, &VertexArrayName[0]);
		
		glVertexArrayAttribBinding(VertexArrayName[0], semantic::attr::POSITION, 0);
		glVertexArrayAttribFormat(VertexArrayName[0], semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, 0);
		glEnableVertexArrayAttrib(VertexArrayName[0], semantic::attr::POSITION);

		glVertexArrayAttribBinding(VertexArrayName[0], semantic::attr::TEXCOORD, 0);
		glVertexArrayAttribFormat(VertexArrayName[0], semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2));
		glEnableVertexArrayAttrib(VertexArrayName[0], semantic::attr::TEXCOORD);

		glVertexArrayElementBuffer(VertexArrayName[0], BufferName[buffer::ELEMENT]);
		glVertexArrayVertexBuffer(VertexArrayName[0], 0, BufferName[buffer::VERTEX], 0, sizeof(glf::vertex_v2fv2f));

		return Validated;
	}

	bool initFramebuffer()
	{
		bool Validated(true);

		glCreateFramebuffers(1, &FramebufferName);
		glNamedFramebufferTexture(FramebufferName, GL_COLOR_ATTACHMENT0, TextureName[texture::COLORBUFFER], 0);

		return this->checkFramebuffer(FramebufferName);
	}

	bool begin()
	{
		bool Validated(true);

		caps Caps(caps::CORE);

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

		GLint SubPixelBiasX = 0;
		GLint SubPixelBiasY = 0;
		GLint MaxSubPixelBias = 0;
		glGetIntegerv(GL_SUBPIXEL_PRECISION_BIAS_X_BITS_NV, &SubPixelBiasX);
		glGetIntegerv(GL_SUBPIXEL_PRECISION_BIAS_Y_BITS_NV, &SubPixelBiasY);
		glGetIntegerv(GL_MAX_SUBPIXEL_PRECISION_BIAS_BITS_NV, &MaxSubPixelBias);

		return Validated;
	}

	bool end()
	{
		glDeleteProgramPipelines(pipeline::MAX, &PipelineName[0]);
		glDeleteProgram(ProgramName[pipeline::SPLASH]);
		glDeleteProgram(ProgramName[pipeline::TEXTURE]);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteFramebuffers(1, &FramebufferName);
		glDeleteTextures(texture::MAX, &TextureName[0]);
		glDeleteVertexArrays(pipeline::MAX, &VertexArrayName[0]);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		{
			glm::mat4* Pointer = reinterpret_cast<glm::mat4*>(glMapNamedBufferRange(BufferName[buffer::TRANSFORM],
				0, sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

			//glm::mat4 Projection = glm::perspectiveFov(glm::pi<float>() * 0.25f, 640.f, 480.f, 0.1f, 100.0f);
			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.1f, 100.0f);
			glm::mat4 Model = glm::mat4(1.0f);
		
			*Pointer = Projection * this->view() * Model;

			// Make sure the uniform buffer is uploaded
			glUnmapNamedBuffer(BufferName[buffer::TRANSFORM]);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glViewportIndexedf(0, 0, 0, WindowSize.x / 16.0f, WindowSize.y / 16.0f);

		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

		// Bind rendering objects
		glViewportIndexedf(0, 0, 0, WindowSize.x / 16.0f, WindowSize.y / 16.0f);
		glBindProgramPipeline(PipelineName[pipeline::TEXTURE]);
		glBindVertexArray(VertexArrayName[pipeline::TEXTURE]);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::MATERIAL, BufferName[buffer::MATERIAL0]);

		//glSubpixelPrecisionBiasNV(-8, -8);
		glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
		glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 1, 0, 0);

		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::MATERIAL, BufferName[buffer::MATERIAL1]);

		glDisable(GL_CONSERVATIVE_RASTERIZATION_NV);
		glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 1, 0, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);

		glBindProgramPipeline(PipelineName[pipeline::SPLASH]);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(VertexArrayName[pipeline::SPLASH]);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::COLORBUFFER]);

		glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 3, 1, 0);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_500_conservative_raster_nv Test(argc, argv);
	Error += Test();

	return Error;
}

