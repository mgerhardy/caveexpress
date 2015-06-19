uniform sampler2D u_texture;
uniform sampler2D u_normals;
uniform int u_time;
uniform vec2 u_screenres;
in vec2 v_texcoord;
in vec4 v_color;
in vec2 v_pos;
out vec4 o_color;

const float speed = 0.2;
const float frequency = 8.0;

vec2 shift(vec2 p) {
	vec2 f = frequency * (p + float(u_time / 1000.0) * speed);
	vec2 q = cos(vec2(cos(f.x - f.y) * cos(f.y), sin(f.x + f.y) * sin(f.y)));
	return q;
}

void main(void) {
	vec2 r = v_texcoord.xy;
	vec2 p = shift(r); 
	vec2 q = shift(r + 1.0);
	float amplitude = 2.0 / u_screenres.x;
	vec2 s = r + amplitude * (p - q);
	o_color = texture2D(u_texture, s);
}
