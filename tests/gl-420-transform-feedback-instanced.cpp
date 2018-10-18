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
	char const * VERT_SHADER_SOURCE_TRANSFORM("gl-420/transform-stream.vert");
	char const * GEOM_SHADER_SOURCE_TRANSFORM("gl-420/transform-stream.geom");
	char const * VERT_SHADER_SOURCE_FEEDBACK("gl-420/feedback-stream.vert");
	char const * FRAG_SHADER_SOURCE_FEEDBACK("gl-420/feedback-stream.frag");

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

	namespace pipeline
	{
		enum type
		{
			TRANSFORM,
			FEEDBACK,
			MAX
		};
	}//namespace pipeline

	namespace buffer
	{
		enum type
		{
			TRANSFORM_ELEMENT,
			TRANSFORM_VERTEX,
			FEEDBACK_VERTEX,
			MAX
		};
	}//namespace buffer
}//namespace

class gl_420_transform_feedback_instanced : public test
{
public:
	gl_420_transform_feedback_instanced(int argc, char* argv[]) :
		test(argc, argv, "gl-420-transform-feedback-instanced", test::CORE, 4, 2, glm::vec2(glm::pi<float>() * 0.2f)),
		FeedbackName(0),
		Query(0),
		TransformUniformMVP(0),
		FeedbackUniformMVP(0)
	{}

private:
	std::array<GLuint, pipeline::MAX> PipelineName;
	std::array<GLuint, pipeline::MAX> ProgramName;
	std::array<GLuint, pipeline::MAX> VertexArrayName;
	std::array<GLuint, buffer::MAX> BufferName;
	GLuint FeedbackName;
	GLuint Query;
	GLint TransformUniformMVP;
	GLint FeedbackUniformMVP;

	bool initProgram()
	{
		bool Validated(true);
	
		compiler Compiler;

		// Create program
		if(Validated)
		{
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE_TRANSFORM, "--version 420 --profile core");
			GLuint GeomShaderName = Compiler.create(GL_GEOMETRY_SHADER, getDataDirectory() + GEOM_SHADER_SOURCE_TRANSFORM, "--version 420 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName[pipeline::TRANSFORM] = glCreateProgram();
			glProgramParameteri(ProgramName[pipeline::TRANSFORM], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[pipeline::TRANSFORM], VertShaderName);
			glAttachShader(ProgramName[pipeline::TRANSFORM], GeomShaderName);

			GLchar const * Strings[] = {"gl_Position", "block.Color"}; 
			glTransformFeedbackVaryings(ProgramName[pipeline::TRANSFORM], 2, Strings, GL_INTERLEAVED_ATTRIBS);
			glLinkProgram(ProgramName[pipeline::TRANSFORM]);

			Validated = Validated && Compiler.checkProgram(ProgramName[pipeline::TRANSFORM]);
		}

		// Get variables locations
		if(Validated)
		{
			TransformUniformMVP = glGetUniformLocation(ProgramName[pipeline::TRANSFORM], "MVP");
			Validated = Validated && (TransformUniformMVP >= 0);
		}

		// Create program
		if(Validated)
		{
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE_FEEDBACK, "--version 420 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE_FEEDBACK, "--version 420 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName[pipeline::FEEDBACK] = glCreateProgram();
			glProgramParameteri(ProgramName[pipeline::FEEDBACK], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(ProgramName[pipeline::FEEDBACK], VertShaderName);
			glAttachShader(ProgramName[pipeline::FEEDBACK], FragShaderName);
			glLinkProgram(ProgramName[pipeline::FEEDBACK]);
			Validated = Validated && Compiler.checkProgram(ProgramName[pipeline::FEEDBACK]);
		}

		// Get variables locations
		if(Validated)
		{
			FeedbackUniformMVP = glGetUniformLocation(ProgramName[pipeline::FEEDBACK], "MVP");
			Validated = Validated && (FeedbackUniformMVP >= 0);
		}

		if(Validated)
		{
			glGenProgramPipelines(pipeline::MAX, &PipelineName[0]);
			glUseProgramStages(PipelineName[pipeline::FEEDBACK], GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName[pipeline::FEEDBACK]);
			glUseProgramStages(PipelineName[pipeline::TRANSFORM], GL_VERTEX_SHADER_BIT | GL_GEOMETRY_SHADER_BIT, ProgramName[pipeline::TRANSFORM]);
		}

		return Validated;
	}

	bool initVertexArray()
	{
		// Build a vertex array objects
		glGenVertexArrays(pipeline::MAX, &VertexArrayName[0]);

		glBindVertexArray(VertexArrayName[pipeline::TRANSFORM]);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::TRANSFORM_VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 4, GL_FLOAT, GL_FALSE, 0, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::TRANSFORM_ELEMENT]);
		glBindVertexArray(0);

