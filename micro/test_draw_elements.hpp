#ifndef TEST_DRAW_ELEMENTS_INCLUDED
#define TEST_DRAW_ELEMENTS_INCLUDED

#include "test.hpp"

class testDrawElements : public test
{
public:
	enum drawType
	{
		INSTANCED,
		MULTI_DISCARD,
		MULTI_DRAW,
		DRAW_PACKED,
		DRAW_PARAMS
	};

	enum vertexDataType
	{
		SEPARATED_VERTEX_DATA,
		SHARED_VERTEX_DATA
	};

private:
	enum buffer
	{
		BUFFER_FRAME,
		BUFFER_ARRAY,
		BUFFER_ELEMENT,
		BUFFER_INDIRECT,
		BUFFER_MAX
	};

public:
	testDrawElements(
		int argc, char* argv[], profile Profile, std::size_t FrameCount,
		drawType const DrawType, vertexDataType const VertexDataType, std::size_t const DrawCount);
	virtual ~testDrawElements();

	virtual bool begin();
	virtual bool end();
	virtual bool render();

	bool initProgram();
	bool initBuffer();
	bool initVertexArray();

private:
	drawType const DrawType;
	vertexDataType const VertexDataType;
	std::size_t const DrawCount;
	GLuint VertexArrayName;
	GLuint PipelineName;
	GLuint ProgramName;
	std::array<GLuint, BUFFER_MAX> BufferName;
};

#endif//TEST_DRAW_ELEMENTS_INCLUDED
