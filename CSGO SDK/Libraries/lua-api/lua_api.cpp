#include "lua_api.h"
#include "../../source.hpp"
#include "../../Features/Visuals/EventLogger.hpp"
#include "../../Utils/LogSystem.hpp"
#include "../../Features/Miscellaneous/Miscellaneous.hpp"
#include "../../SDK/Displacement.hpp"
#include "../../SDK/Classes/entity.hpp"
#include "../../SDK/Classes/Player.hpp"
#define engine_console(x) ILoggerEvent::Get()->PushEvent(x, FloatColor(1.f, 1.f, 1.f), true, "")
void lua_panic(sol::optional<std::string> message) {

	if (message) {
		std::string m = message.value();
		MessageBoxA(0, m.c_str(), ("Lua: panic state"), MB_APPLMODAL | MB_OK);
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

namespace lua_events {

	void gameevent_callback(sol::this_state s, std::string eventname, sol::function func) {
		sol::state_view lua_state(s);
		sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
		std::string source = rs["source"];
		std::string filename = std::filesystem::path(source.substr(1)).filename().string();

		//if (eventname != "player_death" || "player_hurt") {

		//	engine_console(eventname + ": unknown / unregistered gameevent, if you wish to have this event added dm exon or calli. " + filename);
		//	g_lua.unload_script(g_lua.get_script_id(filename));
		//}
		//else {

		g_luagameeventmanager.register_gameevent(eventname, g_lua.get_script_id(filename), func);
		engine_console(filename + ": subscribed to gameevent " + eventname);

		//}
		

	}

}

namespace lua_ui {
	float keybinds_pos_x() {
		return g_Vars.esp.spec_window_x;
	}

	float keybinds_pos_y() {
		return g_Vars.esp.spec_window_y;
	}

}

namespace lua_config {
	/*
	config.get(key)
	Returns value of given key or nil if key not found.
	*/
	std::tuple<sol::object, sol::object, sol::object, sol::object> get(sol::this_state s, std::string key) {
		std::tuple<sol::object, sol::object, sol::object, sol::object> retn = std::make_tuple(sol::nil, sol::nil, sol::nil, sol::nil);

		for (auto kv : LuaConfigSystem::C_BOOL)
			if (kv.first == key)
				retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

		for (auto kv : LuaConfigSystem::C_COLOR)
			if (kv.first == key)
				retn = std::make_tuple(sol::make_object(s, (int)(kv.second[0] * 255)), sol::make_object(s, (int)(kv.second[1] * 255)), sol::make_object(s, (int)(kv.second[2] * 255)), sol::make_object(s, (int)(kv.second[3] * 255)));

		for (auto kv : LuaConfigSystem::C_FLOAT)
			if (kv.first == key)
				retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

		for (auto kv : LuaConfigSystem::C_INT)
			if (kv.first == key)
				retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

		for (auto kv : LuaConfigSystem::C_MULTI)
			if (kv.first == key)
				retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

		return retn;
	}

	/*
	config.set(key, value)
	Sets value for key
	*/
	void set_bool(std::string key, bool v) {
		LuaConfigSystem::C_BOOL[key] = v;
	}

	void set_float(std::string key, float v) {
		if (ceilf(v) != v)
			LuaConfigSystem::C_FLOAT[key] = v;
		else
			LuaConfigSystem::C_INT[key] = (int)v;
	}

	void set_color(std::string key, int r, int g, int b, int a) {
		LuaConfigSystem::C_COLOR[key][0] = r / 255.f;
		LuaConfigSystem::C_COLOR[key][1] = g / 255.f;
		LuaConfigSystem::C_COLOR[key][2] = b / 255.f;
		LuaConfigSystem::C_COLOR[key][3] = a / 255.f;
	}

	void set_multiselect(std::string key, int pos, bool e) {
		LuaConfigSystem::C_MULTI[key][pos] = e;
	}

	void set_int(std::string key, int value) {
		LuaConfigSystem::C_INT[key] = value;
	}

	bool watermark_enabled() {
		return g_Vars.misc.watermark;
	}

	bool autopeek_enabled() {
		return g_Vars.misc.autopeek_bind.enabled;
	}

	bool pingspike_enabled() {
		return g_Vars.misc.extended_backtrack_key.enabled;
	}

	float pingspike_value() {
		return g_Vars.misc.extended_backtrack_time;
	}

	bool move_exploit_enabled() {
		return g_Vars.misc.move_exploit_key.enabled;
	}

	bool fakeduck_enabled() {
		return g_Vars.misc.fakeduck_bind.enabled;
	}

	bool doubletap_enabled() {
		return g_Vars.rage.key_dt.enabled;
	}

	bool antiaim_manual_left_enabled() {
		return g_Vars.antiaim.manual_left_bind.enabled;
	}

	bool antiaim_manual_right_enabled() {
		return g_Vars.antiaim.manual_right_bind.enabled;
	}

	bool antiaim_manual_back_enabled() {
		return g_Vars.antiaim.manual_back_bind.enabled;
	}

	int antiaim_base_yaw() {
		return g_Vars.antiaim_stand.base_yaw;
	}

	void antiaim_base_yaw_set(int value) {
		g_Vars.antiaim_stand.base_yaw = value;
	}

	int antiaim_jitter_value() {
		return g_Vars.antiaim.Jitter_range;
	}

	void antiaim_jitter_set(int value) {
		g_Vars.antiaim.Jitter_range = value;
	}

	int antiaim_fakewalk_enabled() {
		return g_Vars.misc.slow_walk_bind.enabled;
	}

	bool forcebaim_enabled() {
		return g_Vars.rage.prefer_body.enabled;
	}

	bool slide_walk_set(bool value) {
		return g_Vars.misc.slide_walk = value;
	}
}

namespace lua_cheat {
	void set_event_callback(sol::this_state s, std::string eventname, sol::function func) {
		sol::state_view lua_state(s);
		sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
		std::string source = rs["source"];
		std::string filename = std::filesystem::path(source.substr(1)).filename().string();

		g_luahookmanager.register_hook(eventname, g_lua.get_script_id(filename), func);
		engine_console(filename + ": subscribed to event " + eventname);
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

	void log(const std::string& text) {
		engine_console(text);
	}

	void set_button(int button, int mode)
	{
		if (mode <= 0) g_Vars.globals.m_pCmd->buttons |= (1 << button);
		if (mode >= 1) g_Vars.globals.m_pCmd->buttons &= ~(1 << button);
	}

	bool get_button(int button)
	{
		if (g_Vars.globals.m_pCmd->buttons & (1 << button)) return true;

		return false;
	}

	std::string username()
	{
		return g_Vars.globals.user_info.username;
	}

}

namespace lua_math {
	QAngle calc_angle(Vector& src, Vector& dst)
	{
		return Math::CalcAngle(src, dst);
	}

	float get_fov(Vector& src, Vector& dst)
	{
		return Math::VecGetFov(src, dst);
	}
}

namespace lua_modelinfo {
	studiohdr_t* get_studio_model(model_t* mdl) {
		return Interfaces::m_pModelInfo->GetStudiomodel(mdl);
	}

	int get_model_index(std::string name) {
		return  Interfaces::m_pModelInfo->GetModelIndex(name.c_str());
	}
}
namespace lua_debugoverlay {
	void add_box_overlay(Vector pos, Vector mins, Vector maxs, QAngle orientation, int r, int g, int b, int a, float duration) {
		Interfaces::m_pDebugOverlay->AddBoxOverlay(pos, mins, maxs, orientation, r, g, b, a, duration);
	}

	void add_line_overlay(Vector src, Vector dst, Color clr, bool nodepthtest, float duration) {
		Interfaces::m_pDebugOverlay->AddLineOverlayAlpha(src, dst, clr.r(), clr.g(), clr.b(), clr.a(), nodepthtest, duration);
	}

	void add_text_overlay(Vector pos, int line_offset, float duration, sol::variadic_args txt) {
		std::string text = "";
		for (auto v : txt)
			text += v;

		Interfaces::m_pDebugOverlay->AddTextOverlay(pos, duration, text.c_str());
	}
}
namespace lua_engine {
	float get_screen_width() {
		return Render::GetScreenSize().x;
	}

	float get_screen_height() {
		return Render::GetScreenSize().y;
	}

	int get_player_for_user_id(int userid) {
		return Interfaces::m_pEngine->GetPlayerForUserID(userid);
	}

	int get_local_player() {
		return Interfaces::m_pEngine->GetLocalPlayer();
	}

	QAngle get_view_angles() {
		QAngle vaa;
		Interfaces::m_pEngine->GetViewAngles(vaa);
		return vaa;
	}

	void set_view_angles(QAngle va) {
		Interfaces::m_pEngine->SetViewAngles(va);
	}

	int get_max_clients() {
		return Interfaces::m_pEngine->GetMaxClients();
	}

	bool is_in_game() {
		return Interfaces::m_pEngine->IsInGame();
	}

	bool is_connected() {
		return Interfaces::m_pEngine->IsConnected();
	}


	INetChannel* get_net_channel_info() {
		return (INetChannel* )Interfaces::m_pEngine->GetNetChannelInfo();
	}

	void execute_client_cmd(std::string cmd) {
		Interfaces::m_pEngine->ClientCmd_Unrestricted(cmd.c_str());
	}
}
namespace lua_entitylist {
	C_CSPlayer* get_client_entity(int idx) {
		return (C_CSPlayer*)Interfaces::m_pEntList->GetClientEntity(idx);
	}

	int get_highest_entity_index() {
		return Interfaces::m_pEntList->GetHighestEntityIndex();
	}
}

namespace lua_utils {
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

	void apply_clan_tag_func(const char* name) {
		static auto apply_clan_tag = (int(__fastcall*)(const char*, const char*)) Engine::Displacement.Function.m_uClanTagChange;
		apply_clan_tag(name, name);
	}

	void apply_clan_tag(std::string clan) {
		apply_clan_tag_func(clan.c_str());
	}

	Color create_color(int r, int g, int b, int a) {
		return Color(r, g, b, a);
	}

	FloatColor create_floatcolor(float r, float g, float b, float a) {
		return FloatColor(r, g, b, a);
	}

	Color create_color_rainbow(int alpha) {
		static float rainbow;
		rainbow += 0.001f;
		if (rainbow > 1.f)
			rainbow = 0.f;



		return Color::HSBtoRGB(rainbow, 1.0f, 1.0f, alpha);
	}

	float lowerbody_yaw() {

		auto local = C_CSPlayer::GetLocalPlayer();

		if (local->m_vecVelocity().Length2D() > 30.f && !g_Vars.misc.slow_walk_bind.enabled) // bad fix for localplayer walking
			return 0.f;

		return std::abs(Math::AngleNormalize(g_Vars.globals.m_flBody - g_Vars.globals.RegularAngles.y));
	}

}

namespace lua_globals {
	float realtime() {
		return Interfaces::m_pGlobalVars->realtime;
	}

	int framecount() {
		return Interfaces::m_pGlobalVars->framecount;
	}

	float absoluteframetime() {
		return Interfaces::m_pGlobalVars->frametime;
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

namespace lua_cvar {

	ConVar* find_var(std::string name) {
		return Interfaces::m_pCvar->FindVar(name.c_str());
	}

	void console_color_print(Color col, sol::variadic_args args) {
		std::string txt = "";
		for (auto v : args)
			txt += v;

		Interfaces::m_pCvar->ConsoleColorPrintf(col, txt.c_str());
	}

	void console_print(sol::variadic_args args) {
		std::string txt = "";
		for (auto v : args)
			txt += v;

		Interfaces::m_pCvar->ConsoleColorPrintf(Color(255, 255, 255, 255), txt.c_str());
	}

	void unlock_cvar(ConVar* var)
	{
		if (var)
		{
			var->nFlags &= ~FCVAR_DEVELOPMENTONLY;
			var->nFlags &= ~FCVAR_HIDDEN;
		}
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

	void set_float(ConVar* var, float f) {
		if (var) var->SetValue(std::to_string(f).c_str());
	}

	void set_int(ConVar* var, int i) {
		if (var) var->SetValue(std::to_string(i).c_str());
	}

	void set_string(ConVar* var, const char* v) {
		if (var)
			var->SetValue(v);
	}
}

namespace lua_surface {
	void set_texture(int id) {
		Interfaces::m_pSurface->DrawSetTexture(id);
	}

	void set_texture_rgba(int id, const unsigned char* rgba, int w, int h) {
		Interfaces::m_pSurface->DrawSetTextureRGBA(id, rgba, w, h);
	}

	void set_color(Color col) {
		Interfaces::m_pSurface->DrawSetColor(col.r(), col.g(), col.b(), col.a());
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

	Vector2D get_text_size(int font, std::wstring text) {
		int w, h;
		Interfaces::m_pSurface->GetTextSize(font, text.c_str(), w, h);
		return Vector2D(w, h);
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

	void set_text_color(Color col) {
		Interfaces::m_pSurface->DrawSetTextColor(col.r(), col.g(), col.b(), col.a());
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
}

namespace lua_render {

	void draw_text(int x, int y, unsigned long font, std::string string, bool text_centered, Color color)
	{
		Render::Engine::text((int)x, (int)y, font, string, text_centered, color);
	}

	Vector2D text_size(unsigned long font, std::string text) {
		return Render::Engine::GetTextSize(font, text);
	}

	void draw_line(float x, float y, float x2, float y2, Color col)
	{
		Render::Engine::Line((int)x, (int)y, (int)x2, (int)y2, col);
	}

	void draw_rect(float x, float y, float w, float h, Color col)
	{
		Render::Engine::Rect((int)x, (int)y, (int)w, (int)h, col);
	}

	void draw_rect_filled(float x, float y, float w, float h, Color col)
	{
		Render::Engine::RectFilled((int)x, (int)y, (int)w, (int)h, col);
	}

	void draw_rect_outlined(int x, int y, int w, int h, Color color, Color color2)
	{
		Render::Engine::RectOutlined((int)x, (int)y, (int)w, (int)h, color, color2);
	}

	void draw_circle(int x, int y, int radius, int segments, Color color)
	{
		Render::Engine::CircleFilled((int)x, (int)y, (int)radius, (int)segments, color);
	}

	Vector world_to_screen(Vector pos) {
		Vector2D scr;
		Render::Engine::WorldToScreen(pos, scr);
		return Vector(scr.x, scr.y, 0);
	}

}

// ----- lua functions -----

c_lua g_lua;
bool c_lua::initialize() {
	this->lua = sol::state(sol::c_call<decltype(&lua_panic), &lua_panic>);
	this->lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::debug, sol::lib::package, sol::lib::coroutine);

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

	this->lua["print"] = [](std::string s) { engine_console(s); };
	this->lua["error"] = [](std::string s) { engine_console(s); };

	this->lua.new_usertype<Color>("color", sol::constructors <Color(), Color(int, int, int), Color(int, int, int, int)>(),
		(std::string)XorStr("r"), &Color::r,
		(std::string)XorStr("g"), &Color::g,
		(std::string)XorStr("b"), &Color::b,
		(std::string)XorStr("a"), &Color::a
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
	this->lua.new_enum("hitboxes",
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
	this->lua.new_enum("fontflags",
		"italic", FontFlags_t::FONTFLAG_ITALIC,
		"underline", FontFlags_t::FONTFLAG_UNDERLINE,
		"strikeout", FontFlags_t::FONTFLAG_STRIKEOUT,
		"symbol", FontFlags_t::FONTFLAG_SYMBOL,
		"antialias", FontFlags_t::FONTFLAG_ANTIALIAS,
		"gaussianblur", FontFlags_t::FONTFLAG_GAUSSIANBLUR,
		"rotary", FontFlags_t::FONTFLAG_ROTARY,
		"dropshadow", FontFlags_t::FONTFLAG_DROPSHADOW,
		"additive", FontFlags_t::FONTFLAG_ADDITIVE,
		"outline", FontFlags_t::FONTFLAG_OUTLINE
	);
	this->lua.new_usertype<ConVar>("c_convar");
	this->lua.new_usertype<INetChannel>("INetChannelnelinfo",
		"out_sequence_nr", sol::readonly(&INetChannel::m_nOutSequenceNr),
		"in_sequence_nr", sol::readonly(&INetChannel::m_nInSequenceNr),
		"out_sequence_nr_ack", sol::readonly(&INetChannel::m_nOutSequenceNrAck),
		"out_reliable_state", sol::readonly(&INetChannel::m_nOutReliableState),
		"in_reliable_state", sol::readonly(&INetChannel::m_nInReliableState),
		"choked_packets", sol::readonly(&INetChannel::m_nChokedPackets),
		"get_latency", &INetChannel::GetLatency,
		"get_avg_latency", &INetChannel::GetAvgLatency
		);
	this->lua.new_usertype<Vector2D>("vector2d",
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
	this->lua.new_usertype<Vector>("vector",
		sol::constructors<Vector(), Vector(float, float, float)>(),
		"x", &Vector::x,
		"y", &Vector::y,
		"z", &Vector::z,
		"length", &Vector::Length,
		"length_sqr", &Vector::LengthSquared,
		"length2d", &Vector::Length2D,
		"length2dsqr", &Vector::Length2DSquared,
		"dist_to", &Vector::Distance,
		"cross_product", &Vector::Cross,
		"normalize", &Vector::Normalize
		);
	this->lua.new_usertype<QAngle>("qangle",
		sol::constructors<QAngle(), QAngle(float, float, float)>(),
		"x", &QAngle::x,
		"y", &QAngle::y,
		"z", &QAngle::z,
		"normalize", &QAngle::Normalize
		);
	this->lua.new_usertype<C_CSPlayer>("c_baseentity",
		"is_player", &C_CSPlayer::IsPlayer,
		"is_alive", &C_CSPlayer::IsAlive,
		"is_dormant", &C_CSPlayer::IsDormant,
		"get_abs_origin", &C_CSPlayer::GetAbsOrigin,
		"get_eye_position", &C_CSPlayer::GetEyePosition,
		"get_shoot_position", &C_CSPlayer::GetShootPosition,
		"get_hitbox_position", &C_CSPlayer::GetHitboxPosition,
		"get_index", &C_CSPlayer::EntIndex,
		"get_team", &C_CSPlayer::m_iTeamNum,
		"set_clientside_angle", &C_CSPlayer::ForceAngleTo
		);

	auto events = this->lua.create_table();
	events["gameevent_callback"] = lua_events::gameevent_callback;

	auto config = this->lua.create_table();
	config["get"] = lua_config::get;
	config["set"] = sol::overload(lua_config::set_bool, lua_config::set_color, lua_config::set_float, lua_config::set_multiselect, lua_config::set_int);
	config["watermark_enabled"] = lua_config::watermark_enabled;
	config["autopeek_enabled"] = lua_config::autopeek_enabled;
	config["pingspike_enabled"] = lua_config::pingspike_enabled;
	config["pingspike_value"] = lua_config::pingspike_value;
	config["move_exploit_enabled"] = lua_config::move_exploit_enabled;
	config["fakeduck_enabled"] = lua_config::fakeduck_enabled;
	config["doubletap_enabled"] = lua_config::doubletap_enabled;
	config["antiaim_manual_left_enabled"] = lua_config::antiaim_manual_left_enabled;
	config["antiaim_manual_right_enabled"] = lua_config::antiaim_manual_right_enabled;
	config["antiaim_manual_back_enabled"] = lua_config::antiaim_manual_back_enabled;
	config["antiaim_base_yaw"] = lua_config::antiaim_base_yaw;
	config["antiaim_base_yaw_set"] = lua_config::antiaim_base_yaw_set;
	config["antiaim_jitter_value"] = lua_config::antiaim_jitter_value;
	config["antiaim_jitter_set"] = lua_config::antiaim_jitter_set;
	config["antiaim_fakewalk_enabled"] = lua_config::antiaim_fakewalk_enabled;
	config["forcebaim_enabled"] = lua_config::forcebaim_enabled;
	config["slide_walk_set"] = lua_config::slide_walk_set;


	auto cheat = this->lua.create_table();
	cheat["set_event_callback"] = lua_cheat::set_event_callback;
	cheat["run_script"] = lua_cheat::run_script;
	cheat["reload_active_scripts"] = lua_cheat::reload_active_scripts;
	cheat["get_button"] = lua_cheat::get_button;
	cheat["set_button"] = lua_cheat::set_button;
	cheat["username"] = lua_cheat::username;

	auto math = this->lua.create_table();
	math["calc_angle"] = lua_math::calc_angle;
	math["get_fov"] = lua_math::get_fov;

	auto surface = this->lua.create_table();
	surface["create_font"] = lua_surface::create_font;
	surface["draw_line"] = lua_surface::draw_line;
	surface["draw_circle"] = lua_surface::draw_outlined_circle;
	surface["draw_rect"] = lua_surface::draw_outlined_rect;
	surface["draw_filled_rect"] = lua_surface::draw_filled_rect;
	surface["draw_text"] = lua_surface::draw_text;
	surface["get_text_size"] = lua_surface::get_text_size;
	surface["set_color"] = lua_surface::set_color;
	surface["set_text_color"] = lua_surface::set_text_color;
	surface["set_text_font"] = lua_surface::set_text_font;
	surface["set_text_pos"] = lua_surface::set_text_pos;

	auto debugoverlay = this->lua.create_table();
	debugoverlay["add_box_overlay"] = lua_debugoverlay::add_box_overlay;
	debugoverlay["add_line_overlay"] = lua_debugoverlay::add_line_overlay;
	debugoverlay["add_text_overlay"] = lua_debugoverlay::add_text_overlay;

	auto engine = this->lua.create_table();
	engine["execute_client_cmd"] = lua_engine::execute_client_cmd;
	engine["get_player_for_user_id"] = lua_engine::get_player_for_user_id;
	engine["get_local_player_index"] = lua_engine::get_local_player;
	engine["get_max_clients"] = lua_engine::get_max_clients;
	engine["get_net_channel_info"] = lua_engine::get_net_channel_info;
	engine["get_screen_width"] = lua_engine::get_screen_width;
	engine["get_screen_height"] = lua_engine::get_screen_height;
	engine["get_view_angles"] = lua_engine::get_view_angles;
	engine["set_view_angles"] = lua_engine::set_view_angles;
	engine["is_connected"] = lua_engine::is_connected;
	engine["is_in_game"] = lua_engine::is_in_game;

	auto entity_list = this->lua.create_table();
	entity_list["get_client_entity"] = lua_entitylist::get_client_entity;
	entity_list["get_highest_entity_index"] = lua_entitylist::get_highest_entity_index;

	auto utils = this->lua.create_table();
	utils["get_player_data"] = lua_utils::get_player_data;
	utils["clamp"] = lua_utils::clamp;
	utils["find_signature"] = lua_utils::find_signature;
	utils["apply_clan_tag"] = lua_utils::apply_clan_tag;
	utils["color"] = lua_utils::create_color;
	utils["floatcolor"] = lua_utils::create_floatcolor;
	utils["color_rainbow"] = lua_utils::create_color_rainbow; // i couldn't make rainbow work in lua idk why so i just put this in - remove if you dont want to make lua api look bad ( lol )
	utils["lowerbody_yaw"] = lua_utils::lowerbody_yaw;

	auto globals = this->lua.create_table();
	globals["realtime"] = lua_globals::realtime;
	globals["framecount"] = lua_globals::framecount;
	globals["absoluteframetime"] = lua_globals::absoluteframetime;
	globals["curtime"] = lua_globals::curtime;
	globals["frametime"] = lua_globals::frametime;
	globals["maxclients"] = lua_globals::maxclients;
	globals["tickcount"] = lua_globals::tickcount;
	globals["tickinterval"] = lua_globals::tickinterval;

	auto cvar = this->lua.create_table();
	cvar["console_print"] = lua_cvar::console_print;
	cvar["find_var"] = lua_cvar::find_var;
	cvar["get_float"] = lua_cvar::get_float;
	cvar["get_int"] = lua_cvar::get_int;
	cvar["set_float"] = lua_cvar::set_float;
	cvar["set_int"] = lua_cvar::set_int;
	cvar["get_string"] = lua_cvar::get_string;
	cvar["set_string"] = lua_cvar::set_string;
	cvar["unlock_cvar"] = lua_cvar::unlock_cvar;
	
	auto render = this->lua.create_table();
	render["draw_text"] = lua_render::draw_text;
	render["text_size"] = lua_render::text_size;
	render["draw_line"] = lua_render::draw_line;
	render["draw_rect"] = lua_render::draw_rect;
	render["draw_rect_filled"] = lua_render::draw_rect_filled;
	render["draw_circle"] = lua_render::draw_circle;
	render["draw_rect_outlined"] = lua_render::draw_rect_outlined;
	render["world_to_screen"] = lua_render::world_to_screen;

	auto ui = this->lua.create_table();
	ui["keybinds_pos_x"] = lua_ui::keybinds_pos_x;
	ui["keybinds_pos_y"] = lua_ui::keybinds_pos_y;


	this->lua["event"] = events;
	this->lua["config"] = config;
	this->lua["cheat"] = cheat;
	this->lua["math2"] = math;
	this->lua["surface"] = surface;
	this->lua["debugoverlay"] = debugoverlay;
	this->lua["engine"] = engine;
	this->lua["entity_list"] = entity_list;
	this->lua["utils"] = utils;
	this->lua["globals"] = globals;
	this->lua["cvar"] = cvar;
	this->lua["render"] = render;
	this->lua["ui"] = ui;

	this->refresh_scripts();
	//this->load_script(this->get_script_id("autorun.lua"));

	return true;
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
			engine_console(err.what());
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

	g_luahookmanager.unregister_hooks(id);
	g_luagameeventmanager.unregister_gameevents(id);
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

	std::filesystem::path full_path(std::filesystem::current_path());
	std::wstring str = full_path.wstring() + XorStr(L"\\vader.tech\\scripts");

	CreateDirectoryW(str.c_str(), nullptr);

	for (auto& entry : std::filesystem::directory_iterator(str)) {
		if (entry.path().extension() == (".lua") || entry.path().extension() == (".luac")) {
			auto path = entry.path();
			auto filename = path.filename().string();

			bool didPut = false;
			for (size_t i = 0; i < oldScripts.size(); i++) {
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
	for (size_t i = 0; i < this->scripts.size(); i++) {
		if (this->scripts.at(i) == name)
			return i;
	}

	return -1;
}

int c_lua::get_script_id_by_path(std::string path) {
	for (size_t i = 0; i < this->pathes.size(); i++) {
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
