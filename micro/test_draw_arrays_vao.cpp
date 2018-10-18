#include "test_draw_arrays_vao.hpp"

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

	char const * VERT_SHADER_SOURCE("hz-430/draw-range.vert");
	char const * FRAG_SHADER_SOURCE("hz-430/draw-range.frag");

	GLint UniformDiffuse(-1);
	std::vector<GLuint> VertexArrayName;
}//namespace

#define GLF_BUFFER_OFFSET(i) ((char *)NULL + (i))

testDrawArraysVAO::testDrawArraysVAO(
	int argc, char* argv[], profile Profile, std::size_t FrameCount,
	drawType const DrawType,
	vaoMode const VAOMode,
	std::size_t const DrawCount
) :
	test(argc, argv, "testDrawArraysVAO", Profile, 4, 2, FrameCount),
	DrawType(DrawType),
	VAOMode(VAOMode),
	DrawCount(DrawCount),
	PipelineName(0),
	ProgramName(0)
{}

testDrawArraysVAO::~testDrawArraysVAO()
{}

bool testDrawArraysVAO::begin()
{
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
	
	if(this->DrawType == MULTI_DRAW)
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, BufferName[BUFFER_INDIRECT]);

	return Success;
}

bool testDrawArraysVAO::end()
{
	glDeleteBuffers(static_cast<GLsizei>(BUFFER_MAX), &this->BufferName[0]);
	glDeleteProgramPipelines(1, &this->PipelineName);
	glDeleteProgram(this->ProgramName);

	glDeleteVertexArrays(static_cast<GLsizei>(this->VertexArrayName.size()), &this->VertexArrayName[0]);
	if(this->VAOMode == VABS_SEPARATED_BUFFER || this->VAOMode == VAOS_SEPARATED_BUFFER)
		glDeleteBuffers(static_cast<GLsizei>(this->ArrayBufferName.size()), &this->ArrayBufferName[0]);

	return true;
}

bool testDrawArraysVAO::initProgram()
{
	bool Validated(true);
	
	compiler Compiler;
	GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 420 --profile core");
	GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER,	getDataDirectory() + FRAG_SHADER_SOURCE, "--version 420 --profile core");
	Validated = Validated && Compiler.check();

	this->ProgramName = glCreateProgram();
	glProgramParameteri(this->ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
	glAttachShader(this->ProgramName, VertShaderName);
	glAttachShader(this->ProgramName, FragShaderName);
	glLinkProgram(this->ProgramName);
	Validated = Validated && Compiler.checkProgram(this->ProgramName);

	if(Validated)
	{
		glGenProgramPipelines(1, &this->PipelineName);
		glUseProgramStages(this->PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);
	}

	return Validated;
}

