#include "AnimationSystem.hpp"
#include "../../SDK/displacement.hpp"
#include "../../Utils/Math.h"
#include "../Game/SetupBones.hpp"
#include "../../Utils/Threading/threading.h"
#include "LagCompensation.hpp"
#include "Resolver.hpp"

#define MT_SETUP_BONES

void SimulateMovement(C_CSPlayer* player, Vector velocity, Engine::C_AnimationRecord* record, bool isInAir) {
	auto local = C_CSPlayer::GetLocalPlayer();
	auto weapon = Encrypted_t<C_WeaponCSBaseGun>((C_WeaponCSBaseGun*)local->m_hActiveWeapon().Get());
	if (!weapon.IsValid())
		return;

	auto weaponInfo = weapon->GetCSWeaponData();
	if (!weaponInfo.IsValid())
		return;

	Vector mins = player->OBBMins(), maxs = player->OBBMaxs();

	Vector                start, end, normal;
	CGameTrace            trace;
	CTraceFilterWorldOnly filter;

	// define trace start.
	start = record->m_vecOrigin;

	// move trace end one tick into the future using predicted velocity.
	end = start + (velocity * Interfaces::m_pGlobalVars->interval_per_tick);

	// trace.
	Interfaces::m_pEngineTrace->TraceRay(Ray_t(start, end, mins, maxs), CONTENTS_SOLID, &filter, &trace);

	// we hit shit
	// we need to fix hit.
	if (trace.fraction != 1.f) {
		// fix sliding on planes.
		for (int i{}; i < 2; ++i) {
			record->m_vecVelocity -= trace.plane.normal * velocity.Dot(trace.plane.normal);

			float adjust = record->m_vecVelocity.Dot(trace.plane.normal);
			if (adjust < 0.f)
				record->m_vecVelocity -= (trace.plane.normal * adjust);

			start = trace.endpos;
			end = start + (record->m_vecVelocity * (Interfaces::m_pGlobalVars->interval_per_tick * (1.f - trace.fraction)));

			Interfaces::m_pEngineTrace->TraceRay(Ray_t(start, end, mins, maxs), CONTENTS_SOLID, &filter, &trace);
			if (trace.fraction == 1.f)
				break;
		}
	}

	// set new final origin.
	start = end = record->m_vecOrigin = trace.endpos;

	// move endpos 2 units down.
	// this way we can check if we are in/on the ground.
	end.z -= 2.f;

	// trace.
	Interfaces::m_pEngineTrace->TraceRay(Ray_t(start, end, mins, maxs), CONTENTS_SOLID, &filter, &trace);

	// strip onground flag.
	record->m_fFlags &= ~FL_ONGROUND;

	// add back onground flag if we are onground.
	if (trace.fraction != 1.f && trace.plane.normal.z > 0.7f)
		record->m_fFlags |= FL_ONGROUND;
}

void AimwareExtrapolation(C_CSPlayer* player, Vector velocity, Engine::C_AnimationRecord* record, Engine::C_AnimationRecord* previousRecord) {
	auto pNetChannelInfo = Encrypted_t<INetChannelInfo>(Interfaces::m_pEngine->GetNetChannelInfo());
	if (!pNetChannelInfo.IsValid())
		return;

	float interval_per_tick; // ST20_4
	float simulationTime; // xmm4_4
	float deltaTime; // xmm2_4
	float simulationTimeDelta; // xmm4_4
	signed int simulationTicks; // edi
	float v15; // xmm1_4
	float velocityDegree; // xmm2_4
	float velocityAngle; // xmm3_4
	int v18; // ebp
	float v19; // xmm3_4 MAPDST
	signed int v20; // esi
	float v21; // xmm0_4
	float tickInterval; // [esp+14h] [ebp-34h]
	float incomingAvgLatency; // [esp+4Ch] [ebp+4h]
	float v29; // [esp+4Ch] [ebp+4h]
	float v30; // [esp+4Ch] [ebp+4h]
	float velocityLength2D; // [esp+4Ch] [ebp+4h]
	float outgoingAvgLatency; // [esp+50h] [ebp+8h]
	float extrapolatedMovement; // [esp+50h] [ebp+8h]

	Vector backupVelocity = velocity;
	Vector backupOrigin = record->m_vecOrigin;

	interval_per_tick = Interfaces::m_pGlobalVars->interval_per_tick;
	tickInterval = 1.0 / Interfaces::m_pGlobalVars->interval_per_tick;
	incomingAvgLatency = pNetChannelInfo->GetAvgLatency(1);
	outgoingAvgLatency = pNetChannelInfo->GetAvgLatency(0);
	simulationTime = record->m_flSimulationTime;

	deltaTime = (((((incomingAvgLatency + outgoingAvgLatency) * tickInterval) + 0.5) + Interfaces::m_pGlobalVars->tickcount + 1) * interval_per_tick) - simulationTime;
	if (deltaTime > 1.0)
		deltaTime = 1.0;

	simulationTimeDelta = simulationTime - previousRecord->m_flSimulationTime;
	simulationTicks = ((simulationTimeDelta * tickInterval) + 0.5);
	if (simulationTicks <= 15) {
		if (simulationTicks < 1)
			simulationTicks = 1;
	}
	else {
		simulationTicks = 15;
	}

	v29 = atan2(velocity.y, velocity.x);
	v15 = (tickInterval * deltaTime) + 0.5;
	velocityDegree = v29 * 57.295776;
	v30 = atan2(previousRecord->m_vecVelocity.y, previousRecord->m_vecVelocity.x);

	velocityAngle = velocityDegree - (v30 * 57.295776);
	if (velocityAngle <= 180.0) {
		if (velocityAngle < -180.0)
			velocityAngle = velocityAngle + 360.0;
	}
	else {
		velocityAngle = velocityAngle - 360.0;
	}

	v18 = v15 - simulationTicks;
	v19 = velocityAngle / simulationTimeDelta;
	velocityLength2D = sqrt((velocity.y * velocity.y) + (velocity.x * velocity.x));
	if (v18 < 0) {
		record->m_vecOrigin = backupOrigin;
		record->m_vecVelocity = backupVelocity;
	}
	else {
		do {
			if (simulationTicks > 0) {
				v20 = simulationTicks;
				do {
					extrapolatedMovement = velocityDegree + (Interfaces::m_pGlobalVars->interval_per_tick * v19);

					float extrapolatedSin, extrapolatedCos;
					Math::SinCos((extrapolatedMovement * 0.017453292), &extrapolatedSin, &extrapolatedCos);

					velocity.x = extrapolatedCos * velocityLength2D;
					v21 = extrapolatedSin;
					velocity.y = v21 * velocityLength2D;

					SimulateMovement(player, velocity, record, !(record->m_fFlags & FL_ONGROUND));

					velocityDegree = extrapolatedMovement;
					--v20;
				} while (v20);
			}
			v18 -= simulationTicks;
		} while (v18 >= 0);
	}
}

