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
	char const * SAMPLE_VERT_SHADER("gl-440/interface-matching.vert");
	char const * SAMPLE_CONT_SHADER("gl-440/interface-matching.cont");
	char const * SAMPLE_EVAL_SHADER("gl-440/interface-matching.eval");
	char const * SAMPLE_GEOM_SHADER("gl-440/interface-matching.geom");
	char const * SAMPLE_FRAG_SHADER("gl-440/interface-matching.frag");

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fc4d);
	glf::vertex_v2fc4d const VertexData[VertexCount] =
	{
		glf::vertex_v2fc4d(glm::vec2(-1.0f,-1.0f), glm::dvec4(1.0f, 0.0f, 0.0f, 1.0f)),
		glf::vertex_v2fc4d(glm::vec2( 1.0f,-1.0f), glm::dvec4(1.0f, 1.0f, 0.0f, 1.0f)),
		glf::vertex_v2fc4d(glm::vec2( 1.0f, 1.0f), glm::dvec4(0.0f, 1.0f, 0.0f, 1.0f)),
		glf::vertex_v2fc4d(glm::vec2(-1.0f, 1.0f), glm::dvec4(0.0f, 0.0f, 1.0f, 1.0f))
	};

	namespace program
	{
		enum type
		{
			VERT,
			FRAG,
			MAX
		};
	}//namespace program

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			TRANSFORM,
			MAX
		};
	}//namespace buffer
}//namespace

class gl_440_interface_matching : public test
{
public:
	gl_440_interface_matching(int argc, char* argv[]) :
		test(argc, argv, "gl-440-interface-matching", test::CORE, 4, 4),
		PipelineName(0),
		VertexArrayName(0)
	{}

private:
	GLuint PipelineName;
	GLuint VertexArrayName;
	std::array<GLuint, program::MAX> ProgramName;
	std::array<GLuint, buffer::MAX> BufferName;

