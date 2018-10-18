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
	char const * VERTEX_SHADER_SOURCE("gl-430/texture-buffer.vert");
	char const * FRAGMENT_SHADER_SOURCE("gl-430/texture-buffer.frag");

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glm::vec2);
	glm::vec2 const VertexData[VertexCount] = 
	{
		glm::vec2(-1.0f,-1.0f), 
		glm::vec2( 1.0f,-1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2(-1.0f, 1.0f)
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] = 
	{
		0, 1, 2, 
		2, 3, 0
	};

	enum buffer_type
	{
		BUFFER_VERTEX,
		BUFFER_ELEMENT,
		BUFFER_DISPLACEMENT,
		BUFFER_DIFFUSE,
		BUFFER_TRANSFORM,
		BUFFER_MAX
	};

	enum texture_type
	{
		TEXTURE_DISPLACEMENT,
		TEXTURE_DIFFUSE,
		TEXTURE_MAX
	};
}//namespace

class gl_430_texture_buffer : public test
{
public:
	gl_430_texture_buffer(int argc, char* argv[]) :
		test(argc, argv, "gl-430-texture-buffer", test::CORE, 4, 2, glm::uvec2(640, 480), glm::vec2(glm::pi<float>() * 0.2f)),
		VertexArrayName(0),
		ProgramName(0),
		UniformMVP(0)
	{}

private:
	std::array<GLuint, BUFFER_MAX> BufferName;
	std::array<GLuint, TEXTURE_MAX> TextureName;
	GLuint VertexArrayName;
	GLuint ProgramName;
	GLint UniformMVP;

	bool initTest()
	{
		bool Validated = true;
		glEnable(GL_DEPTH_TEST);

		return Validated && this->checkError("initTest");
	}

	bool initProgram()
	{
		bool Validated = true;
	
		// Create program
		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERTEX_SHADER_SOURCE, "--version 430 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAGMENT_SHADER_SOURCE, "--version 430 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);
			glLinkProgram(ProgramName);
			Validated = Compiler.checkProgram(ProgramName);
		}

		// Get variables locations
		if(Validated)
		{
			UniformMVP = glGetUniformLocation(ProgramName, "MVP");
		}

		return Validated;
	}

	bool initBuffer()
	{
		glm::vec3 Displacement[5] = 
		{
			glm::vec3( 0.1f, 0.3f,-1.0f), 
			glm::vec3(-0.5f, 0.0f,-0.5f),
			glm::vec3(-0.2f,-0.2f, 0.0f),
			glm::vec3( 0.3f, 0.2f, 0.5f),
			glm::vec3( 0.1f,-0.3f, 1.0f)
		};

		glm::vec3 Diffuse[5] = 
		{
			glm::vec3(1.0f, 0.0f, 0.0f), 
			glm::vec3(1.0f, 0.5f, 0.0f),
			glm::vec3(1.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f)
		};	

		GLint TextureBufferOffsetAlignment(0);
		glGetIntegerv(GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT, &TextureBufferOffsetAlignment);

		int DisplacementSize = sizeof(Displacement);
		int DiffuseSize = sizeof(Diffuse);
		int DisplacementMultiple = glm::ceilMultiple(int(sizeof(Displacement)), int(TextureBufferOffsetAlignment));
		int DiffuseMultiple = glm::ceilMultiple(int(sizeof(Diffuse)), int(TextureBufferOffsetAlignment));

		glGenBuffers(BUFFER_MAX, &BufferName[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[BUFFER_ELEMENT]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[BUFFER_VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_TEXTURE_BUFFER, BufferName[BUFFER_DISPLACEMENT]);
		glBufferData(GL_TEXTURE_BUFFER, TextureBufferOffsetAlignment + DisplacementMultiple, 0, GL_STATIC_DRAW);
		glBufferSubData(GL_TEXTURE_BUFFER, TextureBufferOffsetAlignment, sizeof(Displacement), Displacement);
	/*
		void * PointerDisplacement = glMapBufferRange(GL_TEXTURE_BUFFER,
			0, sizeof(Displacement), GL_MAP_WRITE_BIT );
		memcpy(PointerDisplacement, &Displacement[0], sizeof(Displacement));
		glUnmapBuffer(GL_TEXTURE_BUFFER);
	*/
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		glBindBuffer(GL_TEXTURE_BUFFER, BufferName[BUFFER_DIFFUSE]);
		glBufferData(GL_TEXTURE_BUFFER, TextureBufferOffsetAlignment + DiffuseMultiple, 0, GL_STATIC_DRAW);
		glBufferSubData(GL_TEXTURE_BUFFER, TextureBufferOffsetAlignment, sizeof(Diffuse), Diffuse);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[BUFFER_TRANSFORM]);
		glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return true;
	}

	bool initTexture()
	{
		GLint TextureBufferOffsetAlignment(0);
		glGetIntegerv(GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT, &TextureBufferOffsetAlignment);

		glGenTextures(TEXTURE_MAX, &TextureName[0]);

		glBindTexture(GL_TEXTURE_BUFFER, TextureName[TEXTURE_DISPLACEMENT]);
		glTexBufferRange(GL_TEXTURE_BUFFER, GL_RGB32F, BufferName[BUFFER_DISPLACEMENT], TextureBufferOffsetAlignment, sizeof(glm::vec3) * 5);
		glBindTexture(GL_TEXTURE_BUFFER, 0);

		glBindTexture(GL_TEXTURE_BUFFER, TextureName[TEXTURE_DIFFUSE]);
		glTexBufferRange(GL_TEXTURE_BUFFER, GL_RGB32F, BufferName[BUFFER_DIFFUSE], TextureBufferOffsetAlignment, sizeof(glm::vec3) * 5);
		glBindTexture(GL_TEXTURE_BUFFER, 0);	

		return true;
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[BUFFER_VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, 0, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
		glBindVertexArray(0);

		return true;
	}

	bool begin()
	{
		bool Validated(true);
	
		Validated = Validated && this->checkExtension("GL_ARB_texture_buffer_range");

		if(Validated)
			Validated = initTest();
		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initTexture();
		if(Validated)
			Validated = initVertexArray();

		return Validated;
	}

	bool end()
	{
		glDeleteTextures(TEXTURE_MAX, &TextureName[0]);
		glDeleteBuffers(BUFFER_MAX, &BufferName[0]);
		glDeleteProgram(ProgramName);
		glDeleteVertexArrays(1, &VertexArrayName);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		{
			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[BUFFER_TRANSFORM]);
			glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

			glm::mat4 Projection = glm::perspectiveFov(glm::pi<float>() * 0.25f, WindowSize.x, WindowSize.y, 0.1f, 100.0f);
			glm::mat4 Model = glm::mat4(1.0f);
			*Pointer = Projection * this->view() * Model;

			// Make sure the uniform buffer is uploaded
			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}

		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);

		float Depth(1.0f);
		glClearBufferfv(GL_DEPTH, 0, &Depth);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

		glUseProgram(ProgramName);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_BUFFER, TextureName[TEXTURE_DISPLACEMENT]);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, TextureName[TEXTURE_DIFFUSE]);

		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[BUFFER_TRANSFORM]);

		glBindVertexArray(VertexArrayName);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[BUFFER_ELEMENT]);
		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, nullptr, 5, 0);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_430_texture_buffer Test(argc, argv);
	Error += Test();

	return Error;
}

