#include "../Hooked.hpp"
#include "../../SDK/Displacement.hpp"
#include "../../Features/Rage/TickbaseShift.hpp"
#include "../../source.hpp"
#include "../../SDK/sdk.hpp"
#include "../../SDK/Classes/Exploits.hpp"

void WriteUsercmdD( bf_write* buf, CUserCmd* incmd, CUserCmd* outcmd ) {
	__asm
	{
		mov     ecx, buf
		mov     edx, incmd
		push    outcmd
		call    Engine::Displacement.Function.m_WriteUsercmd
		add     esp, 4
	}
}


bool __fastcall Hooked::WriteUsercmdDeltaToBuffer( void* ECX, void* EDX, int nSlot, bf_write* buffer, int o_from, int o_to, bool isnewcommand ) {
    g_Vars.globals.szLastHookCalled = XorStr("48");

    if (Interfaces::m_pEngine->IsConnected() && Interfaces::m_pEngine->IsInGame() && Interfaces::m_pEngine->GetLocalPlayer()) {

        if (g_Vars.globals.TicksToShift <= 0)
            return Hooked::oWriteUsercmdDeltaToBuffer(ECX, nSlot, buffer, o_from, o_to, isnewcommand);

        if (o_from != -1)
            return true;

        uintptr_t stackbase;
        __asm mov stackbase, ebp;
        CCLCMsg_Move_t* msg = reinterpret_cast<CCLCMsg_Move_t*>(stackbase + 0xFCC);
        auto net_channel = *reinterpret_cast <INetChannel**> (reinterpret_cast <uintptr_t> (&Interfaces::m_pClientState) + 0x9C);
        int32_t new_commands = msg->m_nNewCommands;

        auto next_cmd_nr = Interfaces::m_pClientState->m_nLastOutgoingCommand() + Interfaces::m_pClientState->m_nChokedCommands() + 1;

        auto total_new_commands = std::clamp(g_Vars.globals.TicksToShift, 0, 16);
        g_Vars.globals.TicksToShift -= total_new_commands;

        o_from = -1;

        msg->m_nNewCommands = total_new_commands;
        msg->m_nBackupCommands = 0;

        for (o_to = next_cmd_nr - new_commands + 1; o_to <= next_cmd_nr; o_to++)
        {
            if (!Hooked::oWriteUsercmdDeltaToBuffer(ECX, nSlot, buffer, o_from, o_to, true))
                return false;

            o_from = o_to;
        }

        CUserCmd* last_real_cmd = Interfaces::m_pInput->GetUserCmd(nSlot, o_from);
        CUserCmd from_cmd;

        if (last_real_cmd)
            memcpy(&from_cmd, last_real_cmd, sizeof(CUserCmd));

        CUserCmd to_cmd;
        memcpy(&to_cmd, &from_cmd, sizeof(CUserCmd));

        to_cmd.command_number++;
        to_cmd.tick_count += 200;

        for (int i = new_commands; i <= total_new_commands; i++)
        {
            WriteUsercmdD(buffer, &to_cmd, &from_cmd);
            memcpy(&from_cmd, &to_cmd, sizeof(CUserCmd));
            to_cmd.command_number++;
            to_cmd.tick_count++;
        }

        return true;
    }
    else 
        return Hooked::oWriteUsercmdDeltaToBuffer(ECX, nSlot, buffer, o_from, o_to, isnewcommand);



}
