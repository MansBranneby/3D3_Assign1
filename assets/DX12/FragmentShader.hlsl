struct PS_IN
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float2 uv : TEXTCOORD;
};

float4 PS_main(PS_IN input) : SV_TARGET0
{
	return input.col;
}