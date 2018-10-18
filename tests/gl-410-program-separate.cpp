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
	char const * VERTEX_SHADER_SOURCE("gl-410/separate.vert");
	char const * FRAGMENT_SHADER_SOURCE("gl-410/separate.frag");
	char const * TEXTURE_DIFFUSE( "kueken1-dxt5.dds");

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
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLuint);
	GLuint const ElementData[ElementCount] =
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
			MAX
		};
	}//namespace buffer

	namespace program
	{
		enum type
		{
			VERTEX,
			FRAGMENT,
			MAX
		};
	}//namespace program


	GLuint PipelineName(0);
	GLuint SeparateProgramName[program::MAX];
	GLint SeparateUniformMVP(0);
	GLint SeparateUniformDiffuse(0);
	
	GLuint UnifiedProgramName;
	GLint UnifiedUniformMVP(0);
	GLint UnifiedUniformDiffuse(0);

	GLuint BufferName[buffer::MAX];
	GLuint VertexArrayName;
	GLuint TextureName(0);
}//namespace

class gl_410_program_separate : public test
{
public:
	gl_410_program_separate(int argc, char* argv[]) :
		test(argc, argv, "gl-410-program-separate", test::CORE, 4, 1)
	{}

private:
	bool initUnifiedProgram()
	{
		bool Validated = true;

		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERTEX_SHADER_SOURCE);
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAGMENT_SHADER_SOURCE);

			UnifiedProgramName = glCreateProgram();
			glAttachShader(UnifiedProgramName, VertShaderName);
			glAttachShader(UnifiedProgramName, FragShaderName);
			glLinkProgram(UnifiedProgramName);

			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.checkProgram(UnifiedProgramName);
		}

		if(Validated)
		{
			UnifiedUniformMVP = glGetUniformLocation(UnifiedProgramName, "MVP");
			UnifiedUniformDiffuse = glGetUniformLocation(UnifiedProgramName, "Diffuse");
		}

		return Validated && this->checkError("initProgram");
	}

	bool initSeparateProgram()
	{
		bool Validated = true;

		compiler Compiler;

		if(Validated)
		{
			std::string VertexSourceContent = this->loadFile(getDataDirectory() + VERTEX_SHADER_SOURCE);
			char const * VertexSourcePointer = VertexSourceContent.c_str();
			SeparateProgramName[program::VERTEX] = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &VertexSourcePointer);
		}

		if(Validated)
		{
			std::string FragmentSourceContent = this->loadFile(getDataDirectory() + FRAGMENT_SHADER_SOURCE);
			char const * FragmentSourcePointer = FragmentSourceContent.c_str();
			SeparateProgramName[program::FRAGMENT] = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &FragmentSourcePointer);
		}

		if(Validated)
		{
			Validated = Validated && Compiler.checkProgram(SeparateProgramName[program::VERTEX]);
			Validated = Validated && Compiler.checkProgram(SeparateProgramName[program::FRAGMENT]);
		}

		if(Validated)
		{
			glGenProgramPipelines(1, &PipelineName);
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT, SeparateProgramName[program::VERTEX]);
			glUseProgramStages(PipelineName, GL_FRAGMENT_SHADER_BIT, SeparateProgramName[program::FRAGMENT]);
		}

		if(Validated)
		{
			SeparateUniformMVP = glGetUniformLocation(SeparateProgramName[program::VERTEX], "MVP");
			SeparateUniformDiffuse = glGetUniformLocation(SeparateProgramName[program::FRAGMENT], "Diffuse");
		}

		return Validated && this->checkError("initProgram");
	}

	bool initTexture()
	{
		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));

		glGenTextures(1, &TextureName);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);

		for(std::size_t Level(0); Level < Texture.levels(); ++Level)
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

	bool initVertexBuffer()
	{
		glGenBuffers(buffer::MAX, BufferName);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		return this->checkError("initArrayBuffer");
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

		return this->checkError("initVertexArray");
	}

	bool begin()
	{
		bool Validated = true;

		if(Validated)
			Validated = initSeparateProgram();
		if(Validated)
			Validated = initUnifiedProgram();
		if(Validated)
			Validated = initVertexBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initTexture();

		return Validated;
	}

	bool end()
	{
		glDeleteBuffers(buffer::MAX, BufferName);
		glDeleteVertexArrays(1, &VertexArrayName);
		glDeleteTextures(1, &TextureName);
		glDeleteProgram(SeparateProgramName[program::VERTEX]);
		glDeleteProgram(SeparateProgramName[program::FRAGMENT]);
		glDeleteProgram(UnifiedProgramName);
		glDeleteProgramPipelines(1, &PipelineName);

		return true;
	}


	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		// Compute the MVP (Model View Projection matrix)
		glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x * 0.5f / WindowSize.y, 0.1f, 100.0f);
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * this->view() * Model;

		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f)[0]);

		glBindTexture(GL_TEXTURE_2D, TextureName);
		glBindVertexArray(VertexArrayName);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);

		// Render with the separate programs
		glViewportIndexedfv(0, &glm::vec4(0, 0, WindowSize.x / 2, WindowSize.y)[0]);
		glProgramUniformMatrix4fv(SeparateProgramName[program::VERTEX], SeparateUniformMVP, 1, GL_FALSE, &MVP[0][0]);
		glProgramUniform1i(SeparateProgramName[program::FRAGMENT], SeparateUniformDiffuse, 0);
		glBindProgramPipeline(PipelineName);
			glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_INT, NULL, 1, 0);
		glBindProgramPipeline(0);

		// Render with the unified programs
		glViewportIndexedfv(0, &glm::vec4(WindowSize.x / 2, 0, WindowSize.x / 2, WindowSize.y)[0]);
		glProgramUniformMatrix4fv(UnifiedProgramName, UnifiedUniformMVP, 1, GL_FALSE, &MVP[0][0]);
		glProgramUniform1i(UnifiedProgramName, UnifiedUniformDiffuse, 0);
		glUseProgram(UnifiedProgramName);
			glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_INT, NULL, 1, 0);
		glUseProgram(0);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_410_program_separate Test(argc, argv);
	Error += Test();

	return Error;
}

