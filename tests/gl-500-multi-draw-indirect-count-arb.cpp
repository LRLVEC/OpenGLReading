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
	char const * VERT_SHADER_SOURCE("gl-500/multi-draw-indirect-count.vert");
	char const * FRAG_SHADER_SOURCE("gl-500/multi-draw-indirect-count.frag");
	char const * TEXTURE_DIFFUSE("kueken1-bgr8.dds");

	GLsizei const ElementCount(15);
	GLsizeiptr const ElementSize = ElementCount * sizeof(glm::uint16);
	glm::uint16 const ElementData[ElementCount] =
	{
		0, 1, 2,
		0, 2, 3,
		0, 1, 2,
		0, 1, 2,
		0, 2, 3
	};

	GLsizei const VertexCount(11);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2(1.0f, -1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f)),

		glf::vertex_v2fv2f(glm::vec2(-0.5f, -1.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2(1.5f, -1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2(0.5f, 1.0f), glm::vec2(1.0f, 0.0f)),

		glf::vertex_v2fv2f(glm::vec2(-0.5f, -1.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2(0.5f, -1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2(1.5f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.5f, 1.0f), glm::vec2(0.0f, 0.0f))
	};

	GLsizei const DrawDataCount(3);
	GLsizeiptr const DrawSize = DrawDataCount * sizeof(glm::uint);
	glm::uint const DrawIDData[DrawDataCount] =
	{
		0, 1, 2
	};

	GLsizei const IndirectBufferCount(3);

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			DRAW_ID,
			TRANSFORM,
			INDIRECT,
			PARAMETER,
			VERTEX_INDIRECTION,
			MAX
		};
	}//namespace buffer

	namespace texture
	{
		enum type
		{
			TEXTURE_A,
			TEXTURE_B,
			TEXTURE_C,
			MAX
		};
	}//namespace texture
}//namespace

class gl_500_multi_draw_indirect_count : public test
{
public:
	gl_500_multi_draw_indirect_count(int argc, char* argv[]) :
		test(argc, argv, "gl-500-multi-draw-indirect-count-arb", test::CORE, 4, 5, glm::uvec2(640, 480),
			glm::vec2(-glm::pi<float>() * 0.2f, glm::pi<float>() * 0.2f)),
		VertexArrayName(0),
		PipelineName(0),
		ProgramName(0),
		UniformArrayStride(256)
	{}

private:
	std::array<GLuint, buffer::MAX> BufferName;
	std::array<GLuint, texture::MAX> TextureName;
	std::array<glm::vec4, IndirectBufferCount> Viewport;
	GLuint VertexArrayName;
	GLuint PipelineName;
	GLuint ProgramName;
	GLint UniformArrayStride;

	bool initProgram()
	{
		bool Validated(true);
	
		compiler Compiler;
		GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 420 --profile core");
		GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 420 --profile core");
		Validated = Validated && Compiler.check();

		ProgramName = glCreateProgram();
		glProgramParameteri(ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
		glAttachShader(ProgramName, VertShaderName);
		glAttachShader(ProgramName, FragShaderName);
		glLinkProgram(ProgramName);
		Validated = Validated && Compiler.checkProgram(ProgramName);

		GLint ActiveUniform(0);
		glGetProgramiv(ProgramName, GL_ACTIVE_UNIFORMS, &ActiveUniform);

		for (GLuint i = 0; i < static_cast<GLuint>(ActiveUniform); ++i)
		{
			char Name[128];
			memset(Name, '\0', sizeof(Name));
			GLsizei Length(0);

			glGetActiveUniformName(ProgramName, i, GLsizei(sizeof(Name)), &Length, Name);

			std::string StringName(Name);

			if(StringName == std::string("transform.MVP[0]"))
				glGetActiveUniformsiv(ProgramName, 1, &i, GL_UNIFORM_ARRAY_STRIDE, &UniformArrayStride);
		}
	
		if(Validated)
		{
			glGenProgramPipelines(1, &PipelineName);
			glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);
		}

		return Validated;
	}

