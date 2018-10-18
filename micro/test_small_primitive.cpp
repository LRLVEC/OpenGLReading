///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Samples Pack (ogl-samples.g-truc.net)
///
/// Copyright (c) 2004 - 2014 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////

#include "test.hpp"
#include "test_small_primitive.hpp"

namespace
{
	char const * VERT_SHADER_SOURCE("micro/test_small_primitive.vert");
	char const * FRAG_SHADER_SOURCE("micro/test_small_primitive.frag");

	struct vertex
	{
		vertex(){}

		vertex
		(
			glm::vec3 const & Position,
			glm::u8vec4 const & Color
		) :
			Position(Position),
			Color(Color)
		{}

		glm::vec3 Position;
		glm::u8vec4 Color;
	};

	namespace buffer
	{
		enum type
		{
			VERTEX,
			TRANSFORM,
			MAX
		};
	}//namespace buffer

	enum layout
	{
		LAYOUT_LINEAR,
		LAYOUT_MORTON,
		LAYOUT_RANDOM
	};
}//namespace

class test_small_primitive : public test
{
public:
	test_small_primitive(int argc, char* argv[], std::size_t FrameCount, glm::uvec2 const & WindowSize, glm::vec2 const & TileSize, std::size_t DrawCount, layout Layout) :
		test(argc, argv, "test_small_primitive", test::CORE, 4, 2, FrameCount, RUN_ONLY, WindowSize),
		VertexArrayName(0),
		ProgramName(0),
		SamplerName(0),
		TextureName(0),
		VertexCount(0),
		TileSize(TileSize),
		DrawCount(DrawCount),
		Layout(Layout)
	{
		assert((Layout == LAYOUT_MORTON && TileSize.x == TileSize.y && glm::bitCount(glm::uint(TileSize.x)) == 1u) || Layout != LAYOUT_MORTON);
	}

private:
	std::array<GLuint, buffer::MAX> BufferName;
	GLuint VertexArrayName;
	GLuint PipelineName;
	GLuint ProgramName;
	GLuint SamplerName;
	GLuint TextureName;
	GLsizei VertexCount;
	glm::vec2 const TileSize;
	std::size_t const DrawCount;
	layout const Layout;

	bool initProgram()
	{
		bool Validated = true;
	
		if(Validated)
		{
			compiler Compiler;
			GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, getDataDirectory() + VERT_SHADER_SOURCE);
			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, getDataDirectory() + FRAG_SHADER_SOURCE);

			ProgramName = glCreateProgram();
			glProgramParameteri(this->ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
			glAttachShader(this->ProgramName, VertShaderName);
			glAttachShader(this->ProgramName, FragShaderName);
			glLinkProgram(this->ProgramName);

			Validated = Validated && Compiler.check();
			Validated = Validated && Compiler.checkProgram(this->ProgramName);
		}

		if(Validated)
		{
			glGenProgramPipelines(1, &this->PipelineName);
			glUseProgramStages(this->PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, this->ProgramName);
		}

