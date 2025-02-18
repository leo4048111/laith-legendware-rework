#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"
#include "../prediction/Networking.h"
enum
{
	MAIN,
	NONE,
	FIRST,
	SECOND,
	LOW_FIRST,
	LOW_SECOND,
	LOW_FIRST_20,
	LOW_SECOND_20
};
enum EFixedVelocity
{
	Unresolved = 0,
	MovementLayer,
	AliveLoopLayer,
	LeanLayer
};

enum EPlayerActivityC
{
	CNoActivity = 0,
	CJump = 1,
	CLand = 2
};

enum resolver_type
{
	ORIGINAL,
	LBY,
	LAYERS,
	TRACE,
	DIRECTIONAL,
	ENGINE,
	FREESTAND,
	HURT,
	NON_RESOLVED
};
enum resolver_side
{
	RESOLVER_ORIGINAL,
	RESOLVER_ZERO,
	RESOLVER_FIRST,
	RESOLVER_SECOND,
	RESOLVER_LOW_FIRST,
	RESOLVER_LOW_SECOND,
	RESOLVER_LOW_FIRST_20,
	RESOLVER_LOW_SECOND_20,
	RESOLVER_ON_SHOT
};

enum modes
{
	AIR,
	SLOW_WALKING,
	MOVING,
	STANDING,
	FREESTANDING,
	NO_MODE
};

enum get_side_move
{
	NO_SIDE,
	LEFT,
	RIGHT,
};

enum EMatrixFlags
{
	Interpolated = (1 << 1),
	BoneUsedByHitbox = (1 << 2),
	VisualAdjustment = (1 << 3),
	None = (1 << 4),
};


enum MatrixBoneSide
{
	MiddleMatrix,
	LeftMatrix,
	RightMatrix,
	LowLeftMatrix,
	LowRightMatrix,
	ZeroMatrix,
};
enum ADVANCED_ACTIVITY : int
{
	ACTIVITY_NONE = 0,
	ACTIVITY_JUMP,
	ACTIVITY_LAND
};//

struct GameGlobals_t
{
	float m_flCurTime;
	float m_flRealTime;
	float m_flFrameTime;
	float m_flAbsFrameTime;
	float m_flInterpolationAmount;
	int m_nTickCount, m_nFrameCount;

	void AdjustData()
	{
		m_globals()->m_curtime = this->m_flCurTime;
		m_globals()->m_realtime = this->m_flRealTime;
		m_globals()->m_frametime = this->m_flFrameTime;
		m_globals()->m_absoluteframetime = this->m_flAbsFrameTime;
		m_globals()->m_interpolation_amount = this->m_flInterpolationAmount;
		m_globals()->m_tickcount = this->m_nTickCount;
		m_globals()->m_framecount = this->m_nFrameCount;
	}
	void CaptureData()
	{
		this->m_flCurTime = m_globals()->m_curtime;
		this->m_flRealTime = m_globals()->m_realtime;
		this->m_flFrameTime = m_globals()->m_frametime;
		this->m_flAbsFrameTime = m_globals()->m_absoluteframetime;
		this->m_flInterpolationAmount = m_globals()->m_interpolation_amount;
		this->m_nTickCount = m_globals()->m_tickcount;
		this->m_nFrameCount = m_globals()->m_framecount;
	}
};

struct PlayersGlobals_t
{
	Vector m_vecVelocity;
	Vector m_vecAbsVelocity;
	Vector m_vecAbsOrigin;
	int m_fFlags;
	int m_iEFlags;
	float m_flDuckAmount, m_flLowerBodyYawTarget, m_flThirdpersonRecoil;
	

	void AdjustData(player_t* e)
	{
		e->m_vecVelocity() = this->m_vecVelocity;
		e->m_vecAbsVelocity() = this->m_vecAbsVelocity;
		e->m_fFlags() = this->m_fFlags;
		e->m_iEFlags() = this->m_iEFlags;
		e->m_flDuckAmount() = this->m_flDuckAmount;
		e->m_flLowerBodyYawTarget() = this->m_flLowerBodyYawTarget;
		e->m_flThirdpersonRecoil() = this->m_flThirdpersonRecoil;
		e->set_abs_origin(this->m_vecAbsOrigin);
	}
	void CaptureData(player_t* e)
	{
		this->m_vecVelocity = e->m_vecVelocity();
		this->m_vecAbsVelocity = e->m_vecAbsVelocity();
		this->m_fFlags = e->m_fFlags();
		this->m_iEFlags = e->m_iEFlags();
		this->m_flDuckAmount = e->m_flDuckAmount();
		this->m_flLowerBodyYawTarget = e->m_flLowerBodyYawTarget();
		this->m_flThirdpersonRecoil = e->m_flThirdpersonRecoil();
		this->m_vecAbsOrigin = e->get_abs_origin();
	}
};

