#include "Runtime/Application/Public/SandboxApplication.h"
#include <Core/Application/Public/EntryPoint.h>
#include <Core/Logging/Public/Log.h>
#include <Core/SECI/Subsystem/Public/SECISubsystem.h>
#include <Runtime/RHI/Public/RHIDevice.h>
#include <Runtime/RHI/Public/RHISwapchain.h>
#include <Runtime/RHI/Public/RHICommandList.h>
#include <Runtime/RHI/Public/Pipeline/RHIPipeline.h>
#include <Runtime/RHI/Public/Pipeline/RHIShaderModule.h>
#include <Runtime/RHI/Public/Vulkan/VulkanDevice.h>
#include <Runtime/Slang/SlangSubsystem.h>
#include <Runtime/Rendering/RenderingTypes.h>
#include <Runtime/Rendering/Public/Camera.h>
#include <Runtime/RHI/Public/RHIBuffer.h>
#include <filesystem>
#include <iostream>

struct Vertex3D {
    Sora::Vec3 Position;
    Sora::Vec3 Color;
};

inline const std::vector<Vertex3D> g_CubeVertices = {
    // Front Face (Z = +0.5) - Red/Yellow tint
    { { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f } },
    { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.5f, 0.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 0.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 0.0f } },
    { { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.5f } },
    { { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f } },

    // Back Face (Z = -0.5) - Green/Cyan tint
    { {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
    { { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.5f } },
    { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 1.0f } },
    { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 1.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 0.5f, 1.0f, 0.0f } },
    { {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },

    // Left Face (X = -0.5) - Blue/Purple tint
    { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } },
    { { -0.5f, -0.5f,  0.5f }, { 0.5f, 0.0f, 1.0f } },
    { { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f } },
    { { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f } },
    { { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.5f, 1.0f } },
    { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } },

    // Right Face (X = +0.5) - Yellow/Orange tint
    { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.8f, 0.0f } },
    { {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.4f, 0.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.2f, 0.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.2f, 0.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.6f, 0.0f } },
    { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.8f, 0.0f } },

    // Top Face (Y = +0.5) - White/Cyan tint
    { { -0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 0.8f, 1.0f, 1.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 0.5f, 1.0f, 1.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 0.5f, 1.0f, 1.0f } },
    { { -0.5f,  0.5f, -0.5f }, { 0.8f, 0.8f, 1.0f } },
    { { -0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f } },

    // Bottom Face (Y = -0.5) - Magenta/Dark tint
    { { -0.5f, -0.5f, -0.5f }, { 0.3f, 0.0f, 0.3f } },
    { {  0.5f, -0.5f, -0.5f }, { 0.5f, 0.0f, 0.5f } },
    { {  0.5f, -0.5f,  0.5f }, { 0.8f, 0.0f, 0.8f } },
    { {  0.5f, -0.5f,  0.5f }, { 0.8f, 0.0f, 0.8f } },
    { { -0.5f, -0.5f,  0.5f }, { 0.5f, 0.0f, 0.3f } },
    { { -0.5f, -0.5f, -0.5f }, { 0.3f, 0.0f, 0.3f } }
};

namespace Sora
{

    class SandboxSubsystem : public SECI::SECISubsystem {
    public:
        SandboxSubsystem() = default;
		explicit SandboxSubsystem(std::shared_ptr<RHI::RHIDevice> InDevice, std::shared_ptr<SlangSubsystem> InSlangSubsystem, std::shared_ptr<RenderingSubsystem> InRenderingSubsystem)
            : Device(InDevice), pSlangSubsystem(InSlangSubsystem), pRenderingSubsystem(InRenderingSubsystem) {}

        virtual ~SandboxSubsystem() override = default;

