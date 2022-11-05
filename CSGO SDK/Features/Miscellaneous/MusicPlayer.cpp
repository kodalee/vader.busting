#include "MusicPlayer.hpp"

namespace Interfaces
{

	void MusicPlayer::init(void) {
		/// folder the wav files have to be in
		//m_sound_files_path = "D:\\_Users\\Ô\\Desktop\\ExWare\\csgo\\";

		/// create it if it's not there
		//CreateDirectoryA(m_sound_files_path.c_str(), 0);

		/// setting this to 1 let's us hear the music
		m_voice_loopback = Interfaces::m_pCvar->FindVar(XorStr("voice_loopback"));

		m_playing = false;
		/// find all the sound files in the folder
		//load_sound_files();
	}

	void voice_record_start(const char* uncompressed_file, const char* decompressed_file, const char* mic_input_file) {
		using voice_record_start_t = void(__fastcall*)(const char*, const char*, const char*);
		//offsets->voice_record_start = util::find_pattern( "engine.dll", "55 8B EC 83 EC 0C 83 3D ? ? ? ? ? 56 57" );
		static auto voice_record_start_fn = reinterpret_cast<voice_record_start_t>(Memory::Scan(XorStr("engine.dll"), XorStr("55 8B EC 83 EC 0C 83 3D ? ? ? ? ? 56 57")));

		voice_record_start_fn(mic_input_file, nullptr, nullptr);
	}

	void MusicPlayer::play(std::size_t file_idx) {
		/// is the index valid?
		/*if (file_idx >= m_sound_files.size())
			return;

		/// is the client ingame?
		if (!csgo.m_engine()->IsInGame())
			return;

		/// this makes us hear our own music
		m_voice_loopback->SetValue(1);

		/// use voice chat
		csgo.m_engine()->ExecuteClientCmd("+voicerecord");

		/// the path of the wav file
		auto file = m_sound_files_path + m_sound_files.at(file_idx);

		/// play the wav
		voice_record_start(nullptr, nullptr, file.c_str());*/
	}

	void MusicPlayer::play(std::string file, float delay_time) {
		// is the index valid?
		if (file.length() <= 2 || m_playing)
			return;

		// is the client ingame?
		if (!Interfaces::m_pEngine->IsInGame())
			return;

		time_to_reset_sound = Interfaces::m_pGlobalVars->realtime + delay_time;

		// this makes us hear our own music
		m_voice_loopback->SetValue(1);

		// use voice chat
		Interfaces::m_pEngine->ClientCmd_Unrestricted(XorStr("+voicerecord"));

		// play the wav
		voice_record_start(nullptr, nullptr, file.c_str());

		m_playing = true;
	}

	void MusicPlayer::stop(void) {
		// so we don't hear ourself when we use voice chat normally

		if (m_voice_loopback->GetInt() == 0)
			return; // idk

		m_voice_loopback->SetValue(0);

		/// stop using voice chat
		Interfaces::m_pEngine->ClientCmd_Unrestricted(XorStr("-voicerecord"));
		time_to_reset_sound = 0.f;
		m_playing = false;
	}

	void MusicPlayer::run(void)
	{
		/*if (ctx.get_key_press('N')) {
			if (!m_playing)
				play(ctx.m_settings.music_curtrack);
			else
				stop();

			m_playing = !m_playing;
		}*/
	}

	void MusicPlayer::load_sound_files(void) {
		/*m_sound_files.clear();

		WIN32_FIND_DATAA data;
		auto file_handle = FindFirstFileA((m_sound_files_path + "*.wav").c_str(), &data);
		if (file_handle == INVALID_HANDLE_VALUE)
			return;

		do {
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			m_sound_files.push_back(data.cFileName);
		} while (FindNextFileA(file_handle, &data));*/
	}
}