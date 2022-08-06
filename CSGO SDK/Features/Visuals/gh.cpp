#include "gh.h"

void IEngineTrace::TraceLine(const Vector& src, const Vector& dst, int mask, IHandleEntity* entity, int collision_group, CGameTrace* trace) {
    static auto trace_filter_simple = Memory::Scan(XorStr("client.dll"), XorStr("55 8B EC 83 E4 F0 83 EC 7C 56 52")) + 0x3D;

    std::uintptr_t filter[4] = { *reinterpret_cast<std::uintptr_t*>(trace_filter_simple), reinterpret_cast<std::uintptr_t>(entity), collision_group, 0 };

    TraceRay(Ray_t(src, dst), mask, reinterpret_cast<CTraceFilter*>(&filter), trace);
}

void IEngineTrace::TraceHull(const Vector& src, const Vector& dst, const Vector& mins, const Vector& maxs, int mask, IHandleEntity* entity, int collision_group, CGameTrace* trace) {
    static auto trace_filter_simple = Memory::Scan(XorStr("client.dll"), XorStr("55 8B EC 83 E4 F0 83 EC 7C 56 52")) + 0x3D;

    std::uintptr_t filter[4] = { *reinterpret_cast<std::uintptr_t*>(trace_filter_simple), reinterpret_cast<std::uintptr_t>(entity), collision_group, 0 };

    TraceRay(Ray_t(src, dst, mins, maxs), mask, reinterpret_cast<CTraceFilter*>(&filter), trace);
}

void rotate_point(Vector2D& point, Vector2D origin, bool clockwise, float angle) {
    Vector2D delta = point - origin;
    Vector2D rotated;

    if (clockwise) {
        rotated = Vector2D(delta.x * cosf(angle) - delta.y * sinf(angle), delta.x * sinf(angle) + delta.y * cosf(angle));
    }
    else {
        rotated = Vector2D(delta.x * sinf(angle) - delta.y * cosf(angle), delta.x * cosf(angle) + delta.y * sinf(angle));
    }

    point = rotated + origin;
}

float& C_CSPlayer::get_creation_time() {
    return *reinterpret_cast<float*>(0x29B0);
}

void c_grenade_prediction::on_create_move(CUserCmd* cmd) {
    m_data = {};

    C_CSPlayer* pLocalPlayer = C_CSPlayer::GetLocalPlayer();

    if (!Interfaces::m_pEngine->IsInGame() || !pLocalPlayer /*|| !g_menu.main.visuals.tracers.get()*/)
        return;

    const auto weapon = reinterpret_cast<C_WeaponCSBaseGun*>(Interfaces::m_pEntList->GetClientEntityFromHandle(pLocalPlayer->m_hActiveWeapon()));
    if (!weapon || !weapon->m_bPinPulled() && weapon->m_fThrowTime() == 0.f)
        return;

    const auto weapon_data = weapon->GetCSWeaponData();
    if (!weapon_data.IsValid() || weapon_data.Xor()->m_iWeaponType != 9)
        return;

    m_data.m_owner = pLocalPlayer;
    m_data.m_index = weapon->m_iItemDefinitionIndex();

    auto view_angles = cmd->viewangles;

    if (view_angles.x < -90.f) {
        view_angles.x += 360.f;
    }
    else if (view_angles.x > 90.f) {
        view_angles.x -= 360.f;
    }

    view_angles.x -= (90.f - std::fabsf(view_angles.x)) * 10.f / 90.f;

    auto direction = Vector();

    Math::AngleVectors(view_angles, direction);

    const auto throw_strength = std::clamp< float >(weapon->m_flThrowStrength(), 0.f, 1.f);
    const auto eye_pos = pLocalPlayer->GetShootPosition();
    const auto src = Vector(eye_pos.x, eye_pos.y, eye_pos.z + (throw_strength * 12.f - 12.f));

    auto trace = CGameTrace();

    Interfaces::m_pEngineTrace->TraceHull(src, src + direction * 22.f, { -2.f, -2.f, -2.f }, { 2.f, 2.f, 2.f }, MASK_SOLID | CONTENTS_CURRENT_90, pLocalPlayer, COLLISION_GROUP_NONE, &trace);

    m_data.predict(trace.endpos - direction * 6.f, direction * (std::clamp< float >(weapon_data.Xor()->m_flThrowVelocity * 0.9f, 15.f, 750.f) * (throw_strength * 0.7f + 0.3f)) + pLocalPlayer->m_vecVelocity() * 1.25f, Interfaces::m_pGlobalVars->curtime, 0);
}

