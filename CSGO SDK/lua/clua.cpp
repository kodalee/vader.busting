#include "clua.h"

void lua_panic(sol::optional<std::string> message) {

	if (message) {
		std::string m = message.value();
		MessageBoxA(0, m.c_str(), XorStr("Lua: panic state"), MB_APPLMODAL | MB_OK);
	}
}

// ----- lua functions -----

int extract_owner(sol::this_state st) {
	sol::state_view lua_state(st);
	sol::table rs = lua_state["debug"]["getinfo"](2, "S");
	std::string source = rs["source"];
	std::string filename = std::filesystem::path(source.substr(1)).filename().string();
	return g_lua.get_script_id(filename);
}

namespace ns_config {
	///*
	//config.get(key)
	//Returns value of given key or nil if key not found.
	//*/
	//std::tuple<sol::object, sol::object, sol::object, sol::object> get(sol::this_state s, std::string key) {
	//	std::tuple<sol::object, sol::object, sol::object, sol::object> retn = std::make_tuple(sol::nil, sol::nil, sol::nil, sol::nil);

	//	for (auto kv : g_cfg::c_bool)
	//		if (kv.first == key)
	//			retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

	//	for (auto kv : g_cfg::c_color)
	//		if (kv.first == key)
	//			retn = std::make_tuple(sol::make_object(s, (int)(kv.second[0] * 255)), sol::make_object(s, (int)(kv.second[1] * 255)), sol::make_object(s, (int)(kv.second[2] * 255)), sol::make_object(s, (int)(kv.second[3] * 255)));

	//	for (auto kv : g_cfg::c_float)
	//		if (kv.first == key)
	//			retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

	//	for (auto kv : g_cfg::c_int)
	//		if (kv.first == key)
	//			retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

	//	for (auto kv : g_cfg::c_multi)
	//		if (kv.first == key)
	//			retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

	//	return retn;
	//}

	///*
	//config.set(key, value)
	//Sets value for key
	//*/
	//void set_bool(std::string key, bool v) {
	//	g_cfg::c_bool[key] = v;
	//}

	//void set_float(std::string key, float v) {
	//	if (ceilf(v) != v)
	//		g_cfg::c_float[key] = v;
	//	else
	//		g_cfg::c_int[key] = (int)v;
	//}

	//void set_color(std::string key, int r, int g, int b, int a) {
	//	g_cfg::c_color[key][0] = r / 255.f;
	//	g_cfg::c_color[key][1] = g / 255.f;
	//	g_cfg::c_color[key][2] = b / 255.f;
	//	g_cfg::c_color[key][3] = a / 255.f;
	//}

	//void set_multiselect(std::string key, int pos, bool e) {
	//	g_cfg::c_multi[key][pos] = e;
	//}

	//void set_int(std::string key, int value) {
	//	g_cfg::c_int[key] = value;
	//}

	///*
	//config.load()
	//Loads selected config
	//*/
	//void load() {
	//	g_cfg::load();
	//}

	///*
	//config.save()
	//Saves selected config

	//*/
	//void save() {
	//	g_cfg::save();
	//}
}
namespace ns_cheat {
	void set_event_callback(sol::this_state s, std::string eventname, sol::function func) {
		sol::state_view lua_state(s);
		sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
		std::string source = rs["source"];
		std::string filename = std::filesystem::path(source.substr(1)).filename().string();

		g_lua_hook.registerHook(eventname, g_lua.get_script_id(filename), func);

		ILoggerEvent::Get()->PushEvent(filename + ": subscribed to event " + eventname  +"\n", FloatColor(1.f, 1.f, 1.f), true, XorStr(""));
	}

	void run_script(std::string scriptname) {
		int scrid = g_lua.get_script_id(scriptname);
		if (scrid < 0)
			return;

		g_lua.load_script(scrid);
	}

	void reload_active_scripts() {
		g_lua.reload_all_scripts();
	}

	//void chat_print(int iPlayerIndex, int iFilter, const char* fmt) {
	//	g_csgo.m_client_mode->m_pChatElement->ChatPrintf(iPlayerIndex, iFilter, fmt);
	//}

	void log(const std::string& text, FloatColor clr, bool visualise) {
		ILoggerEvent::Get()->PushEvent(text +"\n", clr, visualise, XorStr(""));
	}
}
namespace ns_models {
	/*
	models.get_studio_model(mdl)
	Returns studio model of mdl
	*/
	studiohdr_t* get_studio_model(model_t* mdl) {
		return Interfaces::m_pModelInfo->GetStudiomodel(mdl);
	}

	/*
	models.get_model_index(name)
	Returns model index of given name
	*/
	int get_model_index(std::string name) {
		return Interfaces::m_pModelInfo->GetModelIndex(name.c_str());
	}
}
namespace ns_engine {
	//std::tuple<int, int> get_screen_size() {
	//	int w, h;
	//	g_csgo.m_engine->GetScreenSize(w, h);
	//	return std::make_tuple(w, h);
	//}

	player_info_t get_player_info(int ent) {
		player_info_t p;
		Interfaces::m_pEngine->GetPlayerInfo(ent, &p);
		return p;
	}

	int get_player_for_user_id(int userid) {
		return Interfaces::m_pEngine->GetPlayerForUserID(userid);
	}

	int get_local_player_index() {
		return Interfaces::m_pEngine->GetLocalPlayer();
	}

	float get_last_timestamp() {
		return Interfaces::m_pEngine->GetLastTimeStamp();
	}

	QAngle get_view_angles() {
		QAngle va;
		Interfaces::m_pEngine->GetViewAngles(va);
		return va;
	}

	void set_view_angles(QAngle va) {
		Interfaces::m_pEngine->SetViewAngles(va);
	}

	int get_max_clients() {
		return Interfaces::m_pGlobalVars->maxClients;
	}

