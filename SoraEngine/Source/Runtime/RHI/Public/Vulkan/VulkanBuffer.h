#pragma once

#include "Runtime/RHI/Public/RHIBuffer.h"
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace Sora::RHI
{
	class VulkanBuffer : public RHIBuffer {
	public:
		VulkanBuffer(
			VmaAllocator InAllocator,
			VkDevice InLogicalDevice,
			const RHIBufferDesc& InDesc,
			VkBufferUsageFlags InVkUsage,
			VmaMemoryUsage InVmaMemoryUsage = VMA_MEMORY_USAGE_AUTO
		);

		virtual ~VulkanBuffer() override;

		virtual void* Map() override;
		virtual void Unmap() override;
		virtual void UpdateData(const void* data, uint64_t size, uint64_t offset = 0) override;

		virtual const RHIBufferDesc& GetDesc() const override { return Desc; }
		virtual uint64_t GetSize() const override { return Desc.Size; }

		VkBuffer GetVkBuffer() const { return Buffer; }
		VmaAllocation GetVmaAllocation() const { return Allocation; }

	private:
		void Cleanup();

	private:
		RHIBufferDesc Desc{};

		VkDevice LogicalDevice{ VK_NULL_HANDLE };
		VmaAllocator Allocator{ VK_NULL_HANDLE };

		VkBuffer Buffer{ VK_NULL_HANDLE };
		VmaAllocation Allocation{ VK_NULL_HANDLE };

		void* MappedData{ nullptr };
		bool bIsMapped{ false };
	};
} // namespace Sora::RHI
