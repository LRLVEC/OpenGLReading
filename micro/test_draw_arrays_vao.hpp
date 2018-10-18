#ifndef TEST_DRAW_ARRAYS_VAO_INCLUDED
#define TEST_DRAW_ARRAYS_VAO_INCLUDED

#include "test.hpp"

class testDrawArraysVAO : public test
{
public:
	enum drawType
	{
		INSTANCED,
		MULTI_DRAW,
		DRAW_PARAMS
	};

	enum vaoMode
	{
		VABS_SEPARATED_BUFFER,
		VABS_UNIQUE_BUFFER,
		VAOS_SEPARATED_BUFFER,
		VAOS_UNIQUE_BUFFER,
		UNIQUE_VAO
	};

private:
	enum buffer
	{
		BUFFER_FRAME,
		BUFFER_ARRAY,
		BUFFER_INDIRECT,
		BUFFER_MAX
	};

public:
	testDrawArraysVAO(
		int argc, char* argv[], profile Profile, std::size_t FrameCount,
		drawType const DrawType,
		vaoMode const VAOMode,
		std::size_t const DrawCount);
	virtual ~testDrawArraysVAO();

	virtual bool begin();
	virtual bool end();
	virtual bool render();

	bool initProgram();
	bool initBuffer();
	bool initVertexArray();

private:
	drawType const DrawType;
	vaoMode const VAOMode;
	std::size_t const DrawCount;
	std::vector<GLuint> VertexArrayName;
	std::vector<GLuint> ArrayBufferName;
	GLuint PipelineName;
	GLuint ProgramName;
	std::array<GLuint, BUFFER_MAX> BufferName;
};

#endif//TEST_DRAW_ARRAYS_VAO_INCLUDED
