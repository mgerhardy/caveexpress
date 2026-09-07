uniform sampler2D u_texture;
uniform sampler2D u_normals;
uniform int u_time;
uniform vec2 u_bandv;
uniform vec2 u_bandu;
uniform vec2 u_fadeuv;
in vec2 v_texcoord;
in vec4 v_color;
out vec4 o_color;

void main(void) {
	float fadeU = max(u_fadeuv.x, 0.0001);
	float fadeV = max(u_fadeuv.y, 0.0001);

	float fromLeft = v_texcoord.x - u_bandu.x;
	float fromRight = u_bandu.y - v_texcoord.x;
	float fromTop = u_bandv.x - v_texcoord.y;
	float fromBottom = v_texcoord.y - u_bandv.y;

	float edge = smoothstep(0.0, fadeU, fromLeft)
	           * smoothstep(0.0, fadeU, fromRight)
	           * smoothstep(0.0, fadeV, fromTop)
	           * smoothstep(0.0, fadeV * 0.7, fromBottom);

	float band = max(u_bandv.x - u_bandv.y, 0.0001);
	float along = clamp(fromTop / band, 0.0, 1.0);
	float heat = edge * smoothstep(0.05, 0.55, along);

	float t = float(u_time) * 0.001;
	vec2 nUV = v_texcoord * vec2(2.2, 1.5) + vec2(t * 0.03, -t * 0.10);
	vec4 n = texture2D(u_normals, nUV);
	vec2 nUV2 = v_texcoord * 4.4 + vec2(-t * 0.05, -t * 0.16);
	vec4 n2 = texture2D(u_normals, nUV2);

	vec2 distort = (n.rg - 0.5) * 0.006 * heat;
	distort += (n2.rg - 0.5) * 0.0025 * heat;

	vec4 base = texture2D(u_texture, v_texcoord);
	vec4 warped = texture2D(u_texture, v_texcoord + distort);
	vec4 color = mix(base, warped, heat);

	vec3 tint = vec3(1.10, 0.58, 0.24);
	color.rgb = mix(color.rgb, color.rgb * tint, 0.18 * heat);
	color.rgb += vec3(0.05, 0.015, 0.0) * heat * n.b;
	o_color = vec4(color.rgb, heat) * v_color;
}
