#pragma once

#include "Core/SECI/Subsystem/Public/SECISubsystem.h"
#include "Core/SECI/EventLayer/Public/SECIEventBus.h"
#include "Core/VFS/VFSSubsystem.h"
#include "Core/Logging/Public/Log.h"
#include <slang.h>
#include <slang-com-ptr.h>
#include <string>
#include <vector>
#include <memory>

namespace Sora
{
	struct ShaderCompilationResult {
		std::vector<uint32_t> Bytecode;
		std::string ErrorMessage;
		bool bSuccess = false;
	};

	enum class ShaderTargetAPI {
		SPIRV,		// Vulkan
		DXIL,		// DirectX 12
	};

	constexpr SlangCompileTarget ToSlangTarget(ShaderTargetAPI InAPI) {
		switch (InAPI) {
			case ShaderTargetAPI::SPIRV: return SLANG_SPIRV;
			case ShaderTargetAPI::DXIL:  return SLANG_DXIL;
			default:                     return SLANG_TARGET_UNKNOWN;
		}
	}

	class SORA_API SlangSubsystem : public SECI::SECISubsystem {
	public:
		explicit SlangSubsystem(SECI::SECIEventBus* InEventBus, std::shared_ptr<VFSSubsystem> InVfsSubsystem, ShaderTargetAPI InTargetAPI = ShaderTargetAPI::SPIRV)
			: EventBus(InEventBus), TargetAPI(InTargetAPI), pVfsSubsystem(InVfsSubsystem) {}

		virtual ~SlangSubsystem() override {
			OnShutdown();
		}

