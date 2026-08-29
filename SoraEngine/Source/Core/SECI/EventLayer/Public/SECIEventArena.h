#pragma once

#include "Core/Core.h"
#include "SECIEvent.h"

#include <array>
#include <cstddef>
#include <utility>

namespace Sora::SECI
{

	/**
     * @brief Fixed-size Arena Allocator for zero-allocation event buffering.
     */
    class SORA_API SECIEventArena
    {
    public:
        static constexpr size_t ARENA_SIZE_BYTES = 1024 * 512;

        SECIEventArena();

		template<typename T, typename... Args>
		T* Allocate(Args&&... args) {
			static_assert(std::is_base_of_v<SECIEvent, T>, "T must derive from SECIEvent!");

			constexpr size_t Size = sizeof(T);
			constexpr size_t Align = alignof(T);

			size_t CurrentPointer = reinterpret_cast<size_t>(Buffer.data() + Offset);
			size_t AlignedPointer = (CurrentPointer + (Align - 1)) & ~(Align - 1);
			size_t Padding = AlignedPointer - CurrentPointer;

			if (Offset + Size + Padding > ARENA_SIZE_BYTES) {
				// Fallback / Overflow
				return nullptr;
			}

			Offset += Size + Padding;
			return new (reinterpret_cast<void*>(AlignedPointer)) T(std::forward<Args>(args)...);
		}

        /**
         * @brief Instant O(1) reset for frame execution cycles.
         */
        void Reset();

    private:
        alignas(16) std::array<uint8_t, ARENA_SIZE_BYTES> Buffer;
        size_t Offset = 0;
    };

} // namespace Sora::SECI