		return Validated;
	}

	bool initBuffer()
	{
		std::size_t tileAddress();
		
		glm::vec2 const WindowSize(this->getWindowSize());

		glm::uvec2 const TileCount((WindowSize / this->TileSize) + glm::mix(glm::vec2(0), glm::vec2(1), glm::greaterThan(glm::mod(glm::vec2(WindowSize), this->TileSize), glm::vec2(0))));
		std::vector<vertex> Vertices;
		Vertices.resize(TileCount.x * TileCount.y * 6);
		VertexCount = static_cast<GLsizei>(Vertices.size());

		for(glm::uint TileIndexY = 0; TileIndexY < TileCount.y; ++TileIndexY)
		for(glm::uint TileIndexX = 0; TileIndexX < TileCount.x; ++TileIndexX)
		{
			glm::uint TileIndex = 0;

			switch(this->Layout)
			{
			case LAYOUT_LINEAR:
				TileIndex = (TileIndexX + TileIndexY * TileCount.x);
				break;
			case LAYOUT_MORTON:
				TileIndex = glm::bitfieldInterleave(TileIndexY, TileIndexX);
				break;
			case LAYOUT_RANDOM:
			{
				glm::uvec2 RandTileIndex(glm::clamp(glm::linearRand(glm::vec2(0.0f), glm::vec2(TileCount - 1u)), glm::vec2(0.0f), glm::vec2(TileCount - 1u)));
				TileIndex = (RandTileIndex.x + RandTileIndex.y * TileCount.x);
				break;
			}
			default: // Unknown layout
				assert(0);
			}

			//glm::vec4 const RGBA(glm::rgbColor(glm::vec3(float(TileIndex % 255) / 255.f * 360.f, 1.0f, 1.0f)), 1.0f);
			//glm::u8vec4 Color(RGBA * 255.f);

			glm::vec4 const RandColor = glm::linearRand(glm::vec4(0.0), glm::vec4(255.0));
			glm::u8vec4 Color(glm::clamp(RandColor, glm::vec4(0.0), glm::vec4(255.0)));

			Vertices[TileIndex * 6 + 0] = vertex(glm::vec3((TileIndexX + 0) * this->TileSize.x, (TileIndexY + 0) * this->TileSize.y, 0.0f), Color);
			Vertices[TileIndex * 6 + 1] = vertex(glm::vec3((TileIndexX + 1) * this->TileSize.x, (TileIndexY + 0) * this->TileSize.y, 0.0f), Color);
			Vertices[TileIndex * 6 + 2] = vertex(glm::vec3((TileIndexX + 1) * this->TileSize.x, (TileIndexY + 1) * this->TileSize.y, 0.0f), Color);
			Vertices[TileIndex * 6 + 3] = vertex(glm::vec3((TileIndexX + 1) * this->TileSize.x, (TileIndexY + 1) * this->TileSize.y, 0.0f), Color);
			Vertices[TileIndex * 6 + 4] = vertex(glm::vec3((TileIndexX + 0) * this->TileSize.x, (TileIndexY + 1) * this->TileSize.y, 0.0f), Color);
			Vertices[TileIndex * 6 + 5] = vertex(glm::vec3((TileIndexX + 0) * this->TileSize.x, (TileIndexY + 0) * this->TileSize.y, 0.0f), Color);
		}

		GLsizei VertexSize = static_cast<GLsizei>(Vertices.size() * sizeof(vertex));

		glm::mat4 Perspective = glm::ortho(0.0f, static_cast<float>(WindowSize.x), 0.0f, static_cast<float>(WindowSize.y));

		glGenBuffers(buffer::MAX, &BufferName[0]);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, VertexSize, &Vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Perspective), &Perspective[0][0], GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return true;
	}

	bool initVertexArray()
	{
		glGenVertexArrays(1, &VertexArrayName);
		glBindVertexArray(VertexArrayName);
			glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(semantic::attr::POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), BUFFER_OFFSET(0));
			glVertexAttribPointer(semantic::attr::COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex), BUFFER_OFFSET(sizeof(glm::vec3)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(semantic::attr::POSITION);
			glEnableVertexAttribArray(semantic::attr::COLOR);
		glBindVertexArray(0);

		return true;
	}

	bool init()
	{
		bool Validated = true;
		if(Validated)
			Validated = initBuffer();
		if(Validated)
			Validated = initVertexArray();
		if(Validated)
			Validated = initProgram();
		return Validated;
	}

	bool begin()
	{
		bool Validated = this->init();

		glm::vec2 WindowSize(this->getWindowSize());

		glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.5f, 1.0f, 1.0f)[0]);

		glUseProgram(ProgramName);

		glBindVertexArray(VertexArrayName);
		glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

		glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);

		return Validated;
	}

	bool end()
	{
		glDeleteBuffers(buffer::MAX, &this->BufferName[0]);
		glDeleteProgramPipelines(1, &this->PipelineName);
		glDeleteProgram(this->ProgramName);
		glDeleteVertexArrays(1, &this->VertexArrayName);

		return true;
	}

	bool render()
	{
		this->beginTimer();

		for(std::size_t DrawIndex = 0; DrawIndex < this->DrawCount; ++DrawIndex)
			glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, VertexCount, 2, 0);

		this->endTimer();

		return true;
	}
};

struct entry
{
	entry(
		std::string const & String,
		glm::uvec2 const & WindowSize,
		glm::vec2 const & TileSize,
		std::size_t const & DrawCount,
		layout Layout
	) :
		String(String),
		WindowSize(WindowSize),
		TileSize(TileSize),
		DrawCount(DrawCount),
		Layout(Layout)
	{}

	std::string const String;
	glm::uvec2 const WindowSize;
	glm::vec2 const TileSize;
	std::size_t const DrawCount;
	layout Layout;
};