class adjust_data;

class CResolver
{
	player_t* player = nullptr;
	adjust_data* player_record = nullptr;
	adjust_data* prev_record = nullptr;

	bool side = false;
	bool fake = false;
	bool was_first_bruteforce = false;
	bool was_second_bruteforce = false;

	struct
	{
		int missed_shots_corrected[65] = {0};
	}restype[8];


	float lock_side = 0.0f;
	float original_goal_feet_yaw = 0.0f;
	float original_pitch = 0.0f;

public:
	void initialize(player_t* e, adjust_data* record, const float& goal_feet_yaw, const float& pitch, adjust_data* prev_record);
	void initialize_yaw(player_t* e, adjust_data* record, adjust_data* prev_record);
	void reset();

	bool IsAdjustingBalance();

	bool is_breaking_lby(AnimationLayer cur_layer, AnimationLayer prev_layer);
	void OtLayersResolver();
	void get_side_standing();
	void detect_side();
	void get_side_trace();
	int GetChokedPackets();
	void solve_animes();
	void NemsisResolver();
	bool DesyncDetect();
	bool update_walk_data();
	void setmode();
	bool MatchShot();
	bool is_slow_walking();


	void final_detection();

	void missed_shots_correction(adjust_data* record, int missed_shots);

	void reset_resolver();

	float b_yaw(player_t* player, float angle, int n);

	void resolve_desync();

	AnimationLayer resolver_layers[7][13];
	AnimationLayer previous_layers[13];
	float resolver_goal_feet_yaw[7];



	resolver_side last_side = RESOLVER_ORIGINAL;
	resolver_type type = ORIGINAL;
};

class adjust_data
{
public:
	player_t* player;
	int i;
	int m_nSimulationTicks = 0;
	AnimationLayer previous_layers[13];
	AnimationLayer layers[13];
	//matrixes matrixes_data;
	std::array < std::array < matrix3x4_t, MAXSTUDIOBONES >, 7 > m_Matricies;
	EFixedVelocity m_nVelocityMode = EFixedVelocity::Unresolved;
	resolver_type type;
	resolver_side side;
	float desync_amount = 0.0f;
	struct
	{
		bool LowDelta = false;
		bool ShouldFlip = false;
	}types[7];

	bool break_lagcomp;
	get_side_move curSide;
	modes curMode;
	bool m_bAnimResolved;
	bool invalid;
	bool m_bHasBrokenLC;
	bool immune;
	bool dormant;
	bool bot;
	bool shot;
	int shot_tick;
	int flags;
	int bone_count;
	float last_shot_time;
	bool check_hurt;

	float left;
	float right;
	float Eye;
	float Middle;

	float simulation_time;

	int m_nActivityTick = 0;
	float m_flActivityPlayback = 0.0f;
	float m_flDurationInAir = 0.0f;
	EPlayerActivityC m_nActivityType = EPlayerActivityC::CNoActivity;

	float old_simtime;
	float duck_amount;
	float lby;
	float lby_diff;
	bool flipped_s = false;
	bool low_delta_s = false;
	float m_flThirdPersonRecoil;
	Vector angles;
	bool m_bRestoreData;
	float m_flExploitTime;
	float m_flLastSimTime;
	Vector abs_angles;
	Vector m_vecAbsOrigin;
	Vector velocity;
	Vector origin;
	float m_flAnimationVelocity;
	Vector mins;
	Vector maxs;
	bool m_fDidBacktrack;
	matrix3x4_t* m_pBoneCache;

	float m_fSpawnTime;
	adjust_data()
	{
		reset();
	}

	void reset()
	{
		player = nullptr;
		i = -1;

		/*type = ORIGINAL;
		side = RESOLVER_ORIGINAL;
		curSide = NO_SIDE;
		curMode = NO_MODE;*/
		bot = false;
		invalid = false;
		m_bHasBrokenLC = false;
		immune = false;
		dormant = false;
		m_flAnimationVelocity = 0;
		m_flExploitTime = 0.0f;
		m_flLastSimTime = 0.0f;
		shot = false;
		shot_tick = 0;
		flags = 0;
		bone_count = 0;
		m_fSpawnTime = 0.0f;
		lby_diff = 0.0f;
		simulation_time = 0.0f;
		old_simtime = 0.0f;
		duck_amount = 0.0f;
		lby = 0.0f;
		m_flThirdPersonRecoil = 0.0f;
		m_nSimulationTicks = 0;
		angles.Zero();
		abs_angles.Zero();
		velocity.Zero();
		origin.Zero();
		mins.Zero();
		m_vecAbsOrigin.Zero();
		maxs.Zero();
		m_bRestoreData = false;
	}

