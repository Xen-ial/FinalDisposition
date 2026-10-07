struct UniformBufferObject
{
	float4x4 model;
	float4x4 view;
	float4x4 proj;
	float4   camPos;

	int lightCount;
};
