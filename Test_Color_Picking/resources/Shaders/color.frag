#version 430 core

in VOUT {
	vec3 normal;
}v;

layout(location = 15) uniform float color_id;

layout(location = 0) out vec4 frag_color;

void main()
{
	frag_color = vec4(color_id, 0.f, 0.f, 0.f);
}