void LinearExtrapolations(Engine::C_AnimationRecord* record)
{
	auto m_local = C_CSPlayer::GetLocalPlayer();

	if (m_local && m_local->IsAlive()) {

		for (int i = 1; i < Interfaces::m_pGlobalVars->maxClients; i++)
		{

			auto m_entity = (C_CSPlayer*)Interfaces::m_pEntList->GetClientEntity(i);

			if (m_entity && !m_entity->IsDormant() && !m_entity->IsTeammate(m_local)) {

				int choked_ticks = record->m_iChokeTicks;
				if (record->m_bTeleportDistance)
				{
					Vector velocity_per_tick = record->m_vecVelocity * Interfaces::m_pGlobalVars->interval_per_tick;

					auto new_origin = m_entity->m_vecOrigin() + (velocity_per_tick * choked_ticks);

					m_entity->SetAbsOrigin(new_origin);
					Interfaces::m_pDebugOverlay->AddBoxOverlay(new_origin, Vector(-20.f, -20.f, -20.f), Vector(20.f, 100.f, 80.f), QAngle(0, 0, 0), 30, 30, 255, 255, 0.01f);

				}

			}

		}

	}
}


namespace Engine
{
	struct SimulationRestore {
		int m_fFlags;
		float m_flDuckAmount;
		float m_flFeetCycle;
		float m_flFeetYawRate;
		QAngle m_angEyeAngles;
		Vector m_vecOrigin;

		void Setup( C_CSPlayer* player ) {
			m_fFlags = player->m_fFlags( );
			m_flDuckAmount = player->m_flDuckAmount( );
			m_vecOrigin = player->m_vecOrigin( );
			m_angEyeAngles = player->m_angEyeAngles( );

			auto animState = player->m_PlayerAnimState( );
			m_flFeetCycle = animState->m_flFeetCycle;
			m_flFeetYawRate = animState->m_flFeetYawRate;
		}

		void Apply( C_CSPlayer* player ) const {
			player->m_fFlags( ) = m_fFlags;
			player->m_flDuckAmount( ) = m_flDuckAmount;
			player->m_vecOrigin( ) = m_vecOrigin;
			player->m_angEyeAngles( ) = m_angEyeAngles;

			auto animState = player->m_PlayerAnimState( );
			animState->m_flFeetCycle = m_flFeetCycle;
			animState->m_flFeetYawRate = m_flFeetYawRate;
		}
	};

	struct AnimationBackup {

		CCSGOPlayerAnimState anim_state;
		C_AnimationLayer layers[ 13 ];
		float pose_params[ 19 ];

		AnimationBackup( ) {

		}

		void Apply( C_CSPlayer* player ) const;
		void Setup( C_CSPlayer* player );
	};

	void AnimationBackup::Apply( C_CSPlayer* player ) const {
		*player->m_PlayerAnimState( ) = this->anim_state;
		std::memcpy( player->m_AnimOverlay( ).m_Memory.m_pMemory, layers, sizeof( layers ) );
		std::memcpy( player->m_flPoseParameter( ), pose_params, sizeof( pose_params ) );
	}

	void AnimationBackup::Setup( C_CSPlayer* player ) {
		this->anim_state = *player->m_PlayerAnimState( );
		std::memcpy( layers, player->m_AnimOverlay( ).m_Memory.m_pMemory, sizeof( layers ) );
		std::memcpy( pose_params, player->m_flPoseParameter( ), sizeof( pose_params ) );
	}

	inline void FixBonesRotations( C_CSPlayer* player, matrix3x4_t* bones ) {
		// copypasted from supremacy/fatality, no difference imo
		// also seen that in aimware multipoints, but was lazy to paste, kek
		auto studio_hdr = player->m_pStudioHdr( );
		if( studio_hdr ) {
			auto hdr = *( studiohdr_t** )studio_hdr;
			if( hdr ) {
				auto hitboxSet = hdr->pHitboxSet( player->m_nHitboxSet( ) );
				for( int i = 0; i < hitboxSet->numhitboxes; i++ ) {
					auto hitbox = hitboxSet->pHitbox( i );
					if( hitbox->m_angAngles.IsZero( ) )
						continue;

					matrix3x4_t hitboxTransform;
					hitboxTransform.AngleMatrix( hitbox->m_angAngles );
					bones[ hitbox->bone ] = bones[ hitbox->bone ].ConcatTransforms( hitboxTransform );
				}
			}
		}
	}