		glBindVertexArray(VertexArrayName[pipeline::FEEDBACK]);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::FEEDBACK_VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 4, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v4fc4f), 0);
			glVertexAttribPointer(semantic::attr::COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v4fc4f), BUFFER_OFFSET(sizeof(glm::vec4)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
			glEnableVertexAttribArray(semantic::attr::COLOR);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		return true;
	}

	bool initFeedback()
	{
		// Generate a buffer object
		glGenTransformFeedbacks(1, &FeedbackName);
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, FeedbackName);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, BufferName[buffer::FEEDBACK_VERTEX]); 
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
		
		return true;
	}

	bool initBuffer()
	{
		glGenBuffers(static_cast<GLsizei>(buffer::MAX), &BufferName[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::TRANSFORM_ELEMENT]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::TRANSFORM_VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::FEEDBACK_VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glf::vertex_v4fc4f) * 6, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return true;
	}

	bool begin()
	{
		bool Validated(true);

		glGenQueries(1, &Query);
		glEnable(GL_DEPTH_TEST);

		if(Validated)
			Validated = initProgram();
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initFeedback();

		return Validated;
	}

	bool end()
	{
		glDeleteProgramPipelines(pipeline::MAX, &PipelineName[0]);
		glDeleteVertexArrays(pipeline::MAX, &VertexArrayName[0]);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteProgram(ProgramName[pipeline::TRANSFORM]);
		glDeleteProgram(ProgramName[pipeline::FEEDBACK]);
		glDeleteQueries(1, &Query);
		glDeleteTransformFeedbacks(1, &FeedbackName);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * this->view() * Model;

		glProgramUniformMatrix4fv(ProgramName[pipeline::TRANSFORM], TransformUniformMVP, 1, GL_FALSE, &MVP[0][0]);
		glProgramUniformMatrix4fv(ProgramName[pipeline::FEEDBACK], FeedbackUniformMVP, 1, GL_FALSE, &MVP[0][0]);

		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);

		float Depth(1.0f);
		glClearBufferfv(GL_DEPTH, 0, &Depth);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

		// First draw, capture the attributes
		// Disable rasterisation, vertices processing only!
		glEnable(GL_RASTERIZER_DISCARD);

		glBindProgramPipeline(PipelineName[pipeline::TRANSFORM]);
		glBindVertexArray(VertexArrayName[pipeline::TRANSFORM]);

		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, FeedbackName);
		glBeginTransformFeedback(GL_TRIANGLES);
			glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_INT, NULL, 1, 0);
		glEndTransformFeedback();

		glDisable(GL_RASTERIZER_DISCARD);

		// Second draw, reuse the captured attributes
		glBindProgramPipeline(PipelineName[pipeline::FEEDBACK]);
		glBindVertexArray(VertexArrayName[pipeline::FEEDBACK]);

		glDrawTransformFeedbackStreamInstanced(GL_TRIANGLE_STRIP, FeedbackName, 0, 5);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_420_transform_feedback_instanced Test(argc, argv);
	Error += Test();

	return Error;
}



