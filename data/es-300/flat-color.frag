#version 300 es

uniform highp vec4 Diffuse;

void main()
{
	gl_FragColor = Diffuse;
}
