#include "Miscellaneous.hpp"
#include "../../SDK/displacement.hpp"

#include "../../Source.hpp"
#include "../../SDK/Classes/player.hpp"
#include "../Game/Prediction.hpp"

namespace Interfaces
{
	inline float rgb_to_srgb( float flLinearValue ) {
		return flLinearValue;
		// float x = Math::Clamp( flLinearValue, 0.0f, 1.0f );
		// return ( x <= 0.0031308f ) ? ( x * 12.92f ) : ( 1.055f * powf( x, ( 1.0f / 2.4f ) ) ) - 0.055f;
	}

	class C_Miscellaneous : public Miscellaneous {
	public:
		static Miscellaneous* Get( );
		virtual void Main( );

		C_Miscellaneous( ) { };
		virtual ~C_Miscellaneous( ) {
		}

	private:
		const char* skynames[ 16 ] = {
			"Default",
			"cs_baggage_skybox_",
			"cs_tibet",
			"embassy",
			"italy",
			"jungle",
			"nukeblank",
			"office",
			"sky_csgo_cloudy01",
			"sky_csgo_night02",
			"sky_csgo_night02b",
			"sky_dust",
			"sky_venice",
			"vertigo",
			"vietnam",
			"sky_descent"
		};

		// wall modulation stuff
		FloatColor walls = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
		FloatColor props = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
		FloatColor skybox = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );

		// clantag
		int clantag_step = 0;

