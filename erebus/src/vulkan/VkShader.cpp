#include <erebus/vulkan/VkShader.h>

#include <fstream>
#include <print>

namespace erb
{
	VkShader::VkShader(VkDevice& device)
	    : m_device(device) {}

	bool VkShader::createFromFile(const std::string& path)
	{
		std::ifstream file(path, std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
			std::println("failed to open file: {}", path);
			return false;
		}

		std::vector<char> buffer(file.tellg());
		file.seekg(0, std::ios::beg);

		file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
		file.close();

		m_shaderModule = std::move(createShaderModule(buffer));
		return true;
	}

	bool VkShader::createFromMemory(const std::vector<char>& buffer)
	{
		m_shaderModule = std::move(createShaderModule(buffer));
		return true;
	}

	void VkShader::addStage(vk::ShaderStageFlagBits stageFlags, const char* entryPoint)
	{
		vk::PipelineShaderStageCreateInfo shaderStage = {
		    .stage  = stageFlags,
		    .module = m_shaderModule,
		    .pName  = entryPoint};

		m_shaderStages.push_back(shaderStage);
	}

	vk::raii::ShaderModule VkShader::createShaderModule(const std::vector<char>& buffer)
	{
		vk::ShaderModuleCreateInfo createInfo{
		    .codeSize = buffer.size() * sizeof(char),
		    .pCode    = reinterpret_cast<const std::uint32_t*>(buffer.data())};

		vk::raii::ShaderModule shaderModule(m_device.get().getDevice(), createInfo);
		return shaderModule;
	}
} // namespace erb