        virtual bool OnInitialize() override {
            SORA_CLIENT_INFO("SandboxSubsystem initializing pipeline...");

            if (!Device) {
                SORA_CLIENT_ERROR("SandboxSubsystem failed: RHIDevice is null!");
                return false;
            }

            RHI::RHIBufferDesc VbDesc{};
            VbDesc.Size = g_CubeVertices.size() * sizeof(Vertex3D);
            VbDesc.Usage = RHI::RHIBufferUsage::VertexBuffer;
            VbDesc.CpuVisible = true;

            VertexBuffer = Device->CreateBuffer(VbDesc);
            if (VertexBuffer) {
                void* MappedData = VertexBuffer->Map();
                if (MappedData) {
					std::memcpy(MappedData, g_CubeVertices.data(), VbDesc.Size);
					VertexBuffer->Unmap();
                }
            }

            RHI::RHIDescriptorBinding CameraBinding{};
            CameraBinding.Binding = 0;
            CameraBinding.Type = RHI::RHIDescriptorType::UniformBuffer;
            CameraBinding.Count = 1;
            CameraBinding.VisibleStages = RHI::RHIShaderStage::Vertex;

            RHI::RHIDescriptorSetLayoutDesc Set0Layout{};
            Set0Layout.Bindings = { CameraBinding };

            Sora::RHI::RHIPipelineLayoutDesc PipelineLayoutDesc{};
			PipelineLayoutDesc.DescriptorSets = { Set0Layout };
			//PipelineLayoutDesc.PushConstantRanges = {};

            ShaderCompilationResult VertexResult = pSlangSubsystem->CompileShader("shaders://Shader.slang", "MainVS");
            ShaderCompilationResult FragmentResult = pSlangSubsystem->CompileShader("shaders://Shader.slang", "MainFS");

            RHI::RHIShaderModuleDesc VSDesc{};
            VSDesc.Stage = RHI::RHIShaderStage::Vertex;
            VSDesc.Bytecode = std::move(VertexResult.Bytecode);
            VSDesc.EntryPoint = "MainVS";
            auto VS = Device->CreateShaderModule(VSDesc);

            RHI::RHIShaderModuleDesc FSDesc{};
            FSDesc.Stage = RHI::RHIShaderStage::Fragment;
            FSDesc.Bytecode = std::move(FragmentResult.Bytecode);
            FSDesc.EntryPoint = "MainFS";
            auto FS = Device->CreateShaderModule(FSDesc);

            RHI::RHIGraphicsPipelineDesc PipelineDesc{};
            PipelineDesc.VertexShader = VS;
            PipelineDesc.FragmentShader = FS;
            PipelineDesc.Topology = RHI::RHIPrimitiveTopology::TriangleList;

			RHI::RHIVertexBinding Binding{};
            Binding.Binding = 0;
            Binding.Stride = sizeof(Vertex3D);
            Binding.InputRate = RHI::RHIVertexInputRate::PerVertex;

            RHI::RHIVertexAttribute PosAttr{};
            PosAttr.Location = 0;
            PosAttr.Binding = 0;
            PosAttr.Format = RHI::RHIFormat::R32G32B32_FLOAT;
            PosAttr.Offset = offsetof(Vertex3D, Position);

            RHI::RHIVertexAttribute ColAttr{};
            ColAttr.Location = 1;
            ColAttr.Binding = 0;
            ColAttr.Format = RHI::RHIFormat::R32G32B32_FLOAT;
            ColAttr.Offset = offsetof(Vertex3D, Color);

            PipelineDesc.VertexLayout.Bindings = { Binding };
            PipelineDesc.VertexLayout.Attributes = { PosAttr, ColAttr };
            PipelineDesc.LayoutDesc = PipelineLayoutDesc;

            PipelineDesc.DepthStencilState.DepthTestEnable = true;
			PipelineDesc.DepthStencilState.DepthWriteEnable = true;
			PipelineDesc.DepthStencilState.DepthCompareOp = Sora::RHI::RHICompareOp::Less;

            PipelineDesc.RasterizerState.CullMode = RHI::RHICullMode::None; // for testing

            PipelineDesc.ColorAttachmentFormats = { RHI::RHIFormat::B8G8R8A8_UNORM };
            PipelineDesc.DepthAttachmentFormat = RHI::RHIFormat::D32_SFLOAT;

            Pipeline = Device->CreateGraphicsPipeline(PipelineDesc);

            SORA_CLIENT_INFO("SandboxSubsystem initialized!");
            return true;
        }

