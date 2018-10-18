#version 420 core

#define POSITION	0
#define COLOR		3

#define MATERIAL	0
#define TRANSFORM0	1
#define TRANSFORM1	2	

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

layout(binding = TRANSFORM0) uniform transform
{
	mat4 MVP;
} Transform;

layout(location = POSITION) in vec2 Position;
layout(location = COLOR) in vec4 Color;

out gl_PerVertex
{
	vec4 gl_Position;
};

out block
{
	vec4 Color;
} Out;

void main()
{	
	gl_Position = Transform.MVP * vec4(Position, float(gl_InstanceID) * 0.25 - 0.5, 1.0);
	Out.Color = Color;
}