	bool is_in_game() {
		return Interfaces::m_pEngine->IsInGame();
	}

	bool is_connected() {
		return Interfaces::m_pEngine->IsConnected();
	}


	INetChannel* get_net_channel_info() {
		return Interfaces::m_pEngine->GetNetChannelInfo();
	}

	bool is_paused() {
		return Interfaces::m_pEngine->IsPaused();
	}

	void execute_client_cmd(std::string cmd) {
		Interfaces::m_pEngine->ExecuteClientCmd(cmd.c_str());
	}
}
//namespace ns_entity_list {
//	IClientEntity* get_client_entity(int idx) {
//		return Interfaces::m_pEntList->GetClientEntity(idx);
//	}
//
//	int get_highest_entity_index() {
//		return Interfaces::m_pEntList->GetHighestEntityIndex();
//	}
//
//	//IClientEntity* get_client_entity_from_handle(const auto ent) { // idk
//	//	return Interfaces::m_pEntList->GetClientEntityFromHandle(ent);
//	//}
//}
namespace ns_utils {
	sol::table get_player_data(player_info_t& p) {
		sol::table t = g_lua.lua.create_table();
		t["name"] = std::string(p.szName);
		t["steamid"] = std::string(p.szSteamID);
		t["userid"] = p.userId;
		return t;
	}

	double clamp(double v, double mi, double ma) {
		return std::clamp(v, mi, ma);
	}

	uint64_t find_signature(const std::string& szModule, const std::string& szSignature)
	{
		return Memory::Scan(szModule.c_str(), szSignature.c_str());
	}
}
namespace ns_globals {
	//std::string get_username()
	//{
	//	return g_cl.m_user;
	//}

	float realtime() {
		return Interfaces::m_pGlobalVars->realtime;
	}

	int framecount() {
		return Interfaces::m_pGlobalVars->framecount;
	}

	float absoluteframetime() {
		return Interfaces::m_pGlobalVars->absoluteframetime;
	}

	float curtime() {
		return Interfaces::m_pGlobalVars->curtime;
	}

	float frametime() {
		return Interfaces::m_pGlobalVars->frametime;
	}

	int maxclients() {
		return Interfaces::m_pGlobalVars->maxClients;
	}

	int tickcount() {
		return Interfaces::m_pGlobalVars->tickcount;
	}

	float tickinterval() {
		return Interfaces::m_pGlobalVars->interval_per_tick;
	}
}
namespace ns_trace {
	int get_point_contents(Vector pos, int mask) {
		return Interfaces::m_pEngineTrace->GetPointContents(pos, mask);
	}

	/*std::tuple<float, Player*> trace_ray(vec3_t from, vec3_t to, int mask) {
		Ray ray;
		ITraceFilter filter;
		trace_t trace;
		trace.start = from;
		trace.end = to;
		filter.pSkip1 = G::LocalPlayer;

		g_csgo.m_engine_trace->TraceRay(ray, mask, &filter, &trace);
		return std::make_tuple(trace.fraction, trace.m_pEnt);
	}*/
}
namespace ns_cvar {

	ConVar* find_var(const char* name) {
		return Interfaces::m_pCvar->FindVar(name);
	}

	void console_color_print(Color color, sol::variadic_args args) {
		std::string txt = "";
		for (auto v : args)
			txt += v;

		Interfaces::m_pCvar->ConsoleColorPrintf(color, txt.c_str());
	}

	void console_color_print_rgba(Color color, sol::variadic_args args) {
		std::string txt = "";
		for (auto v : args)
			txt += v;

		Interfaces::m_pCvar->ConsoleColorPrintf(color, txt.c_str());
	}

	void console_print(sol::variadic_args args) {
		std::string txt = "";
		for (auto v : args)
			txt += v;

		Interfaces::m_pCvar->ConsoleColorPrintf(Color(255,255,255), txt.c_str());
	}

	void unlock_cvar(ConVar* var)
	{
		if (var)
		{
			var->nFlags &= ~FCVAR_DEVELOPMENTONLY;
			var->nFlags &= ~FCVAR_HIDDEN;
		}
	}

	void remove_callbacks(ConVar* var) {
		if (var)
			var->fnChangeCallback.RemoveAll();
	}

	float get_float(ConVar* var) {
		return var ? var->GetFloat() : 0.f;
	}

	int get_int(ConVar* var) {
		return var ? var->GetInt() : 0;
	}

	const char* get_string(ConVar* var) {
		return var ? var->GetString() : "";
	}

	void set_float(ConVar* var, float f, bool unlock = false) {
		if (var) var->SetValue(std::to_string(f).c_str());
	}

	void set_int(ConVar* var, int i, bool unlock = false) {
		if (var) var->SetValue(std::to_string(i).c_str());
	}

	void set_string(ConVar* var, const char* v, bool unlock = false) {
		if (var)
			var->SetValue(v);
	}
}
namespace ns_overlay {
	void add_box_overlay(Vector pos, Vector mins, Vector maxs, QAngle orientation, int r, int g, int b, int a, float duration) {
		Interfaces::m_pDebugOverlay->AddBoxOverlay(pos, mins, maxs, orientation, r, g, b, a, duration);
	}

	void add_text_overlay(Vector pos, int line_offset, float duration, sol::variadic_args txt) {
		std::string text = "";
		for (auto v : txt)
			text += v;

		Interfaces::m_pDebugOverlay->AddTextOverlay(pos, duration, text.c_str());
	}

	void add_capsule_overlay(Vector mins, Vector maxs, float pillradius, int r, int g, int b, int a, float duration) {
		Interfaces::m_pDebugOverlay->AddCapsuleOverlay(mins, maxs, pillradius, r, g, b, a, duration);
	}
}
namespace ns_beams {
	void draw_beam(Beam_t* beam) {
		Interfaces::m_pRenderBeams->DrawBeam(beam);
	}