		virtual bool OnInitialize() override {
			if (slang::createGlobalSession(&SlangGlobalSession) != SLANG_OK) {
				SORA_ENGINE_ERROR("SlangSubsystem, Failed to initialize Slang Subsystem!");
				return false;
			}

			slang::SessionDesc SessionDesc{};

			slang::TargetDesc TargetDesc{};
			TargetDesc.format = ToSlangTarget(TargetAPI);
			TargetDesc.profile = SlangGlobalSession->findProfile("sm_6_6");

			SessionDesc.targets = &TargetDesc;
			SessionDesc.targetCount = 1;

			std::vector<slang::CompilerOptionEntry> options;
			if (TargetAPI == ShaderTargetAPI::SPIRV) {
				options.push_back({
					slang::CompilerOptionName::EmitSpirvDirectly,
					{slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
				});

				options.push_back({
					slang::CompilerOptionName::VulkanUseEntryPointName,
					{slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
				});
			} else {
				SORA_ENGINE_ERROR("SlangSubsystem, Failed to initialize SlangSubsystem: SoraEngine only supports Vulkan as of now.");
				return false;
			}
			
			SessionDesc.compilerOptionEntries = options.data();
			SessionDesc.compilerOptionEntryCount = static_cast<uint32_t>(options.size());

			if (SlangGlobalSession->createSession(SessionDesc, &SlangCompileSession) != SLANG_OK) {
				SORA_ENGINE_ERROR("SlangSubsystem, Failed to create Slang compilation session!");
				return false;
			}

			SORA_ENGINE_INFO("SlangSubsystem initialized successfully.");
			return true;
		}

		ShaderCompilationResult CompileShader(
			const std::string& InVirtualFilePath,
			const std::string& InEntryPointName
		) {
			ShaderCompilationResult Result{};

			auto LogDiagnostics = [](slang::IBlob* diagnosticsBlob) {
				if (diagnosticsBlob) {
					SORA_ENGINE_WARN("Slang Compiler Diagnostics:\n{}", (const char*)diagnosticsBlob->getBufferPointer());
				}
			};

			//std::ifstream File(InFilePath);
			//if (!File.is_open()) {
			//	SORA_ENGINE_ERROR("SlangSubsystem, Failed to open file {0}", InFilePath);
			//	Result.ErrorMessage = "Failed to open file.";
			//	return Result;
			//}

			VFSBlob FileBlob = pVfsSubsystem->ReadFile(InVirtualFilePath);
			if (!FileBlob.IsValid()) {
				SORA_ENGINE_ERROR("SlangSubsystem, Could not open file {0}", InVirtualFilePath);
				Result.ErrorMessage = "Could not open file.";
				return Result;
			}

			FileBlob.Data.push_back('\0');
			const char* ShaderSource = reinterpret_cast<const char*>(FileBlob.Data.data());

			//std::stringstream Buffer;
			//Buffer << File.rdbuf();
			//std::string ShaderSource = Buffer.str();

			Slang::ComPtr<slang::IModule> SlangModule;
			{
				Slang::ComPtr<slang::IBlob> diagnosticsBlob;
				SlangModule = SlangCompileSession->loadModuleFromSourceString(
					InVirtualFilePath.c_str(), // Module logical name
					InVirtualFilePath.c_str(), // Path
					ShaderSource,
					diagnosticsBlob.writeRef()
				);
				LogDiagnostics(diagnosticsBlob);
				if (!SlangModule) {
					Result.ErrorMessage = "Failed to create SlangModule.";
					return Result;
				}
			}

			Slang::ComPtr<slang::IEntryPoint> EntryPoint;
			{
				SlangModule->findEntryPointByName(InEntryPointName.c_str(), EntryPoint.writeRef());
				if (!EntryPoint) {
					SORA_ENGINE_ERROR("SlangSubsystem: Failed to locate entry point name '{0}' inside module '{1}'", InEntryPointName, InVirtualFilePath);
					return Result;
				}
			}

			std::array<slang::IComponentType*, 2> ComponentTypes = { SlangModule, EntryPoint };

			Slang::ComPtr<slang::IComponentType> ComposedProgram;
			{
				Slang::ComPtr<slang::IBlob> diagnosticsBlob;
				SlangResult result = SlangCompileSession->createCompositeComponentType(
					ComponentTypes.data(),
					ComponentTypes.size(),
					ComposedProgram.writeRef(),
					diagnosticsBlob.writeRef());
				LogDiagnostics(diagnosticsBlob);
				if (SLANG_FAILED(result)) {
					Result.ErrorMessage = "Failed to create ComposedProgram.";
					return Result;
				}
			}

			Slang::ComPtr<slang::IComponentType> LinkedProgram;
			{
				Slang::ComPtr<slang::IBlob> diagnosticsBlob;
				SlangResult result = ComposedProgram->link(LinkedProgram.writeRef(), diagnosticsBlob.writeRef());
				LogDiagnostics(diagnosticsBlob);
				if (SLANG_FAILED(result)) {
					Result.ErrorMessage = "Failed to create LinkedProgram.";
					return Result;
				}
			}

			Slang::ComPtr<slang::IBlob> SpirvCode;
			{
				Slang::ComPtr<slang::IBlob> diagnosticsBlob;
				SlangResult result = LinkedProgram->getEntryPointCode(0, 0, SpirvCode.writeRef(), diagnosticsBlob.writeRef());
				LogDiagnostics(diagnosticsBlob);
				if (SLANG_FAILED(result)) {
					Result.ErrorMessage = "Failed to create SpirvCode.";
					return Result;
				}
			}

			size_t sizeInBytes = SpirvCode->getBufferSize();
			size_t wordCount = sizeInBytes / sizeof(uint32_t);

			Result.Bytecode.resize(wordCount);
			std::memcpy(Result.Bytecode.data(), SpirvCode->getBufferPointer(), sizeInBytes);
			Result.bSuccess = true;

			SORA_ENGINE_INFO("SlangSubsystem: Successfully compiled '{0}' [{1}] down to {2} bytes of SPIR-V.", InVirtualFilePath, InEntryPointName, sizeInBytes);
			return Result;
		}

		virtual void OnUpdate(float deltaTime) override {};

		virtual void OnShutdown() override {
			if (SlangCompileSession) {
				SlangCompileSession->release();
				SlangCompileSession = nullptr;
			}
			if (SlangGlobalSession) {
				SlangGlobalSession->release();
				SlangGlobalSession = nullptr;
			}
		}

		virtual std::string GetName() const override { return "SlangSubsystem"; }
        virtual SECI::SECISubsystemPriority GetPriority() const override { return SECI::SECISubsystemPriority::Normal; }

	private:
		ShaderTargetAPI TargetAPI{ ShaderTargetAPI::SPIRV };
		SECI::SECIEventBus* EventBus{ nullptr };
		slang::IGlobalSession* SlangGlobalSession{ nullptr };
		slang::ISession* SlangCompileSession{ nullptr };

		std::shared_ptr<VFSSubsystem> pVfsSubsystem;
	};
}
