#pragma once

#include "Core/Core.h"
#include "Core/SECI/Layer/Public/SECILayer.h"
#include <vector>

namespace Sora::SECI
{

	class SORA_API SECILayerStack {
	public:
		SECILayerStack();
		~SECILayerStack();

		void PushLayer(SECILayer* InLayer);
		void PushOverlay(SECILayer* InOverlay);
		void PopLayer(SECILayer* InLayer);
		void PopOverlay(SECILayer* InOverlay);

		std::vector<SECILayer*>::iterator begin() { return Layers.begin(); }
        std::vector<SECILayer*>::iterator end() { return Layers.end(); }
        std::vector<SECILayer*>::reverse_iterator rbegin() { return Layers.rbegin(); }
        std::vector<SECILayer*>::reverse_iterator rend() { return Layers.rend(); }

	private:
		std::vector<SECILayer*> Layers;
        unsigned int LayerInsertIndex = 0;
	};

} // namespace Sora::SECI