int main_small_primitive_debug(int argc, char* argv[])
{
	std::vector<entry> Entries;

	Entries.push_back(entry("tile(1, 1)", glm::uvec2(640, 480), glm::vec2(1, 1), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(8, 8)", glm::uvec2(640, 480), glm::vec2(8, 8), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(32, 2)", glm::uvec2(640, 480), glm::vec2(32, 2), 1, LAYOUT_LINEAR));

	csv CSV;
	int Error(0);

	for(std::size_t EntryIndex(0); EntryIndex < Entries.size(); ++EntryIndex)
	{
		test_small_primitive Test(
			argc, argv,
			0,
			Entries[EntryIndex].WindowSize,
			Entries[EntryIndex].TileSize,
			Entries[EntryIndex].DrawCount,
			Entries[EntryIndex].Layout);

		Error += Test();
		Test.log(CSV, Entries[EntryIndex].String.c_str());
	}

	CSV.save("../main_small_primitive_debug.csv");

	return Error;
}

int main_small_primitive1(int argc, char* argv[])
{
	std::vector<entry> Entries;

	Entries.push_back(entry("tile(1, 1)", glm::uvec2(1920, 1080), glm::vec2(1, 1), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(2, 2)", glm::uvec2(1920, 1080), glm::vec2(2, 2), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(4, 4)", glm::uvec2(1920, 1080), glm::vec2(4, 4), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(8, 8)", glm::uvec2(1920, 1080), glm::vec2(8, 8), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(16, 16)", glm::uvec2(1920, 1080), glm::vec2(16, 16), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(32, 32)", glm::uvec2(1920, 1080), glm::vec2(32, 32), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(64, 64)", glm::uvec2(1920, 1080), glm::vec2(64, 64), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(128, 128)", glm::uvec2(1920, 1080), glm::vec2(128, 128), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(1920, 1080)", glm::uvec2(1920, 1080), glm::vec2(1920, 1080), 1, LAYOUT_LINEAR));

	csv CSV;
	int Error(0);

	for(std::size_t EntryIndex(0); EntryIndex < Entries.size(); ++EntryIndex)
	{
		test_small_primitive Test(
			argc, argv,
			1000,
			Entries[EntryIndex].WindowSize,
			Entries[EntryIndex].TileSize,
			Entries[EntryIndex].DrawCount,
			Entries[EntryIndex].Layout);

		Error += Test();
		Test.log(CSV, Entries[EntryIndex].String.c_str());
	}

	CSV.save("../main_small_primitive1.csv");

	return Error;
}

int main_small_primitive2(int argc, char* argv[])
{
	std::vector<entry> Entries;

	Entries.push_back(entry("tile(4, 4)", glm::uvec2(1920, 1080), glm::vec2(4, 4), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(2, 8)", glm::uvec2(1920, 1080), glm::vec2(2, 8), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(8, 2)", glm::uvec2(1920, 1080), glm::vec2(8, 2), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(1, 16)", glm::uvec2(1920, 1080), glm::vec2(1, 16), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(16, 1)", glm::uvec2(1920, 1080), glm::vec2(16, 1), 1, LAYOUT_LINEAR));

	Entries.push_back(entry("tile(8, 8)", glm::uvec2(1920, 1080), glm::vec2(8, 8), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(4, 16)", glm::uvec2(1920, 1080), glm::vec2(4, 16), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(16, 4)", glm::uvec2(1920, 1080), glm::vec2(16, 4), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(2, 32)", glm::uvec2(1920, 1080), glm::vec2(2, 32), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(32, 2)", glm::uvec2(1920, 1080), glm::vec2(32, 2), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(1, 64)", glm::uvec2(1920, 1080), glm::vec2(1, 64), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(64, 1)", glm::uvec2(1920, 1080), glm::vec2(64, 1), 1, LAYOUT_LINEAR));

	Entries.push_back(entry("tile(16, 16)", glm::uvec2(1920, 1080), glm::vec2(16, 16), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(8, 32)", glm::uvec2(1920, 1080), glm::vec2(8, 32), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(32, 8)", glm::uvec2(1920, 1080), glm::vec2(32, 8), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(4, 64)", glm::uvec2(1920, 1080), glm::vec2(4, 64), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(64, 4)", glm::uvec2(1920, 1080), glm::vec2(64, 4), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(2, 128)", glm::uvec2(1920, 1080), glm::vec2(2, 128), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(128, 2)", glm::uvec2(1920, 1080), glm::vec2(128, 2), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(1, 256)", glm::uvec2(1920, 1080), glm::vec2(1, 256), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(256, 1)", glm::uvec2(1920, 1080), glm::vec2(256, 1), 1, LAYOUT_LINEAR));

	csv CSV;
	int Error(0);

	for(std::size_t EntryIndex(0); EntryIndex < Entries.size(); ++EntryIndex)
	{
		test_small_primitive Test(
			argc, argv,
			1000,
			Entries[EntryIndex].WindowSize,
			Entries[EntryIndex].TileSize,
			Entries[EntryIndex].DrawCount,
			Entries[EntryIndex].Layout);

		Error += Test();
		Test.log(CSV, Entries[EntryIndex].String.c_str());
	}

	CSV.save("../main_small_primitive2.csv");

	return Error;
}

