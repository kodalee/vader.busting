#include "Menu.hpp"
#include "Fonts.h"
#include <chrono>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "IMGAY/imgui_internal.h"
#include "IMGAY/impl/imgui_impl_dx9.h"
#include "IMGAY/impl/imgui_impl_win32.h"


namespace ImGuiEx
{
    inline bool ColorEdit4(const char* label, Color* v, bool show_alpha = true)
    {
        auto clr = ImVec4{
            v->r() / 255.0f,
            v->g() / 255.0f,
            v->b() / 255.0f,
            v->a() / 255.0f
        };

        if(ImGui::ColorEdit4(label, &clr.x, show_alpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip)) {
        //    v->SetColor(clr.x, clr.y, clr.z, clr.w); ???????????
            return true;
        }
        return false;
    }
    inline bool ColorEdit3(const char* label, Color* v)
    {
        return ColorEdit4(label, v, false);
    }
}

 
int tab = 0, aimbotTab = 1, rageTab = 0, legitTab = 0;

bool testbox = false, testbox1 = false;
void Legitbot()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);
	ImGui::NewLine();
	{
		
	}
	ImGui::EndColumns();
}
void Ragebot()
{
	//autosniper, sniper, pistol, heavy pistol, other

	ImGuiStyle* style = &ImGui::GetStyle();
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);
	ImGui::NewLine();
	{
		static bool testbox1 = false;
		Color testpicker1;
		Color testpicker2;
		InsertCheckbox(Testbox, "Cum", &testbox1);

		ImGuiEx::ColorEdit4("##testpicker1", &testpicker1, true);
		ImGuiEx::ColorEdit4("##testpicker2", &testpicker2, true);
	}
	ImGui::EndColumns();
}

void Antiaim()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	const char* styles[]{ "Static","Jitter","Spin" };

	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);
	ImGui::NewLine();
	{

	}
	ImGui::EndColumns();
}

void Visuals()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);

	ImGui::NewLine();
	{

	}
	ImGui::EndColumns();
}

void Misc()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
	ImGui::Columns(3, nullptr, false);
	ImGui::SetColumnOffset(1, group_w / 3.0f);
	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
	ImGui::SetColumnOffset(3, group_w);

	
	ImGui::NewLine();
	{

	}
	ImGui::EndColumns();
}

bool IMGUIMenu::Initialize(IDirect3DDevice9* pDevice)
{
	static bool initialized = false;
	if (!initialized)
	{
		ImGui::CreateContext();
		CreateStyle();
		IDirect3DSwapChain9* pChain = nullptr;
		D3DPRESENT_PARAMETERS pp = {};
		D3DDEVICE_CREATION_PARAMETERS param = {};
		pDevice->GetCreationParameters(&param);
		pDevice->GetSwapChain(0, &pChain);

		if (pChain) pChain->GetPresentParameters(&pp);

		ImGui_ImplWin32_Init(param.hFocusWindow);
		ImGui_ImplDX9_Init(pDevice);
		ImGui_ImplDX9_CreateDeviceObjects();
		initialized = true;
	}

	return initialized;
}

