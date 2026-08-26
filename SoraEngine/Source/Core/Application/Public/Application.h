#pragma once

#include "Core/Core.h"

namespace Sora
{
	class SORA_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	Application* CreateApplication();
}
