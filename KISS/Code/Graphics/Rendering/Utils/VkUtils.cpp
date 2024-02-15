#include "stdafx_common.h"

#include "VkUtils.h"

#include <SDKs/glslang/Public/ShaderLang.h>
#include <SDKs/glslang/Public/ResourceLimits.h>
#include <SDKs/glslang/SPIRV/GlslangToSpv.h>

namespace VkUtils
{
	void SetImageLayout(vk::CommandBuffer const& commandBuffer, vk::Image image, vk::Format format, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout)
  {
    vk::AccessFlags sourceAccessMask;
    switch (oldImageLayout)
    {
    case vk::ImageLayout::eColorAttachmentOptimal: sourceAccessMask = vk::AccessFlagBits::eColorAttachmentWrite; break;
    case vk::ImageLayout::eTransferDstOptimal:     sourceAccessMask = vk::AccessFlagBits::eTransferWrite; break;
    case vk::ImageLayout::ePreinitialized:         sourceAccessMask = vk::AccessFlagBits::eHostWrite; break;
    case vk::ImageLayout::eGeneral: // sourceAccessMask is empty
    case vk::ImageLayout::eUndefined: break;
    default: assert(false); break;
    }

    vk::PipelineStageFlags sourceStage;
    switch (oldImageLayout)
    {
    case vk::ImageLayout::eGeneral:
    case vk::ImageLayout::ePreinitialized:         sourceStage = vk::PipelineStageFlagBits::eHost; break;
    case vk::ImageLayout::eTransferDstOptimal:     sourceStage = vk::PipelineStageFlagBits::eTransfer; break;
    case vk::ImageLayout::eColorAttachmentOptimal: sourceStage = vk::PipelineStageFlagBits::eColorAttachmentOutput; break;
    case vk::ImageLayout::eUndefined:              sourceStage = vk::PipelineStageFlagBits::eTopOfPipe; break;
    default: assert(false); break;
    }

    vk::AccessFlags destinationAccessMask;
    switch (newImageLayout)
    {
    case vk::ImageLayout::eColorAttachmentOptimal: destinationAccessMask = vk::AccessFlagBits::eColorAttachmentWrite; break;
    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
      destinationAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
      break;
    case vk::ImageLayout::eGeneral: // empty destinationAccessMask
    case vk::ImageLayout::ePresentSrcKHR: break;
    case vk::ImageLayout::eShaderReadOnlyOptimal: destinationAccessMask = vk::AccessFlagBits::eShaderRead; break;
    case vk::ImageLayout::eTransferSrcOptimal: destinationAccessMask = vk::AccessFlagBits::eTransferRead; break;
    case vk::ImageLayout::eTransferDstOptimal: destinationAccessMask = vk::AccessFlagBits::eTransferWrite; break;
    default: assert(false); break;
    }

    vk::PipelineStageFlags destinationStage;
    switch (newImageLayout)
    {
    case vk::ImageLayout::eColorAttachmentOptimal: destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput; break;
    case vk::ImageLayout::eDepthStencilAttachmentOptimal: destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests; break;
    case vk::ImageLayout::eGeneral: destinationStage = vk::PipelineStageFlagBits::eHost; break;
    case vk::ImageLayout::ePresentSrcKHR: destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe; break;
    case vk::ImageLayout::eShaderReadOnlyOptimal: destinationStage = vk::PipelineStageFlagBits::eFragmentShader; break;
    case vk::ImageLayout::eTransferDstOptimal:
    case vk::ImageLayout::eTransferSrcOptimal: destinationStage = vk::PipelineStageFlagBits::eTransfer; break;
    default: assert(false); break;
    }

    vk::ImageAspectFlags aspectMask;
    if (newImageLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
    {
      aspectMask = vk::ImageAspectFlagBits::eDepth;
      if (format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint)
      {
        aspectMask |= vk::ImageAspectFlagBits::eStencil;
      }
    }
    else
    {
      aspectMask = vk::ImageAspectFlagBits::eColor;
    }

    vk::ImageSubresourceRange imageSubresourceRange(aspectMask, 0, 1, 0, 1);
    vk::ImageMemoryBarrier    imageMemoryBarrier(sourceAccessMask,
      destinationAccessMask,
      oldImageLayout,
      newImageLayout,
      VK_QUEUE_FAMILY_IGNORED,
      VK_QUEUE_FAMILY_IGNORED,
      image,
      imageSubresourceRange);
    return commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, nullptr, nullptr, imageMemoryBarrier);
  }

	void CompileGLSLtoSPIRV(const char* shaderSource, vk::ShaderStageFlagBits stage, std::vector<uint32_t>& spirv)
	{
		EShLanguage shaderType = EShLangVertex;
		switch (stage)
		{
			case vk::ShaderStageFlagBits::eVertex:                 shaderType = EShLangVertex; break;
			case vk::ShaderStageFlagBits::eTessellationControl:    shaderType = EShLangTessControl; break;
			case vk::ShaderStageFlagBits::eTessellationEvaluation: shaderType = EShLangTessEvaluation; break;
			case vk::ShaderStageFlagBits::eGeometry:               shaderType = EShLangGeometry; break;
			case vk::ShaderStageFlagBits::eFragment:               shaderType = EShLangFragment; break;
			case vk::ShaderStageFlagBits::eCompute:                shaderType = EShLangCompute; break;
			default: assert(false); break;
		}

		glslang::InitializeProcess();

		const EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

		glslang::TShader shader(shaderType);
		shader.setStrings(&shaderSource, 1);

		if (!shader.parse(GetDefaultResources(), 100, false, messages)) {
			KISS_LOG_WARN("Failed to parse shader:\n%s", shader.getInfoLog());
			return;
		}

		glslang::TProgram program;
		program.addShader(&shader);

		if (!program.link(messages)) {
			KISS_LOG_WARN("Failed to link shader:\n%s", shader.getInfoLog());
			return;
		}

		glslang::GlslangToSpv(*program.getIntermediate(shaderType), spirv);

		glslang::FinalizeProcess();
	}

}

