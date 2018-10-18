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

#pragma once

#include <GL/glew.h>
#include <glm/gtc/type_precision.hpp>

#include <map>
#include <string>
#include <vector>

std::string format(const char* Message, ...);
bool checkError(const char* Title);

class compiler
{
	typedef std::map<std::string, GLuint> names_map;
	typedef std::map<GLuint, std::string> files_map;

	class commandline
	{
		enum profile
		{
			CORE,
			COMPATIBILITY
		};

	public:
		commandline(std::string const & Filename, std::string const & Arguments);

		void parseArguments(std::string const & Arguments);

		int getVersion() const {return this->Version;}
		std::string getProfile() const {return this->Profile;}
		std::string getDefines() const;
		std::vector<std::string> getIncludes() const {return this->Includes;}

	private:
		std::string Profile;
		int Version;
		std::vector<std::string> Defines;
		std::vector<std::string> Includes;
	};

	class parser
	{
	public:
		std::string operator() (commandline const & CommandLine, std::string const & Filename) const;

	private:
		std::string parseInclude(std::string const & Line, std::size_t const & Offset) const;
	};

public:
	~compiler();

	GLuint create(GLenum Type, std::string const & Filename, std::string const & Arguments = std::string());
	bool destroy(GLuint const & Name);

	bool checkProgram(GLuint ProgramName) const;
	bool validateProgram(GLuint ProgramName) const;

	bool check();
	// TODO: Not defined
	bool check(GLuint const & Name);
	void clear();

private:
	names_map ShaderNames;
	files_map ShaderFiles;
	names_map PendingChecks;
};

std::string loadFile(std::string const & Filename);
bool loadBinary(std::string const & Filename, GLenum & Format, std::vector<glm::uint8> & Data, GLint & Size);
bool saveBinary(std::string const & Filename, GLenum const & Format, std::vector<glm::uint8> const & Data, GLint const & Size);
