#include "test_draw_arrays.hpp"

namespace
{
	GLsizei const VertexCount(6);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glm::vec2);
	glm::vec2 const VertexData[VertexCount] =
	{
		glm::vec2(-1.0f,-1.0f),
		glm::vec2( 1.0f,-1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2(-1.0f, 1.0f),
		glm::vec2(-1.0f,-1.0f)
	};

	struct drawArraysIndirectCommand
	{
		GLuint count;
		GLuint instanceCount;
		GLuint first;
		GLuint baseInstance;
	};

	char const * VERT_SHADER_SOURCE[3] = {"hz-430/draw-range.vert", "hz-430/draw-uniform.vert", "hz-430/draw-uniform2.vert"};
	char const * FRAG_SHADER_SOURCE[3] = {"hz-430/draw-range.frag", "hz-430/draw-uniform.frag", "hz-430/draw-uniform2.frag"};

	GLint UniformDiffuse0(-1);
	GLint UniformDiffuse1(-1);
}//namespace

testDrawArrays::testDrawArrays(
	int argc, char* argv[], profile Profile, std::size_t FrameCount,
	drawType const DrawType,
	vertexDataType const VertexDataType,
	uniformUpdate const UniformUpdate,
	std::size_t const DrawCount
) :
	test(argc, argv, "testDrawArrays", Profile, 4, 2, FrameCount, glm::uvec2(640, 480), glm::vec2(0), glm::vec2(0, 4)),
	DrawType(DrawType),
	VertexDataType(VertexDataType),
	UniformUpdate(UniformUpdate),
	DrawCount(DrawCount),
	VertexArrayName(0),
	PipelineName(0),
	ProgramName(0)
{
	assert(1);
}

testDrawArrays::~testDrawArrays()
{}

bool testDrawArrays::begin()
{
	assert(!(VertexDataType == SEPARATED_VERTEX_DATA && DrawType == INSTANCED));
	
	bool Success = true;
	
	Success = Success && this->isExtensionSupported("GL_ARB_draw_elements_base_vertex");
	assert(Success);
	Success = Success && this->isExtensionSupported("GL_ARB_multi_draw_indirect");
	assert(Success);
	Success = Success && this->initProgram();
	assert(Success);
	Success = Success && this->initBuffer();
	assert(Success);
	Success = Success && this->initVertexArray();
	assert(Success);

	glEnable(GL_DEPTH_TEST);
	glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::PER_FRAME, this->BufferName[buffer::BUFFER_FRAME]);
	glBindProgramPipeline(this->PipelineName);
	glBindVertexArray(this->VertexArrayName);
	
	if(this->DrawType == MULTI_DISCARD || this->DrawType == MULTI_DRAW)
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, BufferName[BUFFER_INDIRECT]);

	return Success;
}

bool testDrawArrays::end()
{
	glDeleteBuffers(static_cast<GLsizei>(BUFFER_MAX), &this->BufferName[0]);
	glDeleteProgramPipelines(1, &this->PipelineName);
	glDeleteProgram(this->ProgramName);
	glDeleteVertexArrays(1, &this->VertexArrayName);

	return true;
}