	adjust_data(player_t* e, bool store = true)
	{
		/*type = ORIGINAL;
		side = RESOLVER_ORIGINAL;
		curSide = NO_SIDE;
		curMode = NO_MODE;
		m_flExploitTime = 0.0f;
		m_flLastSimTime = 0.0f;
		invalid = false;
		m_bHasBrokenLC = false;
		immune = false;
		dormant = false;
		shot = false;
		shot_tick = 0;
		m_bRestoreData = false;*/
		reset();
		store_data(e, store);
	}

	void store_data(player_t* e, bool store = true)
	{
		if (!e->is_alive())
			return;

		player = e;
		i = player->EntIndex();

		if (store)
		{
			memcpy(layers, e->get_animlayers(), e->animlayer_count() * sizeof(AnimationLayer));
			memcpy(m_Matricies[MatrixBoneSide::MiddleMatrix].data(), player->m_CachedBoneData().Base(), player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));
		}

		immune = player->m_bGunGameImmunity() || player->m_fFlags() & FL_FROZEN;
		dormant = player->IsDormant();

		player_info_t player_info;
		m_engine()->GetPlayerInfo(i, &player_info);
		bot = player_info.fakeplayer;

		/*Handle shots*/
		weapon_t* pWeapon = e->m_hActiveWeapon();
		if (pWeapon)
		{
			if (pWeapon->m_fLastShotTime() <= e->m_flSimulationTime() && pWeapon->m_fLastShotTime() > e->m_flOldSimulationTime())
			{
				shot = true;
				shot_tick = TIME_TO_TICKS(pWeapon->m_fLastShotTime());
			}
		}