void DrawBeamPaw(Vector src, Vector end, Color color)
{
    BeamInfo_t beamInfo;
    beamInfo.m_vecStart = src;
    beamInfo.m_vecEnd = end;
    beamInfo.m_pszModelName = "sprites/purplelaser1.vmt";//sprites/purplelaser1.vmt
    beamInfo.m_pszHaloName = "sprites/purplelaser1.vmt";//sprites/purplelaser1.vmt
    beamInfo.m_flHaloScale = 0;//0
    beamInfo.m_flWidth = 1;//11
    beamInfo.m_flEndWidth = 1;//11
    beamInfo.m_flFadeLength = 1.0f;
    beamInfo.m_flAmplitude = 2.3;
    beamInfo.m_flBrightness = 255.f;
    beamInfo.m_flSpeed = 0.2f;
    beamInfo.m_nStartFrame = 0.0;
    beamInfo.m_flFrameRate = 0.0;
    beamInfo.m_flRed = color.r();
    beamInfo.m_flGreen = color.g();
    beamInfo.m_flBlue = color.b();
    beamInfo.m_nSegments = 2;//40
    beamInfo.m_bRenderable = true;
    beamInfo.m_flLife = 0.03f;


    Beam_t* myBeam = Interfaces::m_pRenderBeams->CreateBeamPoints(beamInfo);
    if (myBeam)
        Interfaces::m_pRenderBeams->DrawBeam(myBeam);
}

const char* index_to_grenade_name_icon(int index)
{
    switch (index)
    {
    case WEAPON_SMOKE: return "m"; break;
    case WEAPON_HEGRENADE: return "l"; break;
    case WEAPON_MOLOTOV:return "n"; break;
    case WEAPON_FIREBOMB:return "p"; break;
    }
}

int      m_width, m_height;

bool c_grenade_prediction::data_t::draw() const
{
    if (!g_Vars.esp.Grenadewarning)
        return false;

    C_CSPlayer* pLocalPlayer = C_CSPlayer::GetLocalPlayer();

    if (!pLocalPlayer)
        return false;

    if (m_path.size() <= 1u || Interfaces::m_pGlobalVars->curtime >= m_expire_time)
        return false;

    Interfaces::m_pEngine->GetScreenSize(m_width, m_height);

    float dist = pLocalPlayer->m_vecOrigin().Distance(m_origin) / 12;

    if (dist > 200.f)
        return false;

    auto prev_screen = Vector2D();
    auto prev_on_screen = Render::Engine::WorldToScreen(std::get< Vector >(m_path.front()), prev_screen);

    for (auto i = 1u; i < m_path.size(); ++i) {
        auto cur_screen = Vector2D();
        const auto cur_on_screen = Render::Engine::WorldToScreen(std::get< Vector >(m_path.at(i)), cur_screen);

        if (prev_on_screen && cur_on_screen) {

            if (g_Vars.esp.Grenadetracer) {
                //DrawBeamPaw(std::get< Vector >(m_path.at(i - 1)), std::get< Vector >(m_path.at(i)), Color(255, 255, 255, 255)); // beamcolor
                Render::Engine::Line(prev_screen.x, prev_screen.y, cur_screen.x, cur_screen.y, { 255, 255, 255, 200 });
            }
        }

        prev_screen = cur_screen;
        prev_on_screen = cur_on_screen;
    }

    float percent = ((m_expire_time - Interfaces::m_pGlobalVars->curtime) / TICKS_TO_TIME(m_tick));
    int alpha_damage = 0;

    if (m_index == WEAPON_HEGRENADE && dist <= 23) {
        alpha_damage = 255 - 255 * (dist / 23);
    }

    if ((m_index == WEAPON_MOLOTOV || m_index == WEAPON_FIREBOMB) && dist <= 15) {
        alpha_damage = 255 - 255 * (dist / 15);
    }

    Ray_t ray;
    CTraceFilterWorldAndPropsOnly filter;
    CGameTrace trace;

    ray.Init(m_origin, m_origin - Vector(0.0f, 0.0f, 105.0f));
    Interfaces::m_pEngineTrace->TraceRay(ray, MASK_SOLID, &filter, &trace);

    if (trace.fraction < 1.0f && (m_index == WEAPON_MOLOTOV || m_index == WEAPON_FIREBOMB)) {
        auto vvt = trace.endpos + Vector(0.0f, 0.0f, 2.0f);

        Render::Engine::CircleFilled(prev_screen.x, prev_screen.y - 10, 20, 360, Color(26, 26, 30, 199));
        Render::Engine::CircleFilled(prev_screen.x, prev_screen.y - 10, 20, 360, Color(232, 39, 62, alpha_damage));
        Render::Engine::draw_arc(prev_screen.x, prev_screen.y - 10, 20, 0, 360 * percent, 2, Color(255, 255, 255, 225));
        Render::Engine::cs_huge.string(prev_screen.x - 8, prev_screen.y - 22, { 255,255,255,255 }, index_to_grenade_name_icon(m_index));

        Render::Engine::WorldCircle(vvt, percent * 90, Color(255, 0, 0, 255), Color(0, 0, 0, 0));
    }
    else {
        Render::Engine::CircleFilled(prev_screen.x, prev_screen.y - 10, 20, 360, Color(26, 26, 30, 199));
        Render::Engine::CircleFilled(prev_screen.x, prev_screen.y - 10, 20, 360, Color(232, 39, 62, alpha_damage));
        Render::Engine::draw_arc(prev_screen.x, prev_screen.y - 10, 20, 0, 360 * percent, 2, Color(255, 255, 255, 225));
        Render::Engine::cs_huge.string(prev_screen.x - 8, prev_screen.y - 22, { 255,255,255,255 }, index_to_grenade_name_icon(m_index));
    }

    auto is_on_screen = [](Vector origin, Vector2D& screen) -> bool
    {
        if (!Render::Engine::WorldToScreen(origin, screen))
            return false;

        return (screen.x > 0 && screen.x < m_width) && (m_height > screen.y && screen.y > 0);
    };

    Vector2D screenPos;
    Vector vEnemyOrigin = m_origin;
    Vector vLocalOrigin = pLocalPlayer->GetAbsOrigin();
    if (!pLocalPlayer->IsAlive())
        vLocalOrigin = Interfaces::m_pInput->m_vecCameraOffset;

    //if (!is_on_screen(vEnemyOrigin, screenPos))
    //{
    //    const float wm = m_width / 2, hm = m_height / 2;
    //    Vector last_pos = std::get< Vector >(m_path.at(m_path.size() - 1));

    //    QAngle dir;

    //    Interfaces::m_pEngine->GetViewAngles(dir);

    //    float view_angle = dir.y;

    //    if (view_angle < 0)
    //        view_angle += 360;

    //    view_angle = DEG2RAD(view_angle);

    //    auto entity_angle = Math::CalcAngle(vLocalOrigin, vEnemyOrigin);
    //    entity_angle.Normalize();

    //    if (entity_angle.y < 0.f)
    //        entity_angle.y += 360.f;

    //    entity_angle.y = DEG2RAD(entity_angle.y);
    //    entity_angle.y -= view_angle;

    //    auto position = Vector2D(wm, hm);
    //    position.x -= std::clamp(vLocalOrigin.Distance(vEnemyOrigin), 400.f, hm - 40);

    //    rotate_point(position, Vector2D(wm, hm), false, entity_angle.y);

    //    if (dist < 45) {
    //        Render::Engine::CircleFilled(position.x, position.y - 10, 20, 360, Color(26, 26, 30, 200));
    //        Render::Engine::draw_arc(position.x, position.y - 10, 20, 0, 360 * percent, 2, Color(255, 255, 255, 225));
    //        Render::Engine::cs_huge.string(position.x - 8, position.y - 22, { 255,255,255,255 }, index_to_grenade_name_icon(m_index));
    //    }

    //}
    return true;
}