bool testDrawArrays::initProgram()
{
	bool Validated(true);
	
	std::size_t ShaderIndex(0);
	switch(this->UniformUpdate)
	{
	case CONSTANT_UNIFORM:
	case PER_DRAW_UNIFORM_DSA:
	case REDUNDANT_UNIFORM_DSA:
	case PER_DRAW_UNIFORM_B2E:
	case REDUNDANT_UNIFORM_B2E:
		ShaderIndex = 1;
		break;
	case PER_DRAW_UNIFORM2_DSA:
	case REDUNDANT_UNIFORM2_DSA:
	case PER_DRAW_UNIFORM2_B2E:
	case REDUNDANT_UNIFORM2_B2E:
		ShaderIndex = 2;
		break;
	case NO_UNIFORM:
	default:
		ShaderIndex = 0;
		break;
	}

	glGenProgramPipelines(1, &this->PipelineName);

	compiler Compiler;
	GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE[ShaderIndex], "--version 420 --profile core");
	GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE[ShaderIndex], "--version 420 --profile core");
	Validated = Validated && Compiler.check();

	this->ProgramName = glCreateProgram();
	glProgramParameteri(this->ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
	glAttachShader(this->ProgramName, VertShaderName);
	glAttachShader(this->ProgramName, FragShaderName);
	glLinkProgram(this->ProgramName);
	Validated = Validated && Compiler.checkProgram(this->ProgramName);

	UniformDiffuse0 = glGetUniformLocation(ProgramName, "Diffuse0");
	UniformDiffuse1 = glGetUniformLocation(ProgramName, "Diffuse1");

	if(Validated)
		glUseProgramStages(this->PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);

	if(
		this->UniformUpdate == testDrawArrays::CONSTANT_UNIFORM ||
		this->UniformUpdate == testDrawArrays::PER_DRAW_UNIFORM_B2E ||
		this->UniformUpdate == testDrawArrays::REDUNDANT_UNIFORM_B2E ||
		this->UniformUpdate == testDrawArrays::PER_DRAW_UNIFORM2_B2E ||
		this->UniformUpdate == testDrawArrays::REDUNDANT_UNIFORM2_B2E)
		glActiveShaderProgram(this->PipelineName, this->ProgramName);

	if(this->UniformUpdate == testDrawArrays::CONSTANT_UNIFORM)
	{
		glProgramUniform4f(this->ProgramName, UniformDiffuse0, 1.0f, 0.5f, 0.0f, 1.0f);
		if(UniformDiffuse1 != -1)
			glProgramUniform4f(this->ProgramName, UniformDiffuse1, 0.0f, 0.5f, 1.0f, 1.0f);
	}

	return Validated;
}

