Texture2D<float4> tex0 : register(t0, space2);
SamplerState samp0 : register(s0, space2);

cbuffer TextParams : register(b0, space3) {
    float4 col;         // text color
    float4 col_shadow;  // shadow color (a=0 disables)
    float4 col_outline; // outline color (a=0 disables)
    float4 params;      // xy = shadow UV offset, zw = outline UV radius
    float4 params2;     // xy = soft shadow spread UV, z = color glyph flag
};

struct PSInput {
    float4 position : SV_Position;
    float4 fragColor : TEXCOORD0;
    float2 fragTexcoord : TEXCOORD1;
};

float4 main(PSInput input) : SV_Target0 {
    float4 texSample = tex0.Sample(samp0, input.fragTexcoord);
    float textA = texSample.a;
    bool isColor = params2.z > 0.5;

    // shadow (always uses alpha only)
    float shadowA = 0.0;
    if (col_shadow.a > 0.0) {
        float2 shadowCenter = input.fragTexcoord - params.xy;

        if (params2.x > 0.0) {
            // soft shadow: 5-tap diamond kernel
            float2 spread = params2.xy;
            shadowA  = tex0.Sample(samp0, shadowCenter).a                            * 0.4;
            shadowA += tex0.Sample(samp0, shadowCenter + float2(spread.x, 0.0)).a    * 0.15;
            shadowA += tex0.Sample(samp0, shadowCenter - float2(spread.x, 0.0)).a    * 0.15;
            shadowA += tex0.Sample(samp0, shadowCenter + float2(0.0, spread.y)).a    * 0.15;
            shadowA += tex0.Sample(samp0, shadowCenter - float2(0.0, spread.y)).a    * 0.15;
        } else {
            // hard shadow: single sample
            shadowA = tex0.Sample(samp0, shadowCenter).a;
        }
    }

    // outline: 8-tap max (alpha only)
    float outlineA = 0.0;
    if (col_outline.a > 0.0) {
        float2 ox = float2(params.z, 0.0);
        float2 oy = float2(0.0, params.w);
        outlineA = max(outlineA, tex0.Sample(samp0, input.fragTexcoord + ox).a);
        outlineA = max(outlineA, tex0.Sample(samp0, input.fragTexcoord - ox).a);
        outlineA = max(outlineA, tex0.Sample(samp0, input.fragTexcoord + oy).a);
        outlineA = max(outlineA, tex0.Sample(samp0, input.fragTexcoord - oy).a);
        outlineA = max(outlineA, tex0.Sample(samp0, input.fragTexcoord + ox + oy).a);
        outlineA = max(outlineA, tex0.Sample(samp0, input.fragTexcoord + ox - oy).a);
        outlineA = max(outlineA, tex0.Sample(samp0, input.fragTexcoord - ox + oy).a);
        outlineA = max(outlineA, tex0.Sample(samp0, input.fragTexcoord - ox - oy).a);
    }

    // composite back-to-front: outline -> shadow -> text (premultiplied alpha)
    // for color glyphs (emoji): use texture RGB directly
    // for alpha-only glyphs (text): use uniform + per-vertex color
    float4 textCol = isColor ? float4(texSample.rgb, col.a) : col * input.fragColor;

    float oA = outlineA * col_outline.a;
    float4 result = float4(col_outline.rgb * oA, oA);

    float sA = shadowA * col_shadow.a;
    result = float4(col_shadow.rgb * sA, sA) + result * (1.0 - sA);

    float tA = textA * textCol.a;
    result = float4(textCol.rgb * tA, tA) + result * (1.0 - tA);

    // un-premultiply for standard alpha blending
    return (result.a > 0.0) ? float4(result.rgb / result.a, result.a) : float4(0.0, 0.0, 0.0, 0.0);
}
