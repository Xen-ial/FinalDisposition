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

float3 directionalLight(DirectionalLight dLight, VSOutput input, float4 texColor, float3 normal, float3 viewDir);
float3 pointLight(PointLight pLight, VSOutput input, float4 texColor, float3 normal, float3 viewDir);
float3 spotLight(SpotLight sLight, VSOutput input, float4 texColor, float3 normal, float3 viewDir);

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
	float3 viewDir  = normalize(ubo.camPos.xyz - input.WorldPos);

	float3 result = directionalLight(ubo.dirLight, input, texColor, normal, viewDir);

	for (int i = 0; i < ubo.activePointLightCount; i++)
	{
		PointLight pLight = ubo.pointLights[i];
		result += pointLight(pLight, input, texColor, normal, viewDir);
	}

	for (int i = 0; i < ubo.activeSpotLightCount; i++)
	{
		SpotLight sLight = ubo.spotLights[i];
		result += spotLight(sLight, input, texColor, normal, viewDir);
	}

	return float4(result, texColor.a);
}

float3 directionalLight(DirectionalLight dLight, VSOutput input, float4 texColor, float3 normal, float3 viewDir)
{
	float3 lightDir   = normalize(dLight.direction.xyz);
	float3 reflectDir = reflect(-lightDir, normal);

	float3 ambient = material.ambientColor.rgb * float3(0.1F, 0.1F, 0.1F);

	float  diff     = max(dot(normal, lightDir), 0.0F);
	float3 radiance = dLight.color.rgb * dLight.color.w;
	float3 diffuse  = diff * material.diffuseColor.rgb * radiance;

	float  spec     = pow(max(dot(viewDir, reflectDir), 0.0F), material.shininess);
	float3 specular = spec * material.specularColor.rgb;

	return (ambient + diffuse + specular) * texColor.rgb;
}

float3 pointLight(PointLight pLight, VSOutput input, float4 texColor, float3 normal, float3 viewDir)
{
	float3 toLight         = pLight.position.xyz - input.WorldPos;
	float  distanceToLight = length(toLight);
	float3 reflectDir      = reflect(-toLight, normal);

	float3 light       = toLight / max(distanceToLight, 0.0001F);
	float4 params      = pLight.params;
	float  attenuation = 1.0F / max(params.x + params.y * distanceToLight + params.z * distanceToLight * distanceToLight, 0.0001F);
	float3 radiance    = pLight.color.rgb * pLight.color.w * attenuation;

	float3 ambient = material.ambientColor.rgb * float3(0.1F, 0.1F, 0.1F);

	float  diff    = max(dot(normal, toLight), 0.0F);
	float3 diffuse = diff * material.diffuseColor.rgb * radiance;

	float  spec     = pow(max(dot(viewDir, reflectDir), 0.0F), material.shininess);
	float3 specular = spec * material.specularColor.rgb * radiance;

	return (ambient + diffuse + specular) * texColor.rgb;
}

float3 spotLight(SpotLight sLight, VSOutput input, float4 texColor, float3 normal, float3 viewDir)
{
	float3 fromLightToFragment = normalize(input.WorldPos - sLight.position.xyz);

	float  distanceToLight = length(fromLightToFragment);
	float3 rayDir          = fromLightToFragment / max(distanceToLight, 0.0001F);

	float3 reflectDir = reflect(-rayDir, normal);

	float theta = dot(rayDir, normalize(sLight.direction.xyz));
	float cone  = smoothstep(sLight.outerCos, sLight.innerCos, theta);

	float4 params      = sLight.params;
	float  attenuation = 1.0F / max(params.x + params.y * distanceToLight + params.z * distanceToLight * distanceToLight, 0.0001F);
	float3 radiance    = sLight.color.rgb * sLight.color.w * attenuation * cone;

	float3 ambient = material.ambientColor.rgb * float3(0.1F, 0.1F, 0.1F);

	float  diff    = max(dot(normal, distanceToLight), 0.0F);
	float3 diffuse = diff * material.diffuseColor.rgb * radiance;

	float  spec     = pow(max(dot(viewDir, reflectDir), 0.0F), material.shininess);
	float3 specular = spec * material.specularColor.rgb * radiance;

	return (ambient + diffuse + specular) * texColor.rgb;
}