	Beam_t* create_points(BeamInfo_t beam) {
		return Interfaces::m_pRenderBeams->CreateBeamPoints(beam);
	}
}

namespace ns_ui {
	std::string new_checkbox(sol::this_state s, std::string tab, std::string container, std::string label, std::string key, std::optional<bool> def, std::optional<sol::function> cb) {
		std::transform(tab.begin(), tab.end(), tab.begin(), ::tolower);
		std::transform(container.begin(), container.end(), container.begin(), ::tolower);

		MenuItem_t item;
		item.type = MENUITEM_CHECKBOX;
		item.script = extract_owner(s);
		item.label = label;
		item.key = key;
		item.b_default = def.value_or(false);
		item.callback = cb.value_or(sol::nil);

		g_lua.menu_items[tab][container].push_back(item);
		return key;
	}

	std::string new_slider_int(sol::this_state s, std::string tab, std::string container, std::string label, std::string key, int min, int max, std::optional<std::string> format, std::optional<int> def, std::optional<sol::function> cb) {
		std::transform(tab.begin(), tab.end(), tab.begin(), ::tolower);
		std::transform(container.begin(), container.end(), container.begin(), ::tolower);

		MenuItem_t item;
		item.type = MENUITEM_SLIDERINT;
		item.script = extract_owner(s);
		item.label = label;
		item.key = key;
		item.i_default = def.value_or(0);
		item.i_min = min;
		item.i_max = max;
		item.format = format.value_or("%d");
		item.callback = cb.value_or(sol::nil);

		g_lua.menu_items[tab][container].push_back(item);
		return key;
	}

	std::string new_slider_float(sol::this_state s, std::string tab, std::string container, std::string label, std::string key, float min, float max, std::optional<std::string> format, std::optional<float> def, std::optional<sol::function> cb) {
		std::transform(tab.begin(), tab.end(), tab.begin(), ::tolower);
		std::transform(container.begin(), container.end(), container.begin(), ::tolower);

		MenuItem_t item;
		item.type = MENUITEM_SLIDERFLOAT;
		item.script = extract_owner(s);
		item.label = label;
		item.key = key;
		item.f_default = def.value_or(0.f);
		item.f_min = min;
		item.f_max = max;
		item.format = format.value_or("%.0f");
		item.callback = cb.value_or(sol::nil);

		g_lua.menu_items[tab][container].push_back(item);
		return key;
	}

	std::string new_keybind(sol::this_state s, std::string tab, std::string container, std::string id, std::string key, std::optional<bool> allow_sc, std::optional<int> def, std::optional<sol::function> cb) {
		std::transform(tab.begin(), tab.end(), tab.begin(), ::tolower);
		std::transform(container.begin(), container.end(), container.begin(), ::tolower);

		MenuItem_t item;
		item.type = MENUITEM_KEYBIND;
		item.script = extract_owner(s);
		item.label = id;
		item.key = key;
		item.allow_style_change = allow_sc.value_or(true);
		item.i_default = def.value_or(0);
		item.callback = cb.value_or(sol::nil);

		g_lua.menu_items[tab][container].push_back(item);
		return key;
	}

	std::string new_text(sol::this_state s, std::string tab, std::string container, std::string label, std::string key) {
		std::transform(tab.begin(), tab.end(), tab.begin(), ::tolower);
		std::transform(container.begin(), container.end(), container.begin(), ::tolower);

		MenuItem_t item;
		item.type = MENUITEM_TEXT;
		item.script = extract_owner(s);
		item.label = label;
		item.key = key;

		g_lua.menu_items[tab][container].push_back(item);
		return key;
	}

	std::string new_multiselect(sol::this_state s, std::string tab, std::string container, std::string label, std::string key, std::vector<const char*> items, std::optional<std::map<int, bool>> def, std::optional<sol::function> cb) {
		std::transform(tab.begin(), tab.end(), tab.begin(), ::tolower);
		std::transform(container.begin(), container.end(), container.begin(), ::tolower);

		MenuItem_t item;
		item.type = MENUITEM_MULTISELECT;
		item.script = extract_owner(s);
		item.label = label;
		item.key = key;
		item.m_default = def.value_or(std::map<int, bool> {});
		item.items = items;
		item.callback = cb.value_or(sol::nil);

		g_lua.menu_items[tab][container].push_back(item);
		return key;
	}

	std::string new_colorpicker(sol::this_state s, std::string tab, std::string container, std::string id, std::string key, std::optional<int> r, std::optional<int> g, std::optional<int> b, std::optional<int> a, std::optional<sol::function> cb) {
		std::transform(tab.begin(), tab.end(), tab.begin(), ::tolower);
		std::transform(container.begin(), container.end(), container.begin(), ::tolower);

		MenuItem_t item;
		item.type = MENUITEM_COLORPICKER;
		item.script = extract_owner(s);
		item.label = id;
		item.key = key;
		item.c_default[0] = r.value_or(255) / 255.f;
		item.c_default[1] = g.value_or(255) / 255.f;
		item.c_default[2] = b.value_or(255) / 255.f;
		item.c_default[3] = a.value_or(255) / 255.f;
		item.callback = cb.value_or(sol::nil);

		g_lua.menu_items[tab][container].push_back(item);
		return key;
	}

	std::string new_button(sol::this_state s, std::string tab, std::string container, std::string id, std::string key, std::optional<sol::function> cb) {
		std::transform(tab.begin(), tab.end(), tab.begin(), ::tolower);
		std::transform(container.begin(), container.end(), container.begin(), ::tolower);

		MenuItem_t item;
		item.type = MENUITEM_BUTTON;
		item.script = extract_owner(s);
		item.label = id;
		item.key = key;
		item.callback = cb.value_or(sol::nil);

		g_lua.menu_items[tab][container].push_back(item);
		return key;
	}

