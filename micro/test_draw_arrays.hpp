#ifndef TEST_DRAW_ARRAYS_INCLUDED
#define TEST_DRAW_ARRAYS_INCLUDED

#include "test.hpp"

class testDrawArrays : public test
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

	enum uniformUpdate
	{
		PER_DRAW_UNIFORM_DSA,
		REDUNDANT_UNIFORM_DSA,
		PER_DRAW_UNIFORM2_DSA,
		REDUNDANT_UNIFORM2_DSA,
		PER_DRAW_UNIFORM_B2E,
		REDUNDANT_UNIFORM_B2E,
		PER_DRAW_UNIFORM2_B2E,
		REDUNDANT_UNIFORM2_B2E,
		CONSTANT_UNIFORM,
		NO_UNIFORM
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
	testDrawArrays(
		int argc, char* argv[], profile Profile, std::size_t FrameCount,
		drawType const DrawType,
		vertexDataType const VertexDataType,
		uniformUpdate const UniformUpdate,
		std::size_t const DrawCount);
	virtual ~testDrawArrays();

	virtual bool begin();
	virtual bool end();
	virtual bool render();

	bool initProgram();
	bool initBuffer();
	bool initVertexArray();

private:
	drawType const DrawType;
	vertexDataType const VertexDataType;
	uniformUpdate const UniformUpdate;
	std::size_t const DrawCount;
	GLuint VertexArrayName;
	GLuint PipelineName;
	GLuint ProgramName;
	std::array<GLuint, BUFFER_MAX> BufferName;
};

#endif//TEST_DRAW_ARRAYS_INCLUDED
