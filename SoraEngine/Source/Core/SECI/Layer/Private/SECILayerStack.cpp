#include "Core/SECI/Layer/Public/SECILayerStack.h"
#include <algorithm>

namespace Sora::SECI
{

	SECILayerStack::SECILayerStack() {}

	SECILayerStack::~SECILayerStack() {
		for (SECILayer* Layer : Layers) {
			Layer->OnDetach();
			delete Layer;
		}
	}

	void SECILayerStack::PushLayer(SECILayer* InLayer) {
		Layers.emplace(Layers.begin() + LayerInsertIndex, InLayer);
		LayerInsertIndex++;
		InLayer->OnAttach();
	}

	void SECILayerStack::PushOverlay(SECILayer* InOverlay) {
		Layers.emplace_back(InOverlay);
        InOverlay->OnAttach();
	}

	void SECILayerStack::PopLayer(SECILayer* InLayer) {
        auto it = std::find(Layers.begin(), Layers.begin() + LayerInsertIndex, InLayer);
        if (it != Layers.begin() + LayerInsertIndex) {
            InLayer->OnDetach();
            Layers.erase(it);
            LayerInsertIndex--;
        }
    }

	void SECILayerStack::PopOverlay(SECILayer* InOverlay) {
        auto it = std::find(Layers.begin() + LayerInsertIndex, Layers.end(), InOverlay);
        if (it != Layers.end()) {
            InOverlay->OnDetach();
            Layers.erase(it);
        }
    }
	
} // namespace Sora::SECI