	void set_visibility(std::string key, bool v) {
		for (auto t : g_lua.menu_items) {
			for (auto c : t.second) {
				for (auto& i : c.second) {
					if (i.key == key)
						i.is_visible = v;
				}
			}
		}
	}

	void set_items(std::string key, std::vector<const char*> items) {
		for (auto t : g_lua.menu_items) {
			for (auto c : t.second) {
				for (auto& i : c.second) {
					if (i.key == key)
						i.items = items;
				}
			}
		}
	}

	void set_callback(std::string key, sol::function v) {
		for (auto t : g_lua.menu_items) {
			for (auto c : t.second) {
				for (auto& i : c.second) {
					if (i.key == key)
						i.callback = v;
				}
			}
		}
	}

	void set_label(std::string key, std::string v) {
		for (auto t : g_lua.menu_items) {
			for (auto c : t.second) {
				for (auto& i : c.second) {
					if (i.key == key)
						i.label = v;
				}
			}
		}
	}

	//bool is_bind_active(std::string key) {
	//	return g_cfg::auto_check(key);
	//}
}

namespace ns_surface {
	void set_texture(int id) {
		Interfaces::m_pSurface->DrawSetTexture(id);
	}

	void set_texture_rgba(int id, const unsigned char* rgba, int w, int h) {
		Interfaces::m_pSurface->DrawSetTextureRGBA(id, rgba, w, h);
	}

	void set_color(Color color) {
		Interfaces::m_pSurface->DrawSetColor(color);
	}

	int create_texture() {
		return Interfaces::m_pSurface->CreateNewTextureID();
	}

	void draw_filled_rect(int x, int y, int x2, int y2) {
		Interfaces::m_pSurface->DrawFilledRect(x, y, x2, y2);
	}

	void draw_outlined_rect(int x, int y, int x2, int y2) {
		Interfaces::m_pSurface->DrawOutlinedRect(x, y, x2, y2);
	}

	std::tuple<int, int> get_text_size(int font, std::wstring text) {
		int w, h;
		Interfaces::m_pSurface->GetTextSize(font, text.c_str(), w, h);
		return std::make_tuple(w, h);
	}

	void draw_line(int x, int y, int x2, int y2) {
		Interfaces::m_pSurface->DrawLine(x, y, x2, y2);
	}

	void draw_outlined_circle(int x, int y, int radius, int segments) {
		Interfaces::m_pSurface->DrawOutlinedCircle(x, y, radius, segments);
	}

	int create_font(std::string fontname, int w, int h, int blur, int flags) {
		auto f = Interfaces::m_pSurface->CreateFont_();
		Interfaces::m_pSurface->SetFontGlyphSet(f, fontname.c_str(), w, h, blur, 0, flags);
		return f;
	}

	void set_text_font(int font) {
		Interfaces::m_pSurface->DrawSetTextFont(font);
	}

	void set_text_color(Color color) {
		Interfaces::m_pSurface->DrawSetTextColor(color);
	}

	void set_text_pos(int x, int y) {
		Interfaces::m_pSurface->DrawSetTextPos(x, y);
	}

	void draw_text(std::wstring str) {
		Interfaces::m_pSurface->DrawPrintText(str.c_str(), str.length());
	}

	void draw_textured_rect(int x, int y, int x2, int y2) {
		Interfaces::m_pSurface->DrawTexturedRect(x, y, x2, y2);
	}

	/*void indicator(std::string str, int r, int g, int b, int a) {
		Visuals->custom_inds.push_back({ str, CColor(r, g, b, a) });
	}*/

	void draw_filled_rect_fade(int x, int y, int x2, int y2, int alpha, int alpha2, bool horizontal) {
		Interfaces::m_pSurface->DrawFilledRectFade(x, y, x2, y2, alpha, alpha2, horizontal);
	}
}

namespace ns_render {

	void draw_text(int x, int y, Color color, const std::string& text, Render::Engine::StringFlags_t flags)
	{
		Render::Engine::watermark.string((int)x, (int)y, color, text, flags); // idk how to make it custom fonts
	}

	void draw_line(float x, float y, float x2, float y2, Color color)
	{
		Render::Engine::Line((int)x, (int)y, (int)x2, (int)y2, color);
	}

	void draw_rect(float x, float y, float w, float h, Color color)
	{
		Render::Engine::Rect((int)x, (int)y, (int)w, (int)h, color);
	}

	void draw_rect_filled(float x, float y, float w, float h, Color color)
	{
		Render::Engine::RectFilled((int)x, (int)y, (int)w, (int)h, color);
	}

	//void draw_rect_filled_fade(int x, int y, int w, int h, Color color, int a1, int a2)
	//{
	//	render::rect_filled_fade((int)x, (int)y, (int)w, (int)h, color, (int)a1, (int)a2);
	//}

	void draw_rect_outlined(int x, int y, int w, int h, Color color, Color color2)
	{
		Render::Engine::RectOutlined((int)x, (int)y, (int)w, (int)h, color, color2);
	}

	void draw_circle(int x, int y, int radius, int segments, Color color)
	{
		Render::Engine::CircleFilled((int)x, (int)y, (int)radius, (int)segments, color);
	}

	void draw_gradient(int x, int y, int w, int h, Color color, Color color2, bool horizontal)
	{
		Render::Engine::Gradient((int)x, (int)y, (int)w, (int)h, color, color2, horizontal);
	}

	Vector2D getscreensize()
	{
		return Render::GetScreenSize();
	}

	Vector2D world_to_screen(Vector pos) {
		Vector2D scr;
		Render::Engine::WorldToScreen(pos, scr);
		return scr;
	}

}

