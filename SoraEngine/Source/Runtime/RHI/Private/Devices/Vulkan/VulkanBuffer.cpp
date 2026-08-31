#include "Runtime/RHI/Public/Devices/Vulkan/VulkanBuffer.h"

namespace Sora::RHI
{

	VulkanBuffer::VulkanBuffer(VmaAllocator InAllocator, const RHIBufferDesc& InDesc)
		: Allocator(InAllocator), Desc(InDesc) {
		VkBufferCreateInfo BufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		BufferInfo.size = InDesc.Size;
		BufferInfo.usage = static_cast<VkBufferUsageFlags>(InDesc.Usage);

		VmaAllocationCreateInfo AllocInfo{};
		AllocInfo.usage = InDesc.CpuVisible ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		if (InDesc.CpuVisible) {
			AllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		}

		vmaCreateBuffer(Allocator, &BufferInfo, &AllocInfo, &Buffer, &Allocation, &AllocationInfo);
		MappedPtr = AllocationInfo.pMappedData;
	}

	VulkanBuffer::~VulkanBuffer() {
		if (Buffer != VK_NULL_HANDLE) {
			vmaDestroyBuffer(Allocator, Buffer, Allocation);
		}
	}

	void* VulkanBuffer::Map() {
		if (!MappedPtr && Buffer != VK_NULL_HANDLE) {
			vmaMapMemory(Allocator, Allocation, &MappedPtr);
		}
		return MappedPtr;
	}

	void VulkanBuffer::Unmap() {
		if (MappedPtr && Buffer != VK_NULL_HANDLE) {
			vmaUnmapMemory(Allocator, Allocation);
			MappedPtr = nullptr;
		}
	}

	void VulkanBuffer::UpdateData(const void* InData, uint64_t InSize, uint64_t InOffset) {
		if (!InData || InSize == 0) {
			return;
		}

		void* DestinationPtr = Map();
		if (DestinationPtr) {
			memcpy(static_cast<uint8_t*>(DestinationPtr) + InOffset, InData, InSize);
			Unmap();
		}
	}

} // namespace Sora::RHI