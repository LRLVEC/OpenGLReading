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
	char const * VERT_SHADER_SOURCE_TRANSFORM("gl-400/transform-stream.vert");
	char const * GEOM_SHADER_SOURCE_TRANSFORM("gl-400/transform-stream.geom");
	char const * VERT_SHADER_SOURCE_FEEDBACK("gl-400/feedback-stream.vert");
	char const * FRAG_SHADER_SOURCE_FEEDBACK("gl-400/feedback-stream.frag");

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glm::vec4);
	glm::vec4 const VertexData[VertexCount] =
	{
		glm::vec4(-1.0f,-1.0f, 0.0f, 1.0f),
		glm::vec4( 1.0f,-1.0f, 0.0f, 1.0f),
		glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f),
		glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f)
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLuint);
	GLuint const ElementData[ElementCount] =
	{
		0, 1, 2, 
		2, 3, 0
	};

	GLuint FeedbackName(0);
	GLuint TransformProgramName(0);
	GLuint TransformArrayBufferName(0);
	GLuint TransformElementBufferName(0);
	GLuint TransformVertexArrayName(0);
	GLint TransformUniformMVP(0);
	GLuint FeedbackProgramName(0);
	GLuint FeedbackArrayBufferName(0);
	GLuint FeedbackVertexArrayName(0);
	GLuint Query(0);
}//namespace

class gl_400_transform_feadback_stream : public test
{
public:
	gl_400_transform_feadback_stream(int argc, char* argv[]) :
		test(argc, argv, "gl-410-transform-feadback-stream", test::CORE, 4, 0)
	{}

private:
	bool initProgram()
	{
		bool Validated = true;
	
		compiler Compiler;

		if(Validated)
		{
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE_TRANSFORM);
			GLuint GeomShaderName = Compiler.create(GL_GEOMETRY_SHADER, getDataDirectory() + GEOM_SHADER_SOURCE_TRANSFORM);

			TransformProgramName = glCreateProgram();
			glAttachShader(TransformProgramName, VertShaderName);
			glAttachShader(TransformProgramName, GeomShaderName);

			GLchar const * Strings[] = {"gl_Position", "block.Color"}; 
			glTransformFeedbackVaryings(TransformProgramName, 2, Strings, GL_INTERLEAVED_ATTRIBS);
			glLinkProgram(TransformProgramName);
		}

		if(Validated)
		{
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE_FEEDBACK);
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE_FEEDBACK);

			FeedbackProgramName = glCreateProgram();
			glAttachShader(FeedbackProgramName, VertShaderName);
			glAttachShader(FeedbackProgramName, FragShaderName);
			glLinkProgram(FeedbackProgramName);
		}

		if(Validated)
		{
			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.checkProgram(TransformProgramName);
			Validated = Validated && Compiler.checkProgram(FeedbackProgramName);
		}

		if(Validated)
		{
			TransformUniformMVP = glGetUniformLocation(TransformProgramName, "MVP");
			Validated = Validated && (TransformUniformMVP >= 0);
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

	bool initArrayBuffer()
	{
		glGenBuffers(1, &TransformElementBufferName);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TransformElementBufferName);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glGenBuffers(1, &TransformArrayBufferName);
		glBindBuffer(GL_ARRAY_BUFFER, TransformArrayBufferName);
		glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &FeedbackArrayBufferName);
		glBindBuffer(GL_ARRAY_BUFFER, FeedbackArrayBufferName);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glf::vertex_v4fc4f) * 6, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return this->checkError("initArrayBuffer");
	}

	bool begin()
	{
		bool Validated = true;

		glGenQueries(1, &Query);
		glPointSize(64);

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initArrayBuffer();
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
		glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.1f, 100.0f);
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * this->view() * Model;

		// Set the display viewport
		glViewport(0, 0, static_cast<GLsizei>(WindowSize.x), static_cast<GLsizei>(WindowSize.y));

		// Clear color buffer
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

		// First draw, capture the attributes
		// Disable rasterisation, vertices processing only!
		glEnable(GL_RASTERIZER_DISCARD);

		glUseProgram(TransformProgramName);
		glUniformMatrix4fv(TransformUniformMVP, 1, GL_FALSE, &MVP[0][0]);

		glBindVertexArray(TransformVertexArrayName);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TransformElementBufferName);

		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, FeedbackName);
		glBeginTransformFeedback(GL_TRIANGLES);
			glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_INT, NULL, 1, 0);
		glEndTransformFeedback();
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

		glDisable(GL_RASTERIZER_DISCARD);

		// Second draw, reuse the captured attributes
		glUseProgram(FeedbackProgramName);

		glBindVertexArray(FeedbackVertexArrayName);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDrawTransformFeedbackStream(GL_TRIANGLES, FeedbackName, 0);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_400_transform_feadback_stream Test(argc, argv);
	Error += Test();

	return Error;
}