	bool initMax()
	{
		GLint MaxVertexAttribs(0);
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &MaxVertexAttribs);
		//GL_MAX_DRAW_BUFFERS 8
		//GL_MAX_COLOR_ATTACHMENTS 8
		GLint MaxVertexOutput(0);
		glGetIntegerv(GL_MAX_VERTEX_OUTPUT_COMPONENTS, &MaxVertexOutput);
		GLint MaxControlInput(0);
		glGetIntegerv(GL_MAX_TESS_CONTROL_INPUT_COMPONENTS, &MaxControlInput);
		GLint MaxControlOutput(0);
		glGetIntegerv(GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS, &MaxControlOutput);
		GLint MaxControlTotalOutput(0);
		glGetIntegerv(GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS, &MaxControlTotalOutput);
		GLint MaxEvaluationInput(0);
		glGetIntegerv(GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS, &MaxEvaluationInput);
		GLint MaxEvaluationOutput(0);
		glGetIntegerv(GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS, &MaxEvaluationOutput);
		GLint MaxGeometryInput(0);
		glGetIntegerv(GL_MAX_GEOMETRY_INPUT_COMPONENTS, &MaxGeometryInput);	
		GLint MaxGeometryOutput(0);
		glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS, &MaxGeometryOutput);	
		GLint MaxFragmentInput(0);
		glGetIntegerv(GL_MAX_FRAGMENT_INPUT_COMPONENTS, &MaxFragmentInput);

		return true;
	}

	bool validate(GLuint const & ProgramName)
	{
		bool Error = false;

		// Pipeline object validation
		{
			GLint Status(0);
			GLint LengthMax(0);
			glValidateProgramPipeline(PipelineName);
			glGetProgramPipelineiv(PipelineName, GL_VALIDATE_STATUS, &Status);
			glGetProgramPipelineiv(PipelineName, GL_INFO_LOG_LENGTH, &LengthMax);

			GLsizei LengthQuery(0);
			std::vector<GLchar> InfoLog(LengthMax + 1, '\0');
			glGetProgramPipelineInfoLog(PipelineName, GLsizei(InfoLog.size()), &LengthQuery, &InfoLog[0]);

			glDebugMessageInsertARB(
				GL_DEBUG_SOURCE_APPLICATION_ARB, 
				GL_DEBUG_TYPE_OTHER_ARB, 76,
				GL_DEBUG_SEVERITY_LOW_ARB,
				LengthQuery, 
				&InfoLog[0]);
		}

		GLint ActiveAttributeMaxLength(0);
		GLint ActiveAttribute(0);
		glGetProgramiv(ProgramName, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &ActiveAttributeMaxLength);
		glGetProgramiv(ProgramName, GL_ACTIVE_ATTRIBUTES, &ActiveAttribute);

		GLsizei AttribLength(0);
		GLint AttribSize(0);
		GLenum AttribType(0);
		std::vector<GLchar> AttribName(ActiveAttributeMaxLength, '\0');

		for(GLint i = 0; i < ActiveAttribute; ++i)
		{
			GLenum Props[3] = {GL_TYPE, GL_ARRAY_SIZE, GL_LOCATION};
			GLint Params[3];
			GLsizei Length(0);
			glGetProgramResourceiv(ProgramName, GL_PROGRAM_INPUT, i, 3, Props, 3, &Length, Params);

			vertexattrib VertexAttrib;
			glGetActiveAttrib(ProgramName,
 				GLuint(i),
 				GLsizei(ActiveAttributeMaxLength),
 				&AttribLength,
 				&AttribSize,
 				&AttribType,
 				&AttribName[0]);

			std::string NameString;
			NameString.insert(NameString.begin(), AttribName.begin(), AttribName.end());
			std::vector<GLchar> NameSwap(ActiveAttributeMaxLength, '\0');
			std::swap(AttribName, NameSwap);

			GLint AttribLocation = glGetAttribLocation(ProgramName, NameString.c_str());

			glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &VertexAttrib.Enabled);
			glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &VertexAttrib.Binding);
			glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_SIZE, &VertexAttrib.Size);
			glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &VertexAttrib.Stride);
			glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_TYPE, &VertexAttrib.Type);
			glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &VertexAttrib.Normalized);
			glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_INTEGER, &VertexAttrib.Integer);
			glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_DIVISOR, &VertexAttrib.Divisor);
			glGetVertexAttribiv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_LONG, &VertexAttrib.Long);
			glGetVertexAttribPointerv(AttribLocation, GL_VERTEX_ATTRIB_ARRAY_POINTER, &VertexAttrib.Pointer);

			if(VertexAttrib.Integer == GL_TRUE)
			{
				if(!(
					VertexAttrib.Type == GL_INT ||  
					VertexAttrib.Type == GL_INT_VEC2 || 
					VertexAttrib.Type == GL_INT_VEC3 || 
					VertexAttrib.Type == GL_INT_VEC4 || 
					VertexAttrib.Type == GL_UNSIGNED_INT || 
					VertexAttrib.Type == GL_UNSIGNED_INT_VEC2 || 
					VertexAttrib.Type == GL_UNSIGNED_INT_VEC3 || 
					VertexAttrib.Type == GL_UNSIGNED_INT_VEC4))
					return true;

				if(!(
					VertexAttrib.Type == GL_BYTE || 
					VertexAttrib.Type == GL_UNSIGNED_BYTE || 
					VertexAttrib.Type == GL_SHORT || 
					VertexAttrib.Type == GL_UNSIGNED_SHORT || 
					VertexAttrib.Type == GL_INT || 
					VertexAttrib.Type == GL_UNSIGNED_INT))
					return true;

				//if(AttribSize > 1)
				//GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT, GL_FLOAT, and GL_DOUBLE
			}
			else if(VertexAttrib.Long == GL_TRUE) 
			{
				if(!(
					VertexAttrib.Type == GL_DOUBLE || 
					VertexAttrib.Type == GL_DOUBLE_VEC2 || 
					VertexAttrib.Type == GL_DOUBLE_VEC3 || 
					VertexAttrib.Type == GL_DOUBLE_VEC4 || 
					VertexAttrib.Type == GL_DOUBLE_MAT2 || 
					VertexAttrib.Type == GL_DOUBLE_MAT3 || 
					VertexAttrib.Type == GL_DOUBLE_MAT4 || 
					VertexAttrib.Type == GL_DOUBLE_MAT2x3 || 
					VertexAttrib.Type == GL_DOUBLE_MAT2x4 || 
					VertexAttrib.Type == GL_DOUBLE_MAT3x2 ||
					VertexAttrib.Type == GL_DOUBLE_MAT3x4 || 
					VertexAttrib.Type == GL_DOUBLE_MAT4x2 || 
					VertexAttrib.Type == GL_DOUBLE_MAT4x3))
					return true;
			}
			else// if((VertexAttrib.Normalized == GL_TRUE) || (GL_VERTEX_ATTRIB_ARRAY_FLOAT == GL_TRUE))
			{
				if(!(
					VertexAttrib.Type == GL_FLOAT ||  
					VertexAttrib.Type == GL_FLOAT_VEC2 || 
					VertexAttrib.Type == GL_FLOAT_VEC3 || 
					VertexAttrib.Type == GL_FLOAT_VEC4 || 
					VertexAttrib.Type == GL_FLOAT_MAT2 || 
					VertexAttrib.Type == GL_FLOAT_MAT3 || 
					VertexAttrib.Type == GL_FLOAT_MAT4 || 
					VertexAttrib.Type == GL_FLOAT_MAT2x3 || 
					VertexAttrib.Type == GL_FLOAT_MAT2x4 || 
					VertexAttrib.Type == GL_FLOAT_MAT3x2 || 
					VertexAttrib.Type == GL_FLOAT_MAT3x4 || 
					VertexAttrib.Type == GL_FLOAT_MAT4x2 || 
					VertexAttrib.Type == GL_FLOAT_MAT4x3))
					return true;

				// It could be any vertex array attribute type
			}

			printf("glGetActiveAttrib(\n\t%d, \n\t%d, \n\t%d, \n\t%d, \n\t%d, \n\t%s)\n", 
				i, AttribLocation, AttribLength, AttribSize, AttribType, NameString.c_str());
		}

		return Error;
	}

	bool initBuffer()
	{
		glGenBuffers(buffer::MAX, &BufferName[0]);

		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferStorage(GL_ARRAY_BUFFER, VertexSize, VertexData, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLint UniformBufferOffset(0);

		glGetIntegerv(
			GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
			&UniformBufferOffset);

		GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferStorage(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return true;
	}

	bool initProgram()
	{
		bool Validated = true;

		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + SAMPLE_VERT_SHADER, "--version 440 --profile core");
			GLuint ContShaderName = Compiler.create(GL_TESS_CONTROL_SHADER, getDataDirectory() + SAMPLE_CONT_SHADER, "--version 440 --profile core");
			GLuint EvalShaderName = Compiler.create(GL_TESS_EVALUATION_SHADER, getDataDirectory() + SAMPLE_EVAL_SHADER, "--version 440 --profile core");
			GLuint GeomShaderName = Compiler.create(GL_GEOMETRY_SHADER, getDataDirectory() + SAMPLE_GEOM_SHADER, "--version 440 --profile core");
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + SAMPLE_FRAG_SHADER, "--version 440 --profile core");
			Validated = Validated && Compiler.check();

			ProgramName[program::VERT] = glCreateProgram();
			ProgramName[program::FRAG] = glCreateProgram();
			glProgramParameteri(ProgramName[program::VERT], GL_PROGRAM_SEPARABLE, GL_TRUE);
			glProgramParameteri(ProgramName[program::FRAG], GL_PROGRAM_SEPARABLE, GL_TRUE);

			glAttachShader(ProgramName[program::VERT], VertShaderName);
			glAttachShader(ProgramName[program::VERT], ContShaderName);
			glAttachShader(ProgramName[program::VERT], EvalShaderName);
			glAttachShader(ProgramName[program::VERT], GeomShaderName);
			glLinkProgram(ProgramName[program::VERT]);

			glAttachShader(ProgramName[program::FRAG], FragShaderName);
			glLinkProgram(ProgramName[program::FRAG]);

			Validated = Validated && Compiler.checkProgram(ProgramName[program::VERT]);
			Validated = Validated && Compiler.checkProgram(ProgramName[program::FRAG]);
		}

		if(Validated)
		{
			glGenProgramPipelines(1, &PipelineName);
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_TESS_CONTROL_SHADER_BIT | GL_TESS_EVALUATION_SHADER_BIT | GL_GEOMETRY_SHADER_BIT, ProgramName[program::VERT]);
			glUseProgramStages(PipelineName, GL_FRAGMENT_SHADER_BIT, ProgramName[program::FRAG]);
		}

		return Validated;
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION + 0, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fc4d), BUFFER_OFFSET(0));
			glVertexAttribPointer(semantic::attr::POSITION + 1, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fc4d), BUFFER_OFFSET(0));
			glVertexAttribLPointer(semantic::attr::COLOR, 4, GL_DOUBLE, sizeof(glf::vertex_v2fc4d), BUFFER_OFFSET(sizeof(glm::vec2)));
			//glVertexAttribLPointer(semantic::attr::COLOR, 4, GL_DOUBLE, sizeof(glf::vertex_v2fc4d), BUFFER_OFFSET(sizeof(glm::vec2)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION + 0);
			glEnableVertexAttribArray(semantic::attr::POSITION + 1);
			glEnableVertexAttribArray(semantic::attr::COLOR);
		glBindVertexArray(0);

		/*
		std::vector<glf::vertexattrib> Valid(16); 
		Valid[semantic::attr::POSITION + 0] = glf::vertexattrib(GL_TRUE, 2, sizeof(glf::vertex_v2fc4d), GL_FLOAT, GL_FALSE, GL_FALSE, GL_FALSE, 0, NULL);
		Valid[semantic::attr::POSITION + 1] = glf::vertexattrib(GL_TRUE, 2, sizeof(glf::vertex_v2fc4d), GL_FLOAT, GL_FALSE, GL_FALSE, GL_FALSE, 0, NULL);
		Valid[semantic::attr::COLOR] = glf::vertexattrib(GL_TRUE, 4, sizeof(glf::vertex_v2fc4d), GL_DOUBLE, GL_FALSE, GL_FALSE, GL_FALSE, 0, BUFFER_OFFSET(sizeof(glm::vec2)));

		// TODO
		//glf::validateVAO(VertexArrayName, Valid);
		*/

		return true;
	}

	bool begin()
	{
		bool Validated(true);

		if(Validated)
			Validated = initMax();;
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
		glDeleteVertexArrays(1, &VertexArrayName);
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		for(std::size_t i = 0; i < program::MAX; ++i)
			glDeleteProgram(ProgramName[i]);
		glDeleteProgramPipelines(1, &PipelineName);

		return true;
	}

	bool render()
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glm::vec2 WindowSize(this->getWindowSize());
		{
			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(
				GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.1f, 100.0f);
			*Pointer = Projection * this->view() * glm::mat4(1.0f);

			// Make sure the uniform buffer is uploaded
			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}

		glViewportIndexedfv(0, &glm::vec4(0, 0, WindowSize)[0]);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f)[0]);

		glBindProgramPipeline(PipelineName);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);
		glBindVertexArray(VertexArrayName);
		glPatchParameteri(GL_PATCH_VERTICES, VertexCount);

		assert(!validate(ProgramName[program::VERT]));
		glDrawArraysInstancedBaseInstance(GL_PATCHES, 0, VertexCount, 1, 0);

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_440_interface_matching Test(argc, argv);
	Error += Test();

	return Error;
}


