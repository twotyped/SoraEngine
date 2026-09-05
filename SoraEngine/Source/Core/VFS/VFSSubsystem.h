#pragma once

#include "Core/SECI/Subsystem/Public/SECISubsystem.h"
#include "Core/Logging/Public/Log.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <filesystem>
#include <fstream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace Sora
{

	struct VFSBlob {
		std::vector<uint8_t> Data;
		bool IsValid() const { return !Data.empty(); }
	};

	class IVFSProvider {
	public:
		virtual ~IVFSProvider() = default;
		virtual VFSBlob ReadFile(const std::string& InSubPath) = 0;
		virtual bool Exists(const std::string& InSubPath) = 0;
	};

	class DiskProvider : public IVFSProvider {
	public:
		inline explicit DiskProvider(const std::filesystem::path& InRootPath)
			: RootPath(InRootPath) {}

		virtual VFSBlob ReadFile(const std::string& InSubPath) override {
			std::filesystem::path FullPath = RootPath / InSubPath;
			VFSBlob Blob{};

			std::ifstream File(FullPath, std::ios::binary | std::ios::ate);
			if (!File.is_open()) return Blob;

			size_t Size = static_cast<size_t>(File.tellg());
			Blob.Data.resize(Size);
			File.seekg(0, std::ios::beg);
			File.read(reinterpret_cast<char*>(Blob.Data.data()), Size);

			return Blob;
		}

		virtual bool Exists(const std::string& InSubPath) override {
			return std::filesystem::exists(RootPath / InSubPath);
		}

	private:
		std::filesystem::path RootPath;
	};

	class SORA_API VFSSubsystem : public SECI::SECISubsystem {
	public:
		inline VFSSubsystem() = default;
		inline virtual ~VFSSubsystem() override { OnShutdown(); }

		inline virtual bool OnInitialize() override {
			std::filesystem::path ExecDir = GetExecutableDirectory();
			std::filesystem::path ShadersPath;

			// Dev source path
		#ifdef SORA_SOURCE_DIR
			std::filesystem::path DevShadersPath = std::filesystem::path(SORA_SOURCE_DIR) / "RuntimeSandbox/Source/Shaders";
			if (std::filesystem::exists(DevShadersPath)) {
				ShadersPath = DevShadersPath;
				SORA_ENGINE_INFO("VFS: Development environment detected.");
			}
		#endif

			// Standalone fallback
			if (ShadersPath.empty()) {
				std::filesystem::path DistShadersPath = ExecDir / "Shaders";
				if (std::filesystem::exists(DistShadersPath)) {
					ShadersPath = DistShadersPath;
					SORA_ENGINE_INFO("VFS: Standalone distribution environment detected.");
				}
			}

			if (!ShadersPath.empty()) {
				RegisterMountPoint("shaders://", std::make_shared<DiskProvider>(ShadersPath));
				SORA_ENGINE_INFO("VFS: Mounted scheme 'shaders://' -> '{}'", ShadersPath.string());
				return true;
			}

			SORA_ENGINE_ERROR("VFS: Could not locate shaders directory!");
			return false;
		}

		inline virtual void OnUpdate(float deltaTime) override {}

		inline virtual void OnShutdown() override {
			MountPoints.clear();
		}
		
		inline void RegisterMountPoint(const std::string& InProtocol, std::shared_ptr<IVFSProvider> InProvider) {
			if (InProvider) {
				MountPoints[InProtocol] = InProvider;
				SORA_ENGINE_TRACE("VFS: Mounted protocol scheme '{}'", InProtocol);
			}
		}

		inline VFSBlob ReadFile(const std::string& InVirtualPath) {
			size_t ProtocolEnd = InVirtualPath.find("://");
			if (ProtocolEnd == std::string::npos) {
				SORA_ENGINE_ERROR("VFS ReadFile failed: Invalid virtual path protocol format in '{}'", InVirtualPath);
				return VFSBlob{};
			}

			std::string Protocol = InVirtualPath.substr(0, ProtocolEnd + 3); // "shaders://"
			std::string SubPath = InVirtualPath.substr(ProtocolEnd + 3);   // "Triangle.slang"

			auto It = MountPoints.find(Protocol);
			if (It == MountPoints.end()) {
				SORA_ENGINE_ERROR("VFS ReadFile failed: Unregistered protocol scheme '{}'", Protocol);
				return VFSBlob{};
			}

			return It->second->ReadFile(SubPath);
		}

		inline bool Exists(const std::string& InVirtualPath) {
			size_t ProtocolEnd = InVirtualPath.find("://");
			if (ProtocolEnd == std::string::npos) return false;

			std::string Protocol = InVirtualPath.substr(0, ProtocolEnd + 3);
			std::string SubPath = InVirtualPath.substr(ProtocolEnd + 3);

			auto It = MountPoints.find(Protocol);
			if (It == MountPoints.end()) return false;

			return It->second->Exists(SubPath);
		}

		inline virtual std::string GetName() const override { return "VFSSubsystem"; }
		inline virtual SECI::SECISubsystemPriority GetPriority() const override { return SECI::SECISubsystemPriority::High; }

	private:
		inline std::filesystem::path GetExecutableDirectory() {
#ifdef _WIN32
			char Buffer[MAX_PATH];
			GetModuleFileNameA(nullptr, Buffer, MAX_PATH);
			return std::filesystem::path(Buffer).parent_path();
#else
			return std::filesystem::current_path(); // Fallback for other platforms
#endif
		}

	private:
		std::unordered_map<std::string, std::shared_ptr<IVFSProvider>> MountPoints;
	};

} // namespace Sora
