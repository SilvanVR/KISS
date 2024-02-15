#pragma once

namespace VkUtils
{
	void SetImageLayout(vk::CommandBuffer const& commandBuffer, vk::Image image, vk::Format format, 
		vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout);

	void CompileGLSLtoSPIRV(const char* shaderSource, vk::ShaderStageFlagBits shaderType,
		std::vector<uint32_t>& spirv);
}

