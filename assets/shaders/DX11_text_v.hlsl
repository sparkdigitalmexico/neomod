###DirectX11Interface::VertexShader#############################################################################################

##D3D11_INPUT_ELEMENT_DESC::VS_INPUT::POSITION::DXGI_FORMAT_R32G32B32_FLOAT::D3D11_INPUT_PER_VERTEX_DATA
##D3D11_INPUT_ELEMENT_DESC::VS_INPUT::COLOR0::DXGI_FORMAT_R32G32B32A32_FLOAT::D3D11_INPUT_PER_VERTEX_DATA
##D3D11_INPUT_ELEMENT_DESC::VS_INPUT::TEXCOORD0::DXGI_FORMAT_R32G32_FLOAT::D3D11_INPUT_PER_VERTEX_DATA

##D3D11_BUFFER_DESC::D3D11_BIND_CONSTANT_BUFFER::TextConstantBuffer::mvp::float4x4
##D3D11_BUFFER_DESC::D3D11_BIND_CONSTANT_BUFFER::TextConstantBuffer::col::float4
##D3D11_BUFFER_DESC::D3D11_BIND_CONSTANT_BUFFER::TextConstantBuffer::col_shadow::float4
##D3D11_BUFFER_DESC::D3D11_BIND_CONSTANT_BUFFER::TextConstantBuffer::col_outline::float4
##D3D11_BUFFER_DESC::D3D11_BIND_CONSTANT_BUFFER::TextConstantBuffer::params::float4
##D3D11_BUFFER_DESC::D3D11_BIND_CONSTANT_BUFFER::TextConstantBuffer::params2::float4

cbuffer TextConstantBuffer : register(b0)
{
	float4x4 mvp;
	float4 col;
	float4 col_shadow;
	float4 col_outline;
	float4 params;
	float4 params2;
};

struct VS_INPUT
{
	float4 pos	: POSITION;
	float4 col	: COLOR0;
	float2 tex	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 pos         : SV_Position;
	float2 tex         : TEXCOORD0;
	float4 col         : TEXCOORD1;
	float4 col_shadow  : TEXCOORD2;
	float4 col_outline : TEXCOORD3;
	float4 params      : TEXCOORD4;
	float4 params2     : TEXCOORD5;
};

VS_OUTPUT vsmain(in VS_INPUT In)
{
	VS_OUTPUT Out;
	In.pos.w = 1.0f;
	Out.pos = mul(In.pos, mvp);
	Out.tex = In.tex;
	Out.col = col;
	Out.col_shadow = col_shadow;
	Out.col_outline = col_outline;
	Out.params = params;
	Out.params2 = params2;
	return Out;
};