		flags = player->m_fFlags();
		bone_count = player->m_CachedBoneData().Count();
		simulation_time = player->m_flSimulationTime();
		old_simtime = player->m_flOldSimulationTime();
		duck_amount = player->m_flDuckAmount();
		lby = player->m_flLowerBodyYawTarget();
		m_flThirdPersonRecoil = player->m_flThirdpersonRecoil();
		angles = player->m_angEyeAngles();
		lby_diff = std::abs(math::AngleDiff(angles.y, lby));
		abs_angles = player->get_abs_angles();
		m_vecAbsOrigin = player->get_abs_origin();
		velocity = player->m_vecVelocity();
		origin = player->m_vecOrigin();
		player->UpdateCollisionBounds();
		mins = player->GetCollideable()->OBBMins();
		maxs = player->GetCollideable()->OBBMaxs();
		m_fSpawnTime = player->m_flSpawnTime();
		m_bRestoreData = true;
	}

	void adjust_player()
	{
		if (!valid() || !m_bRestoreData)
			return;

		memcpy(player->get_animlayers(), layers, player->animlayer_count() * sizeof(AnimationLayer));
		memcpy(player->m_CachedBoneData().Base(), m_Matricies[MatrixBoneSide::MiddleMatrix].data(), player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));

		player->m_fFlags() = flags;
		player->m_CachedBoneData().m_Size = bone_count;
		player->get_abs_origin() = m_vecAbsOrigin;
		player->m_flSimulationTime() = simulation_time;
		player->m_flDuckAmount() = duck_amount;
		player->m_flLowerBodyYawTarget() = lby;
		player->m_flThirdpersonRecoil() = m_flThirdPersonRecoil;
		player->m_angEyeAngles() = angles;
		player->set_abs_angles(abs_angles);
		player->m_vecVelocity() = velocity;
		player->m_vecOrigin() = origin;
		player->set_abs_origin(origin);

		player->UpdateCollisionBounds();
		player->SetCollisionBounds(mins, maxs);

		m_bRestoreData = false;
	}

	bool IsTimeValid(float flSimulationTime, int nTickBase)
	{
		const float flLerpTime = util::get_interpolation();

		float flDeltaTime = fmin(g_Networking->latency + flLerpTime, 0.2f) - TICKS_TO_TIME(nTickBase - TIME_TO_TICKS(flSimulationTime));
		if (fabs(flDeltaTime) > 0.2f)
			return false;

		int nDeadTime = (int)((float)(TICKS_TO_TIME(g_Networking->server_tick())) - 0.2f);
		if (TIME_TO_TICKS(flSimulationTime + flLerpTime) < nDeadTime)
			return false;

		return true;
	}

	float GetLerpTime() {
		int cl_interpolate = m_cvar()->FindVar("cl_interpolate")->GetInt();

		if (!cl_interpolate)
			return 0.f;

		const auto update_rate = m_cvar()->FindVar("cl_updaterate")->GetInt();
		const auto cl_interp = m_cvar()->FindVar("cl_interp")->GetInt();

		auto cl_interp_ratio = m_cvar()->FindVar("cl_interp_ratio");
		if (cl_interp_ratio->GetFloat() == 0)
			cl_interp_ratio->SetValue(1.0f);

		auto interp_min = m_cvar()->FindVar("sv_client_min_interp_ratio");
		auto interp_max = m_cvar()->FindVar("sv_client_max_interp_ratio");

		if (interp_min->GetFloat() != -1)
		{
			float min_max_interp_ratio{ };

			min_max_interp_ratio = std::clamp(min_max_interp_ratio, interp_min->GetFloat(), interp_max->GetFloat());

			cl_interp_ratio->SetValue(min_max_interp_ratio);
		}
		else
		{
			if (cl_interp_ratio->GetFloat() == 0)
				cl_interp_ratio->SetValue(1.0f);
		}

		return fmaxf(cl_interp, cl_interp_ratio->GetFloat() / update_rate);
	}

	//bool valid() {
	//	// no lagcomp = we always have valid tick
	//	// but not for old one
	//	/*if (!CVars::cl_lagcompensation->get_int())
	//		return true;*/

	///*	if (!this)
	//		return false;

	//	if (i > 0)
	//		player = (player_t*)m_entitylist()->GetClientEntity(i);

	//	if (!player)
	//		return false;*/

	//	if (!this)
	//		return false;

	//	if (!player)
	//		return false; 

	//	if (player->m_lifeState() != LIFE_ALIVE)
	//		return false;

	//	if (immune)
	//		return false;

	//	if (dormant)
	//		return false;


	//	float range = .2f;
	//	float max_unlag = .2f;

	//	auto netchannel = m_engine()->GetNetChannelInfo();
	//	if (!netchannel || invalid || m_bHasBrokenLC)
	//		return false;

	//	const auto correct = std::clamp(netchannel->GetLatency(FLOW_INCOMING)
	//		+ netchannel->GetLatency(FLOW_INCOMING)
	//		+ GetLerpTime(), 0.f, max_unlag);

	//	float curtime = g_ctx.local()->is_alive() ? TICKS_TO_TIME(g_ctx.globals.fixed_tickbase) : m_globals()->m_curtime;
	//	//float curtime = m_globals()->m_curtime;
	//	auto result = std::fabs(correct - (curtime - simulation_time));
	//	return  result < range;
	//}

	bool valid()
	{
		if (!this)
			return false;

		if (i > 0)
			player = (player_t*)m_entitylist()->GetClientEntity(i);

		if (!player)
			return false;

		if (player->m_lifeState() != LIFE_ALIVE)
			return false;

		if (immune)
			return false;

		if (dormant)
			return false;

		if (invalid || m_bHasBrokenLC)
			return false;
		

		auto net_channel_info = m_engine()->GetNetChannelInfo();

		if (!net_channel_info)
			return false;

		static auto sv_maxunlag = m_cvar()->FindVar(crypt_str("sv_maxunlag"));

		auto outgoing = net_channel_info->GetLatency(FLOW_OUTGOING);
		auto incoming = net_channel_info->GetLatency(FLOW_INCOMING);

		auto correct = math::clamp(outgoing + incoming + util::get_interpolation(), 0.0f, sv_maxunlag->GetFloat());

		auto curtime = g_ctx.local()->is_alive() ? TICKS_TO_TIME(g_ctx.globals.fixed_tickbase) : m_globals()->m_curtime;
		//auto curtime = m_globals()->m_curtime;
		auto delta_time = correct - (curtime - simulation_time);

		if (fabs(delta_time) > 0.2f)
			return false;

		auto extra_choke = 0;

		if (g_ctx.globals.fakeducking)
			extra_choke = 14 - m_clientstate()->iChokedCommands;

		auto server_tickcount = extra_choke + m_globals()->m_tickcount + TIME_TO_TICKS(outgoing + incoming);
		auto dead_time = (int)(TICKS_TO_TIME(server_tickcount) - sv_maxunlag->GetFloat());

		if (simulation_time < (float)dead_time)
			return false;

		return true;
	}
};

