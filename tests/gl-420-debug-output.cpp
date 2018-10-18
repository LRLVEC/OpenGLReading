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
	char const * VERT_SHADER_SOURCE("gl-420/debug-output.vert");
	char const * FRAG_SHADER_SOURCE("gl-420/debug-output.frag");

	GLsizei const VertexCount(4);
	GLsizeiptr const PositionSize = VertexCount * sizeof(glm::vec2);
	glm::vec2 const PositionData[VertexCount] =
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

	GLuint const Instances(2);

	namespace buffer
	{
		enum type
		{
			ELEMENT,
			VERTEX,
			TRANSFORM,
			MATERIAL,
			MAX
		};
	}//namespace pipeline

	GLuint PipelineName(0);
	GLuint ProgramName(0);
	GLuint VertexArrayName(0);
	GLuint BufferName[buffer::MAX] = {0, 0, 0, 0};
	GLint UniformBufferOffset(0);
	GLint UniformBlockSize(0);
	GLint UniformInstance(0);
}//namespace

class gl_420_debug_output : public test
{
public:
	gl_420_debug_output(int argc, char* argv[]) :
		test(argc, argv, "gl-420-debug-output", test::COMPATIBILITY, 4, 2)
	{}

private:
	bool initProgram()
	{
		bool Validated(true);
	
		glGenProgramPipelines(1, &PipelineName);
		glBindProgramPipeline(PipelineName);

		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, 
				"--version 420 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE,
				"--version 420 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName = glCreateProgram();
			glProgramParameteri(ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName, VertShaderName);
			glAttachShader(ProgramName, FragShaderName);
			glLinkProgram(ProgramName);

			Validated = Validated && Compiler.checkProgram(ProgramName);
		}

		if(Validated)
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);

		if(Validated)
		{
			UniformInstance = glGetUniformLocation(ProgramName, "Instance");
		}

		glBindProgramPipeline(0);

		return Validated;
	}

	bool initVertexArray()
	{
		bool Validated(true);

		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, 0, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBindVertexArray(0);

		return Validated;
	}

	bool initBuffer()
	{
		bool Validated(true);

		glGenBuffers(buffer::MAX, BufferName);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, PositionSize, PositionData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		{
			glGetActiveUniformBlockiv(
				ProgramName, 
				semantic::uniform::TRANSFORM0,
				GL_UNIFORM_BLOCK_DATA_SIZE,
				&UniformBlockSize);
			UniformBlockSize = glm::max(UniformBufferOffset, UniformBlockSize);

			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize * Instances, 0, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		{
			glm::vec4 Diffuse(1.0f, 0.5f, 0.0f, 1.0f);

			glGetActiveUniformBlockiv(
				ProgramName, 
				semantic::uniform::MATERIAL,
				GL_UNIFORM_BLOCK_DATA_SIZE,
				&UniformBlockSize);

			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::MATERIAL]);
			glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, &Diffuse[0], GL_DYNAMIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		return Validated;
	}

	bool initDebugOutput()
	{
		bool Validated(true);

		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageCallbackARB(&test::debugOutput, this);
		GLuint MessageId(4);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
		glDebugMessageControlARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB, GL_DONT_CARE, 0, NULL, GL_TRUE);
		glDebugMessageControlARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB, GL_DONT_CARE, 1, &MessageId, GL_FALSE);
		std::string Message1("Message 1");
		glDebugMessageInsertARB(
			GL_DEBUG_SOURCE_APPLICATION_ARB, 
			GL_DEBUG_TYPE_OTHER_ARB, 1, 
			GL_DEBUG_SEVERITY_MEDIUM_ARB,
			GLsizei(Message1.size()), Message1.c_str());
		std::string Message2("Message 2");
		glDebugMessageInsertARB(
			GL_DEBUG_SOURCE_THIRD_PARTY_ARB, 
			GL_DEBUG_TYPE_OTHER_ARB, 2, 
			GL_DEBUG_SEVERITY_MEDIUM_ARB,
			GLsizei(Message2.size()), Message2.c_str());
		glDebugMessageInsertARB(
			GL_DEBUG_SOURCE_APPLICATION_ARB, 
			GL_DEBUG_TYPE_OTHER_ARB, 2, 
			GL_DEBUG_SEVERITY_MEDIUM_ARB,
			-1, "Message 3");
		glDebugMessageInsertARB(
			GL_DEBUG_SOURCE_APPLICATION_ARB, 
			GL_DEBUG_TYPE_OTHER_ARB, MessageId, 
			GL_DEBUG_SEVERITY_MEDIUM_ARB,
			-1, "Message 4");

		return Validated;
	}

	bool begin()
	{
		bool Validated = true;

		glGetIntegerv(
			GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
			&UniformBufferOffset);

		if(Validated && this->checkExtension("GL_ARB_debug_output"))
			Validated = initDebugOutput();
		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();

		return Validated;
	}

	bool end()
	{
		bool Validated(true);

		glDeleteProgramPipelines(1, &PipelineName);
		glDeleteVertexArrays(1, &VertexArrayName);
		glDeleteBuffers(buffer::MAX, BufferName);
		glDeleteProgram(ProgramName);

		return Validated;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		GLsizeiptr BufferSize = sizeof(glm::mat4);

		{
			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);

			glm::byte* Pointer = (glm::byte*)glMapBufferRange(
				GL_UNIFORM_BUFFER, 0, BufferSize,
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.1f, 100.0f);

			{
				glm::mat4 View = this->view();
				glm::mat4 Model = glm::rotate(glm::mat4(1.0f), glm::pi<float>() * 0.25f, glm::vec3(0.f, 1.f, 0.f));
				glm::mat4 MVP = Projection * View * Model;

				*(glm::mat4*)Pointer = MVP;
			}
			{
				glm::mat4 View = this->view();
				glm::mat4 Model = glm::rotate(glm::mat4(1.0f), glm::pi<float>() * 0.50f + glm::pi<float>() * 0.25f, glm::vec3(0.f, 1.f, 0.f));
				glm::mat4 MVP = Projection * View * Model;

				*(glm::mat4*)(Pointer + UniformBlockSize) = MVP;
			}

			// Make sure the uniform buffer is uploaded
			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}

		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);
		glClearBufferfv(GL_UNIFORM_BUFFER, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

		glBindProgramPipeline(PipelineName);
		glBindVertexArray(VertexArrayName);
		glBindBufferRange(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM], 0, BufferSize);
		glBindBufferRange(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM1, BufferName[buffer::TRANSFORM], UniformBlockSize, BufferSize);
		glBindBufferBase(GL_TEXTURE_BUFFER, semantic::uniform::MATERIAL, BufferName[buffer::MATERIAL]);

		for(GLint i = 0; i < 2; ++i)
		{
			glProgramUniform1i(ProgramName, UniformInstance, i);
			glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLE_STRIP, ElementCount, GL_FLOAT, 0, 1, 0,-1);
		}

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_420_debug_output Test(argc, argv);
	Error += Test();

	return Error;
}


