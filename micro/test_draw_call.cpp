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
#include "test_draw_call.hpp"

namespace
{
	char const * VERT_SHADER_SOURCE("micro/test_draw_call.vert");
	char const * FRAG_SHADER_SOURCE("micro/test_draw_call.frag");

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

	enum drawMode
	{
		DRAW_SINGLE,
		DRAW_PER_TILE
	};

	enum layout
	{
		LAYOUT_LINEAR,
		LAYOUT_MORTON,
		LAYOUT_RANDOM
	};
}//namespace

class test_draw_call : public test
{
public:
	test_draw_call(int argc, char* argv[], std::size_t FrameCount, glm::uvec2 const & WindowSize, glm::vec2 const & TileSize, std::size_t TrianglePairPerTile, std::size_t DrawPerTile, layout Layout, drawMode DrawMode) :
		test(argc, argv, "test_draw_call", test::CORE, 3, 3, FrameCount, RUN_ONLY, WindowSize),
		VertexArrayName(0),
		ProgramName(0),
		SamplerName(0),
		TextureName(0),
		VertexCount(0),
		TileSize(TileSize),
		TrianglePairPerTile(TrianglePairPerTile),
		DrawPerTile(DrawPerTile),
		Layout(Layout),
		DrawMode(DrawMode)
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
	std::size_t const TrianglePairPerTile;
	std::size_t const DrawPerTile;
	layout const Layout;
	drawMode const DrawMode;

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
		Vertices.resize(TileCount.x * TileCount.y * 6 * this->TrianglePairPerTile);
		VertexCount = static_cast<GLsizei>(Vertices.size());

		for(glm::uint TileIndexY = 0; TileIndexY < TileCount.y; ++TileIndexY)
		for(glm::uint TileIndexX = 0; TileIndexX < TileCount.x; ++TileIndexX)
		{
			glm::uint TileIndex = (TileIndexX + TileIndexY * TileCount.x);

			glm::vec4 const RandColor = glm::linearRand(glm::vec4(0.0), glm::vec4(255.0));
			glm::u8vec4 Color(glm::clamp(RandColor, glm::vec4(0.0), glm::vec4(255.0)));

			for(std::size_t DrawIndex = 0; DrawIndex < this->TrianglePairPerTile; ++DrawIndex)
			{
				Vertices[TileIndex * (6 * this->TrianglePairPerTile) + DrawIndex * 6 + 0] = vertex(glm::vec3((TileIndexX + 0) * this->TileSize.x, (TileIndexY + 0) * this->TileSize.y, static_cast<float>(2 + DrawIndex)), Color);
				Vertices[TileIndex * (6 * this->TrianglePairPerTile) + DrawIndex * 6 + 1] = vertex(glm::vec3((TileIndexX + 1) * this->TileSize.x, (TileIndexY + 0) * this->TileSize.y, static_cast<float>(2 + DrawIndex)), Color);
				Vertices[TileIndex * (6 * this->TrianglePairPerTile) + DrawIndex * 6 + 2] = vertex(glm::vec3((TileIndexX + 1) * this->TileSize.x, (TileIndexY + 1) * this->TileSize.y, static_cast<float>(2 + DrawIndex)), Color);
				Vertices[TileIndex * (6 * this->TrianglePairPerTile) + DrawIndex * 6 + 3] = vertex(glm::vec3((TileIndexX + 1) * this->TileSize.x, (TileIndexY + 1) * this->TileSize.y, static_cast<float>(2 + DrawIndex)), Color);
				Vertices[TileIndex * (6 * this->TrianglePairPerTile) + DrawIndex * 6 + 4] = vertex(glm::vec3((TileIndexX + 0) * this->TileSize.x, (TileIndexY + 1) * this->TileSize.y, static_cast<float>(2 + DrawIndex)), Color);
				Vertices[TileIndex * (6 * this->TrianglePairPerTile) + DrawIndex * 6 + 5] = vertex(glm::vec3((TileIndexX + 0) * this->TileSize.x, (TileIndexY + 0) * this->TileSize.y, static_cast<float>(2 + DrawIndex)), Color);
			}
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

		if(Validated)
		{
			glm::vec2 WindowSize(this->getWindowSize());

			glUseProgram(ProgramName);

			glBindVertexArray(VertexArrayName);
			glBindBufferBase(GL_UNIFORM_BUFFER, semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

			glViewportIndexedf(0, 0, 0, WindowSize.x, WindowSize.y);

			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
		}

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
		float Depth(1.0f);
		glClearBufferfv(GL_DEPTH, 0, &Depth);

		this->beginTimer();

		switch(this->DrawMode)
		{
			case DRAW_SINGLE:
			{
				glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
			}
			break;
			case DRAW_PER_TILE:
			{
				for(std::size_t i = 0; i < VertexCount; i += 6 * this->TrianglePairPerTile / this->DrawPerTile)
				{
					glDrawArraysInstanced(GL_TRIANGLES,
						static_cast<GLint>(i),
						static_cast<GLsizei>(6 * this->TrianglePairPerTile / this->DrawPerTile),
						1);
				}
			}
			break;
			default:
			{
				assert(0);
			}
			break;
		}

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
		std::size_t const & TrianglePairPerTile,
		std::size_t const & DrawPerTile,
		layout Layout,
		drawMode DrawMode
	) :
		String(String),
		WindowSize(WindowSize),
		TileSize(TileSize),
		TrianglePairPerTile(TrianglePairPerTile),
		DrawPerTile(DrawPerTile),
		Layout(Layout),
		DrawMode(DrawMode)
	{}

	std::string const String;
	glm::uvec2 const WindowSize;
	glm::vec2 const TileSize;
	std::size_t const TrianglePairPerTile;
	std::size_t const DrawPerTile;
	layout Layout;
	drawMode DrawMode;
};

int main_draw_call(int argc, char* argv[])
{
	std::vector<entry> Entries;

	for(glm::uint TileSizeIndex = 3; TileSizeIndex < 4; ++TileSizeIndex)
	{	
		for(std::size_t DrawPerTile = 1; DrawPerTile <= 512; DrawPerTile <<= 1)
			Entries.push_back(entry(
			message_format("window(%d), tile(%d), triangle-per-draw(%d)", 64 * (TileSizeIndex + 1), 8 * (TileSizeIndex + 1), 1024 / DrawPerTile),
			glm::uvec2(64) * (TileSizeIndex + 1), glm::vec2(glm::uvec2(8, 8) * (TileSizeIndex + 1)), 512, DrawPerTile, LAYOUT_LINEAR, DRAW_PER_TILE));
	}

	csv CSV;
	int Error(0);

	for(std::size_t EntryIndex(0); EntryIndex < Entries.size(); ++EntryIndex)
	{
		test_draw_call Test(
			argc, argv,
			1000,
			Entries[EntryIndex].WindowSize,
			Entries[EntryIndex].TileSize,
			Entries[EntryIndex].TrianglePairPerTile,
			Entries[EntryIndex].DrawPerTile,
			Entries[EntryIndex].Layout,
			Entries[EntryIndex].DrawMode);

		Error += Test();
		Test.log(CSV, Entries[EntryIndex].String.c_str());
	}

	CSV.save("../main_draw_call.csv");

	return Error;
}

