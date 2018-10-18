#version 150 core

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

out vec4 Red;
out vec4 Green;
out vec4 Blue;

void main()
{
	Red = vec4(1.0, 0.0, 0.0, 1.0);
	Green = vec4(0.0, 1.0, 0.0, 1.0);
	Blue = vec4(0.0, 0.0, 1.0, 1.0);
}
