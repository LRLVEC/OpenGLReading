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
	char const * VERT_SHADER_SOURCE_TRANSFORM("gl-400/transform.vert");
	char const * VERT_SHADER_SOURCE_FEEDBACK("gl-400/feedback.vert");
	char const * FRAG_SHADER_SOURCE_FEEDBACK("gl-400/feedback.frag");

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
			TRANSFORM,
			MAX
		};
	}//namespace buffer


	GLuint FeedbackName(0);
	GLuint TransformProgramName(0);
	GLuint TransformArrayBufferName(0);
	GLuint TransformVertexArrayName(0);
	GLuint FeedbackProgramName(0);
	GLuint FeedbackArrayBufferName(0);
	GLuint FeedbackVertexArrayName(0);
	GLuint Query(0);
}//namespace

class gl_400_transform_feadback_object : public test
{
public:
	gl_400_transform_feadback_object(int argc, char* argv[]) :
		test(argc, argv, "gl-410-transform-feadback-object", test::CORE, 4, 0)
	{}

private:
	std::array<GLuint, buffer::MAX> BufferName;

	bool initProgram()
	{
		bool Validated = true;
	
		compiler Compiler;

		if(Validated)
		{
			GLuint VertexShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE_TRANSFORM);

			TransformProgramName = glCreateProgram();
			glAttachShader(TransformProgramName, VertexShaderName);

			GLchar const * Strings[] = {"gl_Position", "block.Color"}; 
			glTransformFeedbackVaryings(TransformProgramName, 2, Strings, GL_INTERLEAVED_ATTRIBS);
			glLinkProgram(TransformProgramName);
		}

		if(Validated)
		{
			GLuint VertexShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE_FEEDBACK);
			GLuint FragmentShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE_FEEDBACK);

			FeedbackProgramName = glCreateProgram();
			glAttachShader(FeedbackProgramName, VertexShaderName);
			glAttachShader(FeedbackProgramName, FragmentShaderName);
			glLinkProgram(FeedbackProgramName);
		}

		if(Validated)
		{
			glUniformBlockBinding(TransformProgramName, glGetUniformBlockIndex(TransformProgramName, "transform"), semantic::uniform::TRANSFORM0);
		}

		if(Validated)
		{
			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.checkProgram(TransformProgramName);
			Validated = Validated && Compiler.checkProgram(FeedbackProgramName);
		}

		return Validated && this->checkError("initProgram");
	}

	bool initVertexArray()
	{
		this->checkError("initVertexArray 0");

		// Build a vertex array object
		glGenVertexArrays(1, &TransformVertexArrayName);
		glBindVertexArray(TransformVertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, TransformArrayBufferName);
			glVertexAttribPointer(semantic::attr::POSITION, 4, GL_FLOAT, GL_FALSE, 0, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
		glBindVertexArray(0);

		this->checkError("initVertexArray 1");

		// Build a vertex array object
		glGenVertexArrays(1, &FeedbackVertexArrayName);
		glBindVertexArray(FeedbackVertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, FeedbackArrayBufferName);
			glVertexAttribPointer(semantic::attr::POSITION, 4, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v4fc4f), 0);
			glVertexAttribPointer(semantic::attr::COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v4fc4f), BUFFER_OFFSET(sizeof(glm::vec4)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
			glEnableVertexAttribArray(semantic::attr::COLOR);
		glBindVertexArray(0);

		return this->checkError("initVertexArray");
	}

	bool initFeedback()
	{
		// Generate a buffer object
		glGenTransformFeedbacks(1, &FeedbackName);
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, FeedbackName);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, FeedbackArrayBufferName); 
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

		return this->checkError("initFeedback");
	}

	bool initBuffer()
	{
		glGenBuffers(buffer::MAX, &BufferName[0]);

		GLint UniformBufferOffset(0);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBufferOffset);
		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glGenBuffers(1, &TransformArrayBufferName);
		glBindBuffer(GL_ARRAY_BUFFER, TransformArrayBufferName);
		glBufferData(GL_ARRAY_BUFFER, PositionSize, PositionData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &FeedbackArrayBufferName);
		glBindBuffer(GL_ARRAY_BUFFER, FeedbackArrayBufferName);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glf::vertex_v4fc4f) * VertexCount, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return this->checkError("initArrayBuffer");
	}

	bool begin()
	{
		bool Validated = true;

		glGenQueries(1, &Query);

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initFeedback();

		return Validated && this->checkError("begin");
	}

	bool end()
	{
		glDeleteVertexArrays(1, &TransformVertexArrayName);
		glDeleteBuffers(1, &TransformArrayBufferName);
		glDeleteProgram(TransformProgramName);

		glDeleteVertexArrays(1, &FeedbackVertexArrayName);
		glDeleteBuffers(1, &FeedbackArrayBufferName);
		glDeleteProgram(FeedbackProgramName);

		glDeleteQueries(1, &Query);
		glDeleteTransformFeedbacks(1, &FeedbackName);

		return this->checkError("end");
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		// Compute the MVP (Model View Projection matrix)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glm::mat4* Pointer = reinterpret_cast<glm::mat4*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.1f, 100.0f);
			glm::mat4 Model = glm::mat4(1.0f);
		
			*Pointer = Projection * this->view() * Model;

			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}

		// Set the display viewport
		glViewport(0, 0, static_cast<GLsizei>(WindowSize.x), static_cast<GLsizei>(WindowSize.y));

		// Clear color buffer
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

		// First draw, capture the attributes
		// Disable rasterisation, vertices processing only!
		glEnable(GL_RASTERIZER_DISCARD);

		glUseProgram(TransformProgramName);

		glBindVertexArray(TransformVertexArrayName);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, FeedbackName);
		glBeginTransformFeedback(GL_TRIANGLES);
			glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
		glEndTransformFeedback();
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

		glDisable(GL_RASTERIZER_DISCARD);

		// Second draw, reuse the captured attributes
		glUseProgram(FeedbackProgramName);

		glBindVertexArray(FeedbackVertexArrayName);
		glDrawTransformFeedback(GL_TRIANGLES, FeedbackName);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_400_transform_feadback_object Test(argc, argv);
	Error += Test();

	return Error;
}
