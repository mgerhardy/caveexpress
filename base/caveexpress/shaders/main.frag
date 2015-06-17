uniform sampler2D u_texture;
uniform sampler2D u_normals;
uniform vec2 u_screenres;
uniform vec2 u_mousepos;
in vec2 v_texcoord;
in vec4 v_color;
out vec4 o_color;

const float DEFAULT_LIGHT_Z = 0.075;
const vec4 LIGHT_COLOR = vec4(1.0, 1.0, 1.0, 1.0);
const vec4 AMBIENT_COLOR = vec4(1.0, 1.0, 1.0, 1.0);
const vec3 FALLOFF = vec3(0.4, 3.0, 20.0);

void main(void) {
	vec3 lightpos = vec3(u_mousepos.x / u_screenres.x, 1.0 - u_mousepos.y / -u_screenres.y, DEFAULT_LIGHT_Z);
	vec4 color = texture2D(u_texture, v_texcoord).rgba;
	vec3 normal = texture2D(u_normals, v_texcoord).rgb;

	// The delta position of light
	vec3 lightdir = vec3(lightpos.xy - (gl_FragCoord.xy / u_screenres.xy), lightpos.z);

	// Correct for aspect ratio
	lightdir.x *= u_screenres.x / u_screenres.y;

	// Determine distance (used for attenuation) BEFORE we normalize our lightdir
	float D = length(lightdir);

	// normalize our vectors
	vec3 N = normalize(normal * 2.0 - 1.0);
	vec3 L = normalize(lightdir);

	// Pre-multiply light color with intensity
	// Then perform "N dot L" to determine our diffuse term
	vec3 diffuse = (LIGHT_COLOR.rgb * LIGHT_COLOR.a) * max(dot(N, L), 0.0);

	// pre-multiply ambient color with intensity
	vec3 ambient = AMBIENT_COLOR.rgb * AMBIENT_COLOR.a;

	// calculate attenuation
	float attenuation = 1.0 / (FALLOFF.x + (FALLOFF.y * D) + (FALLOFF.z * D * D));

	// the calculation which brings it all together
	vec3 intensity = ambient + diffuse * attenuation;
	vec3 finalcolor = color.rgb * intensity;

	vec4 fcolor = v_color / 255.0;
	o_color = vec4(finalcolor, color.a) * fcolor * 255.0;
}
