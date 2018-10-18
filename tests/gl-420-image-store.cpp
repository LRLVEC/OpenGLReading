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
	char const * VERT_SHADER_SOURCE_SAVE("gl-420/image-store-write.vert");
	char const * FRAG_SHADER_SOURCE_SAVE("gl-420/image-store-write.frag");
	char const * VERT_SHADER_SOURCE_READ("gl-420/image-store-read.vert");
	char const * FRAG_SHADER_SOURCE_READ("gl-420/image-store-read.frag");

	namespace program
	{
		enum type
		{
			VERT_SAVE,
			FRAG_SAVE,
			VERT_READ,
			FRAG_READ,
			MAX
		};
	}//namespace program

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

class gl_420_image_store : public test
{
public:
	gl_420_image_store(int argc, char* argv[]) :
		test(argc, argv, "gl-420-image-store", test::CORE, 4, 2),
		VertexArrayName(0),
		TextureName(0)
	{}

private:
	std::array<GLuint, program::MAX> ProgramName;
	std::array<GLuint, pipeline::MAX> PipelineName;
	GLuint VertexArrayName;
	GLuint TextureName;
	glm::uvec2 ImageSize;

	bool initProgram()
	{
		bool Validated(true);

		if(Validated)
		{
			std::string VertexSourceContent = this->loadFile(getDataDirectory() + VERT_SHADER_SOURCE_READ);
			char const * VertexSourcePointer = VertexSourceContent.c_str();
			ProgramName[program::VERT_READ] = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &VertexSourcePointer);
		}

		if(Validated)
		{
			std::string FragmentSourceContent = this->loadFile(getDataDirectory() + FRAG_SHADER_SOURCE_READ);
			char const * FragmentSourcePointer = FragmentSourceContent.c_str();
			ProgramName[program::FRAG_READ] = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &FragmentSourcePointer);
		}

		if(Validated)
		{
			std::string VertexSourceContent = this->loadFile(getDataDirectory() + VERT_SHADER_SOURCE_SAVE);
			char const * VertexSourcePointer = VertexSourceContent.c_str();
			ProgramName[program::VERT_SAVE] = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &VertexSourcePointer);
		}

		if(Validated)
		{
			std::string FragmentSourceContent = this->loadFile(getDataDirectory() + FRAG_SHADER_SOURCE_SAVE);
			char const * FragmentSourcePointer = FragmentSourceContent.c_str();
			ProgramName[program::FRAG_SAVE] = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &FragmentSourcePointer);
		}

		if(Validated)
		{
			compiler Compiler;
			Validated = Validated && Compiler.checkProgram(ProgramName[program::VERT_READ]);
			Validated = Validated && Compiler.checkProgram(ProgramName[program::FRAG_READ]);
			Validated = Validated && Compiler.checkProgram(ProgramName[program::VERT_SAVE]);
			Validated = Validated && Compiler.checkProgram(ProgramName[program::FRAG_SAVE]);
		}

		if(Validated)
		{
			glGenProgramPipelines(pipeline::MAX, &PipelineName[0]);
			glUseProgramStages(PipelineName[pipeline::READ], GL_VERTEX_SHADER_BIT, ProgramName[program::VERT_READ]);
			glUseProgramStages(PipelineName[pipeline::READ], GL_FRAGMENT_SHADER_BIT, ProgramName[program::FRAG_READ]);
			glUseProgramStages(PipelineName[pipeline::SAVE], GL_VERTEX_SHADER_BIT, ProgramName[program::VERT_SAVE]);
			glUseProgramStages(PipelineName[pipeline::SAVE], GL_FRAGMENT_SHADER_BIT, ProgramName[program::FRAG_SAVE]);
		}

		return Validated;
	}

	bool initTexture()
	{
		glm::ivec2 WindowSize(this->getWindowSize());

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
		glBindTexture(GL_TEXTURE_2D, 0);

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

		this->logImplementationDependentLimit(GL_MAX_IMAGE_UNITS, "GL_MAX_IMAGE_UNITS");
		this->logImplementationDependentLimit(GL_MAX_VERTEX_IMAGE_UNIFORMS, "GL_MAX_VERTEX_IMAGE_UNIFORMS");
		this->logImplementationDependentLimit(GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS, "GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS");
		this->logImplementationDependentLimit(GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS, "GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS");
		this->logImplementationDependentLimit(GL_MAX_GEOMETRY_IMAGE_UNIFORMS, "GL_MAX_GEOMETRY_IMAGE_UNIFORMS");
		this->logImplementationDependentLimit(GL_MAX_FRAGMENT_IMAGE_UNIFORMS, "GL_MAX_FRAGMENT_IMAGE_UNIFORMS");
		this->logImplementationDependentLimit(GL_MAX_COMBINED_IMAGE_UNIFORMS, "GL_MAX_COMBINED_IMAGE_UNIFORMS");
		this->logImplementationDependentLimit(GL_MAX_ARRAY_TEXTURE_LAYERS, "GL_MAX_ARRAY_TEXTURE_LAYERS");
		this->logImplementationDependentLimit(GL_MAX_TEXTURE_IMAGE_UNITS, "GL_MAX_TEXTURE_IMAGE_UNITS");
		this->logImplementationDependentLimit(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS");
		this->logImplementationDependentLimit(GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS, "GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS");
		//this->logImplementationDependentLimit(GL_MAX_TEXTURE_UNITS, "GL_MAX_TEXTURE_UNITS");

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
		glDeleteProgram(ProgramName[program::VERT_SAVE]);
		glDeleteProgram(ProgramName[program::FRAG_SAVE]);
		glDeleteProgram(ProgramName[program::VERT_READ]);
		glDeleteProgram(ProgramName[program::FRAG_READ]);
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
			GLint Border(8);
			glEnable(GL_SCISSOR_TEST);
			glScissorIndexed(0, Border, Border, static_cast<GLsizei>(WindowSize.x - 2) * Border, static_cast<GLsizei>(WindowSize.y - 2) * Border);

			glDrawBuffer(GL_BACK);

			glBindProgramPipeline(PipelineName[pipeline::READ]);
			glBindImageTexture(semantic::image::DIFFUSE, TextureName, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
			glBindVertexArray(VertexArrayName);
			glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 3, 1, 0);

			glDisable(GL_SCISSOR_TEST);
		}

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_420_image_store Test(argc, argv);
	Error += Test();

	return Error;
}

