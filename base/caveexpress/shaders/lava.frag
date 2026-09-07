uniform sampler2D u_texture;
uniform sampler2D u_normals;
uniform int u_time;
uniform vec2 u_bandv;
in vec2 v_texcoord;
in vec4 v_color;
out vec4 o_color;

void main(void) {
	float band = max(u_bandv.x - u_bandv.y, 0.0001);
	float heat = clamp((u_bandv.x - v_texcoord.y) / band, 0.0, 1.0);
	heat *= heat;

	float t = float(u_time) * 0.001;
	vec2 nUV = v_texcoord * vec2(2.4, 1.6) + vec2(t * 0.04, -t * 0.14);
	vec4 n = texture2D(u_normals, nUV);
	vec2 nUV2 = v_texcoord * 4.8 + vec2(-t * 0.07, -t * 0.22);
	vec4 n2 = texture2D(u_normals, nUV2);

	vec2 distort = (n.rg - 0.5) * 0.016 * heat;
	distort += (n2.rg - 0.5) * 0.007 * heat;
	vec4 color = texture2D(u_texture, v_texcoord + distort);

	vec3 tint = vec3(1.18, 0.52, 0.18);
	color.rgb = mix(color.rgb, color.rgb * tint, 0.32 * heat);
	color.rgb += vec3(0.10, 0.03, 0.0) * heat * n.b;
	o_color = vec4(color.rgb, 1.0) * v_color;
}
