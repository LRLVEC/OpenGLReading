#ifndef TEST_SCREENSPACE_COHERENCE_INCLUDED
#define TEST_SCREENSPACE_COHERENCE_INCLUDED

#include "test.hpp"

class testScreenspaceCoherence : public test
{
private:
	enum buffer
	{
		BUFFER_FRAME,
		BUFFER_ARRAY,
		BUFFER_MAX
	};

public:
	testScreenspaceCoherence(
		int argc, char* argv[], profile Profile, std::size_t FrameCount,
		glm::uvec2 const & WindowSize,
		glm::uvec2 const & TileSize,
		std::size_t const ViewportDrawCount,
		std::size_t const TileDrawCount);
	virtual ~testScreenspaceCoherence();

	virtual bool begin();
	virtual bool end();
	virtual bool render();

	bool initProgram();
	bool initBuffer();
	bool initVertexArray();

private:
	glm::uvec2 const WindowSize;
	glm::uvec2 const TileSize;
	std::size_t const ViewportDrawCount;
	std::size_t const TileDrawCount;
	GLsizei const DrawVertexCount;
	GLuint VertexArrayName;
	GLuint PipelineName;
	GLuint ProgramName;
	std::array<GLuint, BUFFER_MAX> BufferName;
};

#endif//TEST_SCREENSPACE_COHERENCE_INCLUDED