bool testDrawArraysVAO::initBuffer()
{
	glGenBuffers(BUFFER_MAX, &this->BufferName[0]);

	glBindBuffer(GL_UNIFORM_BUFFER, this->BufferName[BUFFER_FRAME]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, this->BufferName[BUFFER_ARRAY]);
	glBufferData(GL_ARRAY_BUFFER, VertexSize * this->DrawCount, NULL, GL_STATIC_DRAW);
	glm::uint8* Pointer = reinterpret_cast<glm::uint8*>(glMapBufferRange(GL_ARRAY_BUFFER, 
		0, VertexSize * this->DrawCount, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
	for(std::size_t i = 0; i < this->DrawCount; ++i)
		memcpy(Pointer + i * VertexSize, &VertexData, VertexSize);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if(this->VAOMode == VAOS_SEPARATED_BUFFER || this->VAOMode == VABS_SEPARATED_BUFFER)
	{
		this->ArrayBufferName.resize(this->DrawCount);
		glGenBuffers(static_cast<GLuint>(this->ArrayBufferName.size()), &this->ArrayBufferName[0]);
		for(std::size_t DrawIndex = 0; DrawIndex < this->DrawCount; ++DrawIndex)
		{
			glBindBuffer(GL_ARRAY_BUFFER, this->ArrayBufferName[DrawIndex]);
			glBufferData(GL_ARRAY_BUFFER, VertexSize, &VertexData, GL_STATIC_DRAW);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	std::vector<drawArraysIndirectCommand> Commands;
	Commands.resize(this->DrawCount);
	for(std::size_t i = 0; i < Commands.size(); ++i)
	{
		Commands[i].count = static_cast<GLuint>(VertexCount);
		Commands[i].instanceCount = 1;
		Commands[i].first = VertexCount;
		Commands[i].baseInstance = 0;
	}

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, BufferName[BUFFER_INDIRECT]);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(drawArraysIndirectCommand) * Commands.size(), &Commands[0], GL_STATIC_DRAW);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

	return true;
}

bool testDrawArraysVAO::initVertexArray()
{
	switch(this->VAOMode)
	{
		case UNIQUE_VAO:
		{
			this->VertexArrayName.resize(1);

			glGenVertexArrays(1, &this->VertexArrayName[0]);
			glBindVertexArray(this->VertexArrayName[0]);
				glBindBuffer(GL_ARRAY_BUFFER, this->BufferName[BUFFER_ARRAY]);
				glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
		
				glEnableVertexAttribArray(semantic::attr::POSITION);
			glBindVertexArray(0);
		}
		break;
		case VABS_SEPARATED_BUFFER:
		case VABS_UNIQUE_BUFFER:
		{
			this->VertexArrayName.resize(1);

			glGenVertexArrays(1, &this->VertexArrayName[0]);
			glBindVertexArray(this->VertexArrayName[0]);
				glVertexAttribFormat(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, 0);
				glVertexAttribBinding(semantic::attr::POSITION, 0);

				glEnableVertexAttribArray(semantic::attr::POSITION);
			glBindVertexArray(0);
		}
		break;
		case VAOS_SEPARATED_BUFFER:
		{
			this->VertexArrayName.resize(this->DrawCount);
			glGenVertexArrays(static_cast<GLuint>(this->DrawCount), &this->VertexArrayName[0]);

			for(std::size_t DrawIndex = 0; DrawIndex < this->VertexArrayName.size(); ++DrawIndex)
			{
				glBindVertexArray(this->VertexArrayName[DrawIndex]);
					glBindBuffer(GL_ARRAY_BUFFER, this->ArrayBufferName[DrawIndex]);
					glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), GLF_BUFFER_OFFSET(0));
					glBindBuffer(GL_ARRAY_BUFFER, 0);
		
					glEnableVertexAttribArray(semantic::attr::POSITION);
				glBindVertexArray(0);
			}
		}
		break;
		case VAOS_UNIQUE_BUFFER:
		{
			this->VertexArrayName.resize(this->DrawCount);
			glGenVertexArrays(static_cast<GLuint>(this->DrawCount), &this->VertexArrayName[0]);

			for(std::size_t DrawIndex = 0; DrawIndex < this->VertexArrayName.size(); ++DrawIndex)
			{
				glBindVertexArray(this->VertexArrayName[DrawIndex]);
					glBindBuffer(GL_ARRAY_BUFFER, this->BufferName[BUFFER_ARRAY]);
					glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), GLF_BUFFER_OFFSET(DrawIndex * VertexSize));
					glBindBuffer(GL_ARRAY_BUFFER, 0);
		
					glEnableVertexAttribArray(semantic::attr::POSITION);
				glBindVertexArray(0);
			}
		}
		break;
		default:
			break;
	}

	return true;
}

bool testDrawArraysVAO::render()
{
	glm::vec2 WindowSize(this->getWindowSize());

	float Depth(1.0f);
	glClearBufferfv(GL_DEPTH, 0, &Depth);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

	{
		glBindBuffer(GL_UNIFORM_BUFFER, this->BufferName[buffer::BUFFER_FRAME]);
		glm::mat4* Pointer = reinterpret_cast<glm::mat4*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

		glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.1f, 2048.0f);
		glm::mat4 View = this->view();
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * View * Model;

		*Pointer = Projection * View * Model;
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}

	glViewportIndexedfv(0, &glm::vec4(0, 0, this->getWindowSize())[0]);
	if(this->VAOMode == UNIQUE_VAO || this->VAOMode == VABS_SEPARATED_BUFFER || this->VAOMode == VABS_UNIQUE_BUFFER)
		glBindVertexArray(this->VertexArrayName[0]);

	this->beginTimer();
	switch(this->DrawType)
	{
	case INSTANCED:
		glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, VertexCount, static_cast<GLsizei>(DrawCount), 0);
		break;
	case MULTI_DRAW:
		for(int i = 0; i < 2; ++i)
			glMultiDrawArraysIndirect(GL_TRIANGLES, 0, static_cast<GLsizei>(DrawCount / 2), 0);
		break;
	case DRAW_PARAMS:
		for(std::size_t DrawIndex(0); DrawIndex < DrawCount; ++DrawIndex)
		{
			switch(this->VAOMode)
			{
			case VAOS_SEPARATED_BUFFER:
			case VAOS_UNIQUE_BUFFER:
				glBindVertexArray(this->VertexArrayName[DrawIndex]);
				break;
			case VABS_SEPARATED_BUFFER:
				glBindVertexBuffer(0, this->ArrayBufferName[DrawIndex], 0, sizeof(glm::vec2));
				break;
			case VABS_UNIQUE_BUFFER:
				glBindVertexBuffer(0, this->BufferName[BUFFER_ARRAY], DrawIndex * VertexSize, sizeof(glm::vec2));
				break;
			default:
				break;
			}
			glDrawArraysInstancedBaseInstance(GL_TRIANGLES, static_cast<GLint>(VertexCount * DrawIndex), VertexCount, 1, 0);
		}
		break;
	default:
		assert(0);
		break;
	}
	this->endTimer();

	return true;
}
