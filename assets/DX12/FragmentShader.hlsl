struct PS_IN
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float2 uv : TEXTCOORD;
};

Texture2D tex : register(t0);

SamplerState texSampler : register(s0);



float4 PS_main(PS_IN input) : SV_TARGET0
{
#ifdef DIFFUSE_SLOT
	return tex.Sample(texSampler, input.uv);
#endif
	return input.col;
}