		virtual void ModulateWorld( );
		virtual void ClantagChanger( );
		virtual void ViewModelChanger( );
		virtual void SkyboxChanger( );
	};

	C_Miscellaneous g_Misc;
	Miscellaneous* C_Miscellaneous::Get( ) {
		return &g_Misc;
	}

	void C_Miscellaneous::Main( ) {
		ModulateWorld( );
		SkyboxChanger( );

		if( !g_Vars.globals.HackIsReady )
			return;

		ClantagChanger( );
		ViewModelChanger( );
	}

	void C_Miscellaneous::ModulateWorld( ) {
		if( !g_Vars.globals.HackIsReady ) {
			walls = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
			props = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
			skybox = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
			return;
		}

		if( !C_CSPlayer::GetLocalPlayer( ) )
			return;

		static auto w = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
		static auto p = FloatColor( 0.9f, 0.9f, 0.9f, 1.0f );
		static auto s = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );;

		if( g_Vars.esp.night_mode ) {
			float power = g_Vars.esp.world_adjustement_value / 100.f;
			float power_props = g_Vars.esp.prop_adjustement_value / 100.f;

			w = FloatColor( power, power, power, 1.f );
			p = FloatColor( power_props, power_props, power_props, std::clamp<float>( ( g_Vars.esp.transparent_props + 0.1f ) / 100.f, 0.f, 1.f ) );
		}
		else {
			w = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
			p = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
			//s = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
		}

		if( g_Vars.esp.skybox ) {
			s = g_Vars.esp.skybox_modulation;
		}
		else {
			s = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
		}

		if( walls != w || props != p || skybox != s ) {
			walls = w;
			props = p;
			skybox = s;

			auto invalid_material = Interfaces::m_pMatSystem->InvalidMaterial( );
			for( auto i = Interfaces::m_pMatSystem->FirstMaterial( );
				i != invalid_material;
				i = Interfaces::m_pMatSystem->NextMaterial( i ) ) {
				auto material = Interfaces::m_pMatSystem->GetMaterial( i );

				if( !material || material->IsErrorMaterial( ) )
					continue;

				FloatColor color = walls;
				auto group = material->GetTextureGroupName( );

				if( !material->GetName( ) )
					continue;

				if( *group == 'W' ) { // world textures
					if( group[ 4 ] != 'd' )
						continue;
					color = walls;
				}
				else if( *group == 'S' ) { // staticprops & skybox
					auto thirdCharacter = group[ 3 ];
					if( thirdCharacter == 'B' ) {
						color = skybox;
					}
					else if( thirdCharacter == 't' && group[ 6 ] == 'P' ) {
						color = props;
					}
					else {
						continue;
					}
				}
				else {
					continue;
				}

				color.r = rgb_to_srgb( color.r );
				color.g = rgb_to_srgb( color.g );
				color.b = rgb_to_srgb( color.b );

				material->AlphaModulate( color.a );
				material->ColorModulate( color.r, color.g, color.b );
			}
		}
	}

	void C_Miscellaneous::ClantagChanger() {
		static bool run_once = false;
		static auto fnClantagChanged = (int(__fastcall*)(const char*, const char*)) Engine::Displacement.Function.m_uClanTagChange;

		if (!g_Vars.misc.clantag_changer) {
			if (run_once) {
				fnClantagChanged(XorStr(""), XorStr(""));
				run_once = false;
			}

			return;
		}

		if (!Interfaces::m_pPrediction->GetUnpredictedGlobals()) {
			if (run_once) {
				fnClantagChanged(XorStr(""), XorStr(""));
				run_once = false;
			}

			return;
		}

		run_once = true;

		std::string szClanTag = XorStr("vader.tech");
		std::string szSuffix = XorStr("");
		static int iPrevFrame = 0;
		static bool bReset = false;
		int iCurFrame = (int(Interfaces::m_pGlobalVars->curtime * 2.9) % 23);

		if (iPrevFrame != int(Interfaces::m_pGlobalVars->curtime * 2.9) % 20) {
			switch (iCurFrame) {
			case 0: szClanTag = XorStr("v "); break;
			case 1: szClanTag = XorStr("va "); break;
			case 2: szClanTag = XorStr("vad "); break;
			case 3: szClanTag = XorStr("vade "); break;
			case 4: szClanTag = XorStr("vader "); break;
			case 5: szClanTag = XorStr("vader. "); break;
			case 6: szClanTag = XorStr("vader.t "); break;
			case 7: szClanTag = XorStr("vader.te "); break;
			case 8: szClanTag = XorStr("vader.tec "); break;
			case 9: szClanTag = XorStr("vader.tech "); break;
			case 10: szClanTag = XorStr("vader.tech "); break;
			case 11: szClanTag = XorStr("vader.tech "); break;
			case 12: szClanTag = XorStr("vader.tec "); break;
			case 13: szClanTag = XorStr("vader.te "); break;
			case 14: szClanTag = XorStr("vader.t "); break;
			case 15: szClanTag = XorStr("vader. "); break;
			case 16: szClanTag = XorStr("vader "); break;
			case 17: szClanTag = XorStr("vade "); break;
			case 18: szClanTag = XorStr("vad "); break;
			case 19: szClanTag = XorStr("va "); break;
			case 20: szClanTag = XorStr("v "); break;
			case 21: szClanTag = XorStr(" "); break;
			case 22: szClanTag = XorStr(" "); break;
			}

			// set our clantag
			fnClantagChanged(szClanTag.data(), szClanTag.data());

			// set current/last frame.
			iPrevFrame = iCurFrame;
		}
	}

	void C_Miscellaneous::ViewModelChanger( ) {
		g_Vars.viewmodel_fov->SetValue( g_Vars.misc.viewmodel_fov );
	}

	void C_Miscellaneous::SkyboxChanger( ) {
		static int iOldSky = 0;

		if( !g_Vars.globals.HackIsReady ) {
			iOldSky = 0;
			return;
		}

		static auto fnLoadNamedSkys = ( void( __fastcall* )( const char* ) )Engine::Displacement.Function.m_uLoadNamedSkys;
		static ConVar* default_skyname = Interfaces::m_pCvar->FindVar( XorStr( "sv_skyname" ) );
		if( default_skyname ) {
			if( iOldSky != g_Vars.esp.sky_changer ) {
				const char* sky_name = g_Vars.esp.sky_changer != 0 ? skynames[ g_Vars.esp.sky_changer ] : default_skyname->GetString( );
				fnLoadNamedSkys( sky_name );
				iOldSky = g_Vars.esp.sky_changer;
			}
		}
	}

	Miscellaneous* Miscellaneous::Get( ) {
		static C_Miscellaneous instance;
		return &instance;
	}
}
