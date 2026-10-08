struct DirectionalLight
{
	float4 direction;
	float4 color;
};

struct PointLight
{
	float4 position;
	float4 color;
	float4 params;
};

struct SpotLight
{
	float4 position;
	float4 direction;
	float4 color;
	float4 params;
	float  outerCos;
	float  innerCos;
};

struct UniformBufferObject
{
	float4x4 model;
	float4x4 view;
	float4x4 proj;
	float4   camPos;

	DirectionalLight dirLight;

	float      activePointLightCount;
	PointLight pointLights[5];

	float     activeSpotLightCount;
	SpotLight spotLights[5];
};
