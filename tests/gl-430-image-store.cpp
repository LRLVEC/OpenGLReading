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
	char const * VERT_SHADER_SOURCE_SAVE("gl-430/image-store-write.vert");
	char const * FRAG_SHADER_SOURCE_SAVE("gl-430/image-store-write.frag");
	char const * VERT_SHADER_SOURCE_READ("gl-430/image-store-read.vert");
	char const * FRAG_SHADER_SOURCE_READ("gl-430/image-store-read.frag");

	namespace pipeline
	{
		enum type
		{
			READ,
			SAVE,
			MAX
		};
	}//namespace pipeline
}//namespace

class gl_430_image_store : public test
{
public:
	gl_430_image_store(int argc, char* argv[]) :
		test(argc, argv, "gl-430-image-store", test::CORE, 4, 2),
		VertexArrayName(0),
		TextureName(0),
		ImageSize(0)
	{}

private:
	std::array<GLuint, pipeline::MAX> PipelineName;
	std::array<GLuint, pipeline::MAX> ProgramName;
	GLuint VertexArrayName;
	GLuint TextureName;
	glm::uvec2 ImageSize;

	bool initProgram()
	{
		bool Validated(true);

		compiler Compiler;

		glGenProgramPipelines(pipeline::MAX, &PipelineName[0]);

		if(Validated)
		{
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE_READ, "--version 420 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE_READ, "--version 420 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName[pipeline::READ] = glCreateProgram();
			glProgramParameteri(ProgramName[pipeline::READ], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[pipeline::READ], VertShaderName);
			glAttachShader(ProgramName[pipeline::READ], FragShaderName);
			glLinkProgram(ProgramName[pipeline::READ]);
			Validated = Validated && Compiler.checkProgram(ProgramName[pipeline::READ]);
		}

		if(Validated)
			glUseProgramStages(PipelineName[pipeline::READ], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName[pipeline::READ]);

		if(Validated)
		{
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE_SAVE, "--version 420 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE_SAVE, "--version 420 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName[pipeline::SAVE] = glCreateProgram();
			glProgramParameteri(ProgramName[pipeline::SAVE], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[pipeline::SAVE], VertShaderName);
			glAttachShader(ProgramName[pipeline::SAVE], FragShaderName);
			glLinkProgram(ProgramName[pipeline::SAVE]);
			Validated = Validated && Compiler.checkProgram(ProgramName[pipeline::SAVE]);
		}

		if(Validated)
			glUseProgramStages(PipelineName[pipeline::SAVE], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName[pipeline::SAVE]);

		return Validated;
	}

	bool initTexture()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		glGenTextures(1, &TextureName);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexStorage2D(GL_TEXTURE_2D, GLint(1), GL_RGBA8, GLsizei(WindowSize.x), GLsizei(WindowSize.y));

		return true;
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
		glBindVertexArray(0);

		return true;
	}

	bool begin()
	{
		bool Validated(true);
		Validated = Validated && this->checkExtension("GL_ARB_shader_image_size");

		this->logImplementationDependentLimit(GL_MAX_TEXTURE_IMAGE_UNITS, "GL_MAX_TEXTURE_IMAGE_UNITS");

		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initVertexArray();

		return Validated;
	}

	bool end()
	{
		glDeleteTextures(1, &TextureName);
		glDeleteVertexArrays(1, &VertexArrayName);
		glDeleteProgram(ProgramName[pipeline::READ]);
		glDeleteProgram(ProgramName[pipeline::SAVE]);
		glDeleteProgramPipelines(pipeline::MAX, &PipelineName[0]);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);
		glDrawBuffer(GL_BACK);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.5f, 1.0f, 1.0f)[0]);

		// Renderer to image
		{
			glDrawBuffer(GL_NONE);

			glBindProgramPipeline(PipelineName[pipeline::SAVE]);
			glBindImageTexture(semantic::image::DIFFUSE, TextureName, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
			glBindVertexArray(VertexArrayName);
			glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 3, 1, 0);
		}

		// Read from image
		{
			glDrawBuffer(GL_BACK);

			glBindProgramPipeline(PipelineName[pipeline::READ]);
			glBindImageTexture(semantic::image::DIFFUSE, TextureName, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
			glBindVertexArray(VertexArrayName);
			glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 3, 1, 0);
		}

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_430_image_store Test(argc, argv);
	Error += Test();

	return Error;
}
