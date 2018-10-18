#version 420 core

#define FRAG_COLOR		0
#define DIFFUSE			0

#define MATERIAL	0
#define TRANSFORM0	1
#define TRANSFORM1	2	

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

layout(binding = DIFFUSE, rgba8) coherent uniform image2D Diffuse;

layout(binding = MATERIAL) uniform material
{
	uvec2 ImageSize;
} Material;

in block
{
	vec2 Texcoord;
} In;

layout(location = FRAG_COLOR, index = 0) out vec4 Color;

void main()
{
	Color = imageLoad(Diffuse, ivec2(In.Texcoord * Material.ImageSize));
}
