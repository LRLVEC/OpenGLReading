#version 420 core

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

in vec4 gl_FragCoord;
layout(binding = 0, rgba8) coherent uniform image2D Diffuse;

void main()
{
	imageStore(Diffuse, ivec2(gl_FragCoord.xy), vec4(1.0, 0.5, 0.0, 1.0));
}
