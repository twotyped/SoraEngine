#pragma once

#include "RHIPipelineTypes.h"
#include "Core/Core.h"
#include <vector>
#include <string>
#include <cstdint>

namespace Sora::RHI
{

	enum class RHIShaderStage : uint32_t {
		None     = 0,
		Vertex   = 1 << 0,
		Fragment = 1 << 1, // Pixel
		Compute  = 1 << 2,
	};

	constexpr RHIShaderStage operator|(RHIShaderStage InLHS, RHIShaderStage InRHS) {
		return static_cast<RHIShaderStage>(static_cast<uint32_t>(InLHS) | static_cast<uint32_t>(InRHS));
	}

	constexpr RHIShaderStage operator&(RHIShaderStage InLHS, RHIShaderStage InRHS) {
		return static_cast<RHIShaderStage>(static_cast<uint32_t>(InLHS) & static_cast<uint32_t>(InRHS));
	}

	inline RHIShaderStage& operator|=(RHIShaderStage& InLHS, RHIShaderStage InRHS) {
		InLHS = InLHS | InRHS;
		return InLHS;
	}

	// e.g. HasShaderStage(Binding.VisibleStages, RHIShaderStage::Fragment)
	constexpr bool HasShaderStage(RHIShaderStage InFlags, RHIShaderStage InTest) {
		return static_cast<uint32_t>(InFlags & InTest) != 0;
	}

	struct RHIShaderModuleDesc {
		RHIShaderStage Stage{ RHIShaderStage::Vertex };
		std::vector<uint32_t> Bytecode; // Bytecode blob (SPIR-V/DXIL)
		std::string EntryPoint{ "main" };
	};

	class SORA_API RHIShaderModule {
	public:
		virtual ~RHIShaderModule() = default;

		virtual const RHIShaderModuleDesc& GetDesc() const = 0;
		virtual RHIShaderStage GetStage() const = 0;
	};

} // namespace Sora::RHI