void IMGUIMenu::Shutdown()
{
    ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void IMGUIMenu::OnDeviceLost()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

void IMGUIMenu::OnDeviceReset()
{
    ImGui_ImplDX9_CreateDeviceObjects();
}

auto windowFlags = (ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding);
ImFont* gravity, *gravityBold, *StarWars, *watermark;

void IMGUIMenu::Render()
{
	if (!Opened) return;

	//ImGui::GetIO().MouseDrawCursor = _visible;

	ImGuiStyle* style = &ImGui::GetStyle();

	ImGui::PushFont(gravityBold);

	ImGui::SetNextWindowSize(ImVec2(600.f, 550.f));
	ImGui::Begin("##menu", &_visible, windowFlags);

	style->WindowPadding = ImVec2(7.f, 7.f);

	style->Colors[ImGuiCol_MenuAccent] = ImColor(255, 215, 0);
	style->Colors[ImGuiCol_Logo] = ImColor(0, 87, 255);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	{
		ImGui::BeginTitleBar("Title Bar", ImVec2(586.f, 55.f), false);

		ImGui::PopFont();
		ImGui::PushFont(StarWars);
		ImGui::SameLine(5.f);
		ImGui::PushStyleColor(ImGuiCol_Text, ImGuiCol_Logo);
		ImGui::Text("VADER");
		ImGui::PopStyleColor();
		ImGui::PopFont();
		ImGui::PushFont(gravityBold);

		ImGui::EndTitleBar();
	}
	ImGui::PopStyleVar();
	style->Colors[ImGuiCol_ChildBg] = ImColor(41, 32, 59);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	{
		ImGui::BeginChild("##tabs", ImVec2(586.f, 35.f), false);

		ImGui::SameLine(8.f);
		ImGui::TrueTab("  AIMBOT  ", tab, 0, ImVec2(0.f, 35.f)); ImGui::SameLine();
		ImGui::TrueTab("  ANTI-AIM  ", tab, 1, ImVec2(0.f, 35.f)); ImGui::SameLine();
		ImGui::TrueTab("  VISUALS  ", tab, 2, ImVec2(0.f, 35.f)); ImGui::SameLine();
		ImGui::TrueTab("  MISC  ", tab, 3, ImVec2(0.f, 35.f)); ImGui::SameLine();
		ImGui::TrueTab("  SKINS  ", tab, 4, ImVec2(0.f, 35.f)); ImGui::SameLine();
		ImGui::TrueTab("  SCRIPTS  ", tab, 5, ImVec2(0.f, 35.f)); ImGui::SameLine();

		ImGui::EndChild();
	}
	ImGui::PopStyleVar();

	style->Colors[ImGuiCol_ChildBg] = ImColor(31, 24, 46);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0)); /* all SubTabs are handled here */
	{
		ImGui::BeginChild("##subtabs", ImVec2(586.f, 25.f), false);

		ImGui::SameLine(8.f);


		switch (tab)
		{
			case 0:
			{
				ImGui::TrueSubTab("  Rage  ", aimbotTab, 0, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  Legit  ", aimbotTab, 1, ImVec2(0.f, 25.f)); ImGui::SameLine();

				ImGui::SameLine(195.f);
				ImGui::TrueSubTab("  Rifle  ", legitTab, 0, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  Sniper  ", legitTab, 1, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  Pistol  ", legitTab, 2, ImVec2(0.f, 25.f)); ImGui::SameLine();
				ImGui::TrueSubTab("  Other  ", legitTab, 3, ImVec2(0.f, 25.f)); ImGui::SameLine();

				break;
			}
			case 1:
			{
				ImGui::SelectedSubTab("  Basic  ", ImVec2(0.f, 25.f));
				break;
			}
			case 2:
			{
				ImGui::SelectedSubTab("  Player  ", ImVec2(0.f, 25.f));
				break;
			}
			case 3:
			{
				ImGui::SelectedSubTab("  Misc  ", ImVec2(0.f, 25.f));
				break;
			}
		}

		ImGui::EndChild();
	}
	ImGui::PopStyleVar();

	style->Colors[ImGuiCol_ChildBg] = ImColor(25, 20, 27);
	ImGui::BeginChild("##main", ImVec2(586.f, 420.f), false);
	ImGui::SameLine(7.f);
	style->Colors[ImGuiCol_ChildBg] = ImColor(21, 17, 29);
	switch (tab)
	{
		case 0:
			(aimbotTab == 1) ? Ragebot() : Legitbot();
			break;
		case 1:
			Antiaim();
			break;
		case 2:
			Visuals();
			break;
		case 3:
			Misc();
		default:
			break;
	}
	style->Colors[ImGuiCol_Border] = ImColor(0, 0, 0, 0);
	ImGui::EndChild();
	ImGui::End();
	ImGui::PopFont();
  
}

void IMGUIMenu::Toggle()
{
    _visible = !_visible;
}

void IMGUIMenu::CreateStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::StyleColorsDark(&style);
	style.WindowRounding = 0.0f;
	style.WindowBorderSize = 1.0f;
	style.ChildBorderSize = 1.0f;
	style.GrabMinSize = 5.0f;
	style.ScrollbarSize = 12.f;
	style.ScrollbarRounding = 0.0f;
	style.GrabRounding = 0.0f;
	style.FrameRounding = 0.0f;
	style.PopupRounding = 0.0f;
	style.FrameBorderSize = 0.4f;
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;
	//io.MouseDrawCursor = true;
	char buffer[MAX_PATH];
	//GetWindowsDirectoryA(buffer, MAX_PATH);

	gravity = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(gravity_compressed_data, gravity_compressed_size, 13.f);
	StarWars = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(star_wars_compressed_data, star_wars_compressed_size, 50.f);
	watermark = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(gravityb_compressed_data, gravityb_compressed_size, 16.f);
	gravityBold = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(gravityb_compressed_data, gravityb_compressed_size, 13.f);


}

void CustomCheckbox::Draw(std::string title, bool* value)
{
	using namespace std::chrono;
	auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() / 10;

	auto balls = *value;

	if (!FirstDraw)
	{
		AnimStep = balls ? 200.f : 0.f;
		LastStep = ms;
		FirstDraw = true;
	}

	ImGui::CheckBoxAnimated((title).c_str(), value, AnimStep);


	auto delta = ms - LastStep;
	auto mod = 5 * delta; //9 / (1000 / 60) = 0.54
	AnimStep += balls ? mod : -mod;
	LastStep = ms;

	if (AnimStep <= 0) {
		AnimStep = 0;
		LastStep = ms;
	}
	else if (AnimStep >= 200) {
		AnimStep = 200;
		LastStep = ms;
	}
}