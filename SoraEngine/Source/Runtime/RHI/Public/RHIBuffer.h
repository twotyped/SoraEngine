#pragma once

#include "RHITypes.h"

namespace Sora::RHI
{

	class SORA_API RHIBuffer {
	public:
		virtual ~RHIBuffer() = default;

		virtual const RHIBufferDesc& GetDesc() const = 0;
		virtual uint64_t GetSize() const = 0;

		virtual void* Map() = 0;
        virtual void Unmap() = 0;
        virtual void UpdateData(const void* data, uint64_t size, uint64_t offset = 0) = 0;
	};

} // namespace Sora::RHI
