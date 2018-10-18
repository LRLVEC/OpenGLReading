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
	char const * VERT_SHADER_SOURCE_TRANSFORM("gl-440/transform-feedback-transform.vert");
	char const * VERT_SHADER_SOURCE_FEEDBACK("gl-440/transform-feedback-feedback.vert");
	char const * FRAG_SHADER_SOURCE_FEEDBACK("gl-440/transform-feedback-feedback.frag");

	GLsizei const VertexCount(6);
	GLsizeiptr const PositionSize = VertexCount * sizeof(glm::vec4);
	glm::vec4 const PositionData[VertexCount] =
	{
		glm::vec4(-1.0f,-1.0f, 0.0f, 1.0f),
		glm::vec4( 1.0f,-1.0f, 0.0f, 1.0f),
		glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(-1.0f,-1.0f, 0.0f, 1.0f)
	};

	namespace buffer
	{
		enum type
		{
			VERTEX,
			FEEDBACK,
			TRANSFORM,
			MAX
		};
	}//namespace buffer

	namespace program
	{
		enum type
		{
			FEEDBACK,
			TRANSFORM,
			MAX
		};
	}//namespace program

	std::vector<GLuint> BufferName(buffer::MAX);
	GLuint FeedbackName(0);

	std::vector<GLuint> PipelineName(program::MAX);
	std::vector<GLuint> ProgramName(program::MAX);
	std::vector<GLuint> VertexArrayName(program::MAX);

	glm::mat4* UniformPointer(nullptr);
}//namespace

class gl_440_transform_feedback : public test
{
public:
	gl_440_transform_feedback(int argc, char* argv[]) :
		test(argc, argv, "gl-440-transform-feedback", test::CORE, 4, 4)
	{}

private:
	bool initProgram()
	{
		bool Validated = true;

		compiler Compiler;
		GLuint VertTransformShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE_TRANSFORM, "--version 440 --profile core");
		GLuint VertFeedbackShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE_FEEDBACK, "--version 440 --profile core");
		GLuint FragFeedbackShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE_FEEDBACK, "--version 440 --profile core");

		if(Validated)
		{
			ProgramName[program::TRANSFORM] = glCreateProgram();
			glProgramParameteri(ProgramName[program::TRANSFORM], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[program::TRANSFORM], VertTransformShaderName);
			glLinkProgram(ProgramName[program::TRANSFORM]);
		}

		if(Validated)
		{
			ProgramName[program::FEEDBACK] = glCreateProgram();
			glProgramParameteri(ProgramName[program::FEEDBACK], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[program::FEEDBACK], VertFeedbackShaderName);
			glAttachShader(ProgramName[program::FEEDBACK], FragFeedbackShaderName);
			glLinkProgram(ProgramName[program::FEEDBACK]);
		}

		if(Validated)
		{
			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.checkProgram(ProgramName[program::TRANSFORM]);
			Validated = Validated && Compiler.checkProgram(ProgramName[program::FEEDBACK]);
		}

		if(Validated)
		{
			glGenProgramPipelines(program::MAX, &PipelineName[0]);
			glUseProgramStages(PipelineName[program::TRANSFORM], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName[program::TRANSFORM]);
			glUseProgramStages(PipelineName[program::FEEDBACK], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName[program::FEEDBACK]);
		}

		return Validated && this->checkError("initProgram");
	}

	bool initVertexArray()
	{
		glGenVertexArrays(program::MAX, &VertexArrayName[0]);

		glBindVertexArray(VertexArrayName[program::TRANSFORM]);
			glVertexAttribFormat(semantic::attr::POSITION, 4, GL_FLOAT, GL_FALSE, 0);
			glVertexAttribBinding(semantic::attr::POSITION, semantic::buffer::STATIC);
			glEnableVertexAttribArray(semantic::attr::POSITION);
		glBindVertexArray(0);

		glBindVertexArray(VertexArrayName[program::FEEDBACK]);
			glVertexAttribFormat(semantic::attr::POSITION, 4, GL_FLOAT, GL_FALSE, 0);
			glVertexAttribBinding(semantic::attr::POSITION, semantic::buffer::STATIC);
			glEnableVertexAttribArray(semantic::attr::POSITION);

			glVertexAttribFormat(semantic::attr::COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4));
			glVertexAttribBinding(semantic::attr::COLOR, semantic::buffer::STATIC);
			glEnableVertexAttribArray(semantic::attr::COLOR);
		glBindVertexArray(0);

		return this->checkError("initVertexArray");
	}

	bool initFeedback()
	{
		glGenTransformFeedbacks(1, &FeedbackName);
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, FeedbackName);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, BufferName[buffer::FEEDBACK]);
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

		return this->checkError("initFeedback");
	}

	bool initBuffer()
	{
		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferStorage(GL_ARRAY_BUFFER, PositionSize, PositionData, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::FEEDBACK]);
		glBufferStorage(GL_ARRAY_BUFFER, sizeof(glf::vertex_v4fc4f) * VertexCount, NULL, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferStorage(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return this->checkError("initBuffer");
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
			Validated = initFeedback();

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		UniformPointer = (glm::mat4*)glMapBufferRange(
			GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		return Validated;
	}

	bool end()
	{
		if(!UniformPointer)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glUnmapBuffer(GL_UNIFORM_BUFFER);
			UniformPointer = nullptr;
		}

		glDeleteVertexArrays(program::MAX, &VertexArrayName[0]);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteProgram(ProgramName[program::FEEDBACK]);
		glDeleteProgram(ProgramName[program::TRANSFORM]);
		glDeleteProgramPipelines(program::MAX, &PipelineName[0]);
		glDeleteTransformFeedbacks(1, &FeedbackName);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		{
			glm::mat4 Model = glm::mat4(1.0f);
			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.1f, 100.0f);
			glm::mat4 MVP = Projection * this->view() * Model;

			*UniformPointer = MVP;
		}

		// Set the display viewport
		glViewportIndexedf(0, 0.0f, 0.0f, WindowSize.x, WindowSize.y);

		// Clear color buffer
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

		// First draw, capture the attributes
		// Disable rasterisation, vertices processing only!
		glEnable(GL_RASTERIZER_DISCARD);

		glBindProgramPipeline(PipelineName[program::TRANSFORM]);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);
		glBindVertexArray(VertexArrayName[program::TRANSFORM]);
		glBindVertexBuffer(semantic::buffer::STATIC, BufferName[buffer::VERTEX], 0, sizeof(glm::vec4));

		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, FeedbackName);
		glBeginTransformFeedback(GL_TRIANGLES);
			glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
		glEndTransformFeedback();
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

		glDisable(GL_RASTERIZER_DISCARD);

		// Second draw, reuse the captured attributes
		glBindProgramPipeline(PipelineName[program::FEEDBACK]);
		glBindVertexArray(VertexArrayName[program::FEEDBACK]);
		glBindVertexBuffer(semantic::buffer::STATIC, BufferName[buffer::FEEDBACK], 0, sizeof(glf::vertex_v4fc4f));
	
		glDrawTransformFeedback(GL_TRIANGLES, FeedbackName);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_440_transform_feedback Test(argc, argv);
	Error += Test();

	return Error;
}

