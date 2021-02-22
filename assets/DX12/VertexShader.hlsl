struct VSIn
{
	float3 pos		: POSITION;
	float2 uv	: TEXTCOORD;
};

struct VSOut
{
	float4 pos		: SV_POSITION;
	float4 col : COLOR;
	float2 uv	: TEXTCOORD;
};

cbuffer VS_CB : register(b0)
{
	float4 color;
	float4 translate;
	float3 rotate;
};

VSOut VS_main(VSIn input)
{
	VSOut output = (VSOut)0;
	output.pos = float4(input.pos, 1.0f) + translate;
	output.col = color;
	output.uv = input.uv;

	return output;
}