	class C_AnimationSystem : public AnimationSystem {
	public:
		virtual void CollectData( );
		virtual void Update( );

		virtual C_AnimationData* GetAnimationData( int index ) {
			if( m_AnimatedEntities.count( index ) < 1 )
				return nullptr;

			return &m_AnimatedEntities[ index ];
		}

		std::map<int, C_AnimationData> m_AnimatedEntities = { };

		C_AnimationSystem( ) { };
		virtual ~C_AnimationSystem( ) { };
	};

	Encrypted_t<AnimationSystem> AnimationSystem::Get( ) {
		static C_AnimationSystem instance;
		return &instance;
	}

	void C_AnimationSystem::CollectData( ) {
		if( !Interfaces::m_pEngine->IsInGame( ) || !Interfaces::m_pEngine->GetNetChannelInfo( ) ) {
			this->m_AnimatedEntities.clear( );
			return;
		}

		auto local = C_CSPlayer::GetLocalPlayer( );
		if( !local || !g_Vars.globals.HackIsReady )
			return;

		for( int i = 1; i <= Interfaces::m_pGlobalVars->maxClients; ++i ) {
			auto player = C_CSPlayer::GetPlayerByIndex( i );
			if( !player || player == local )
				continue;

			player_info_t player_info;
			if( !Interfaces::m_pEngine->GetPlayerInfo( player->m_entIndex, &player_info ) ) {
				continue;
			}

			this->m_AnimatedEntities[ i ].Collect( player );
		}
	}

	void C_AnimationSystem::Update( ) {
		if( !Interfaces::m_pEngine->IsInGame( ) || !Interfaces::m_pEngine->GetNetChannelInfo( ) ) {
			this->m_AnimatedEntities.clear( );
			return;
		}

		auto local = C_CSPlayer::GetLocalPlayer( );
		if( !local || !g_Vars.globals.HackIsReady )
			return;

		for( auto& [key, value] : this->m_AnimatedEntities ) {
			auto entity = C_CSPlayer::GetPlayerByIndex( key );
			if( !entity )
				continue;

			auto curtime = Interfaces::m_pGlobalVars->curtime;
			auto frametime = Interfaces::m_pGlobalVars->frametime;

			Interfaces::m_pGlobalVars->curtime = entity->m_flOldSimulationTime( ) + Interfaces::m_pGlobalVars->interval_per_tick;
			Interfaces::m_pGlobalVars->frametime = Interfaces::m_pGlobalVars->interval_per_tick;

			if( value.m_bUpdated )
				value.Update( );

			Interfaces::m_pGlobalVars->curtime = curtime;
			Interfaces::m_pGlobalVars->frametime = frametime;

			value.m_bUpdated = false;
		}

	}

	void C_AnimationData::Update( ) {
		if( !this->player || this->m_AnimationRecord.size( ) < 1 )
			return;

		C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );
		if( !pLocal )
			return;

		auto pAnimationRecord = Encrypted_t<Engine::C_AnimationRecord>( &this->m_AnimationRecord.front( ) );
		Encrypted_t<Engine::C_AnimationRecord> pPreviousAnimationRecord( nullptr );
		if( this->m_AnimationRecord.size( ) > 1 ) {
			pPreviousAnimationRecord = &this->m_AnimationRecord.at( 1 );
		}

		this->player->m_vecVelocity( ) = pAnimationRecord->m_vecAnimationVelocity;

		auto weapon = ( C_BaseAttributableItem* )player->m_hActiveWeapon( ).Get( );
		auto weaponWorldModel = weapon ? ( C_CSPlayer* )( weapon )->m_hWeaponWorldModel( ).Get( ) : nullptr;

		auto animState = player->m_PlayerAnimState( );
		if( !animState )
			return;

		if (this->player->IsDormant()) {
			bool insert = true;

			if (!this->m_AnimationRecord.empty()) {
				C_AnimationRecord front = this->m_AnimationRecord.front();

				if (front.dormant())
					insert = false;
			}

			if (insert) {
				C_AnimationRecord current = this->m_AnimationRecord.front();

				current.m_dormant = true;
			}
		}

		bool update = (this->m_AnimationRecord.empty() || player->m_flSimulationTime() > this->m_AnimationRecord.front().m_flSimulationTime);

		if (update) {
			C_AnimationRecord current = this->m_AnimationRecord.front();

			current.m_dormant = false;
		}

		// simulate animations
		SimulateAnimations( pAnimationRecord, pPreviousAnimationRecord );

		// update layers
		std::memcpy( player->m_AnimOverlay( ).Base( ), pAnimationRecord->m_serverAnimOverlays, 13 * sizeof( C_AnimationLayer ) );

		player->InvalidateBoneCache();

		// generate aimbot matrix
		g_BoneSetup.SetupBonesRebuild( player, m_Bones, 128, BONE_USED_BY_ANYTHING & ~BONE_USED_BY_BONE_MERGE, player->m_flSimulationTime( ), BoneSetupFlags::UseCustomOutput );

		// generate visual matrix
		g_BoneSetup.SetupBonesRebuild( player, nullptr, 128, 0x7FF00, player->m_flSimulationTime( ), BoneSetupFlags::ForceInvalidateBoneCache | BoneSetupFlags::AttachmentHelper );

