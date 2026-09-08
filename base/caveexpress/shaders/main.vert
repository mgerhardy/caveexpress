// attributes from the VAOs
in vec2 a_pos;
in vec2 a_texcoord;
in vec4 a_color;
in vec2 a_normalcoord;
in float a_lit;

uniform mat4 u_projection;

out vec2 v_texcoord;
out vec2 v_normalcoord;
out vec4 v_color;
out vec2 v_pos;
out float v_lit;

void main(void) {
	v_color = a_color;
	v_texcoord = a_texcoord;
	v_normalcoord = a_normalcoord;
	v_pos = a_pos;
	v_lit = a_lit;
	gl_Position = u_projection * vec4(a_pos.x, a_pos.y, 0.0, 1.0);
}
