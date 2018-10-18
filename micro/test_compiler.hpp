#ifndef TEST_COMPILER_INCLUDED
#define TEST_COMPILER_INCLUDED

#include "test.hpp"

class testCompiler : public test
{
public:
	enum mode
	{
		MULTITHREADED,
		DUALTHREADED,
		SINGLETHREADED,
		MODE_MAX
	};

public:
	testCompiler(
		int argc, char* argv[], profile Profile, std::size_t FrameCount,
		mode Mode);
	virtual ~testCompiler();

	virtual bool begin();
	virtual bool end();
	virtual bool render();

private:
	std::vector<std::string> VertShaderFile;
	std::vector<GLuint> VertShaderName;
	std::vector<std::string> FragShaderFile;
	std::vector<GLuint> FragShaderName;
	std::vector<GLuint> ProgramName;
	mode Mode;
};

#endif//TEST_COMPILER_INCLUDED