bool testDrawArrays::initBuffer()
{
	glGenBuffers(BUFFER_MAX, &this->BufferName[0]);

	glBindBuffer(GL_UNIFORM_BUFFER, this->BufferName[BUFFER_FRAME]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, this->BufferName[BUFFER_ARRAY]);
	switch(this->VertexDataType)
	{
		case SEPARATED_VERTEX_DATA:
		{
			glBufferData(GL_ARRAY_BUFFER, VertexSize * this->DrawCount, NULL, GL_STATIC_DRAW);
			glm::uint8* Pointer = reinterpret_cast<glm::uint8*>(glMapBufferRange(GL_ARRAY_BUFFER, 
				0, VertexSize * this->DrawCount, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
			for(std::size_t i = 0; i < this->DrawCount; ++i)
				memcpy(Pointer + i * VertexSize, &VertexData, VertexSize);
			glUnmapBuffer(GL_ARRAY_BUFFER);
			break;
		}
		case SHARED_VERTEX_DATA:
		{
			glBufferData(GL_ARRAY_BUFFER, VertexSize, &VertexData, GL_STATIC_DRAW);
			break;
		}
		default:
			assert(0);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	std::vector<drawArraysIndirectCommand> Commands;
	Commands.resize(this->DrawCount);
	for(std::size_t i = 0; i < Commands.size(); ++i)
	{
		Commands[i].count = static_cast<GLuint>(VertexCount);
		Commands[i].instanceCount = static_cast<GLuint>(this->DrawType == MULTI_DISCARD ? 0 : 1);
		Commands[i].first = static_cast<GLuint>(this->VertexDataType == SEPARATED_VERTEX_DATA ? VertexCount * i : 0);
		Commands[i].baseInstance = 0;
	}

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, BufferName[BUFFER_INDIRECT]);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(drawArraysIndirectCommand) * Commands.size(), &Commands[0], GL_STATIC_DRAW);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

	return true;
}

bool testDrawArrays::initVertexArray()
{
	glGenVertexArrays(1, &this->VertexArrayName);
	glBindVertexArray(this->VertexArrayName);
		glBindBuffer(GL_ARRAY_BUFFER, this->BufferName[BUFFER_ARRAY]);
		glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		glEnableVertexAttribArray(semantic::attr::POSITION);
	glBindVertexArray(0);

	return true;
}

void updateUniform(GLuint ProgramName, std::size_t DrawIndex, testDrawArrays::uniformUpdate const & UniformUpdate)
{
	switch(UniformUpdate)
	{
		case testDrawArrays::PER_DRAW_UNIFORM_DSA:
		case testDrawArrays::PER_DRAW_UNIFORM2_DSA:
		{
			if(DrawIndex % 2)
			{
				glProgramUniform4f(ProgramName, UniformDiffuse0, 1.0f, 0.5f, 0.0f, 1.0f);
				if(UniformDiffuse1 != -1)
					glProgramUniform4f(ProgramName, UniformDiffuse1, 0.9f, 0.7f, 0.5f, 1.0f);
			}
			else
			{
				glProgramUniform4f(ProgramName, UniformDiffuse0, 0.0f, 0.5f, 1.0f, 1.0f);
				if(UniformDiffuse1 != -1)
					glProgramUniform4f(ProgramName, UniformDiffuse1, 0.9f, 0.7f, 0.5f, 1.0f);
			}
		}
		break;
		case testDrawArrays::REDUNDANT_UNIFORM_DSA:
		case testDrawArrays::REDUNDANT_UNIFORM2_DSA:
		{
			glProgramUniform4f(ProgramName, UniformDiffuse0, 1.0f, 0.5f, 0.0f, 1.0f);
			if(UniformDiffuse1 != -1)
				glProgramUniform4f(ProgramName, UniformDiffuse1, 0.9f, 0.7f, 0.5f, 1.0f);
		}
		break;
		case testDrawArrays::PER_DRAW_UNIFORM_B2E:
		case testDrawArrays::PER_DRAW_UNIFORM2_B2E:
		{
			if(DrawIndex % 2)
			{
				glUniform4f(UniformDiffuse0, 1.0f, 0.5f, 0.0f, 1.0f);
				if(UniformDiffuse1 != -1)
					glUniform4f(UniformDiffuse1, 0.9f, 0.7f, 0.5f, 1.0f);
			}
			else
			{
				glUniform4f(UniformDiffuse0, 0.0f, 0.5f, 1.0f, 1.0f);
				if(UniformDiffuse1 != -1)
					glUniform4f(UniformDiffuse1, 0.9f, 0.7f, 0.5f, 1.0f);
			}
		}
		break;
		case testDrawArrays::REDUNDANT_UNIFORM_B2E:
		case testDrawArrays::REDUNDANT_UNIFORM2_B2E:
		{
			glUniform4f(UniformDiffuse0, 1.0f, 0.5f, 0.0f, 1.0f);
			if(UniformDiffuse1 != -1)
				glUniform4f(UniformDiffuse1, 0.9f, 0.7f, 0.5f, 1.0f);
		}
		break;
		default:
			break;
	}
}

bool testDrawArrays::render()
{
	glm::vec2 const WindowSize(this->getWindowSize());

	float Depth(1.0f);
	glClearBufferfv(GL_DEPTH, 0, &Depth);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

	{
		glBindBuffer(GL_UNIFORM_BUFFER, this->BufferName[buffer::BUFFER_FRAME]);
		glm::mat4* Pointer = reinterpret_cast<glm::mat4*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

		glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.1f, 1000.0f);
		glm::mat4 View = this->view();
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * View * Model;

		*Pointer = Projection * View * Model;
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}

	glViewportIndexedfv(0, &glm::vec4(0, 0, WindowSize)[0]);

	this->beginTimer();
	switch(this->DrawType)
	{
	case INSTANCED:
		glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, VertexCount, static_cast<GLsizei>(DrawCount), 0);
		break;
	case MULTI_DISCARD:
	case MULTI_DRAW:
		for(int i = 0; i < 2; ++i)
			glMultiDrawArraysIndirect(GL_TRIANGLES, 0, static_cast<GLsizei>(DrawCount / 2), 0);
		break;
	case DRAW_PACKED:
		for(std::size_t DrawIndex(0); DrawIndex < DrawCount; ++DrawIndex)
		{
			if(this->UniformUpdate != testDrawArrays::CONSTANT_UNIFORM && this->UniformUpdate != testDrawArrays::NO_UNIFORM)
				::updateUniform(this->ProgramName, DrawIndex, this->UniformUpdate);
			glDrawArrays(GL_TRIANGLES, static_cast<GLint>(this->VertexDataType == SEPARATED_VERTEX_DATA ? VertexCount * DrawIndex : 0), VertexCount);
		}
		break;
	case DRAW_PARAMS:
		for(std::size_t DrawIndex(0); DrawIndex < DrawCount; ++DrawIndex)
		{
			if(this->UniformUpdate != testDrawArrays::CONSTANT_UNIFORM && this->UniformUpdate != testDrawArrays::NO_UNIFORM)
				::updateUniform(this->ProgramName, DrawIndex, this->UniformUpdate);
			glDrawArraysInstancedBaseInstance(GL_TRIANGLES, static_cast<GLint>(this->VertexDataType == SEPARATED_VERTEX_DATA ? VertexCount * DrawIndex : 0), VertexCount, 1, 0);
		}
		break;
	default:
		assert(0);
		break;
	}
	this->endTimer();

	return true;
}
