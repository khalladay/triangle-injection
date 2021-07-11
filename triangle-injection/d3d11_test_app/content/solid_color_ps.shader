struct VS_OUT
{
    float4 col : COLOR0;
    float4 pos : SV_POSITION;
};

cbuffer Color : register(b0)
{
    float4 Color;
};

float4 main(VS_OUT input) : SV_TARGET
{
    return Color;
}