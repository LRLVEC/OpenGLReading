#ifndef TEST_DRAW_INDEXING_INCLUDED
#define TEST_DRAW_INDEXING_INCLUDED

#include "test.hpp"

class testDrawIndexing : public test
{
public:
	enum indexing
	{
		DRAW,
		UNIFORM_INDEXING,
		ATTRIB_INDEXING,
		DIVISOR_INDEXING,
		DIVISOR_MULTI_INDEXING,
		ID_INDEXING,
		INDEXING_MAX
	};

private:
	enum buffer
	{
		BUFFER_FRAME,
		BUFFER_ARRAY,
		BUFFER_DIVISOR,
		BUFFER_INDIRECT,
		BUFFER_MAX
	};

public:
	testDrawIndexing(
		int argc, char* argv[], profile Profile, std::size_t FrameCount,
		indexing const Indexing,
		std::size_t const DrawCount);
	virtual ~testDrawIndexing();

	virtual bool begin();
	virtual bool end();
	virtual bool render();

	bool initProgram();
	bool initBuffer();
	bool initVertexArray();

private:
	indexing const Indexing;
	std::size_t const DrawCount;
	GLuint VertexArrayName;
	GLuint PipelineName;
	GLuint ProgramName;
	std::array<GLuint, BUFFER_MAX> BufferName;
};

#endif//TEST_DRAW_INDEXING_INCLUDED
