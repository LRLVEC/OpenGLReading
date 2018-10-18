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
#include "test_uniform_caching.hpp"

namespace
{
	char const * VERT_SHADER_SOURCE("micro/test_uniform_caching.vert");
	char const * FRAG_SHADER_SOURCE("micro/test_uniform_caching.frag");

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
			MAX
		};
	}//namespace buffer

	enum uniformMode
	{
		UNIFORM_SINGLE,
		UNIFORM_REDUNDANT,
		UNIFORM_UNIQUE
	};
}//namespace

class test_uniform_caching : public test
{
public:
	test_uniform_caching(int argc, char* argv[], std::size_t FrameCount, glm::uvec2 const & WindowSize, glm::vec2 const & TileSize, std::size_t TrianglePairPerTile, uniformMode UniformMode) :
		test(argc, argv, "test_uniform_caching", test::CORE, 3, 3, FrameCount, RUN_ONLY, WindowSize),
		VertexArrayName(0),
		ProgramName(0),
		SamplerName(0),
		TextureName(0),
		VertexCount(0),
		TileSize(TileSize),
		TrianglePairPerTile(TrianglePairPerTile),
		UniformDiffuse(-1),
		UniformMVP(-1),
		UniformMode(UniformMode)
	{}

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
	GLint UniformDiffuse;
	GLint UniformMVP;
	uniformMode UniformMode;

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

		if(Validated)
		{
			UniformDiffuse = glGetUniformLocation(ProgramName, "Diffuse");
			UniformMVP = glGetUniformLocation(ProgramName, "MVP");
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

		glGenBuffers(buffer::MAX, &BufferName[0]);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glBufferData(GL_ARRAY_BUFFER, VertexSize, &Vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

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
			glm::mat4 Perspective = glm::ortho(0.0f, WindowSize.x, 0.0f, WindowSize.y);

			glUseProgram(ProgramName);
			glUniformMatrix4fv(UniformMVP, 1, GL_FALSE, &Perspective[0][0]);

			glBindVertexArray(VertexArrayName);
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

		if(this->UniformMode == UNIFORM_SINGLE)
			glUniform4f(UniformDiffuse, 1.0f, 0.5f, 0.0f, 1.0f);

		for(std::size_t i = 0; i < VertexCount; i += 6)
		{
			if(this->UniformMode == UNIFORM_REDUNDANT)
				glUniform4f(UniformDiffuse, 1.0f, 0.5f, 0.0f, 1.0f);
			if(this->UniformMode == UNIFORM_UNIQUE)
			{
				glm::vec4 Color = glm::linearRand(glm::vec4(0), glm::vec4(1));
				glUniform4f(UniformDiffuse, Color.r, Color.g, Color.b, Color.a);
			}

			glDrawArraysInstanced(GL_TRIANGLES, static_cast<GLint>(i), static_cast<GLsizei>(6), 1);
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
		uniformMode const & UniformMode
	) :
		String(String),
		WindowSize(WindowSize),
		TileSize(TileSize),
		TrianglePairPerTile(TrianglePairPerTile),
		UniformMode(UniformMode)
	{}

	std::string const String;
	glm::uvec2 const WindowSize;
	glm::vec2 const TileSize;
	std::size_t const TrianglePairPerTile;
	uniformMode UniformMode;
};

int main_uniform_caching(int argc, char* argv[])
{
	std::vector<entry> Entries;

	Entries.push_back(entry(
		message_format("UNIFORM_SINGLE mode"),
		glm::uvec2(128), glm::vec2(8, 8), 16, UNIFORM_SINGLE));
	Entries.push_back(entry(
		message_format("UNIFORM_REDUNDANT mode"),
		glm::uvec2(128), glm::vec2(8, 8), 16, UNIFORM_REDUNDANT));
	Entries.push_back(entry(
		message_format("UNIFORM_UNIQUE mode"),
		glm::uvec2(128), glm::vec2(8, 8), 16, UNIFORM_UNIQUE));

	csv CSV;
	int Error(0);

	for(std::size_t EntryIndex(0); EntryIndex < Entries.size(); ++EntryIndex)
	{
		test_uniform_caching Test(
			argc, argv,
			1000,
			Entries[EntryIndex].WindowSize,
			Entries[EntryIndex].TileSize,
			Entries[EntryIndex].TrianglePairPerTile,
			Entries[EntryIndex].UniformMode);

		Error += Test();
		Test.log(CSV, Entries[EntryIndex].String.c_str());
	}

	CSV.save("../main_uniform_caching.csv");

	return Error;
}