//namespace ns_file
//{
//	void append(sol::this_state s, std::string& path, std::string& data)
//	{
//		std::ofstream out(path, std::ios::app | std::ios::binary);
//
//		if (out.is_open())
//			out << data;
//		else
//			g_notify.add(XOR("Can't append to file: ") + path, false);
//
//		out.close();
//	}
//	void write(sol::this_state s, std::string& path, std::string& data)
//	{
//		std::ofstream out(path, std::ios::binary);
//
//		if (out.is_open())
//			out << data;
//
//		else
//			g_notify.add(XOR("Can't write to file: ") + path, false);
//
//		out.close();
//	}
//	std::string read(sol::this_state s, std::string& path)
//	{
//
//		std::ifstream file(path, std::ios::binary);
//
//		if (file.is_open())
//		{
//			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//			file.close();
//			return content;
//		}
//		else
//		{
//			g_notify.add(XOR("Can't read file: ") + path, false);
//			file.close();
//			return "";
//		}
//	}
//}

// ----- lua functions -----

c_lua g_lua;

void c_lua::init() {
	this->lua = sol::state(sol::c_call<decltype(&lua_panic), &lua_panic>);
	this->lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::debug, sol::lib::package);

	this->lua["collectgarbage"] = sol::nil;
	this->lua["dofile"] = sol::nil;
	this->lua["load"] = sol::nil;
	this->lua["loadfile"] = sol::nil;
	this->lua["pcall"] = sol::nil;
	this->lua["print"] = sol::nil;
	this->lua["xpcall"] = sol::nil;
	this->lua["getmetatable"] = sol::nil;
	this->lua["setmetatable"] = sol::nil;
	this->lua["__nil_callback"] = []() {};

	this->lua["print"] = [](std::string s) { ILoggerEvent::Get()->PushEvent(s.c_str(), FloatColor(1.f, 0.f, 0.f), false, XorStr("")); };
	this->lua["error"] = [](std::string s) { ILoggerEvent::Get()->PushEvent(s.c_str(), FloatColor(1.f, 0.f, 0.f), false, XorStr("")); };

	this->lua.new_usertype <Color>(("color"), sol::constructors <Color(), Color(int, int, int), Color(int, int, int, int)>(),
		(std::string)("r"), &Color::r,
		(std::string)("g"), &Color::g,
		(std::string)("b"), &Color::b,
		(std::string)("a"), &Color::a
		);

	this->lua.new_usertype<CUserCmd>("c_usercmd",
		"command_number", sol::readonly(&CUserCmd::command_number),
		"tick_count", sol::readonly(&CUserCmd::tick_count),
		"viewangles", &CUserCmd::viewangles,
		"aimdirection", &CUserCmd::aimdirection,
		"forwardmove", &CUserCmd::forwardmove,
		"sidemove", &CUserCmd::sidemove,
		"upmove", &CUserCmd::upmove,
		"buttons", &CUserCmd::buttons,
		"impulse", sol::readonly(&CUserCmd::impulse),
		"weaponselect", &CUserCmd::weaponselect,
		"weaponsubtype", sol::readonly(&CUserCmd::weaponsubtype),
		"random_seed", sol::readonly(&CUserCmd::random_seed),
		"mousedx", &CUserCmd::mousedx,
		"mousedy", &CUserCmd::mousedy,
		"hasbeenpredicted", sol::readonly(&CUserCmd::hasbeenpredicted)
		);
	this->lua.new_usertype<IGameEvent>("c_gameevent",
		"get_name", &IGameEvent::GetName,
		"is_reliable", &IGameEvent::IsReliable,
		"is_local", &IGameEvent::IsLocal,
		"is_empty", &IGameEvent::IsEmpty,
		"get_bool", &IGameEvent::GetBool,
		"get_int", &IGameEvent::GetInt,
		"get_uint64", &IGameEvent::GetUint64,
		"get_float", &IGameEvent::GetFloat,
		"get_string", &IGameEvent::GetString
		);
	this->lua.new_enum("HITBOXES",
		"HEAD", Hitboxes::HITBOX_HEAD,
		"NECK", Hitboxes::HITBOX_NECK,
		"LOWER_NECK", Hitboxes::HITBOX_LOWER_NECK,
		"PELVIS", Hitboxes::HITBOX_PELVIS,
		"STOMACH", Hitboxes::HITBOX_STOMACH,
		"UPPER_CHEST", Hitboxes::HITBOX_UPPER_CHEST,
		"CHEST", Hitboxes::HITBOX_CHEST,
		"LOWER_CHEST", Hitboxes::HITBOX_LOWER_CHEST,
		"RIGHT_THIGH", Hitboxes::HITBOX_RIGHT_THIGH,
		"LEFT_THIGH", Hitboxes::HITBOX_LEFT_THIGH,
		"RIGHT_CALF", Hitboxes::HITBOX_RIGHT_CALF,
		"LEFT_CALF", Hitboxes::HITBOX_LEFT_CALF,
		"RIGHT_FOOT", Hitboxes::HITBOX_RIGHT_FOOT,
		"LEFT_FOOT", Hitboxes::HITBOX_LEFT_FOOT,
		"RIGHT_HAND", Hitboxes::HITBOX_RIGHT_HAND,
		"LEFT_HAND", Hitboxes::HITBOX_LEFT_HAND,
		"RIGHT_UPPER_ARM", Hitboxes::HITBOX_RIGHT_UPPER_ARM,
		"RIGHT_FOREARM", Hitboxes::HITBOX_RIGHT_FOREARM,
		"LEFT_UPPER_ARM", Hitboxes::HITBOX_LEFT_UPPER_ARM,
		"LEFT_FOREARM", Hitboxes::HITBOX_LEFT_FOREARM,
		"MAX", Hitboxes::HITBOX_MAX
	);
	this->lua.new_enum("FONTFLAGS",
		"NONE", FontFlags_t::FONTFLAG_NONE,
		"ITALIC", FontFlags_t::FONTFLAG_ITALIC,
		"UNDERLINE", FontFlags_t::FONTFLAG_UNDERLINE,
		"STRIKEOUT", FontFlags_t::FONTFLAG_STRIKEOUT,
		"SYMBOL", FontFlags_t::FONTFLAG_SYMBOL,
		"ANTIALIAS", FontFlags_t::FONTFLAG_ANTIALIAS,
		"GAUSSIANBLUR", FontFlags_t::FONTFLAG_GAUSSIANBLUR,
		"ROTARY", FontFlags_t::FONTFLAG_ROTARY,
		"DROPSHADOW", FontFlags_t::FONTFLAG_DROPSHADOW,
		"ADDITIVE", FontFlags_t::FONTFLAG_ADDITIVE,
		"OUTLINE", FontFlags_t::FONTFLAG_OUTLINE
	);
	this->lua.new_usertype<player_info_t>("c_playerinfo");
	this->lua.new_usertype<ConVar>("c_convar");
	this->lua.new_usertype<INetChannel>("c_netchannelinfo",
		"get_latency", &INetChannel::GetLatency,
		"get_avg_latency", &INetChannel::GetAvgLatency
		);
	this->lua.new_usertype<INetChannel>("c_netchannel",
		"out_sequence_nr", sol::readonly(&INetChannel::m_nOutSequenceNr),
		"in_sequence_nr", sol::readonly(&INetChannel::m_nInSequenceNr),
		"out_sequence_nr_ack", sol::readonly(&INetChannel::m_nOutSequenceNrAck),
		"out_reliable_state", sol::readonly(&INetChannel::m_nOutReliableState),
		"in_reliable_state", sol::readonly(&INetChannel::m_nInReliableState),
		"choked_packets", sol::readonly(&INetChannel::m_nChokedPackets)
		);
	this->lua.new_usertype<Color>("c_color",
		sol::constructors<Color(), Color(int, int, int), Color(int, int, int, int)>(),
		"r", &Color::r,
		"g", &Color::g,
		"b", &Color::b,
		"a", &Color::a
		);
	this->lua.new_usertype<Vector2D>("c_vector2d",
		sol::constructors<Vector2D(), Vector2D(float, float), Vector2D(Vector2D)>(),
		"x", &Vector2D::x,
		"y", &Vector2D::y,
		"length", &Vector2D::Length
		);
	this->lua.new_usertype<Vertex_t>("c_vertex",
		sol::constructors<Vertex_t(), Vertex_t(Vector2D), Vertex_t(Vector2D, Vector2D)>(),
		"init", &Vertex_t::Init,
		"position", &Vertex_t::m_Position,
		"tex_coord", &Vertex_t::m_TexCoord
		);
	this->lua.new_usertype<Vector>("c_vector",
		sol::constructors<Vector(), Vector(float, float, float)>(),
		"x", &Vector::x,
		"y", &Vector::y,
		"z", &Vector::z,
		"length", &Vector::Length,
		"length_sqr", &Vector::LengthSquared,
		"length_2d", &Vector::Length2D,
		"length_2d_sqr", &Vector::Length2DSquared,
		"dist_to", &Vector::Distance,
		"cross_product", &Vector::Cross,
		"normalize", &Vector::Normalize,
		"normalize_angles", &Vector::Normalized
		);
	this->lua.new_usertype<studiohdr_t>("c_studiohdr",
		"id", sol::readonly(&studiohdr_t::id),
		"version", sol::readonly(&studiohdr_t::version),
		"checksum", sol::readonly(&studiohdr_t::checksum),
		"length", sol::readonly(&studiohdr_t::length),
		"eyeposition", sol::readonly(&studiohdr_t::vecEyePos),
		"illumposition", sol::readonly(&studiohdr_t::vecIllumPos),
		"hull_min", sol::readonly(&studiohdr_t::vecHullMin),
		"hull_max", sol::readonly(&studiohdr_t::vecHullMax),
		"view_bbmin", sol::readonly(&studiohdr_t::vecBBMin),
		"view_bbmax", sol::readonly(&studiohdr_t::vecBBMax),
		"flags", sol::readonly(&studiohdr_t::flags),
		"numbones", sol::readonly(&studiohdr_t::numbones),
		"get_bone", &studiohdr_t::pBone,
		"numhitboxsets", sol::readonly(&studiohdr_t::numhitboxsets),
		"get_hitbox_set", &studiohdr_t::pHitboxSet
		);
	this->lua.new_usertype<mstudiohitboxset_t>("c_studiohitboxset",
		"get_hitbox", &mstudiohitboxset_t::pHitbox
		);
	this->lua.new_usertype<mstudiobbox_t>("c_studiobbox",
		"bone", sol::readonly(&mstudiobbox_t::bone),
		"group", sol::readonly(&mstudiobbox_t::group),
		"bbmin", sol::readonly(&mstudiobbox_t::bbmin),
		"bbmax", sol::readonly(&mstudiobbox_t::bbmax),
		"radius", sol::readonly(&mstudiobbox_t::m_flRadius),
		"get_hitbox_name_id", &mstudiobbox_t::getHitboxName
		);
	//this->lua.new_usertype<model_t>("c_model");
	//this->lua.new_usertype<CCSGOPlayerAnimState>("c_animstate",
	//	"entity", &CCSGOPlayerAnimState::m_player,
	//	"last_cs_anim_update_time", &CCSGOPlayerAnimState::m_time,
	//	"last_cs_anim_update_fc", &CCSGOPlayerAnimState::m_frame,
	//	"update_time_delta", &CCSGOPlayerAnimState::m_update_delta,
	//	"yaw", &CCSGOPlayerAnimState::m_eye_yaw,
	//	"pitch", &CCSGOPlayerAnimState::m_eye_pitch,
	//	"goal_feet_yaw", &CCSGOPlayerAnimState::m_goal_feet_yaw,
	//	"current_feet_yaw", &CCSGOPlayerAnimState::m_cur_feet_yaw,
	//	"fall_velocity", &CCSGOPlayerAnimState::m_fall_velocity,
	//	"m_speed", &CCSGOPlayerAnimState::m_speed,
	//	"m_land", &CCSGOPlayerAnimState::m_land,
	//	"m_ground", &CCSGOPlayerAnimState::m_ground
	//	);
	//this->lua.new_usertype<Player>("c_baseentity",
	//	"is_player", &Player::IsPlayer,
	//	"get_abs_origin", &Player::GetAbsOrigin,
	//	"get_index", &Player::index,
	//	"get_spawn_time", &Player::m_flSpawnTime,
	//	"get_anim_state", &Player::m_PlayerAnimState
	//	);
	this->lua.new_usertype<ModelRenderInfo_t>("c_modelrenderinfo",
		"origin", sol::readonly(&ModelRenderInfo_t::origin),
		"angles", sol::readonly(&ModelRenderInfo_t::angles),
		"entity_index", sol::readonly(&ModelRenderInfo_t::entity_index)
		);
	this->lua.new_usertype<CViewSetup>("c_viewsetup",
		"x", &CViewSetup::x,
		"x_old", &CViewSetup::oldX,
		"y", &CViewSetup::y,
		"y_old", &CViewSetup::oldY,
		"width", &CViewSetup::width,
		"width_old", &CViewSetup::oldWidth,
		"height", &CViewSetup::height,
		"height_old", &CViewSetup::oldHeight,
		"fov", &CViewSetup::fov,
		"viewmodel_fov", &CViewSetup::fovViewmodel,
		"origin", &CViewSetup::origin,
		"angles", &CViewSetup::angles
		);

	//auto config = this->lua.create_table();
	//config["get"] = ns_config::get;
	//config["set"] = sol::overload(ns_config::set_bool, ns_config::set_color, ns_config::set_float, ns_config::set_multiselect, ns_config::set_int);
	//config["load"] = ns_config::load;
	//config["save"] = ns_config::save;

	auto cheat = this->lua.create_table();
	cheat["set_event_callback"] = ns_cheat::set_event_callback;
	cheat["run_script"] = ns_cheat::run_script;
	cheat["reload_active_scripts"] = ns_cheat::reload_active_scripts;
	//cheat["chat_print"] = ns_cheat::chat_print;
	cheat["log"] = ns_cheat::log;

	auto surface = this->lua.create_table();
	surface["create_font"] = ns_surface::create_font;
	surface["create_texture"] = ns_surface::create_texture;
	surface["draw_filled_rect"] = ns_surface::draw_filled_rect;
	surface["draw_line"] = ns_surface::draw_line;
	surface["draw_outlined_circle"] = ns_surface::draw_outlined_circle;
	surface["draw_outlined_rect"] = ns_surface::draw_outlined_rect;
	surface["draw_text"] = ns_surface::draw_text;
	surface["draw_textured_rect"] = ns_surface::draw_textured_rect;
	surface["get_text_size"] = ns_surface::get_text_size;
	surface["set_color"] = ns_surface::set_color;
	surface["set_texture"] = sol::overload(ns_surface::set_texture, ns_surface::set_texture_rgba);
	surface["set_text_color"] = ns_surface::set_text_color;
	surface["set_text_font"] = ns_surface::set_text_font;
	surface["set_text_pos"] = ns_surface::set_text_pos;
	surface["draw_filled_rect_fade"] = ns_surface::draw_filled_rect_fade;

	auto models = this->lua.create_table();
	models["get_model_index"] = ns_models::get_model_index;
	models["get_studio_model"] = ns_models::get_studio_model;

	auto engine = this->lua.create_table();
	engine["execute_client_cmd"] = ns_engine::execute_client_cmd;
	engine["get_last_timestamp"] = ns_engine::get_last_timestamp;
	engine["get_local_player_index"] = ns_engine::get_local_player_index;
	engine["get_max_clients"] = ns_engine::get_max_clients;
	engine["get_net_channel_info"] = ns_engine::get_net_channel_info;
	engine["get_player_for_user_id"] = ns_engine::get_player_for_user_id;
	engine["get_player_info"] = ns_engine::get_player_info;
	engine["get_view_angles"] = ns_engine::get_view_angles;
	engine["is_connected"] = ns_engine::is_connected;
	engine["is_in_game"] = ns_engine::is_in_game;
	engine["is_paused"] = ns_engine::is_paused;
	engine["set_view_angles"] = ns_engine::set_view_angles;

	auto utils = this->lua.create_table();
	utils["get_player_data"] = ns_utils::get_player_data;
	utils["clamp"] = ns_utils::clamp;
	utils["find_signature"] = ns_utils::find_signature;

	auto globals = this->lua.create_table();
	//globals["get_username"] = ns_globals::get_username;
	globals["realtime"] = ns_globals::realtime;
	globals["framecount"] = ns_globals::framecount;
	globals["absoluteframetime"] = ns_globals::absoluteframetime;
	globals["curtime"] = ns_globals::curtime;
	globals["frametime"] = ns_globals::frametime;
	globals["maxclients"] = ns_globals::maxclients;
	globals["tickcount"] = ns_globals::tickcount;
	globals["tickinterval"] = ns_globals::tickinterval;

	auto trace = this->lua.create_table();
	trace["get_point_contents"] = ns_trace::get_point_contents;
	//trace["trace_ray"] = ns_trace::trace_ray;

	auto cvar = this->lua.create_table();
	cvar["console_color_print"] = sol::overload(ns_cvar::console_color_print, ns_cvar::console_color_print_rgba);
	cvar["console_print"] = ns_cvar::console_print;
	cvar["find_var"] = ns_cvar::find_var;
	cvar["get_float"] = ns_cvar::get_float;
	cvar["get_int"] = ns_cvar::get_int;
	cvar["set_float"] = ns_cvar::set_float;
	cvar["set_int"] = ns_cvar::set_int;
	cvar["get_string"] = ns_cvar::get_string;
	cvar["set_string"] = ns_cvar::set_string;
	cvar["unlock_cvar"] = ns_cvar::unlock_cvar;
	cvar["remove_callbacks"] = ns_cvar::remove_callbacks;

	auto overlay = this->lua.create_table();
	overlay["add_box_overlay"] = ns_overlay::add_box_overlay;
	overlay["add_capsule_overlay"] = ns_overlay::add_capsule_overlay;
	overlay["add_text_overlay"] = ns_overlay::add_text_overlay;

	auto ui = this->lua.create_table();
	ui["new_checkbox"] = ns_ui::new_checkbox;
	ui["new_colorpicker"] = ns_ui::new_colorpicker;
	ui["new_keybind"] = ns_ui::new_keybind;
	ui["new_multiselect"] = ns_ui::new_multiselect;
	ui["new_slider_float"] = ns_ui::new_slider_float;
	ui["new_slider_int"] = ns_ui::new_slider_int;
	ui["new_text"] = ns_ui::new_text;
	ui["new_button"] = ns_ui::new_button;
	ui["set_callback"] = ns_ui::set_callback;
	ui["set_items"] = ns_ui::set_items;
	ui["set_label"] = ns_ui::set_label;
	ui["set_visibility"] = ns_ui::set_visibility;
	//ui["is_bind_active"] = ns_ui::is_bind_active;

	auto render = this->lua.create_table();
	render["draw_text"] = ns_render::draw_text;
	render["draw_line"] = ns_render::draw_line;
	render["draw_rect"] = ns_render::draw_rect;
	render["draw_rect_filled"] = ns_render::draw_rect_filled;
	render["draw_rect_outlined"] = ns_render::draw_rect_outlined;
	render["draw_circle"] = ns_render::draw_circle;
	render["draw_gradient"] = ns_render::draw_gradient;
	render["world_to_screen"] = ns_render::world_to_screen;
	render["getscreensize"] = ns_render::getscreensize;

	//auto file = this->lua.create_table();
	//file["append"] = ns_file::append;
	//file["write"] = ns_file::write;
	//file["read"] = ns_file::read;

	//this->lua["config"] = config;
	this->lua["cheat"] = cheat;
	this->lua["surface"] = surface;
	this->lua["models"] = models;
	this->lua["engine"] = engine;
	this->lua["utils"] = utils;
	this->lua["globals"] = globals;
	this->lua["trace"] = trace;
	this->lua["cvar"] = cvar;
	this->lua["overlay"] = overlay;
	this->lua["ui"] = ui;
	this->lua["render"] = render;
	//this->lua["file"] = file;

	this->refresh_scripts();
}

