uniform sampler2D u_texture;
uniform sampler2D u_normals;
uniform int u_lightcount;
uniform vec4 u_lights[MAX_RENDER_LIGHTS];
uniform vec3 u_lightcolors[MAX_RENDER_LIGHTS];
uniform float u_lightfalloff[MAX_RENDER_LIGHTS];

in vec2 v_texcoord;
in vec2 v_normalcoord;
in vec4 v_color;
in vec2 v_pos;
in float v_lit;
out vec4 o_color;

void main(void) {
	vec4 color = texture(u_texture, v_texcoord);
	vec4 fcolor = v_color / 255.0;
	vec4 outc = color * fcolor * 255.0;
	if (v_lit > 0.5 && color.a > 0.001) {
		vec3 n = texture(u_normals, v_normalcoord).xyz * 2.0 - 1.0;
		float nlen = length(n);
		if (nlen > 0.001)
			n /= nlen;
		else
			n = vec3(0.0, 0.0, 1.0);
		vec3 lighting = vec3(0.72);
		for (int i = 0; i < MAX_RENDER_LIGHTS; ++i) {
			if (i >= u_lightcount)
				break;
			vec2 lpos = u_lights[i].xy;
			float radius = max(u_lights[i].z, 1.0);
			float intensity = u_lights[i].w;
			vec2 delta = lpos - v_pos;
			// Keep height small so L stays in the map plane. A large Z turns
			// every camera-facing normal into a fill light on the whole sprite.
			vec3 L = vec3(delta.x, -delta.y, radius * 0.05);
			float dist = length(delta);
			float att = 1.0 - clamp(dist / radius, 0.0, 1.0);
			att = pow(max(att, 0.0), max(u_lightfalloff[i], 0.01));
			vec3 Ln = normalize(L);
			float diff = max(dot(n, Ln), 0.0);
			lighting += u_lightcolors[i] * intensity * diff * att;
		}
		lighting = min(lighting, vec3(1.55));
		outc.rgb *= lighting;
	}
	o_color = outc;
}
