#include "../Hooked.hpp"
#include "../../SDK/Classes/Player.hpp"
#include "../../ShittierMenu/Menu.hpp"

void __stdcall Hooked::LockCursor() {
	g_Vars.globals.szLastHookCalled = XorStr("16");

	oLockCursor((void*)Interfaces::m_pSurface.Xor());

	auto pLocal = C_CSPlayer::GetLocalPlayer();

	bool state = true;
	if (!Interfaces::m_pEngine->IsInGame() || (pLocal && pLocal->IsDead()) || ImGui::GetIO().WantTextInput) {
		state = !(g_Vars.globals.menuOpen || g_IMGUIMenu.Opened);
	}

	Interfaces::m_pInputSystem->EnableInput(state);

	if (g_Vars.globals.menuOpen || g_IMGUIMenu.Opened)
		Interfaces::m_pSurface->UnlockCursor();
}