int main_small_primitive3(int argc, char* argv[])
{
	std::vector<entry> Entries;

	Entries.push_back(entry("tile(0.125, 0.125)", glm::uvec2(320, 240), glm::vec2(0.125, 0.125), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(0.25, 0.25)", glm::uvec2(320, 240), glm::vec2(0.25, 0.25), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(0.5, 0.5)", glm::uvec2(320, 240), glm::vec2(0.5, 0.5), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(1, 1)", glm::uvec2(320, 240), glm::vec2(1, 1), 1, LAYOUT_LINEAR));

	csv CSV;
	int Error(0);

	for(std::size_t EntryIndex(0); EntryIndex < Entries.size(); ++EntryIndex)
	{
		test_small_primitive Test(
			argc, argv,
			1000,
			Entries[EntryIndex].WindowSize,
			Entries[EntryIndex].TileSize,
			Entries[EntryIndex].DrawCount,
			Entries[EntryIndex].Layout);

		Error += Test();
		Test.log(CSV, Entries[EntryIndex].String.c_str());
	}

	CSV.save("../main_small_primitive3.csv");

	return Error;
}

int main_small_primitive4_memory_layout(int argc, char* argv[])
{
	std::vector<entry> Entries;

	Entries.push_back(entry("tile(1, 1)", glm::uvec2(1024, 1024), glm::vec2(1, 1), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(1, 1)", glm::uvec2(1024, 1024), glm::vec2(1, 1), 1, LAYOUT_MORTON));
	Entries.push_back(entry("tile(1, 1)", glm::uvec2(1024, 1024), glm::vec2(1, 1), 1, LAYOUT_RANDOM));
	Entries.push_back(entry("tile(2, 2)", glm::uvec2(1024, 1024), glm::vec2(2, 2), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(2, 2)", glm::uvec2(1024, 1024), glm::vec2(2, 2), 1, LAYOUT_MORTON));
	Entries.push_back(entry("tile(2, 2)", glm::uvec2(1024, 1024), glm::vec2(2, 2), 1, LAYOUT_RANDOM));
	Entries.push_back(entry("tile(4, 4)", glm::uvec2(1024, 1024), glm::vec2(4, 4), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(4, 4)", glm::uvec2(1024, 1024), glm::vec2(4, 4), 1, LAYOUT_MORTON));
	Entries.push_back(entry("tile(4, 4)", glm::uvec2(1024, 1024), glm::vec2(4, 4), 1, LAYOUT_RANDOM));
	Entries.push_back(entry("tile(8, 8)", glm::uvec2(1024, 1024), glm::vec2(8, 8), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(8, 8)", glm::uvec2(1024, 1024), glm::vec2(8, 8), 1, LAYOUT_MORTON));
	Entries.push_back(entry("tile(8, 8)", glm::uvec2(1024, 1024), glm::vec2(8, 8), 1, LAYOUT_RANDOM));
	Entries.push_back(entry("tile(16, 16)", glm::uvec2(1024, 1024), glm::vec2(16, 16), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(16, 16)", glm::uvec2(1024, 1024), glm::vec2(16, 16), 1, LAYOUT_MORTON));
	Entries.push_back(entry("tile(16, 16)", glm::uvec2(1024, 1024), glm::vec2(16, 16), 1, LAYOUT_RANDOM));

	csv CSV;
	int Error(0);

	for(std::size_t EntryIndex(0); EntryIndex < Entries.size(); ++EntryIndex)
	{
		test_small_primitive Test(
			argc, argv,
			1000,
			Entries[EntryIndex].WindowSize,
			Entries[EntryIndex].TileSize,
			Entries[EntryIndex].DrawCount,
			Entries[EntryIndex].Layout);

		Error += Test();
		Test.log(CSV, Entries[EntryIndex].String.c_str());
	}

	CSV.save("../main_small_primitive4.csv");

	return Error;
}

int main_small_primitive5(int argc, char* argv[])
{
	std::vector<entry> Entries;

	Entries.push_back(entry("tile(128, 128)", glm::uvec2(640, 480), glm::vec2(128, 128), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(64, 64)", glm::uvec2(640, 480), glm::vec2(64, 64), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(32, 32)", glm::uvec2(640, 480), glm::vec2(32, 32), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(16, 16)", glm::uvec2(640, 480), glm::vec2(16, 16), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(8, 8)", glm::uvec2(640, 480), glm::vec2(8, 8), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(4, 4)", glm::uvec2(640, 480), glm::vec2(4, 4), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(2, 2)", glm::uvec2(640, 480), glm::vec2(2, 2), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(1, 1)", glm::uvec2(640, 480), glm::vec2(1, 1), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(0.5, 0.5)", glm::uvec2(640, 480), glm::vec2(0.5, 0.5), 1, LAYOUT_LINEAR));
	Entries.push_back(entry("tile(0.25, 0.25)", glm::uvec2(640, 480), glm::vec2(0.25, 0.25), 1, LAYOUT_LINEAR));
	//Entries.push_back(entry("tile(0.125, 0.125)", glm::uvec2(640, 480), glm::vec2(0.125, 0.125), 1, LAYOUT_LINEAR));

	csv CSV;
	int Error(0);

	for(std::size_t EntryIndex(0); EntryIndex < Entries.size(); ++EntryIndex)
	{
		test_small_primitive Test(
			argc, argv,
			100,
			Entries[EntryIndex].WindowSize,
			Entries[EntryIndex].TileSize,
			Entries[EntryIndex].DrawCount,
			Entries[EntryIndex].Layout);

		Error += Test();
		Test.log(CSV, Entries[EntryIndex].String.c_str());
	}

	CSV.save("../main_small_primitive5.csv");

	return Error;
}

int main_small_primitive6(int argc, char* argv[])
{
	std::vector<entry> Entries;

	for(std::size_t i = 8; i <= 1024; i *= 2)
	{
		//Entries.push_back(entry("(2048, 1024) tile(32, 32)", glm::uvec2(2048, 1024), glm::vec2(32, 32), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(1536, 768) tile(24, 24)", glm::uvec2(1536, 768), glm::vec2(24, 24), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(1024, 512) tile(16, 16)", glm::uvec2(1024, 512), glm::vec2(16, 16), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(768, 384) tile(12, 12)", glm::uvec2(768, 384), glm::vec2(12, 12), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(512, 256) tile(8, 8)", glm::uvec2(512, 256), glm::vec2(8, 8), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(384, 192) tile(6, 6)", glm::uvec2(384, 192), glm::vec2(6, 6), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(256, 128) tile(4, 4)", glm::uvec2(256, 128), glm::vec2(4, 4), i, LAYOUT_LINEAR));
	}

/*
	for(std::size_t i = 1; i <= 256; i *= 2)
	{
		Entries.push_back(entry("(2048, 1024) tile(64, 64)", glm::uvec2(2048, 1024), glm::uvec2(64, 64), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(1536, 768) tile(48, 48)", glm::uvec2(1536, 768), glm::uvec2(48, 48), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(1024, 512) tile(32, 32)", glm::uvec2(1024, 512), glm::uvec2(32, 32), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(768, 384) tile(24, 24)", glm::uvec2(768, 384), glm::uvec2(24, 24), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(512, 256) tile(16, 16)", glm::uvec2(512, 256), glm::uvec2(16, 16), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(384, 192) tile(12, 12)", glm::uvec2(384, 192), glm::uvec2(12, 12), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(256, 128) tile(8, 8)", glm::uvec2(256, 128), glm::uvec2(8, 8), i, LAYOUT_LINEAR));
	}
*/
/*
	for(std::size_t i = 1; i < 32; i *= 2)
	{
		Entries.push_back(entry("(1920, 1200) tile(64, 64)", glm::uvec2(1920, 1200), glm::uvec2(64, 64), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(1920, 1200) tile(32, 32)", glm::uvec2(1920, 1200), glm::uvec2(32, 32), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(1920, 1200) tile(16, 16)", glm::uvec2(1920, 1200), glm::uvec2(16, 16), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(1920, 1200) tile(8, 8)", glm::uvec2(1920, 1200), glm::uvec2(8, 8), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(1920, 1200) tile(4, 4)", glm::uvec2(1920, 1200), glm::uvec2(4, 4), i, LAYOUT_LINEAR));

		Entries.push_back(entry("(1440, 900) tile(64, 64)", glm::uvec2(1440, 900), glm::uvec2(64, 64), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(1440, 900) tile(32, 32)", glm::uvec2(1440, 900), glm::uvec2(32, 32), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(1440, 900) tile(16, 16)", glm::uvec2(1440, 900), glm::uvec2(16, 16), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(1440, 900) tile(8, 8)", glm::uvec2(1440, 900), glm::uvec2(8, 8), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(1440, 900) tile(4, 4)", glm::uvec2(1440, 900), glm::uvec2(4, 4), i, LAYOUT_LINEAR));

		Entries.push_back(entry("(960, 600) tile(64, 64)", glm::uvec2(960, 600), glm::uvec2(64, 64), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(960, 600) tile(32, 32)", glm::uvec2(960, 600), glm::uvec2(32, 32), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(960, 600) tile(16, 16)", glm::uvec2(960, 600), glm::uvec2(16, 16), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(960, 600) tile(8, 8)", glm::uvec2(960, 600), glm::uvec2(8, 8), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(960, 600) tile(4, 4)", glm::uvec2(960, 600), glm::uvec2(4, 4), i, LAYOUT_LINEAR));

		Entries.push_back(entry("(720, 450) tile(64, 64)", glm::uvec2(720, 450), glm::uvec2(64, 64), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(720, 450) tile(32, 32)", glm::uvec2(720, 450), glm::uvec2(32, 32), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(720, 450) tile(16, 16)", glm::uvec2(720, 450), glm::uvec2(16, 16), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(720, 450) tile(8, 8)", glm::uvec2(720, 450), glm::uvec2(8, 8), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(720, 450) tile(4, 4)", glm::uvec2(720, 450), glm::uvec2(4, 4), i, LAYOUT_LINEAR));

		Entries.push_back(entry("(480, 300) tile(64, 64)", glm::uvec2(480, 300), glm::uvec2(64, 64), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(480, 300) tile(32, 32)", glm::uvec2(480, 300), glm::uvec2(32, 32), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(480, 300) tile(16, 16)", glm::uvec2(480, 300), glm::uvec2(16, 16), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(480, 300) tile(8, 8)", glm::uvec2(480, 300), glm::uvec2(8, 8), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(480, 300) tile(4, 4)", glm::uvec2(480, 300), glm::uvec2(4, 4), i, LAYOUT_LINEAR));

		Entries.push_back(entry("(360, 240) tile(64, 64)", glm::uvec2(360, 240), glm::uvec2(64, 64), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(360, 240) tile(32, 32)", glm::uvec2(360, 240), glm::uvec2(32, 32), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(360, 240) tile(16, 16)", glm::uvec2(360, 240), glm::uvec2(16, 16), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(360, 240) tile(8, 8)", glm::uvec2(360, 240), glm::uvec2(8, 8), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(360, 240) tile(4, 4)", glm::uvec2(360, 240), glm::uvec2(4, 4), i, LAYOUT_LINEAR));

		Entries.push_back(entry("(240, 160) tile(64, 64)", glm::uvec2(240, 160), glm::uvec2(64, 64), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(240, 160) tile(32, 32)", glm::uvec2(240, 160), glm::uvec2(32, 32), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(240, 160) tile(16, 16)", glm::uvec2(240, 160), glm::uvec2(16, 16), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(240, 160) tile(8, 8)", glm::uvec2(240, 160), glm::uvec2(8, 8), i, LAYOUT_LINEAR));
		Entries.push_back(entry("(240, 160) tile(4, 4)", glm::uvec2(240, 160), glm::uvec2(4, 4), i, LAYOUT_LINEAR));
	}
*/
	csv CSV;
	int Error(0);

	for(std::size_t EntryIndex(0); EntryIndex < Entries.size(); ++EntryIndex)
	{
		test_small_primitive Test(
			argc, argv,
			200,
			Entries[EntryIndex].WindowSize,
			Entries[EntryIndex].TileSize,
			Entries[EntryIndex].DrawCount,
			Entries[EntryIndex].Layout);

		Error += Test();
		Test.log(CSV, Entries[EntryIndex].String.c_str());
	}

	CSV.save("../main_small_primitive6.csv");

	return Error;
}

