cbuffer SceneData : register(b0)
{
    float4x4 LocalToWorld;
    float4x4 WorldToClip;
    float DepthAdjust;
};

struct VS_IN
{
    float3 pos : POSITION;
    float3 col : COLOR;
};

struct VS_OUT
{
    float4 col : COLOR0;
    float4 pos : SV_POSITION;
};

VS_OUT main(VS_IN input)
{
    VS_OUT OUT;
    float4 worldPos = mul(LocalToWorld, float4(input.pos, 1.0));
    worldPos.z += DepthAdjust;
    OUT.pos = mul(WorldToClip, worldPos);
    OUT.col = float4(input.col,1.0);
    return OUT;
}