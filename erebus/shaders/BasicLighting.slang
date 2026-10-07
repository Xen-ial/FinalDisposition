import CommonTypes;

struct VSInput
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD0;
	float4 Tangent : TANGENT;
};

struct VSOutput
{
	float4 Position : SV_Position;
	float3 WorldPos : POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD0;
	float4 Tangent : TANGENT;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UniformBufferObject> ubo;

[[vk::binding(1, 0)]]
Sampler2D texSampler;

struct LightingPushConstants
{
	float4 ambientColor;
	float4 diffuseColor;
	float4 specularColor;
	float  shininess;
};

[[vk::push_constant]]
LightingPushConstants material;

[[shader("vertex")]]
VSOutput vertMain(VSInput input)
{
	VSOutput output;

	float4 worldPos = mul(ubo.model, float4(input.Position, 1.0F));
	output.Position = mul(ubo.proj, mul(ubo.view, worldPos));

	output.WorldPos = worldPos.xyz;
	output.Normal   = normalize(mul((float3x3)ubo.model, input.Normal));

	output.TexCoord = input.TexCoord;
	output.Tangent  = input.Tangent;

	return output;
}

[[shader("fragment")]]
float4 fragMain(VSOutput input) : SV_Target
{
	float4 texColor = texSampler.Sample(input.TexCoord);
	float3 normal   = normalize(input.Normal);

	float3 lightDir   = normalize(float3(1.0F, 1.0F, 1.0F));
	float3 viewDir    = normalize(ubo.camPos.xyz - input.WorldPos);
	float3 reflectDir = reflect(-lightDir, normal);

	float3 ambient = material.ambientColor.rgb * float3(0.1F, 0.1F, 0.1F);

	float  diff    = max(dot(normal, lightDir), 0.0F);
	float3 diffuse = diff * material.diffuseColor.rgb;

	float  spec     = pow(max(dot(viewDir, reflectDir), 0.0F), material.shininess);
	float3 specular = spec * material.specularColor.rgb;

	float3 result = (ambient + diffuse + specular) * texColor.rgb;
	return float4(result, texColor.a);
}
