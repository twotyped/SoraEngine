#pragma once

#include "Core/Core.h"
#include "Core/Window/Public/Window.h"
#include "Core/SECI/EventLayer/Public/SECIEvent.h"
#include "Core/SECI/EventLayer/Public/Events/WindowEvent.h"
#include "Core/SECI/Layer/Public/SECILayer.h"
#include "Core/SECI/Layer/Public/SECILayerStack.h"

#include <memory>

namespace Sora
{
	class SORA_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void OnEvent(SECI::SECIEvent& InEvent);

		void PushLayer(SECI::SECILayer* InLayer);
		void PushOverlay(SECI::SECILayer* InOverlay);

	private:
		bool OnWindowClose(SECI::WindowCloseEvent& InEvent);
        bool OnWindowResize(SECI::WindowResizeEvent& InEvent);

	private:
        std::unique_ptr<Window> AppWindow;
		SECI::SECILayerStack AppLayerStack;
        bool IsRunning = true;
	};

	Application* CreateApplication();
}
