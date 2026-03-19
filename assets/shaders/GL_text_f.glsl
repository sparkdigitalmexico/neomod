#version {RUNTIME_VERSION}

#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D tex;
uniform vec4 col;
uniform vec4 col_shadow;
uniform vec4 col_outline;
uniform vec4 params;   // xy = shadow UV offset, zw = outline UV radius
uniform vec4 params2;  // xy = soft shadow spread UV, z = color glyph flag

varying vec2 tex_coord;

void main()
{
	vec4 texSample = texture2D(tex, tex_coord);
	float textA = texSample.a;
	bool isColor = params2.z > 0.5;

	// shadow (always uses alpha only; shadow is a solid-color effect)
	float shadowA = 0.0;
	if (col_shadow.a > 0.0) {
		vec2 shadowCenter = tex_coord - params.xy;

		if (params2.x > 0.0) {
			// soft shadow: 5-tap diamond kernel
			vec2 spread = params2.xy;
			shadowA  = texture2D(tex, shadowCenter).a                        * 0.4;
			shadowA += texture2D(tex, shadowCenter + vec2(spread.x, 0.0)).a  * 0.15;
			shadowA += texture2D(tex, shadowCenter - vec2(spread.x, 0.0)).a  * 0.15;
			shadowA += texture2D(tex, shadowCenter + vec2(0.0, spread.y)).a  * 0.15;
			shadowA += texture2D(tex, shadowCenter - vec2(0.0, spread.y)).a  * 0.15;
		} else {
			// hard shadow: single sample
			shadowA = texture2D(tex, shadowCenter).a;
		}
	}

	// outline: 8-tap max (alpha only)
	float outlineA = 0.0;
	if (col_outline.a > 0.0) {
		vec2 ox = vec2(params.z, 0.0);
		vec2 oy = vec2(0.0, params.w);
		outlineA = max(outlineA, texture2D(tex, tex_coord + ox).a);
		outlineA = max(outlineA, texture2D(tex, tex_coord - ox).a);
		outlineA = max(outlineA, texture2D(tex, tex_coord + oy).a);
		outlineA = max(outlineA, texture2D(tex, tex_coord - oy).a);
		outlineA = max(outlineA, texture2D(tex, tex_coord + ox + oy).a);
		outlineA = max(outlineA, texture2D(tex, tex_coord + ox - oy).a);
		outlineA = max(outlineA, texture2D(tex, tex_coord - ox + oy).a);
		outlineA = max(outlineA, texture2D(tex, tex_coord - ox - oy).a);
	}

	// composite back-to-front: outline -> shadow -> text (premultiplied alpha)
	// for color glyphs (emoji): use texture RGB directly
	// for alpha-only glyphs (text): use uniform color
	vec4 textCol = isColor ? vec4(texSample.rgb, col.a) : col;

	float oA = outlineA * col_outline.a;
	vec4 result = vec4(col_outline.rgb * oA, oA);

	float sA = shadowA * col_shadow.a;
	result = vec4(col_shadow.rgb * sA, sA) + result * (1.0 - sA);

	float tA = textA * textCol.a;
	result = vec4(textCol.rgb * tA, tA) + result * (1.0 - tA);

	// un-premultiply for standard alpha blending
	gl_FragColor = (result.a > 0.0) ? vec4(result.rgb / result.a, result.a) : vec4(0.0);
}