        virtual void OnUpdate(float deltaTime) override {
			if (!Device || !Pipeline) return;

			if (auto* Camera = pRenderingSubsystem->GetActiveCamera()) {
				static float Angle = 0.0f;
				Angle += deltaTime * 0.8f;

				float Radius = 3.0f;

				Vec3 CamPos = Vec3(std::sin(Angle) * Radius, 1.5f, std::cos(Angle) * Radius - 1.5f);
				Camera->SetPosition(CamPos);

				Camera->LookAt(Vec3(0.0f, 0.0f, 0.0f));

				Camera->UpdateUBO();
			}

			auto Swapchain = Device->GetSwapchain();
			if (!Swapchain) {
				Swapchain = Device->CreateSwapchain(nullptr, 1280, 720); 
				if (!Swapchain) return;
			}

			if (!Swapchain->AcquireNextImage()) {
				Swapchain->Resize(Swapchain->GetDesc().Width, Swapchain->GetDesc().Height);
				return;
			}

			Device->BeginFrame();

            auto BackBuffer = Swapchain->GetCurrentBackBuffer();
            auto CmdList = Device->CreateCommandList();

            if (!CmdList || !BackBuffer) {
				SORA_ENGINE_ERROR("CommandList or BackBuffer is null!");
				return;
			}

            CmdList->Begin();

			CmdList->PipelineBarrier({
				{ BackBuffer, RHI::RHITextureLayout::Undefined, RHI::RHITextureLayout::ColorAttachment },
				{ Swapchain->GetDepthTexture(), RHI::RHITextureLayout::Undefined, RHI::RHITextureLayout::DepthStencilAttachment }
			});

            RHI::RHIRenderingInfo RenderInfo{};
            RenderInfo.RenderArea = { 0, 0, Swapchain->GetDesc().Width, Swapchain->GetDesc().Height };
            RenderInfo.ColorAttachments = {
                { BackBuffer, true, { 0.05f, 0.05f, 0.05f, 1.0f } }
            };

            RenderInfo.HasDepthAttachment = true;
            RenderInfo.DepthAttachment.Texture = Swapchain->GetDepthTexture();
            RenderInfo.DepthAttachment.ClearOnLoad = true;
            RenderInfo.DepthAttachment.ClearColor = { 1.0f, 0.0f, 0.0f, 0.0f }; 

            CmdList->BeginRendering(RenderInfo);

            CmdList->BindPipeline(Pipeline);

            CmdList->BindVertexBuffer(VertexBuffer, 0);

            //if (VertexBuffer) {
            //    if (auto* Camera = pRenderingSubsystem->GetActiveCamera()) {
            //        const auto VP = Camera->GetViewProjectionMatrix();
            //        std::vector<Vertex3D> Transformed;
            //        Transformed.reserve(g_CubeVertices.size());
            //        for (const auto& V : g_CubeVertices) {
            //            glm::vec4 Clip = VP * glm::vec4(V.Position, 1.0f);
            //            // Perform perspective divide to get NDC coordinates
            //            glm::vec3 NDC = glm::vec3(Clip) / Clip.w;
            //            Transformed.push_back(Vertex3D{ { NDC.x, NDC.y, NDC.z }, V.Color });
            //        }

            //        void* MappedVerts = VertexBuffer->Map();
            //        if (MappedVerts) {
            //            std::memcpy(MappedVerts, Transformed.data(), Transformed.size() * sizeof(Vertex3D));
            //            VertexBuffer->Unmap();
            //        }
            //    }
            //}

			//if (VertexBuffer) {
			//	if (auto* Camera = pRenderingSubsystem->GetActiveCamera()) {
			//		static int FrameCounter = 0;
			//		if (FrameCounter++ % 60 == 0) { // Log once every 60 frames (~1 second)
			//			const auto VP = Camera->GetViewProjectionMatrix();

			//			SORA_CLIENT_INFO("================= FRAME DEBUG PRINT =================");
			//			SORA_CLIENT_INFO("Camera Pos: ({}, {}, {})", Camera->GetPosition().x, Camera->GetPosition().y, Camera->GetPosition().z);

			//			// Log ViewProjection Matrix Rows
			//			SORA_CLIENT_INFO("VP Matrix Row 0: [{:.2f}, {:.2f}, {:.2f}, {:.2f}]", VP[0][0], VP[1][0], VP[2][0], VP[3][0]);
			//			SORA_CLIENT_INFO("VP Matrix Row 1: [{:.2f}, {:.2f}, {:.2f}, {:.2f}]", VP[0][1], VP[1][1], VP[2][1], VP[3][1]);
			//			SORA_CLIENT_INFO("VP Matrix Row 2: [{:.2f}, {:.2f}, {:.2f}, {:.2f}]", VP[0][2], VP[1][2], VP[2][2], VP[3][2]);
			//			SORA_CLIENT_INFO("VP Matrix Row 3: [{:.2f}, {:.2f}, {:.2f}, {:.2f}]", VP[0][3], VP[1][3], VP[2][3], VP[3][3]);

			//			for (size_t i = 0; i < 3; ++i) { // Print first 3 vertices
			//				glm::vec4 LocalPos = glm::vec4(g_CubeVertices[i].Position, 1.0f);
			//				glm::vec4 ClipPos = VP * LocalPos;

			//				SORA_CLIENT_INFO("Vertex [{}] In Pos: ({:.2f}, {:.2f}, {:.2f})", i, LocalPos.x, LocalPos.y, LocalPos.z);
			//				SORA_CLIENT_INFO("  -> Clip Pos (x, y, z, w): ({:.2f}, {:.2f}, {:.2f}, {:.2f})", ClipPos.x, ClipPos.y, ClipPos.z, ClipPos.w);

			//				if (ClipPos.w != 0.0f) {
			//					glm::vec3 NDC = glm::vec3(ClipPos) / ClipPos.w;
			//					SORA_CLIENT_INFO("  -> NDC Pos (x, y, z): ({:.2f}, {:.2f}, {:.2f})", NDC.x, NDC.y, NDC.z);
			//				} else {
			//					SORA_CLIENT_ERROR("  -> Clip.w is ZERO! Division by zero imminent.");
			//				}
			//			}
			//			SORA_CLIENT_INFO("====================================================");
			//		}
			//	}
			//}

            if (pRenderingSubsystem && pRenderingSubsystem->GetCameraUBO()) {
				CmdList->BindUniformBuffer(0, 0, pRenderingSubsystem->GetCameraUBO()); 
			}

            CmdList->SetViewport({ 0.0f, 0.0f, (float)Swapchain->GetDesc().Width, (float)Swapchain->GetDesc().Height, 0.0f, 1.0f });
            CmdList->SetScissor({ 0, 0, Swapchain->GetDesc().Width, Swapchain->GetDesc().Height });

            CmdList->Draw(static_cast<uint32_t>(g_CubeVertices.size()));

            CmdList->EndRendering();

            CmdList->PipelineBarrier({
                { BackBuffer, RHI::RHITextureLayout::ColorAttachment, RHI::RHITextureLayout::Present }
            });

            CmdList->End();

            CmdList->Submit();

            Swapchain->Present();

            Device->EndFrame();
        }