		this->m_vecSimulationData.clear( );
	}

	void C_AnimationData::Collect( C_CSPlayer* player ) {
		auto local = C_CSPlayer::GetLocalPlayer();

		if( player->IsDead( ) )
			player = nullptr;

		if (!local)
			return;

		auto pThis = Encrypted_t<C_AnimationData>( this );

		if( pThis->player != player ) {
			pThis->m_flSpawnTime = 0.0f;
			pThis->m_flSimulationTime = 0.0f;
			pThis->m_flOldSimulationTime = 0.0f;
			pThis->m_iCurrentTickCount = 0;
			pThis->m_iOldTickCount = 0;
			pThis->m_iTicksAfterDormancy = 0;
			pThis->m_vecSimulationData.clear( );
			pThis->m_AnimationRecord.clear( );
			pThis->m_bIsDormant = pThis->m_bBonesCalculated = false;
			pThis->player = player;
			pThis->m_bIsAlive = false;
		}

		if( !player )
			return;

		pThis->m_bIsAlive = true;
		pThis->m_flOldSimulationTime = pThis->m_flSimulationTime;
		pThis->m_flSimulationTime = pThis->player->m_flSimulationTime( );

		if( pThis->m_flSimulationTime == 0.0f || pThis->player->IsDormant( ) ) {
			pThis->m_bIsDormant = true;
			Engine::g_ResolverData[ player->EntIndex( ) ].m_bWentDormant = true;
			Engine::g_ResolverData[player->EntIndex()].m_vecSavedOrigin = pThis->m_vecOrigin;
			return;
		}

		if( pThis->m_flOldSimulationTime == pThis->m_flSimulationTime ) {
			return;
		}

		if( pThis->m_bIsDormant ) {
			pThis->m_iTicksAfterDormancy = 0;
			pThis->m_AnimationRecord.clear( );

			Engine::g_ResolverData[ player->EntIndex( ) ].m_bWentDormant = true;
		}

		pThis->ent_index = player->m_entIndex;

		pThis->m_bUpdated = true;
		pThis->m_bIsDormant = false;

		pThis->m_iOldTickCount = pThis->m_iCurrentTickCount;
		pThis->m_iCurrentTickCount = Interfaces::m_pGlobalVars->tickcount;

		if( pThis->m_flSpawnTime != pThis->player->m_flSpawnTime( ) ) {
			auto animState = pThis->player->m_PlayerAnimState( );
			if( animState ) {
				animState->m_Player = pThis->player;
				animState->Reset( );
			}

			pThis->m_flSpawnTime = pThis->player->m_flSpawnTime( );
		}

		int nTickRate = int( 1.0f / Interfaces::m_pGlobalVars->interval_per_tick );
		while( pThis->m_AnimationRecord.size( ) > nTickRate ) {
			pThis->m_AnimationRecord.pop_back( );
		}

		pThis->m_iTicksAfterDormancy++;

		Encrypted_t<C_AnimationRecord> previous_record = nullptr;
		Encrypted_t<C_AnimationRecord> penultimate_record = nullptr;

		if( pThis->m_AnimationRecord.size( ) > 0 ) {
			previous_record = &pThis->m_AnimationRecord.front( );
			if( pThis->m_AnimationRecord.size( ) > 1 ) {
				penultimate_record = &pThis->m_AnimationRecord.at( 1 );
			}
		}

		auto record = &pThis->m_AnimationRecord.emplace_front( );

		pThis->m_vecOrigin = pThis->player->m_vecOrigin( );

		record->m_vecOrigin = pThis->player->m_vecOrigin( );
		record->m_angEyeAngles = pThis->player->m_angEyeAngles( );
		record->m_flSimulationTime = pThis->m_flSimulationTime;
		record->m_anim_time = pThis->m_flOldSimulationTime + Interfaces::m_pGlobalVars->interval_per_tick;
		record->m_flLowerBodyYawTarget = pThis->player->m_flLowerBodyYawTarget( );
		record->m_body = pThis->player->m_flLowerBodyYawTarget();

		auto weapon = ( C_WeaponCSBaseGun* )( player->m_hActiveWeapon( ).Get( ) );

		if( weapon ) {
			auto weaponWorldModel = ( C_CSPlayer* )( ( C_BaseAttributableItem* )weapon )->m_hWeaponWorldModel( ).Get( );

			for( int i = 0; i < player->m_AnimOverlay( ).Count( ); ++i ) {
				player->m_AnimOverlay( ).Element( i ).m_pOwner = player;
				player->m_AnimOverlay( ).Element( i ).m_pStudioHdr = player->m_pStudioHdr( );

				if( weaponWorldModel ) {
					if( player->m_AnimOverlay( ).Element( i ).m_nSequence < 2 || player->m_AnimOverlay( ).Element( i ).m_flWeight <= 0.0f )
						continue;

					using UpdateDispatchLayer = void( __thiscall* )( void*, C_AnimationLayer*, CStudioHdr*, int );
					Memory::VCall< UpdateDispatchLayer >( player, 241 )( player, &player->m_AnimOverlay( ).Element( i ),
						weaponWorldModel->m_pStudioHdr( ), player->m_AnimOverlay( ).Element( i ).m_nSequence );
				}
			}
		}

		std::memcpy( record->m_serverAnimOverlays, pThis->player->m_AnimOverlay( ).Base( ), sizeof( record->m_serverAnimOverlays ) );

		record->m_flFeetCycle = record->m_serverAnimOverlays[ 6 ].m_flCycle;
		record->m_flFeetYawRate = record->m_serverAnimOverlays[ 6 ].m_flWeight;

		record->m_fFlags = player->m_fFlags( );
		record->m_flDuckAmount = player->m_flDuckAmount( );

		record->m_bIsShoting = false;
		record->m_flShotTime = 0.0f;
		record->m_bFakeWalking = false;
		Engine::g_ResolverData[player->EntIndex()].fakewalking = false;

		if( previous_record.IsValid( ) ) {
			record->m_flChokeTime = pThis->m_flSimulationTime - pThis->m_flOldSimulationTime;
			record->m_iChokeTicks = TIME_TO_TICKS( record->m_flChokeTime );
		}
		else {
			record->m_flChokeTime = Interfaces::m_pGlobalVars->interval_per_tick;
			record->m_iChokeTicks = 1;
		}

		if( !previous_record.IsValid( ) ) {
			record->m_bIsInvalid = true;
			record->m_vecVelocity.Init( );
			record->m_bIsShoting = false;
			record->m_bTeleportDistance = false;

			//auto animstate = player->m_PlayerAnimState( );
			//if( animstate )
			//	animstate->m_flAbsRotation = record->m_angEyeAngles.yaw;

			return;
		}

		static int m_iFakeFlickCheck = 0;
		static int m_iFakeFlickResetCheck = 0;
		static float m_flLastResetTime1 = Interfaces::m_pGlobalVars->curtime;
		static float m_flMaxResetTime1 = Interfaces::m_pGlobalVars->curtime + 1.f;

		m_flLastResetTime1 = Interfaces::m_pGlobalVars->curtime;

		if (player->m_flOldSimulationTime() > player->m_flSimulationTime()) {
			m_flMaxResetTime1 = Interfaces::m_pGlobalVars->curtime + 1.f;
			m_iFakeFlickCheck++;
			m_iFakeFlickResetCheck = 0;
		}

		else if (player->m_flOldSimulationTime() <= player->m_flSimulationTime() && m_flLastResetTime1 >= m_flMaxResetTime1 && m_iFakeFlickCheck > 0) {
			m_flMaxResetTime1 = Interfaces::m_pGlobalVars->curtime + 1.f;
			m_iFakeFlickResetCheck++;
		}

		//if (player->m_flOldSimulationTime() > player->m_flSimulationTime()) {
		//	m_flMaxResetTime1 = Interfaces::m_pGlobalVars->curtime + 0.5f;
		//}

		//if (m_flLastResetTime1 >= m_flMaxResetTime1) {
		//	m_iFakeFlickCheck++;
		//	m_iFakeFlickResetCheck = 0;
		//	printf("reset check reset\n");
		//	//printf(std::to_string(m_iFakeFlickCheck).c_str());
		//	//printf(" < fake flick check hit\n");
		//}
		//else if (m_flLastResetTime1 >= m_flMaxResetTime1 && m_iFakeFlickCheck >= 0) {
		//	m_iFakeFlickResetCheck++;
		//	printf("reset check increased\n");
		//}

		if (m_iFakeFlickCheck >= 5) {
			g_Vars.globals.m_bDontExtrap[player->EntIndex()] = true;
			//printf("SET TO TRUE \n");
		}

		if (m_iFakeFlickResetCheck >= 3) {
			g_Vars.globals.m_bDontExtrap[player->EntIndex()] = false;
			printf("reset\n");
			m_iFakeFlickResetCheck = 0;
			m_iFakeFlickCheck = 0;
			m_flMaxResetTime1 = Interfaces::m_pGlobalVars->curtime + 1.f;
		}

		/*auto flPreviousSimulationTime = previous_record->m_flSimulationTime;
		auto nTickcountDelta = pThis->m_iCurrentTickCount - pThis->m_iOldTickCount;
		auto nSimTicksDelta = record->m_iChokeTicks;
		auto nChokedTicksUnk = nSimTicksDelta;
		auto bShiftedTickbase = false;
		if( pThis->m_flOldSimulationTime > pThis->m_flSimulationTime ) {
			record->m_bShiftingTickbase = true;
			record->m_iChokeTicks = nTickcountDelta;
			record->m_flChokeTime = TICKS_TO_TIME( record->m_iChokeTicks );
			flPreviousSimulationTime = record->m_flSimulationTime - record->m_flChokeTime;
			nChokedTicksUnk = nTickcountDelta;
			bShiftedTickbase = true;
		}

		if( bShiftedTickbase || abs( nSimTicksDelta - nTickcountDelta ) <= 2 ) {
			if( nChokedTicksUnk ) {
				if( nChokedTicksUnk != 1 ) {
					pThis->m_iTicksUnknown = 0;
				}
				else {
					pThis->m_iTicksUnknown++;
				}
			}
			else {
				record->m_iChokeTicks = 1;
				record->m_flChokeTime = Interfaces::m_pGlobalVars->interval_per_tick;

				flPreviousSimulationTime = record->m_flSimulationTime - Interfaces::m_pGlobalVars->interval_per_tick;

				pThis->m_iTicksUnknown++;
			}
		}*/

		if( weapon ) {
			record->m_flShotTime = weapon->m_fLastShotTime( );
			record->m_bIsShoting = record->m_flSimulationTime >= record->m_flShotTime && record->m_flShotTime > previous_record->m_flSimulationTime;
		}

		record->m_bIsInvalid = false;

		if (previous_record.IsValid()) {

			auto was_in_air = (player->m_fFlags() & FL_ONGROUND) && !(previous_record->m_fFlags & FL_ONGROUND);
			auto animation_speed = 0.0f;
			auto origin_delta = record->m_vecOrigin - previous_record->m_vecOrigin;

			Vector vecPreviousOrigin = previous_record->m_vecOrigin;
			int fPreviousFlags = previous_record->m_fFlags;
			auto time_difference = std::max(Interfaces::m_pGlobalVars->interval_per_tick, record->m_flSimulationTime - previous_record->m_flSimulationTime);

			// fix velocity
			// https://github.com/VSES/SourceEngine2007/blob/master/se2007/game/client/c_baseplayer.cpp#L659
			if (!origin_delta.IsZero() && TIME_TO_TICKS(time_difference) > 0) {
				//record->m_vecVelocity = (record->m_vecOrigin - previous_record->m_vecOrigin) * (1.f / record->m_flChokeTime);
				record->m_vecVelocity = origin_delta * (1.0f / time_difference);

				//if (player->m_vecVelocity().Length2D() >= 100.f)
				//{
				//	player_extrapolation(player, vecPreviousOrigin, record->m_vecVelocity, record->m_fFlags, (fPreviousFlags& FL_ONGROUND), 3);
				//	Interfaces::m_pDebugOverlay->AddBoxOverlay(record->m_vecOrigin, Vector(-20.f, -20.f, -20.f), Vector(20.f, 20.f, 80.f), QAngle(0, 0, 0), 255, 30, 30, 255, 0.01f);
				//}

				if (player->m_fFlags() & FL_ONGROUND && record->m_serverAnimOverlays[11].m_flWeight > 0.0f && record->m_serverAnimOverlays[11].m_flWeight < 1.0f && record->m_serverAnimOverlays[11].m_flCycle > previous_record->m_serverAnimOverlays[11].m_flCycle)
				{
					auto weapon = player->m_hActiveWeapon().Get();

					if (weapon)
					{
						auto max_speed = 260.0f;
						C_WeaponCSBaseGun* Weapon = (C_WeaponCSBaseGun*)player->m_hActiveWeapon().Get();
						auto weapon_info = Weapon->GetCSWeaponData();

						if (weapon_info.IsValid())
							max_speed = player->m_bIsScoped() ? weapon_info->m_flMaxSpeed2 : weapon_info->m_flMaxSpeed;

						auto modifier = 0.35f * (1.0f - record->m_serverAnimOverlays[11].m_flWeight);

						if (modifier > 0.0f && modifier < 1.0f)
							animation_speed = max_speed * (modifier + 0.55f);
					}
				}

				if (animation_speed > 0.0f)
				{
					animation_speed /= record->m_vecVelocity.Length2D();

					record->m_vecVelocity.x *= animation_speed;
					record->m_vecVelocity.y *= animation_speed;
				}

				if (pThis->m_AnimationRecord.size() >= 3 && time_difference > Interfaces::m_pGlobalVars->interval_per_tick)
				{
					auto previous_velocity = (previous_record->m_vecOrigin - pThis->m_AnimationRecord.at(2).m_vecOrigin) * (1.0f / time_difference);

					if (!previous_velocity.IsZero() && !was_in_air)
					{
						auto current_direction = Math::normalize_float(RAD2DEG(atan2(record->m_vecVelocity.y, record->m_vecVelocity.x)));
						auto previous_direction = Math::normalize_float(RAD2DEG(atan2(previous_velocity.y, previous_velocity.x)));

						auto average_direction = current_direction - previous_direction;
						average_direction = DEG2RAD(Math::normalize_float(current_direction + average_direction * 0.5f));

						auto direction_cos = cos(average_direction);
						auto dirrection_sin = sin(average_direction);

						auto velocity_speed = record->m_vecVelocity.Length2D();

						record->m_vecVelocity.x = direction_cos * velocity_speed;
						record->m_vecVelocity.y = dirrection_sin * velocity_speed;
					}
				}

				// fix CGameMovement::FinishGravity
				if (!(player->m_fFlags() & FL_ONGROUND)) {
					static auto sv_gravity = g_Vars.sv_gravity;
					auto fixed_timing = Math::Clamp(time_difference, Interfaces::m_pGlobalVars->interval_per_tick, 1.0f);
					record->m_vecVelocity.z -= sv_gravity->GetFloat() * fixed_timing * 0.5f;
				}
				else
					record->m_vecVelocity.z = 0.0f;

			}

			if (!record->m_bIsInvalid && !g_Vars.globals.m_bDontExtrap[player->EntIndex()] && record->m_vecVelocity.Length2D() > 1.f) {
				AimwareExtrapolation(player, record->m_vecVelocity, record, previous_record.Xor());
				//Interfaces::m_pDebugOverlay->AddBoxOverlay(record->m_vecOrigin, Vector(-20.f, -20.f, -20.f), Vector(20.f, 100.f, 80.f), QAngle(0, 0, 0), 255, 30, 30, 255, 0.01f);
			}

		}

		//// detect fakewalking players
		if (record->m_vecVelocity.Length() > 0.1f
			//&& record->m_iChokeTicks >= 2
			&& record->m_serverAnimOverlays[12].m_flWeight == 0.0f
			&& record->m_serverAnimOverlays[6].m_flWeight == 0.0f
			&& record->m_serverAnimOverlays[6].m_flPlaybackRate < 0.0003f // For a semi-accurate result you could use this being weight being about smaller than 0.0001 (maybe would need to scale it up against faster fakewalks, won't have too big of a impact though.)
			&& (record->m_fFlags & FL_ONGROUND) && record->m_vecVelocity.Length() <= 85.f) {
			record->m_bFakeWalking = true;
			Engine::g_ResolverData[player->EntIndex()].fakewalking = true;
			//printf("FAKEWALKING\n");
		}
		else {
			//if (record->m_serverAnimOverlays[12].m_flWeight > 0.0f)
			//	printf("[12] WEIGHT INCORRECT\n");
			//if (record->m_serverAnimOverlays[6].m_flWeight > 0.0f)
			//	printf("[6] WEIGHT INCORRECT\n");
			//if (record->m_serverAnimOverlays[6].m_flPlaybackRate > 0.0003f)
			//	printf("[6] PLAYBACKRATE INCORRECT\n");
			//if (record->m_iChokeTicks < 2)
			//	printf("iChokeTicks incorrect\n");
			record->m_bFakeWalking = false;
			Engine::g_ResolverData[player->EntIndex()].fakewalking = false;
		}

		// detect players abusing micromovements or other trickery
		if (record->m_vecVelocity.Length() < 18.f
			&& record->m_serverAnimOverlays[6].m_flWeight != 1.0f
			&& record->m_serverAnimOverlays[6].m_flWeight != 0.0f
			&& record->m_serverAnimOverlays[6].m_flWeight != previous_record->m_serverAnimOverlays[6].m_flWeight
			&& (record->m_fFlags & FL_ONGROUND)) {
			record->m_bUnsafeVelocityTransition = true;
		}

		record->m_vecAnimationVelocity = record->m_vecVelocity;

		if( record->m_bFakeWalking ) {
			record->m_vecAnimationVelocity.Init( );
		}

		// delta in time..
		float time = record->m_flSimulationTime - previous_record->m_flSimulationTime;

		if( !record->m_bFakeWalking ) {
			// fix the velocity till the moment of animation.
			Vector velo = record->m_vecVelocity - previous_record->m_vecVelocity;

			// accel per tick.
			Vector accel = ( velo / time ) * Interfaces::m_pGlobalVars->interval_per_tick;

			// set the anim velocity to the previous velocity.
			// and predict one tick ahead.
			record->m_vecAnimationVelocity = previous_record->m_vecVelocity + accel;
		}

		record->m_vecLastNonDormantOrig = record->m_vecOrigin;

		record->m_bTeleportDistance = ((record->m_vecOrigin - previous_record->m_vecOrigin).Length2DSquared() > 4096.0f);

		//LinearExtrapolations(record);

		C_SimulationInfo& data = pThis->m_vecSimulationData.emplace_back( );
		data.m_flTime = previous_record->m_flSimulationTime + Interfaces::m_pGlobalVars->interval_per_tick;
		data.m_flDuckAmount = record->m_flDuckAmount;
		data.m_flLowerBodyYawTarget = record->m_flLowerBodyYawTarget;
		data.m_vecOrigin = record->m_vecOrigin;
		data.m_vecVelocity = record->m_vecAnimationVelocity;
		data.bOnGround = record->m_fFlags & FL_ONGROUND;

		// lets check if its been more than 2 ticks, so we can fix jumpfall.
		if( record->m_iChokeTicks > 2 ) {
			// TODO: calculate jump time
			// calculate landing time
			float flLandTime = 0.0f;
			bool bJumped = false;
			bool bLandedOnServer = false;
			if( record->m_serverAnimOverlays[ 4 ].m_flCycle < 0.5f && ( !( record->m_fFlags & FL_ONGROUND ) || !( previous_record->m_fFlags & FL_ONGROUND ) ) ) {
				// note - VIO;
				// well i guess when llama wrote v3, he was drunk or sum cuz this is incorrect. -> cuz he changed this in v4.
				// and alpha didn't realize this but i did, so its fine.
				// improper way to do this -> flLandTime = record->m_flSimulationTime - float( record->m_serverAnimOverlays[ 4 ].m_flPlaybackRate * record->m_serverAnimOverlays[ 4 ].m_flCycle );
				// we need to divide instead of multiplication.
				flLandTime = record->m_flSimulationTime - float( record->m_serverAnimOverlays[ 4 ].m_flPlaybackRate / record->m_serverAnimOverlays[ 4 ].m_flCycle );
				bLandedOnServer = flLandTime >= previous_record->m_flSimulationTime;
			}

			bool bOnGround = record->m_fFlags & FL_ONGROUND;
			// jump_fall fix
			if( bLandedOnServer && !bJumped ) {
				if( flLandTime <= data.m_flTime ) {
					bJumped = true;
					bOnGround = true;
				}
				else {
					bOnGround = previous_record->m_fFlags & FL_ONGROUND;
				}
			}

			// do the fix. hahaha
			if (bOnGround) {
				player->m_fFlags() |= FL_ONGROUND;
			}
			else {
				player->m_fFlags() &= ~FL_ONGROUND;
			}

			data.bOnGround = bOnGround;

			// delta in duckamt and delta in time..
			float duck = record->m_flDuckAmount - previous_record->m_flDuckAmount;
			float time = record->m_flSimulationTime - previous_record->m_flSimulationTime;

			// get the duckamt change per tick.
			float change = (duck / time) * Interfaces::m_pGlobalVars->interval_per_tick;

			// fix crouching players.
			player->m_flDuckAmount() = previous_record->m_flDuckAmount + change;
		}
	}

	void C_AnimationData::SimulateAnimations( Encrypted_t<Engine::C_AnimationRecord> current, Encrypted_t<Engine::C_AnimationRecord> previous ) {
		auto UpdateAnimations = [ & ] ( C_CSPlayer* player, float flTime ) {
			auto curtime = Interfaces::m_pGlobalVars->curtime;
			auto frametime = Interfaces::m_pGlobalVars->frametime;

			// force to use correct abs origin and velocity ( no CalcAbsolutePosition and CalcAbsoluteVelocity calls )
			player->m_iEFlags( ) &= ~( EFL_DIRTY_ABSTRANSFORM | EFL_DIRTY_ABSVELOCITY );

			int ticks = TIME_TO_TICKS( flTime );

			// calculate animations based on ticks aka server frames instead of render frames
			Interfaces::m_pGlobalVars->curtime = flTime;
			Interfaces::m_pGlobalVars->frametime = Interfaces::m_pGlobalVars->interval_per_tick;

			auto animstate = player->m_PlayerAnimState( );
			if( animstate && animstate->m_nLastFrame >= Interfaces::m_pGlobalVars->framecount )
				animstate->m_nLastFrame = Interfaces::m_pGlobalVars->framecount - 1;

			for( int i = 0; i < player->m_AnimOverlay( ).Count( ); ++i ) {
				player->m_AnimOverlay( ).Base( )[ i ].m_pOwner = player;
				player->m_AnimOverlay( ).Base( )[ i ].m_pStudioHdr = player->m_pStudioHdr( );
			}

			static auto& EnableInvalidateBoneCache = **reinterpret_cast< bool** >( Memory::Scan( XorStr( "client.dll" ), XorStr( "C6 05 ? ? ? ? ? 89 47 70" ) ) + 2 );

			// make sure we keep track of the original invalidation state
			const auto oldInvalidationState = EnableInvalidateBoneCache;

			// is player bot?
			auto IsPlayerBot = [&]( ) -> bool {
				player_info_t info;
				if( Interfaces::m_pEngine->GetPlayerInfo( player->EntIndex( ), &info ) )
					return info.fakeplayer;

				return false;
			};

			if (!player->IsTeammate(C_CSPlayer::GetLocalPlayer())) { // plist overrides
				if (g_Vars.globals.player_list.override_pitch[player->EntIndex()]) {
					player->m_angEyeAngles().x = g_Vars.globals.player_list.override_pitch_slider[player->EntIndex()];
				}
			}

			// attempt to resolve the player	
			if( !player->IsTeammate( C_CSPlayer::GetLocalPlayer( ) ) && !IsPlayerBot( ) ) {
				// predict lby updates
				g_Resolver.ResolveAngles( player, current.Xor( ) );

				bool bValid = previous.Xor( );

				// we're sure that we resolved the player.
				if ( bValid )
				{
					current.Xor()->m_bResolved = current.Xor()->m_iResolverMode == Engine::RModes::MOVING || current.Xor()->m_iResolverMode == Engine::RModes::FLICK;
				}
				else
				{
					current.Xor()->m_bResolved = current.Xor()->m_iResolverMode == Engine::RModes::MOVING || current.Xor()->m_iResolverMode == Engine::RModes::FLICK;
				}

				bool bResolved = current.Xor( )->m_bResolved;
				if( g_Vars.rage.override_resolver_flicks ) {
					if( current.Xor( )->m_iResolverMode == Engine::RModes::FLICK)
						bResolved = false;
				}

				// if the enemy is resolved, why bother overriding?
				//g_Resolver.override_resolver(player, current.Xor());
				//g_Resolver.ResolveManual( player, current.Xor( ), bResolved );
			}

			player->UpdateClientSideAnimationEx( );

			// we don't want to enable cache invalidation by accident
			EnableInvalidateBoneCache = oldInvalidationState;

			Interfaces::m_pGlobalVars->curtime = curtime;
			Interfaces::m_pGlobalVars->frametime = frametime;
		};

		SimulationRestore SimulationRecordBackup;
		SimulationRecordBackup.Setup( player );

		auto animState = player->m_PlayerAnimState( );

		if( previous.IsValid( ) ) {
			if( previous->m_bIsInvalid && current->m_fFlags & FL_ONGROUND ) {
				animState->m_bOnGround = true;
				animState->m_bHitground = false;
			}

			if( previous.IsValid( ) ) {
				if( previous->m_bIsInvalid && current->m_fFlags & FL_ONGROUND ) {
					animState->m_bOnGround = true;
					animState->m_bHitground = false;
				}

				animState->m_flFeetCycle = previous->m_flFeetCycle;
				animState->m_flFeetYawRate = previous->m_flFeetYawRate;
				*( float* )( uintptr_t( animState ) + 0x180 ) = previous->m_serverAnimOverlays[ 12 ].m_flWeight;

				std::memcpy( player->m_AnimOverlay( ).Base( ), previous->m_serverAnimOverlays, sizeof( previous->m_serverAnimOverlays ) );
			}
			else {
				animState->m_flFeetCycle = current->m_flFeetCycle;
				animState->m_flFeetYawRate = current->m_flFeetYawRate;
				*( float* )( uintptr_t( animState ) + 0x180 ) = current->m_serverAnimOverlays[ 12 ].m_flWeight;
			}
		}

		if( current->m_iChokeTicks > 1 ) {
			for( auto it = this->m_vecSimulationData.begin( ); it < this->m_vecSimulationData.end( ); it++ ) {
				m_bForceVelocity = true;
				const auto& simData = *it;
				if( simData.bOnGround ) {
					player->m_fFlags( ) |= FL_ONGROUND;
				}
				else {
					player->m_fFlags( ) &= ~FL_ONGROUND;
				}

				player->m_vecOrigin( ) = simData.m_vecOrigin;
				player->m_flDuckAmount( ) = simData.m_flDuckAmount;
				player->m_vecVelocity( ) = simData.m_vecVelocity;
				player->SetAbsVelocity( simData.m_vecVelocity );
				player->SetAbsOrigin( simData.m_vecOrigin );
				//player->m_flLowerBodyYawTarget( ) = simData.m_flLowerBodyYawTarget;
				player->m_flLowerBodyYawTarget() = current->m_body;

				UpdateAnimations( player, player->m_flOldSimulationTime( ) + Interfaces::m_pGlobalVars->interval_per_tick );

				m_bForceVelocity = false;
			}
		}
		else {
			m_bForceVelocity = true;
			this->player->SetAbsVelocity( current->m_vecAnimationVelocity );
			this->player->SetAbsOrigin( current->m_vecOrigin );
			this->player->m_flLowerBodyYawTarget( ) = current->m_body;

			UpdateAnimations( player, player->m_flOldSimulationTime( ) + Interfaces::m_pGlobalVars->interval_per_tick );

			m_bForceVelocity = false;
		}

		SimulationRecordBackup.Apply( player );
		player->InvalidatePhysicsRecursive( 8 );
	}
}