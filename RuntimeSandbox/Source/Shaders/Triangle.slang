struct VSOutput
{
    float4 position : SV_Position;
    float3 color : COLOR;
};

[shader("vertex")]
VSOutput MainVS(uint vertexID: SV_VertexID)
{
    VSOutput output;

    // Hardcoded triangle vertices and colors
	float3 positions[3] = {
        float3(-0.5, 0.5, 0.0),  // Top Left in Vulkan
        float3(0.5, 0.5, 0.0),   // Top Right in Vulkan
        float3(0.0, -0.5, 0.0)   // Bottom Center in Vulkan
    };

    float3 colors[3] = {
        float3(1.0, 0.0, 0.0), // Red
        float3(0.0, 1.0, 0.0), // Green
        float3(0.0, 0.0, 1.0)  // Blue
    };

    output.position = float4(positions[vertexID], 1.0);
    output.color = colors[vertexID];
    return output;
}

[shader("fragment")]
float4 MainFS(VSOutput input) : SV_Target
{
    return float4(input.color, 1.0f);
}