class optimized_adjust_data
{
public:
	int i;
	player_t* player;

	float simulation_time;
	float duck_amount;

	Vector angles;
	Vector origin;
	bool shot;
	int shot_tick;
	optimized_adjust_data()
	{
		reset();
	}

	void reset()
	{
		i = 0;
		player = nullptr;

		simulation_time = 0.0f;
		duck_amount = 0.0f;
		shot = false;
		shot_tick = 0;
		angles.Zero();
		origin.Zero();
	}
};

extern std::deque <adjust_data> player_records[65];
class DesyncCorrection : public singleton <DesyncCorrection>
{
public:
	AnimationLayer ResolverLayers[7][13];
	float LockSide = 0.0f;

	void Run(adjust_data* Record, player_t* Player);
	void SetMode(adjust_data* Record, player_t* Player);
	void GetSide(adjust_data* Record, player_t* Player);
};
class lagcompensation
{
public:
	void apply_interpolation_flags(player_t* e);

	void HandleTickbaseShift(player_t* pPlayer, adjust_data* record);

	
	void CleanPlayer(player_t* pPlayer, adjust_data* record);

	int GetRecordPriority(adjust_data* m_Record);
	adjust_data* FindBestRecord(player_t* pPlayer, std::deque<adjust_data>* m_LagRecords, int& nPriority, const float& flSimTime);
	void extrapolate(player_t* player, Vector& origin, Vector& velocity, int& flags, bool wasonground);
	void ProccessShiftingPlayers(player_t* e, adjust_data* record, adjust_data* previous_record);
	void fsn(ClientFrameStage_t stage);
	void ExtraPolate(adjust_data* record);
	void RecalculateVelocity(player_t* pPlayer, adjust_data* LagRecord, adjust_data* PreviousRecord, C_CSGOPlayerAnimationState* animstate);
	bool valid(int i, player_t* e);
	adjust_data* FindFirstRecord(player_t* pPlayer, std::deque<adjust_data>* m_LagRecords);

	struct AnimationData_t {
		float m_flPrimaryCycle;
		float m_flMovePlaybackRate;
		float m_flFeetWeight;
		float m_flVelocityLengthXY;
		int m_iMoveState;
	};

	AnimationData_t m_animation_data[65];
	void UpdatePlayerAnimations(player_t* pPlayer, adjust_data* m_LagRecord, C_CSGOPlayerAnimationState* m_AnimationState);
	float BuildFootYaw(player_t* pPlayer, adjust_data* m_LagRecord);
	void SetupData(player_t* pPlayer, adjust_data* m_Record, adjust_data* m_PrevRecord);
	int DetermineAnimationCycle(player_t* pPlayer, adjust_data* m_LagRecord, adjust_data* m_PrevRecord);
	int DetermineSimulationTicks(player_t* player, adjust_data* record, adjust_data* previous_record);
	void SetBones(matrix3x4_t* matrix, player_t* m_pEntity);
	void CopyCachedMatrix(player_t* player, matrix3x4_t* matrix);
	void OnUpdateClientSideAnimations(player_t* player);
	void setup_matrix(player_t* e, const int& matrix, adjust_data* record);
	void SimulatePlayerActivity(player_t* pPlayer, adjust_data* m_LagRecord, adjust_data* m_PrevRecord);
	float ComputeActivityPlayback(player_t* pPlayer, adjust_data* m_Record);
	void SimulatePlayerAnimations(player_t* e);
	void FixPvs(player_t* e);
	void SetupCollision(player_t* pPlayer, adjust_data* m_LagRecord);
	float land_time = 0.0f;
	bool land_in_cycle = false;
	bool is_landed = false;
	bool on_ground = false;
	Vector DeterminePlayerVelocity(player_t* pPlayer, adjust_data* m_LagRecord, adjust_data* m_PrevRecord, C_CSGOPlayerAnimationState* m_AnimationState);
	void HandleDormancyLeaving(player_t* pPlayer, adjust_data* m_Record, C_CSGOPlayerAnimationState* m_AnimationState);
	CResolver player_resolver[65];
	DesyncCorrection c_DesyncCorrection[65];
	bool is_dormant[65];
	float previous_goal_feet_yaw[65];

	bool m_CachedMatrixRetr[65];
	Vector m_BoneOrigins[65][MAXSTUDIOBONES];
	matrix3x4_t m_CachedMatrix[65][MAXSTUDIOBONES];
};

inline lagcompensation* g_Lagcompensation = new lagcompensation();