void c_lua::load_script(int id) {
	if (id == -1)
		return;

	if (this->loaded.at(id))
		return;

	auto path = this->get_script_path(id);
	if (path == (""))
		return;

	this->lua.script_file(path, [](lua_State*, sol::protected_function_result result) {
		if (!result.valid()) {
			sol::error err = result;

			ILoggerEvent::Get()->PushEvent(err.what(), FloatColor(1.f, 0.f, 0.f), false, XorStr(""));
		}

		return result;
		});

	this->loaded.at(id) = true;
}

void c_lua::unload_script(int id) {
	if (id == -1)
		return;

	if (!this->loaded.at(id))
		return;

	std::map<std::string, std::map<std::string, std::vector<MenuItem_t>>> updated_items;
	for (auto i : this->menu_items) {
		for (auto k : i.second) {
			std::vector<MenuItem_t> updated_vec;

			for (auto m : k.second)
				if (m.script != id)
					updated_vec.push_back(m);

			updated_items[k.first][i.first] = updated_vec;
		}
	}
	this->menu_items = updated_items;

	g_lua_hook.unregisterHooks(id);
	this->loaded.at(id) = false;
}

void c_lua::reload_all_scripts() {
	for (auto s : this->scripts) {
		if (this->loaded.at(this->get_script_id(s))) {
			this->unload_script(this->get_script_id(s));
			this->load_script(this->get_script_id(s));
		}
	}
}

