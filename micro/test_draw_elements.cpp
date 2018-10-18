#include "test_draw_elements.hpp"

namespace
{
	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(glm::uint16);
	glm::uint16 const ElementData[ElementCount] =
	{
		0, 1, 2,
		0, 2, 3
	};

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glm::vec2);
	glm::vec2 const VertexData[VertexCount] =
	{
		glm::vec2(-1.0f,-1.0f),
		glm::vec2( 1.0f,-1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2(-1.0f, 1.0f)
	};

	struct drawElementsIndirectCommand
	{
		GLuint count;
		GLuint instanceCount;
		GLuint firstIndex;
		GLint  baseVertex;
		GLuint baseInstance;
	};

	char const * VERT_SHADER_SOURCE("hz-430/vertex-array-object.vert");
	char const * FRAG_SHADER_SOURCE("hz-430/vertex-array-object.frag");
}//namespace

testDrawElements::testDrawElements(
	int argc, char* argv[], profile Profile, std::size_t FrameCount,
	drawType const DrawType, vertexDataType const VertexDataType, std::size_t const DrawCount
) :
	test(argc, argv, "testDrawElements", Profile, 4, 2, FrameCount, glm::uvec2(0, 0), glm::vec2(0, 512)),
	DrawType(DrawType),
	VertexDataType(VertexDataType),
	DrawCount(DrawCount),
	VertexArrayName(0),
	PipelineName(0),
	ProgramName(0)
{}

testDrawElements::~testDrawElements()
{}

bool testDrawElements::begin()
{
	bool Success(true);
	
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

bool testDrawElements::end()
{
	glDeleteBuffers(static_cast<GLsizei>(BUFFER_MAX), &this->BufferName[0]);
	glDeleteProgramPipelines(1, &this->PipelineName);
	glDeleteProgram(this->ProgramName);
	glDeleteVertexArrays(1, &this->VertexArrayName);

	return true;
}

bool testDrawElements::initProgram()
{
	bool Validated(true);
	
	glGenProgramPipelines(1, &this->PipelineName);

	compiler Compiler;
	GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE, "--version 420 --profile core");
	GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE, "--version 420 --profile core");
	Validated = Validated && Compiler.check();

	this->ProgramName = glCreateProgram();
	glProgramParameteri(this->ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
	glAttachShader(this->ProgramName, VertShaderName);
	glAttachShader(this->ProgramName, FragShaderName);
	glLinkProgram(this->ProgramName);
	Validated = Validated && Compiler.checkProgram(this->ProgramName);

	if(Validated)
		glUseProgramStages(this->PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);

	return Validated;
}

bool testDrawElements::initBuffer()
{
	glGenBuffers(BUFFER_MAX, &this->BufferName[0]);

	glBindBuffer(GL_UNIFORM_BUFFER, this->BufferName[BUFFER_FRAME]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
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

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[BUFFER_ELEMENT]);
	switch(this->VertexDataType)
	{
		case SEPARATED_VERTEX_DATA:
		{
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize * this->DrawCount, NULL, GL_STATIC_DRAW);
			glm::uint8* Pointer = reinterpret_cast<glm::uint8*>(glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 
				0, ElementSize * this->DrawCount, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
			for(std::size_t i = 0; i < this->DrawCount; ++i)
				memcpy(Pointer + i * ElementSize, &ElementData, ElementSize);
			glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
			break;
		}
		case SHARED_VERTEX_DATA:
		{
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, &ElementData, GL_STATIC_DRAW);
			break;
		}
		default:
			assert(0);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	std::vector<drawElementsIndirectCommand> Commands;
	Commands.resize(this->DrawCount);
	for(std::size_t DrawIndex = 0; DrawIndex < Commands.size(); ++DrawIndex)
	{
		Commands[DrawIndex].count = ElementCount;
		Commands[DrawIndex].instanceCount = this->DrawType == MULTI_DISCARD ? 0 : 1;
		Commands[DrawIndex].firstIndex = 0;
		Commands[DrawIndex].baseVertex = static_cast<GLint>(DrawIndex * VertexCount);
		Commands[DrawIndex].baseInstance = 0;
	}

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, BufferName[BUFFER_INDIRECT]);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(drawElementsIndirectCommand) * Commands.size(), &Commands[0], GL_STATIC_DRAW);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

	return true;
}

bool testDrawElements::initVertexArray()
{
	glGenVertexArrays(1, &this->VertexArrayName);
	glBindVertexArray(this->VertexArrayName);
		glBindBuffer(GL_ARRAY_BUFFER, this->BufferName[BUFFER_ARRAY]);
		glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		glEnableVertexAttribArray(semantic::attr::POSITION);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[BUFFER_ELEMENT]); 
	glBindVertexArray(0);

	return true;
}

bool testDrawElements::render()
{
	glm::vec2 const WindowSize(this->getWindowSize());

	float Depth(1.0f);
	glClearBufferfv(GL_DEPTH, 0, &Depth);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

	{
		glBindBuffer(GL_UNIFORM_BUFFER, this->BufferName[buffer::BUFFER_FRAME]);
		glm::mat4* Pointer = reinterpret_cast<glm::mat4*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

		glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, WindowSize.x / WindowSize.y, 0.1f, 2048.0f);
		glm::mat4 View = this->view();
		glm::mat4 Model = glm::mat4(1.0f);

		*Pointer = Projection * View * Model;
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}

	glViewportIndexedfv(0, &glm::vec4(0, 0, this->getWindowSize())[0]);

	this->beginTimer();
	switch(this->DrawType)
	{
	case INSTANCED:
		glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, static_cast<GLsizei>(DrawCount), 0, 0);
		break;
	case MULTI_DISCARD:
	case MULTI_DRAW:
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, BufferName[BUFFER_INDIRECT]);
		for(int i = 0; i < 100; ++i)
			glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, 0, static_cast<GLsizei>(DrawCount / 100), 0);
		break;
	case DRAW_PACKED:
		for(std::size_t DrawIndex(0); DrawIndex < DrawCount; ++DrawIndex)
			glDrawElements(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0);
		break;
	case DRAW_PARAMS:
		for(std::size_t DrawIndex(0); DrawIndex < DrawCount; ++DrawIndex)
			glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 1, static_cast<GLint>(DrawIndex * VertexCount), 0);
		break;
	default:
		assert(0);
		break;
	}
	this->endTimer();

	return true;
}
