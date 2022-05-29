#include "../Hooked.hpp"
#include "../../SDK/sdk.hpp"


namespace Hooked
{
	void __stdcall PreEntity(const char* szMapName)
	{
		oPreEntity(Interfaces::m_pClient.pointer, szMapName);
	}
}