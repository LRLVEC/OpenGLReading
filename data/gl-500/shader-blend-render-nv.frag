#version 450 core
#extension GL_NV_fragment_shader_interlock : require

#define DIFFUSE		0

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

layout(pixel_interlock_ordered) in;
in vec4 gl_FragCoord;

in block
{
	vec2 Texcoord;
} In;

layout(binding = DIFFUSE) uniform sampler2D Diffuse;
layout(binding = 0, rgba8) uniform coherent image2D Attachment;

void main()
{
	beginInvocationInterlockNV();

	vec4 FramebufferColor = imageLoad(Attachment, ivec2(gl_FragCoord.xy));
	vec4 TextureColor = texture(Diffuse, In.Texcoord.st);
	imageStore(Attachment, ivec2(gl_FragCoord.xy), TextureColor * 0.75 + FramebufferColor * 0.25);

	endInvocationInterlockNV();
}
