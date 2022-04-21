#pragma once
#include <filesystem>
#include <lua-wrapper/include/lua.hpp>
#include <sol/sol.hpp>
#include "lua_hooks.h"
#include "lua_gameevents.h"
#include "../../SDK/CLuaConfigs.h"

#pragma comment(lib, "../lua-wrapper/include/luajit.lib")
#pragma comment(lib, "../lua-wrapper/include/lua51.lib")

enum MENUITEMTYPE {
	MENUITEM_CHECKBOX = 0,
	MENUITEM_SLIDERINT,
	MENUITEM_SLIDERFLOAT,
	MENUITEM_KEYBIND,
	MENUITEM_TEXT,
	MENUITEM_MULTISELECT,
	MENUITEM_COLORPICKER,
	MENUITEM_BUTTON
};

struct MenuItem_t {
	MENUITEMTYPE type;
	int script = -1;
	std::string label = "";
	std::string key = "";

	bool is_visible = true;

	// defaults
	bool b_default = false;
	int i_default = 0;
	float f_default = 0.f;
	float c_default[4] = { 1.f, 1.f, 1.f, 1.f };
	std::map<int, bool> m_default = {};

	// keybinds
	bool allow_style_change = true;

	// singleselect & multiselect
	std::vector<const char*> items = {};

	// slider_int
	int i_min = 0;
	int i_max = 100;

	// slider_float
	float f_min = 0.f;
	float f_max = 1.f;

	// sliders
	std::string format = "%d";

	// callbacks
	sol::function callback;
};

class c_lua {
public:
	bool initialize();

	void refresh_scripts();

	void load_script(int id);
	void unload_script(int id);

	void reload_all_scripts();
	void unload_all_scripts();

	int get_script_id(std::string name);
	int get_script_id_by_path(std::string path);

	std::vector<bool> loaded;
	std::vector<std::string> scripts;

	std::map<std::string, std::map<std::string, std::vector<MenuItem_t>>> menu_items = {};

	sol::state lua;

private:
	std::string get_script_path(std::string name);
	std::string get_script_path(int id);

	std::vector<std::filesystem::path> pathes;
};

extern c_lua g_lua;