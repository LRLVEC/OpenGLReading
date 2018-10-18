#version 420 core

layout(binding = 0) uniform sampler2D Diffuse;

in vec4 gl_FragCoord;

layout(location = 0, index = 0) out vec4 Color;

void main()
{
	vec2 Size = vec2(textureSize(Diffuse, 0));

	Color = texelFetch(Diffuse, ivec2(gl_FragCoord.x, Size.y - gl_FragCoord.y), 0);
}
