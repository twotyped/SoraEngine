#pragma once

#include "Core/Core.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <memory>

namespace Sora
{
	class SORA_API Log {
	public:
		static void Initialize();

		static std::shared_ptr<spdlog::logger>& GetEngineLogger();
		static std::shared_ptr<spdlog::logger>& GetClientLogger();

	private:
		static std::shared_ptr<spdlog::logger> s_EngineLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

#define SORA_ENGINE_TRACE(...)		::Sora::Log::GetEngineLogger()->trace(__VA_ARGS__)
#define SORA_ENGINE_INFO(...)		::Sora::Log::GetEngineLogger()->info(__VA_ARGS__)
#define SORA_ENGINE_WARN(...)		::Sora::Log::GetEngineLogger()->warn(__VA_ARGS__)
#define SORA_ENGINE_ERROR(...)		::Sora::Log::GetEngineLogger()->error(__VA_ARGS__)
#define SORA_ENGINE_CRITICAL(...)	::Sora::Log::GetEngineLogger()->critical(__VA_ARGS__)

#define SORA_CLIENT_TRACE(...)		::Sora::Log::GetClientLogger()->trace(__VA_ARGS__)
#define SORA_CLIENT_INFO(...)		::Sora::Log::GetClientLogger()->info(__VA_ARGS__)
#define SORA_CLIENT_WARN(...)		::Sora::Log::GetClientLogger()->warn(__VA_ARGS__)
#define SORA_CLIENT_ERROR(...)		::Sora::Log::GetClientLogger()->error(__VA_ARGS__)
#define SORA_CLIENT_CRITICAL(...)	::Sora::Log::GetClientLogger()->critical(__VA_ARGS__)