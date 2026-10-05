import CommonTypes;

struct VSInput
{
	float3 inPos;
	float3 inColor;
	float2 inTexCoord;
};

struct VSOutput
{
	float4 pos : SV_Position;
	float3 fragColor;
	float2 fragTexCoord;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UniformBufferObject> ubo;

[[vk::binding(1, 0)]]
Sampler2D texture;

[shader("vertex")]
VSOutput vertMain(VSInput input)
{
	VSOutput output;

	output.pos          = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inPos, 1.0))));
	output.fragColor    = input.inColor;
	output.fragTexCoord = input.inTexCoord;

	return output;
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_Target
{
	return texture.Sample(vertIn.fragTexCoord);
}
