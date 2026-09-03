struct VSOutput
{
    float4 position : SV_Position;
    float3 color : COLOR;
};

[shader("vertex")]
VSOutput MainVS(uint vertexID: SV_VertexID)
{
    VSOutput output;

	float3 positions[3] = {
        float3(-0.6, 0.6, 0.0),
        float3(0.6, 0.6, 0.0),
        float3(0.0, -0.6, 0.0)
    };

    float3 colors[3] = {
        float3(1.0, 0.0, 0.5),
        float3(0.0, 1.0, 0.8),
        float3(1.0, 0.8, 0.0) 
    };

    output.position = float4(positions[vertexID], 1.0);
    output.color = colors[vertexID];

    return output;
}

[shader("fragment")]
float4 MainFS(VSOutput input) : SV_Target
{
    float3 glowingColor = pow(input.color, float3(1.4));
    return float4(glowingColor, 1.0f);
}