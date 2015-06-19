uniform sampler2D u_texture;
uniform sampler2D u_normal;
uniform vec2 u_screenres;
uniform int u_time;
in vec2 v_texcoord;
in vec4 v_color;
out vec4 o_color;
const vec2 NE = vec2(0.05, 0.0);
const vec4 waterColor = vec4(0.1, 0.1, 0.32, 1.0);
const vec3 lightDir = normalize(vec3(10.0, 15.0, 5.0));

float calcHeight(in vec2 uv) {
	return texture2D(u_texture, uv).b * texture2D(u_normal, uv + vec2(0.0, float(u_time / 1000.0) * 0.1)).b;
}

void main(void) {
	vec2 uv = v_texcoord.xy;
	uv.y += sin(uv.y * 20.0 + float(u_time / 1000.0)) * 0.02;
	uv.x += sin(uv.y * 40.0 + float(u_time / 1000.0)) * 0.01;

	float h = calcHeight(uv);
	vec3 norm = normalize(vec3(calcHeight(uv + NE.xy) - calcHeight(uv - NE.xy), 0.0, calcHeight(uv + NE.yx) - calcHeight(uv - NE.yx)));
	o_color = mix(mix(waterColor + waterColor * max(0.0, dot(lightDir, norm)) * 0.1,
		texture2D(u_texture, uv), 0.2), texture2D(u_texture, norm.xz * 0.5 + 0.5), 0.3);
}