        virtual void OnShutdown() override {
            Pipeline.reset();
            VertexBuffer.reset();
            SORA_CLIENT_INFO("SandboxSubsystem shut down.");
        }

        virtual void OnEvent(SECI::SECIEvent& InEvent) override {
            // Catch sandbox events here
        }

        virtual SECI::SECIEventCategory GetSubscribedEventCategories() const override {
            return SECI::EventCategoryNone;
        }

        virtual std::string GetName() const override { return "SandboxSubsystem"; }
        virtual SECI::SECISubsystemPriority GetPriority() const override { return SECI::SECISubsystemPriority::Normal; }

    private:
        std::shared_ptr<RHI::RHIDevice> Device{ nullptr };
        std::shared_ptr<SlangSubsystem> pSlangSubsystem{ nullptr };
        std::shared_ptr<RenderingSubsystem> pRenderingSubsystem{ nullptr };

        std::shared_ptr<RHI::RHIGraphicsPipeline> Pipeline{ nullptr };
        std::shared_ptr<RHI::RHIBuffer> VertexBuffer{ nullptr };
    };
}

SandboxApplication::SandboxApplication() {}

SandboxApplication::~SandboxApplication() {}

void SandboxApplication::OnRegisterSubsystems(Sora::SECI::SECISubsystemRegistry& InRegistry, std::shared_ptr<Sora::RHI::RHIDevice> InDevice, std::shared_ptr<Sora::SlangSubsystem> InSlangSubsystem, std::shared_ptr<Sora::RenderingSubsystem> InRenderingSubsystem) {
    InRegistry.RegisterSubsystem<Sora::SandboxSubsystem>(InDevice, InSlangSubsystem, InRenderingSubsystem);
}

Sora::Application* Sora::CreateApplication() {
    return new SandboxApplication();
}