#define VMA_IMPLEMENTATION
#include "Runtime/RHI/Public/Vulkan/VulkanBuffer.h"
#include "Core/Logging/Public/Log.h"

namespace Sora::RHI
{
	VulkanBuffer::VulkanBuffer(
		VmaAllocator InAllocator,
		VkDevice InLogicalDevice,
		const RHIBufferDesc& InDesc,
		VkBufferUsageFlags InVkUsage,
		VmaMemoryUsage InVmaMemoryUsage
	) : Allocator(InAllocator), LogicalDevice(InLogicalDevice), Desc(InDesc)
	{
		VkBufferCreateInfo BufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		BufferInfo.size = Desc.Size;
		BufferInfo.usage = InVkUsage;
		BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo AllocCreateInfo{};
		AllocCreateInfo.usage = InVmaMemoryUsage;

		if (Desc.CpuVisible) {
			AllocCreateInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			AllocCreateInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT; // Persistently mapped
		}

		VmaAllocationInfo AllocInfo{};
		if (vmaCreateBuffer(Allocator, &BufferInfo, &AllocCreateInfo, &Buffer, &Allocation, &AllocInfo) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to create Vulkan Buffer of size {0}!", Desc.Size);
			return;
		}

		MappedData = AllocInfo.pMappedData;
		bIsMapped = (MappedData != nullptr);
		bPersistentlyMapped = bIsMapped; // Track persistent mapping state
	}

	VulkanBuffer::~VulkanBuffer() {
		Unmap();
		Cleanup();
	}

	void* VulkanBuffer::Map() {
		if (MappedData) {
			return MappedData;
		}

		if (vmaMapMemory(Allocator, Allocation, &MappedData) != VK_SUCCESS) {
			SORA_ENGINE_ERROR("Failed to map Vulkan Buffer memory!");
			return nullptr;
		}

		bIsMapped = true;
		return MappedData;
	}

	void VulkanBuffer::Unmap() {
		// Do NOT unmap if VMA manages this persistently via VMA_ALLOCATION_CREATE_MAPPED_BIT
		if (bIsMapped && MappedData && !bPersistentlyMapped) {
			vmaUnmapMemory(Allocator, Allocation);
			MappedData = nullptr;
			bIsMapped = false;
		}
	}

	void VulkanBuffer::UpdateData(const void* data, uint64_t size, uint64_t offset) {
		if (!bIsMapped || !MappedData) {
			SORA_ENGINE_ERROR("Failed to update Vulkan Buffer: not mapped!");
			return;
		}

		std::memcpy(static_cast<char*>(MappedData) + offset, data, size);
	}

	void VulkanBuffer::Cleanup() {
		if (Buffer != VK_NULL_HANDLE && Allocator != VK_NULL_HANDLE) {
			// vmaDestroyBuffer automatically handles unmapping persistently mapped buffers!
			vmaDestroyBuffer(Allocator, Buffer, Allocation);
			Buffer = VK_NULL_HANDLE;
			Allocation = VK_NULL_HANDLE;
			MappedData = nullptr;
			bIsMapped = false;
			bPersistentlyMapped = false;
		}
	}
} // namespace Sora::RHI
