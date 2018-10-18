#version 410 core

#define POSITION	0
#define COLOR		3
#define TEXCOORD	4

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

uniform mat4 MVP;

layout(location = POSITION) in vec2 Position;
layout(location = TEXCOORD) in vec2 Texcoord;

out gl_PerVertex
{
	vec4 gl_Position;
};

out block
{
	vec2 Texcoord;
	vec3 Color;
} Out; 

void main()
{	
	gl_Position = MVP * vec4(Position, 0.0, 1.0);
	Out.Texcoord = Texcoord;
	Out.Color = vec3(1.0, 0.9, 0.8);
}

