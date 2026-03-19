###DirectX11Interface::PixelShader##############################################################################################

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct PS_INPUT
{
	float4 pos         : SV_Position;
	float2 tex         : TEXCOORD0;
	float4 col         : TEXCOORD1;
	float4 col_shadow  : TEXCOORD2;
	float4 col_outline : TEXCOORD3;
	float4 params      : TEXCOORD4;
	float4 params2     : TEXCOORD5;
};

struct PS_OUTPUT
{
	float4 col	: SV_Target;
};

PS_OUTPUT psmain(in PS_INPUT In)
{
	PS_OUTPUT Out;

	float4 texSample = txDiffuse.Sample(samLinear, In.tex);
	float textA = texSample.a;
	bool isColor = In.params2.z > 0.5f;

	// shadow (always uses alpha only; shadow is a solid-color effect)
	float shadowA = 0.0f;
	if (In.col_shadow.a > 0.0f) {
		float2 shadowCenter = In.tex - In.params.xy;

		if (In.params2.x > 0.0f) {
			// soft shadow: 5-tap diamond kernel
			float2 spread = In.params2.xy;
			shadowA  = txDiffuse.Sample(samLinear, shadowCenter).a                            * 0.4f;
			shadowA += txDiffuse.Sample(samLinear, shadowCenter + float2(spread.x, 0.0f)).a   * 0.15f;
			shadowA += txDiffuse.Sample(samLinear, shadowCenter - float2(spread.x, 0.0f)).a   * 0.15f;
			shadowA += txDiffuse.Sample(samLinear, shadowCenter + float2(0.0f, spread.y)).a   * 0.15f;
			shadowA += txDiffuse.Sample(samLinear, shadowCenter - float2(0.0f, spread.y)).a   * 0.15f;
		} else {
			// hard shadow: single sample
			shadowA = txDiffuse.Sample(samLinear, shadowCenter).a;
		}
	}

	// outline: 8-tap max (alpha only)
	float outlineA = 0.0f;
	if (In.col_outline.a > 0.0f) {
		float2 ox = float2(In.params.z, 0.0f);
		float2 oy = float2(0.0f, In.params.w);
		outlineA = max(outlineA, txDiffuse.Sample(samLinear, In.tex + ox).a);
		outlineA = max(outlineA, txDiffuse.Sample(samLinear, In.tex - ox).a);
		outlineA = max(outlineA, txDiffuse.Sample(samLinear, In.tex + oy).a);
		outlineA = max(outlineA, txDiffuse.Sample(samLinear, In.tex - oy).a);
		outlineA = max(outlineA, txDiffuse.Sample(samLinear, In.tex + ox + oy).a);
		outlineA = max(outlineA, txDiffuse.Sample(samLinear, In.tex + ox - oy).a);
		outlineA = max(outlineA, txDiffuse.Sample(samLinear, In.tex - ox + oy).a);
		outlineA = max(outlineA, txDiffuse.Sample(samLinear, In.tex - ox - oy).a);
	}

	// composite back-to-front: outline -> shadow -> text (premultiplied alpha)
	float4 textCol = isColor ? float4(texSample.rgb, In.col.a) : In.col;

	float oA = outlineA * In.col_outline.a;
	float4 result = float4(In.col_outline.rgb * oA, oA);

	float sA = shadowA * In.col_shadow.a;
	result = float4(In.col_shadow.rgb * sA, sA) + result * (1.0f - sA);

	float tA = textA * textCol.a;
	result = float4(textCol.rgb * tA, tA) + result * (1.0f - tA);

	// un-premultiply for standard alpha blending
	Out.col = (result.a > 0.0f) ? float4(result.rgb / result.a, result.a) : float4(0.0f, 0.0f, 0.0f, 0.0f);

	return Out;
};
