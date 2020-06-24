#pragma once
#include "IDevice.h"

#ifdef ME_DIRECTX

namespace Moonlight
{
	class DX11Device
		: public IDevice
	{
	public:
		DX11Device();
	};
}

#endif