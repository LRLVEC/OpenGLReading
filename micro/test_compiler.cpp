#include "test_compiler.hpp"
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
}//namespace

testCompiler::testCompiler(
	int argc, char* argv[], profile Profile, std::size_t FrameCount, mode Mode
) :
	test(argc, argv, "testCompiler", Profile, 4, 2),
	Mode(Mode)
{}

testCompiler::~testCompiler()
{}

bool testCompiler::begin()
{
	bool Success = true;

	VertShaderFile.push_back("gl-320/texture-offset.vert");
	VertShaderFile.push_back("gl-320/texture-derivative-x.vert");
	VertShaderFile.push_back("gl-320/texture-derivative-y.vert");
	VertShaderFile.push_back("gl-320/primitive-front-face.vert");
	VertShaderFile.push_back("gl-320/glsl-precision.vert");
	VertShaderFile.push_back("gl-320/glsl-discard.vert");
	VertShaderFile.push_back("gl-320/glsl-builtin-blocks.vert");
	VertShaderFile.push_back("gl-320/fbo-rtt-multiple-output.vert");
	VertShaderFile.push_back("gl-320/draw-without-vertex-attrib.vert");
	VertShaderFile.push_back("gl-320/texture-3d.vert");
	VertShaderFile.push_back("gl-330/texture-rect.vert");
	VertShaderFile.push_back("hz-430/vertex-array-object.vert");
	VertShaderFile.push_back("hz-430/screenspace_coherence.vert");
	VertShaderFile.push_back("hz-430/multi-draw-indirect.vert");
	VertShaderFile.push_back("hz-430/draw-uniform2.vert");
	VertShaderFile.push_back("hz-430/draw-uniform.vert");
	VertShaderFile.push_back("hz-430/draw-range.vert");
	VertShaderName.resize(VertShaderFile.size());

	FragShaderFile.push_back("gl-320/texture-offset-bicubic.frag");
	FragShaderFile.push_back("gl-320/texture-derivative-x.frag");
	FragShaderFile.push_back("gl-320/texture-derivative-y.frag");
	FragShaderFile.push_back("gl-320/primitive-front-face.frag");
	FragShaderFile.push_back("gl-320/glsl-precision.frag");
	FragShaderFile.push_back("gl-320/glsl-discard.frag");
	FragShaderFile.push_back("gl-320/glsl-builtin-blocks.frag");
	FragShaderFile.push_back("gl-320/fbo-rtt-multiple-output.frag");
	FragShaderFile.push_back("gl-320/draw-without-vertex-attrib.frag");
	FragShaderFile.push_back("gl-320/texture-3d.frag");
	FragShaderFile.push_back("gl-330/texture-rect.frag");
	FragShaderFile.push_back("hz-430/vertex-array-object.frag");
	FragShaderFile.push_back("hz-430/screenspace_coherence.frag");
	FragShaderFile.push_back("hz-430/multi-draw-indirect.frag");
	FragShaderFile.push_back("hz-430/draw-uniform2.frag");
	FragShaderFile.push_back("hz-430/draw-uniform.frag");
	FragShaderFile.push_back("hz-430/draw-range.frag");
	FragShaderName.resize(FragShaderFile.size());

	ProgramName.resize(FragShaderFile.size());

	return Success;
}

bool testCompiler::end()
{
	return true;
}

bool testCompiler::render()
{
	compiler Compiler;

	this->beginTimer();

	switch(this->Mode)
	{
		case MULTITHREADED:
		{
			for(std::size_t ShaderIndex = 0; ShaderIndex < VertShaderFile.size(); ++ShaderIndex)
			{
				VertShaderName[ShaderIndex] = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VertShaderFile[ShaderIndex], "--version 420 --profile core");
				FragShaderName[ShaderIndex] = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FragShaderFile[ShaderIndex], "--version 420 --profile core");
			}

			for(std::size_t ProgramIndex = 0; ProgramIndex < ProgramName.size(); ++ProgramIndex)
			{
				ProgramName[ProgramIndex] = glCreateProgram();
				glAttachShader(ProgramName[ProgramIndex], VertShaderName[ProgramIndex]);
				glAttachShader(ProgramName[ProgramIndex], FragShaderName[ProgramIndex]);
				glLinkProgram(ProgramName[ProgramIndex]);
			}

			Compiler.check();
			for(std::size_t ProgramIndex = 0; ProgramIndex < ProgramName.size(); ++ProgramIndex)
				Compiler.checkProgram(ProgramName[ProgramIndex]);
		}
		break;
		case DUALTHREADED:
		{
			for(std::size_t ShaderIndex = 0; ShaderIndex < VertShaderFile.size(); ++ShaderIndex)
			{
				VertShaderName[ShaderIndex] = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VertShaderFile[ShaderIndex], "--version 420 --profile core");
				FragShaderName[ShaderIndex] = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FragShaderFile[ShaderIndex], "--version 420 --profile core");
				Compiler.check();
			}

			for(std::size_t ProgramIndex = 0; ProgramIndex < ProgramName.size(); ++ProgramIndex)
			{
				ProgramName[ProgramIndex] = glCreateProgram();
				glAttachShader(ProgramName[ProgramIndex], VertShaderName[ProgramIndex]);
				glAttachShader(ProgramName[ProgramIndex], FragShaderName[ProgramIndex]);
				glLinkProgram(ProgramName[ProgramIndex]);
				Compiler.checkProgram(ProgramName[ProgramIndex]);
			}
		}
		break;
		case SINGLETHREADED:
		{
			for(std::size_t ProgramIndex = 0; ProgramIndex < ProgramName.size(); ++ProgramIndex)
			{
				VertShaderName[ProgramIndex] = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VertShaderFile[ProgramIndex], "--version 420 --profile core");
				FragShaderName[ProgramIndex] = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FragShaderFile[ProgramIndex], "--version 420 --profile core");
				Compiler.check();

				ProgramName[ProgramIndex] = glCreateProgram();
				glAttachShader(ProgramName[ProgramIndex], VertShaderName[ProgramIndex]);
				glAttachShader(ProgramName[ProgramIndex], FragShaderName[ProgramIndex]);
				glLinkProgram(ProgramName[ProgramIndex]);
				Compiler.checkProgram(ProgramName[ProgramIndex]);
			}
		}
		break;
	}

	for(std::size_t ProgramIndex = 0; ProgramIndex < ProgramName.size(); ++ProgramIndex)
		glUseProgram(ProgramName[ProgramIndex]);

	this->endTimer();

	return true;
}