void c_lua::unload_all_scripts() {
	for (auto s : this->scripts)
		if (this->loaded.at(this->get_script_id(s)))
			this->unload_script(this->get_script_id(s));
}

void c_lua::refresh_scripts() {
	auto oldLoaded = this->loaded;
	auto oldScripts = this->scripts;

	this->loaded.clear();
	this->pathes.clear();
	this->scripts.clear();

	for (auto& entry : std::filesystem::directory_iterator("C:\\vader.tech\\lua")) {
		if (entry.path().extension() == (".lua") || entry.path().extension() == (".luac")) {
			auto path = entry.path();
			auto filename = path.filename().string();

			bool didPut = false;
			for (int i = 0; i < oldScripts.size(); i++) {
				if (filename == oldScripts.at(i)) {
					this->loaded.push_back(oldLoaded.at(i));
					didPut = true;
				}
			}

			if (!didPut)
				this->loaded.push_back(false);

			this->pathes.push_back(path);
			this->scripts.push_back(filename);
		}
	}
}

int c_lua::get_script_id(std::string name) {
	for (int i = 0; i < this->scripts.size(); i++) {
		if (this->scripts.at(i) == name)
			return i;
	}

	return -1;
}

int c_lua::get_script_id_by_path(std::string path) {
	for (int i = 0; i < this->pathes.size(); i++) {
		if (this->pathes.at(i).string() == path)
			return i;
	}

	return -1;
}

std::string c_lua::get_script_path(std::string name) {
	return this->get_script_path(this->get_script_id(name));
}

std::string c_lua::get_script_path(int id) {
	if (id == -1)
		return  "";

	return this->pathes.at(id).string();
}
