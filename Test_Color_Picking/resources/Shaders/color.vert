#version 430 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) uniform mat4 pmat;
layout(location = 1) uniform mat4 vmat;
layout(location = 2) uniform mat4 mmat;

out VOUT {
	vec3 normal;
}v;

void main()
{
	gl_Position = pmat * vmat * mmat * vec4(vertex, 1.f);

	v.normal = mat3(transpose(inverse(mmat))) * normal;
}