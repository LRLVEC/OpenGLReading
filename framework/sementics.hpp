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

namespace semantic
{
	namespace buffer
	{
		enum type
		{
			STATIC = 0,
			DYNAMIC = 1
		};
	};

	namespace uniform
	{
		enum type
		{
			MATERIAL = 0,
			TRANSFORM0 = 1,
			TRANSFORM1 = 2,
			INDIRECTION = 3,
			CONSTANT = 0,
			PER_FRAME = 1,
			PER_PASS = 2
		};
	};

	namespace sampler
	{
		enum type
		{
			DIFFUSE		= 0,
			POSITION	= 4,
			TEXCOORD	= 5,
			COLOR		= 6
		};
	}//namespace sampler

	namespace image
	{
		enum type
		{
			DIFFUSE = 0,
			PICKING = 1
		};
	}//namesapce image

	namespace attr
	{
		enum type
		{
			POSITION = 0,
			COLOR	 = 3,
			TEXCOORD = 4,
			DRAW_ID  = 5
		};
	}//namespace attr

	namespace vert
	{
		enum type
		{
			POSITION = 0,
			COLOR	 = 3,
			TEXCOORD = 4,
			INSTANCE = 7
		};
	}//namespace vert

	namespace frag
	{
		enum type
		{
			COLOR	= 0,
			RED		= 0,
			GREEN	= 1,
			BLUE	= 2,
			ALPHA	= 0
		};
	}//namespace frag

	namespace renderbuffer
	{
		enum type
		{
			DEPTH,
			COLOR0
		};
	}//namespace renderbuffer

	namespace storage
	{
		enum type
		{
			VERTEX	= 0
		};
	}//namespace storage
}//namespace semantic

