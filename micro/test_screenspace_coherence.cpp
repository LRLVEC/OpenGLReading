#include "test_screenspace_coherence.hpp"
#include <array>

namespace
{
	struct drawArraysIndirectCommand
	{
		GLuint count;
		GLuint instanceCount;
		GLuint first;
		GLuint baseInstance;
	};

	GLsizei const VertexCount(6);
	char const * VERT_SHADER_SOURCE = "hz-430/screenspace_coherence.vert";
	char const * FRAG_SHADER_SOURCE = "hz-430/screenspace_coherence.frag";
}//namespace

testScreenspaceCoherence::testScreenspaceCoherence(
	int argc, char* argv[], profile Profile, std::size_t FrameCount,
	glm::uvec2 const & WindowSize,
	glm::uvec2 const & TileSize,
	std::size_t const ViewportDrawCount,
	std::size_t const TileDrawCount
) :
	test(argc, argv, "testScreenspaceCoherence", Profile, 4, 2, FrameCount),
	WindowSize(WindowSize),
	TileSize(TileSize),
	ViewportDrawCount(ViewportDrawCount),
	TileDrawCount(TileDrawCount),
	DrawVertexCount(static_cast<GLsizei>(((WindowSize.x / TileSize.x) * (WindowSize.y / TileSize.y)) * VertexCount * ViewportDrawCount * TileDrawCount)),
	VertexArrayName(0),
	PipelineName(0),
	ProgramName(0)
{}

testScreenspaceCoherence::~testScreenspaceCoherence()
{}

bool testScreenspaceCoherence::begin()
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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);
	glBindProgramPipeline(this->PipelineName);
	glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::PER_FRAME, BufferName[buffer::BUFFER_FRAME]);
	glBindVertexArray(this->VertexArrayName);

	return Success;
}

bool testScreenspaceCoherence::end()
{
	glDeleteBuffers(buffer::BUFFER_MAX, &BufferName[0]);
	glDeleteProgramPipelines(1, &this->PipelineName);
	glDeleteProgram(this->ProgramName);
	glDeleteVertexArrays(1, &this->VertexArrayName);

	return true;
}

bool testScreenspaceCoherence::initProgram()
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
	
bool testScreenspaceCoherence::initBuffer()
{
	glm::uvec2 WindowSize = this->getWindowSize();
	
	glGenBuffers(BUFFER_MAX, &this->BufferName[0]);

	glm::mat4 Projection = glm::ortho(0.0f, float(WindowSize.x), 0.0f, float(WindowSize.y));
	glBindBuffer(GL_UNIFORM_BUFFER, this->BufferName[BUFFER_FRAME]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), &Projection[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	GLsizeiptr const VertexSize = VertexCount * sizeof(glm::vec2);
	std::array<glm::vec2, VertexCount> const VertexData = {
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(1.0f, 1.0f),
		glm::vec2(0.0f, 1.0f),
		glm::vec2(0.0f, 0.0f)};

	glm::uvec2 TileCount = glm::uvec2(WindowSize) / this->TileSize;

	glBindBuffer(GL_ARRAY_BUFFER, this->BufferName[buffer::BUFFER_ARRAY]);
	glBufferData(GL_ARRAY_BUFFER, VertexSize * this->DrawVertexCount, NULL, GL_STATIC_DRAW);

	glm::vec2* Pointer = reinterpret_cast<glm::vec2*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, VertexSize * this->DrawVertexCount, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
	std::size_t Offset = 0;
	std::size_t TotalTileCount = 0;

	for(std::size_t ViewportDrawIndex = 0; ViewportDrawIndex < this->ViewportDrawCount; ++ViewportDrawIndex)
	for(glm::uint TileYIndex = 0; TileYIndex < this->WindowSize.y / this->TileSize.y; ++TileYIndex)
	for(glm::uint TileXIndex = 0; TileXIndex < this->WindowSize.x / this->TileSize.x; ++TileXIndex)
	{
		std::array<glm::vec2, VertexCount> TmpData;
		for(std::size_t i = 0; i < TmpData.size(); ++i)
			TmpData[i] = (VertexData[i] + glm::vec2(TileXIndex, TileYIndex)) * glm::vec2(this->TileSize);

		for(std::size_t TileDrawIndex = 0; TileDrawIndex < this->TileDrawCount; ++TileDrawIndex)
		{
			memcpy(Pointer + Offset, &TmpData[0][0], sizeof(TmpData));
			Offset += VertexCount;
			++TotalTileCount;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	assert(TotalTileCount == this->DrawVertexCount / VertexCount);

	return true;
}

bool testScreenspaceCoherence::initVertexArray()
{
	glGenVertexArrays(1, &this->VertexArrayName);
	glBindVertexArray(this->VertexArrayName);
		glBindBuffer(GL_ARRAY_BUFFER, this->BufferName[buffer::BUFFER_ARRAY]);
		glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		glEnableVertexAttribArray(semantic::attr::POSITION);
	glBindVertexArray(0);

	return true;
}

bool testScreenspaceCoherence::render()
{
	float Depth(1.0f);
	glClearBufferfv(GL_DEPTH, 0, &Depth);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f)[0]);

	glViewportIndexedfv(0, &glm::vec4(0, 0, this->getWindowSize())[0]);

	this->beginTimer();
		glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, this->DrawVertexCount, 1, 0);

		//glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, ::VertexCount, 1, 0);
	this->endTimer();

	return true;
}
