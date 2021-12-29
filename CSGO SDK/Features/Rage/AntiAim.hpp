#pragma once
#include "../../SDK/sdk.hpp"

class freestanding
{
public:
	bool override(float& yaw) const;
	bool get_real(float& yaw);
	void get_targets();
	void update_hotkeys(ClientFrameStage_t stage);
	static float trace_freestanding(C_CSPlayer* player, const Vector point);
	int get_mode() const { return direction; }
	float last_fs_time = 0.f;
private:
	int direction = -1;
	std::vector<C_CSPlayer*> players;
};

namespace Interfaces
{

   class __declspec( novtable ) AntiAimbot : public NonCopyable {
   public:
	  static Encrypted_t<AntiAimbot> Get( );
	  virtual void Main( bool* bSendPacket, bool* bFinalPacket, Encrypted_t<CUserCmd> cmd, bool ragebot ) = 0;
	  virtual void PrePrediction( bool* bSendPacket, Encrypted_t<CUserCmd> cmd ) = 0;
	  virtual void ImposterBreaker( bool* bSendPacket, Encrypted_t<CUserCmd> cmd ) = 0;
	  virtual void fake_duck(bool* bSendPacket, Encrypted_t<CUserCmd> cmd) = 0;
   protected:
	  AntiAimbot( ) { };
	  virtual ~AntiAimbot( ) { };
   };
}
