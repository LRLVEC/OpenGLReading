#version 330 core
#extension GL_ARB_shading_language_420pack : require

#define POSITION		0
#define COLOR			3
#define TRANSFORM0		1

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

layout(binding = TRANSFORM0) uniform transform
{
	mat4 MVP;
} Transform;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec4 ColorA;
layout(location = 2) in vec4 ColorB;
layout(location = 3) in vec4 ColorC;
layout(location = 4) in vec4 ColorD;

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
	Out.Color = (((ColorA + ColorB) * 0.5) + ((ColorC + ColorD) * 0.5)) * 0.5 * vec4(1.0, 0.5, 0.0, 1.0);
	
	gl_Position = Transform.MVP * vec4(Position, 1);
}