void c_grenade_prediction::grenade_warning(C_CSPlayer* entity)
{
    auto& predicted_nades = g_grenades_pred.get_list();

    static auto last_server_tick = Interfaces::m_pEngine->GetServerTick();
    if (last_server_tick != Interfaces::m_pEngine->GetServerTick()) {
        predicted_nades.clear();

        last_server_tick = Interfaces::m_pEngine->GetServerTick();
    }

    C_CSPlayer* pLocalPlayer = C_CSPlayer::GetLocalPlayer();

    if (!pLocalPlayer)
        return;

    if (entity->IsDormant() || !g_Vars.esp.Grenadewarning)
        return;

    C_CSPlayer* player = (C_CSPlayer*)entity->m_hOwnerEntity().Get();

    if (!player)
        return;

    if (player->m_iTeamNum() == pLocalPlayer->m_iTeamNum() && player->EntIndex() != pLocalPlayer->EntIndex() && g_Vars.mp_friendlyfire && g_Vars.mp_friendlyfire->GetInt() == 0)
        return;

    const auto client_class = entity->GetClientClass();
    if (!client_class || client_class->m_ClassID != CMolotovProjectile && client_class->m_ClassID != 9)
        return;

    if (client_class->m_ClassID == 9) {
        const auto model = entity->GetModel();
        if (!model)
            return;

        const auto studio_model = Interfaces::m_pModelInfo->GetStudiomodel(model);
        if (!studio_model || std::string_view(studio_model->szName).find(XorStr("fraggrenade")) == std::string::npos)
            return;
    }

    const auto handle = entity->GetRefEHandle().ToLong();

    if (entity->m_nExplodeEffectTickBegin()) {
        predicted_nades.erase(handle);
        return;
    }

    if (predicted_nades.find(handle) == predicted_nades.end()) {
        predicted_nades.emplace(std::piecewise_construct, std::forward_as_tuple(handle), std::forward_as_tuple(reinterpret_cast<C_WeaponCSBaseGun*>(entity)->m_hThrower(), client_class->m_ClassID == CMolotovProjectile ? WEAPON_MOLOTOV : WEAPON_HEGRENADE, entity->m_vecOrigin(), reinterpret_cast<C_CSPlayer*>(entity)->m_vecVelocity(), entity->get_creation_time(), TIME_TO_TICKS(reinterpret_cast<C_CSPlayer*>(entity)->m_flSimulationTime() - entity->get_creation_time())));
    }

    if (predicted_nades.at(handle).draw())
        return;

    predicted_nades.erase(handle);
}