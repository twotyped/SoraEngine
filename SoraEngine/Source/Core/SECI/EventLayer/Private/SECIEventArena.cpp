#include "Core/SECI/EventLayer/Public/SECIEventArena.h"

namespace Sora::SECI
{

	SECIEventArena::SECIEventArena() : Offset(0) {}

	void SECIEventArena::Reset() { Offset = 0; }

} // namespace Sora::SECI