	bool initBuffer()
	{
		glGenBuffers(buffer::MAX, &this->BufferName[0]);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->BufferName[buffer::VERTEX]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, this->BufferName[buffer::DRAW_ID]);
		glBufferData(GL_ARRAY_BUFFER, DrawSize, DrawIDData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->BufferName[buffer::ELEMENT]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		int VertexIndirection[3] = {0, 1, 2};
		glBindBuffer(GL_UNIFORM_BUFFER, this->BufferName[buffer::VERTEX_INDIRECTION]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(int) * 3, VertexIndirection, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		std::size_t Padding = glm::max(sizeof(glm::mat4), std::size_t(UniformArrayStride));
		glBindBuffer(GL_UNIFORM_BUFFER, this->BufferName[buffer::TRANSFORM]);
		glBufferData(GL_UNIFORM_BUFFER, Padding * 3, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		DrawElementsIndirectCommand Commands[6];
		Commands[0] = DrawElementsIndirectCommand(ElementCount, 1, 0, 0, 0);
		Commands[1] = DrawElementsIndirectCommand(ElementCount >> 1, 1, 6, 4, 1);
		Commands[2] = DrawElementsIndirectCommand(ElementCount, 1, 9, 7, 2);
		Commands[3] = DrawElementsIndirectCommand(ElementCount, 1, 0, 0, 0);
		Commands[4] = DrawElementsIndirectCommand(ElementCount >> 1, 1, 6, 4, 1);
		Commands[5] = DrawElementsIndirectCommand(ElementCount, 1, 9, 7, 2);

/*
		Commands[0] = DrawElementsIndirectCommand(ElementCount, 1, 0, 0, 0);
		Commands[1] = DrawElementsIndirectCommand(ElementCount >> 1, 1, 6, 4, 1);
		Commands[2] = DrawElementsIndirectCommand(ElementCount, 1, 9, 7, 2);
		Commands[3] = DrawElementsIndirectCommand(ElementCount, 1, 0, 0, 0);
		Commands[4] = DrawElementsIndirectCommand(ElementCount >> 1, 1, 6, 4, 1);
		Commands[5] = DrawElementsIndirectCommand(ElementCount, 1, 9, 7, 2);
*/
		GLintptr DrawCount[IndirectBufferCount];
		DrawCount[0] = 1;
		DrawCount[1] = 2;
		DrawCount[2] = 4;

		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, this->BufferName[buffer::INDIRECT]);
		glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(Commands), Commands, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

		glBindBuffer(GL_PARAMETER_BUFFER_ARB, this->BufferName[buffer::PARAMETER]);
		glBufferData(GL_PARAMETER_BUFFER_ARB, sizeof(DrawCount), DrawCount, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_PARAMETER_BUFFER_ARB, 0);

		return true;
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::DRAW_ID]);
			glVertexAttribIPointer(semantic::attr::DRAW_ID, 1, GL_UNSIGNED_INT, sizeof(glm::uint), BUFFER_OFFSET(0));
			glVertexAttribDivisor(semantic::attr::DRAW_ID, 1);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		
			glEnableVertexAttribArray(semantic::attr::DRAW_ID);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]); 
		glBindVertexArray(0);

		return true;
	}

	bool initTexture()
	{
		bool Validated(true);

		gli::texture2D Texture(gli::load_dds((getDataDirectory() + TEXTURE_DIFFUSE).c_str()));
		assert(!Texture.empty());

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(texture::MAX, &TextureName[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::TEXTURE_A]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_NONE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_NONE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexStorage2D(GL_TEXTURE_2D, GLint(Texture.levels()), GL_RGBA8, GLsizei(Texture.dimensions().x), GLsizei(Texture.dimensions().y));
		for(gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexSubImage2D(GL_TEXTURE_2D,
				GLint(Level),
				0, 0, 
				GLsizei(Texture[Level].dimensions().x), 
				GLsizei(Texture[Level].dimensions().y), 
				GL_BGR, GL_UNSIGNED_BYTE, 
				Texture[Level].data());
		}
	
		///////////////////////////////////////////

		glBindTexture(GL_TEXTURE_2D, TextureName[texture::TEXTURE_B]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_NONE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_NONE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexStorage2D(GL_TEXTURE_2D, GLint(Texture.levels()), GL_RGBA8, GLsizei(Texture.dimensions().x), GLsizei(Texture.dimensions().y));
		for(gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexSubImage2D(GL_TEXTURE_2D,
				GLint(Level), 
				0, 0, 
				GLsizei(Texture[Level].dimensions().x), 
				GLsizei(Texture[Level].dimensions().y), 
				GL_BGR, GL_UNSIGNED_BYTE, 
				Texture[Level].data());
		}
	
		///////////////////////////////////////////

		glBindTexture(GL_TEXTURE_2D, TextureName[texture::TEXTURE_C]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_NONE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_NONE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexStorage2D(GL_TEXTURE_2D, GLint(Texture.levels()), GL_RGBA8, GLsizei(Texture.dimensions().x), GLsizei(Texture.dimensions().y));
		for(gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexSubImage2D(GL_TEXTURE_2D,
				GLint(Level), 
				0, 0, 
				GLsizei(Texture[Level].dimensions().x), 
				GLsizei(Texture[Level].dimensions().y), 
				GL_BGR, GL_UNSIGNED_BYTE, 
				Texture[Level].data());
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		return Validated;
	}

	void validate()
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

	bool begin()
	{
		bool Success(true);
		Success = Success && this->checkExtension("GL_ARB_multi_draw_indirect");
		Success = Success && this->checkExtension("GL_ARB_indirect_parameters");
		Success = Success && this->checkExtension("GL_ARB_shader_storage_buffer_object");
		
		if(Success)
			Success = initProgram();
		if(Success)
			Success = initBuffer();
		if(Success)
			Success = initVertexArray();
		if(Success)
			Success = initTexture();

		glm::vec2 WindowSize(this->getWindowSize());
		this->Viewport[0] = glm::vec4(WindowSize.x / 3.0f * 0.0f, 0, WindowSize.x / 3, WindowSize.y);
		this->Viewport[1] = glm::vec4(WindowSize.x / 3.0f * 1.0f, 0, WindowSize.x / 3, WindowSize.y);
		this->Viewport[2] = glm::vec4(WindowSize.x / 3.0f * 2.0f, 0, WindowSize.x / 3, WindowSize.y);

		glEnable(GL_DEPTH_TEST);
		glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);

		return Success;
	}

	bool end()
	{
		glDeleteBuffers(buffer::MAX, &BufferName[0]);
		glDeleteProgramPipelines(1, &PipelineName);
		glDeleteProgram(ProgramName);
		glDeleteVertexArrays(1, &VertexArrayName);

		return true;
	}

	bool render()
	{
		glm::vec2 WindowSize(this->getWindowSize());

		float Depth(1.0f);
		glClearBufferfv(GL_DEPTH, 0, &Depth);
		glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

		{
			std::size_t Padding = glm::max(sizeof(glm::mat4), std::size_t(UniformArrayStride));

			glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
			glm::byte* Pointer = (glm::byte*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, Padding * 3, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

			glm::vec2 WindowSize(this->getWindowSize());
			glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / 3.0f / WindowSize.y, 0.1f, 100.0f);
			glm::mat4 View = this->view();
			glm::mat4 Model = glm::mat4(1.0f);

			*reinterpret_cast<glm::mat4*>(Pointer + Padding * 0) = Projection * View * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.5f));
			*reinterpret_cast<glm::mat4*>(Pointer + Padding * 1) = Projection * View * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
			*reinterpret_cast<glm::mat4*>(Pointer + Padding * 2) = Projection * View * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.5f));
			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::TEXTURE_A]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::TEXTURE_B]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::TEXTURE_C]);

		glBindProgramPipeline(PipelineName);
		glBindVertexArray(VertexArrayName);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, semantic::storage::VERTEX, BufferName[buffer::VERTEX]);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::INDIRECTION, BufferName[buffer::VERTEX_INDIRECTION]);

		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, BufferName[buffer::INDIRECT]);
		glBindBuffer(GL_PARAMETER_BUFFER_ARB, BufferName[buffer::PARAMETER]);

		this->validate();

		for(std::size_t i = 0; i < IndirectBufferCount; ++i)
		{
			glViewportIndexedfv(0, &this->Viewport[i][0]);
			
			glMultiDrawElementsIndirectCountARB(GL_TRIANGLES, GL_UNSIGNED_SHORT,
				BUFFER_OFFSET(sizeof(DrawElementsIndirectCommand) * i), // Offset in the indirect draw buffer
				sizeof(GLintptr) * i, // Offset in the paramter buffer
				IndirectBufferCount, sizeof(DrawElementsIndirectCommand));
		}

		return true;
	}
};

int main(int argc, char* argv[])
{
	int Error(0);

	gl_500_multi_draw_indirect_count Test(argc, argv);
	Error += Test();

	return Error;
}
