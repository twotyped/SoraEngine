#pragma once

#include "Runtime/RHI/Public/RHIBuffer.h"
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <memory>

namespace Sora::RHI
{

	class VulkanBuffer : public RHIBuffer {
	public:
		VulkanBuffer(VmaAllocator InAllocator, const RHIBufferDesc& InDesc);
        virtual ~VulkanBuffer() override;

        virtual const RHIBufferDesc& GetDesc() const override { return Desc; }
        virtual uint64_t GetSize() const override { return Desc.Size; }

        virtual void* Map() override;
        virtual void Unmap() override;
        virtual void UpdateData(const void* InData, uint64_t InSize, uint64_t InOffset = 0) override;

        VkBuffer GetVkBuffer() const { return Buffer; }
	private:
		VmaAllocator Allocator{ VK_NULL_HANDLE };
		VkBuffer Buffer{ VK_NULL_HANDLE };
		VmaAllocation Allocation{ VK_NULL_HANDLE };
		VmaAllocationInfo AllocationInfo{};
        RHIBufferDesc Desc{};
        void* MappedPtr{ nullptr };
	};

} // namespace Sora::RHI
