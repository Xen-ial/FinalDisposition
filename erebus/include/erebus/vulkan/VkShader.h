#pragma once
#include "vulkan/vulkan.hpp"
#include <erebus/vulkan/VkDevice.h>

#include <vulkan/vulkan_raii.hpp>

#include <string>
#include <vector>
#include <functional>

namespace erb
{
	class VkShader
	{
	public:
		VkShader(VkDevice& device);

		bool createFromFile(const std::string& path);
		bool createFromMemory(const std::vector<char>& buffer);

		void addStage(vk::ShaderStageFlagBits stageFlags, const char* entryPoint);

		vk::raii::ShaderModule& getModule() { return m_shaderModule; }

		[[nodiscard]] std::vector<vk::PipelineShaderStageCreateInfo>& getShaderStages() { return m_shaderStages; }

	private:
		std::vector<char>      m_shaderByteBuffer;
		vk::raii::ShaderModule m_shaderModule = nullptr;

		std::reference_wrapper<VkDevice> m_device;

		std::vector<vk::PipelineShaderStageCreateInfo> m_shaderStages;

		vk::raii::ShaderModule createShaderModule(const std::vector<char>& buffer);
	};
} // namespace erb