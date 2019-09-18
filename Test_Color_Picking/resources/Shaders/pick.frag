#version 430 core

in VOUT {
	vec3 normal;
}v;

layout(binding = 0) uniform sampler2D color_map;

layout(location = 3) uniform vec3 pick_color;
layout(location = 4) uniform vec2 mouse_pos;

layout(location = 0) out vec4 frag_color;

void main()
{
	frag_color = vec4(v.normal, 1.f);

	float mouse_pos_color = texture(color_map, mouse_pos).r;
	float frag_pos_color = texelFetch(color_map, ivec2(gl_FragCoord.st), 0).r;
	
	if(abs(mouse_pos_color - frag_pos_color) < 0.001f)
		//frag_color = vec4(0.5f*(frag_color.rgb + pick_color), 1);
		frag_color = vec4(1.f) - frag_color;
}