#ifndef _ZANDIAFY__
#define _ZANDIAFY__

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "imgui.h"

#include <string>
#include <vector>
#include <iostream>
#include <filesystem>

#include "filesHandler.h"

#define EMPTY_MUSIC Music("","",L"")

namespace OBJ {
	class Music {
	private:
		std::string name = "Unknown";
		std::string author = "Unknown";
		ma_uint64 duration = 0;
		UINT64 id;
		std::wstring path = L"";
		bool isValid = true;


	public:

		Music(std::string name, std::string author, std::wstring path) {
			this->name = name;
			this->author = author;
			this->path = path;
		}
		void SetName(std::string name) {
			this->name = name;
		}
		void SetAuthor(std::string author) {
			this->author = author;
		}
		void SetDuration(ma_uint64 duration) {
			this->duration = duration;
		}
		void SetValidity(bool val) {
			isValid = val;
		}
		

		std::string GetAuthor() {
			return this->author;
		}
		std::string GetName() {
			return this->name;
		}
		ma_uint64 GetDuration() {
			return this->duration;
		}
		std::wstring GetPath() {
			return this->path;
		}
		bool GetValidity() {
			return isValid;
		}
	};

	class Playlist {
	private:
		std::string name = "Unknown";
		std::string author = "Unknown";
		UINT64 duration = 0;
		std::vector<Music> musics;

		void TotalDuration() {
			this->duration = 0;
			for (int i = 0; i < musics.size(); i++) {
				this->duration += musics[i].GetDuration();
			}
		}

	public:

		Playlist(std::string name, std::string author) {
			this->name = name;
			this->author = author;
		}

		void Update() {
			TotalDuration();
		}

		void AddMusic(Music music) {
			this->musics.push_back(music);
			Update();
		}

		void RemoveMusic(int index) {
			if (index >= 0 && index < musics.size()) {
				musics.erase(musics.begin() + index);
			}
			Update();
		}

		void ReorderMusic(int fromIndex, int toIndex) {
			if (fromIndex >= 0 && fromIndex < musics.size() && toIndex >= 0 && toIndex < musics.size()) {
				Music music = musics[fromIndex];
				musics.erase(musics.begin() + fromIndex);
				musics.insert(musics.begin() + toIndex, music);
			}
			Update();
		}
	};

	class Player {
	private:
		Music currentMusic = Music("Unknown", "Unknown", L"");
		
		ma_engine engine = { NULL };
		ma_sound currentSound = { NULL };

		ma_uint64 currentTime = 0;
		ma_uint64 cursor = 0;
		
		ma_uint64 currentLength = 0;
		ma_uint64 lengthInFrames = 0;

		float sampleRate = 0;

		bool isPlaying = false;
		bool isPaused = false;
		bool loadedMusic = false;
		UINT8 volume = 100;


	public:
		void Play() {
			std::cout << "Play(1)" << std::endl;
			if ((!isPlaying || isPaused) && loadedMusic) {
				isPaused = false;
				std::cout << "Play(2)" << std::endl;
				if (ma_sound_start(&currentSound) != MA_SUCCESS) {
					std::wcout << "Failed to start sound: " << currentMusic.GetPath() << std::endl;
					return;
				}
				std::cout << "Play(3)" << std::endl;
				ma_sound_seek_to_pcm_frame(&currentSound, cursor);
				std::cout << "Playing -> " << currentMusic.GetName() << " by " << currentMusic.GetAuthor() << std::endl;
			}
		}

		void Stop() {
			if (!isPlaying || !loadedMusic) {
				return;
			}
			
			if (!isPaused) {
				currentTime = 0;
				cursor = 0;
			}

			if (ma_sound_stop(&currentSound) != MA_SUCCESS) {
				std::wcout << "Failed to stop sound: " << currentMusic.GetPath() << std::endl;
				return;
			}
			std::cout << "Stopped -> " << currentMusic.GetName() << " by " << currentMusic.GetAuthor() << std::endl;
		}

		void Pause() {
			isPaused = true;

			if (isPaused) {
				Stop();
			}
		}

		void UpdateAudio() {

			isPlaying = ma_sound_is_playing(&currentSound);
			
			if (!isPaused) {
				sampleRate = ma_engine_get_sample_rate(&engine);
				ma_sound_get_cursor_in_pcm_frames(&currentSound, &cursor);
				currentTime = cursor / sampleRate;
			}

			if (!isPlaying) {
				currentTime = 0;
				currentMusic = Music("NO MUSIC", "Zandiano", L"");
			}

			ma_engine_set_volume(&engine, volume / 100.0f);
		}

		void ResetCurrent() {
			if (!loadedMusic) {
				return;
			}
			cursor = 0;
			currentTime = 0;
			ma_sound_seek_to_pcm_frame(&currentSound, cursor);
		}

		void SetCurrentMusic(Music music) {
			std::cout << "Set(1)" << std::endl;

			this->currentMusic = music;
			
			if (!currentMusic.GetValidity()) {
				std::cout << "Invalid music" << std::endl;
				return;
			}

			if (loadedMusic) {
				ma_sound_uninit(&currentSound);
			}

			std::cout << "Set(2)" << std::endl;

			if (ma_sound_init_from_file_w(&engine, currentMusic.GetPath().c_str(), 0, NULL, NULL, &currentSound) != MA_SUCCESS) {
				std::wcout << "Failed to load sound: " << currentMusic.GetPath() << std::endl;
				currentMusic.SetValidity(false);
				loadedMusic = false;
				return;
			}
			
			loadedMusic = true;
			currentMusic.SetValidity(true);

			std::cout << "Set(3)" << std::endl;

			ma_sound_get_length_in_pcm_frames(&currentSound, &lengthInFrames);
			currentLength = lengthInFrames / sampleRate;
			this->currentMusic.SetDuration(currentLength);
		}

		Music GetCurrentMusic() {
			return this->currentMusic;
		}

		ma_uint64 GetCurrentMusicTime() {
			return this->currentTime;
		}

		ma_uint64 GetCurrentMusicTotalTime() {
			return this->currentLength;
		}

		UINT8* GetVolume() {
			return &this->volume;
		}

		ma_engine* GetEngine() {
			return &engine;
		}

		bool GetPlaying() {
			return isPlaying;
		}
	};

	class Queue {
		struct QueueItem {
			Music music;
			QueueItem *next;
			QueueItem *prev;
		};

	private:
		QueueItem *head = nullptr;
		QueueItem *tail = nullptr;
		QueueItem *current = nullptr;
		bool nextReady = false;
		bool prevReady = false;
		bool isPaused = false;
		bool tryPlaying = false;
		bool tryStop = false;
		bool looping = false;


		bool finishedCurrent = false;

	public:
		void UpdateQueue(Player *player) {
			if(finishedCurrent && !player->GetPlaying()){
				Next();
				finishedCurrent = false;
			}

			if (nextReady || prevReady) {
				tryPlaying = true;

				player->ResetCurrent();

				nextReady = false;
				prevReady = false;
			}

			if (tryPlaying) {
				std::cout << "Try play-> " << current->music.GetName() << std::endl;
				player->Stop();
				player->UpdateAudio();
				player->SetCurrentMusic(current->music);
				player->Play();
				tryPlaying = false;
			}

			if (isPaused) {
				player->Pause();
				isPaused = false;
			}

			if (tryStop) {
				player->Stop();
				tryStop = false;
			}

			if (player->GetCurrentMusicTime() >= player->GetCurrentMusicTotalTime()) {
				finishedCurrent = true;
			}
		}
		
		void Previous() {
			if (!current || !current->prev)
				return;

			current = current->prev;
			std::cout << "Previous-> " << current->music.GetName() << std::endl;
			prevReady = true;
		}
		
		void Next() {
			if (!current || !current->next)
				return;

			current = current->next;
			std::cout << "Next-> " << current->music.GetName() << std::endl;
			nextReady = true;
		}

		void Play() {
			if (!current) {
				return;
			}
			tryPlaying = true;
		}

		void Pause() {
			isPaused = true;
		}

		void Stop() {
			tryStop = true;
		}

		void Looping() {
			if (!head || !tail) {
				std::cout << "Invalid loop" << std::endl;
				return;
			}

			looping = !looping;

			if (looping) {
				head->next = tail;
				tail->prev = head;
				std::cout << "Looping" << std::endl;
			}
			else {
				head->next = nullptr;
				tail->prev = nullptr;
				std::cout << "Unlooping" << std::endl;
			}
		}

		void QueueMusic(Music music) {
			if (tail == nullptr) {
				tail = new QueueItem{ music, nullptr, nullptr };
				head = tail;
				current = tail;

				std::cout << "FQueued-> " << current->music.GetName() << std::endl;
				return;
			}
			
			QueueItem *newItem = new QueueItem{ music, nullptr, head };
			head->next = newItem;
			head = newItem;

			std::cout << "Queued-> " << newItem->music.GetName() << std::endl;
		}

		void RemoveMusic() {
			if (!current) {
				std::cout << "No music to remove" << std::endl;
				return;
			}

			if (current->prev) {
				current->prev->next = current->next;

				if (head == current) {
					head = current->next;
				}
			}
			if (current->next) {
				current->next->prev = current->prev;
				
				if (tail == current) {
					tail = current->next;
				}
			}

			std::cout << "Removed-> " << current->music.GetName() << std::endl;
			
			QueueItem* temp = current->next ? current->next : current->prev;
			delete current;
			current = temp;

			Play();
		}

		void QueueMusicNow(Music music) {
			if (tail == nullptr || current == head) {
				this->QueueMusic(music);
				return;
			}

			QueueItem* newItem = new QueueItem { music, current->next, current };
			current->next->prev = newItem;
			current->next = newItem;
		}

		OBJ::Music* GetCurrentMusic() {
			if (!current || !current->next) {
				return nullptr;
			}
			return &current->next->music;
		}

		OBJ::Music* GetNextMusic() {
			if (!current) {
				return nullptr;
			}
			return &current->music;
		}

		OBJ::Music* GetPreviousMusic() {
			if (!current || !current->prev) {
				return nullptr;
			}
			return &current->prev->music;
		}
	};
}

namespace AUXFUNCS {
	void MusicDiscs(ImDrawList* draw, ImVec2 circle_center, float radius, OBJ::Music* music, int tag) {
		
		ImGui::SetCursorPos(circle_center);
		draw->AddCircleFilled(ImGui::GetCursorScreenPos(), radius, IM_COL32(255, 0, 0, 255));
		draw->AddCircleFilled(ImGui::GetCursorScreenPos(), radius * 0.4f, IM_COL32(50, 50, 50, 100));
		
		if (music == nullptr) {
			return;
		}

		std::string disc_name = music->GetName().c_str();
		ImGui::SetCursorPos(ImVec2(circle_center.x - ImGui::CalcTextSize(disc_name.c_str()).x / 2.0f, circle_center.y));
		ImGui::Text(disc_name.c_str());
	}
}

namespace App {

	class App {
	private:
	public:
		OBJ::Player player;
		OBJ::Queue queue;
		Files::fileHandler fileHandler;

		void Init() {
			if (ma_engine_init(NULL, player.GetEngine()) != MA_SUCCESS)
			{
				exit(-1);
			}
		}

		void Uninit() {
			ma_engine_uninit(player.GetEngine());
		}
	};

	App app;

	void Update() {

		float arq_Width = ImGui::GetIO().DisplaySize.x / 3;
		float arq_Height = ImGui::GetIO().DisplaySize.y / 3 * 2;

		ImGui::SetNextWindowPos(ImVec2(arq_Width*2 + 2, 0));
		ImGui::SetNextWindowSize(ImVec2(arq_Width, arq_Height));
		ImGui::Begin("ARQUIVOS", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize  | ImGuiWindowFlags_NoTitleBar);
		
		ImGui::Text("ARQUIVOS");
		ImGui::SameLine();
		if (ImGui::Button("+##ADDFILE")) {
			app.fileHandler.CopyFileToFolder(app.fileHandler.GetMusicPath());
		}
		ImGui::Dummy(ImVec2(0, 10));


		for (const auto& file : std::filesystem::directory_iterator(app.fileHandler.GetFolderPath())) {
			std::string text = file.path().filename().string();
			if (ImGui::Button(text.c_str())) {
				app.queue.QueueMusic(OBJ::Music(text, "Random", file.path()));
			};
		}

		ImGui::End();


		float music_Width = ImGui::GetIO().DisplaySize.x / 3;
		float music_Height = ImGui::GetIO().DisplaySize.y / 3;

		float music_PosX = music_Width * 2 + 2;
		float music_PosY = music_Height * 2 - 1;

		ImGui::SetNextWindowPos(ImVec2(music_PosX, music_PosY));
		ImGui::SetNextWindowSize(ImVec2(music_Width, music_Height + 2));
		ImGui::Begin("MUSICA", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		
		ImDrawList* draw = ImGui::GetWindowDrawList();

		ImVec2 music_circleCenter = ImVec2(music_Width / 2, music_Height / 2);
		AUXFUNCS::MusicDiscs(draw, music_circleCenter, 100.0f, app.queue.GetCurrentMusic(), 0);
		
		AUXFUNCS::MusicDiscs(draw, ImVec2(music_circleCenter.x + 160, music_circleCenter.y + 20), 50.0f, app.queue.GetNextMusic(), 1);

		AUXFUNCS::MusicDiscs(draw, ImVec2(music_circleCenter.x - 160, music_circleCenter.y + 20), 50.0f, app.queue.GetPreviousMusic(), 2);

		ImGui::SetCursorPos(ImVec2(5, music_Height - 75));
		ImGui::Dummy(ImVec2(0,0));

		app.player.UpdateAudio();
		app.queue.UpdateQueue(&app.player);

		if (ImGui::Button("Play")) {
			app.queue.Play();
		}
		ImGui::SameLine();
		if (ImGui::Button("Pause")) {
			app.queue.Pause();
		}
		ImGui::SameLine();
		if (ImGui::Button("X##Remove")) {
			app.queue.RemoveMusic();
		}
		ImGui::SameLine();
		if (ImGui::Button("<##Prev")) {
			app.queue.Previous();
		}
		ImGui::SameLine();
		if (ImGui::Button(">##Next")) {
			app.queue.Next();
		}
		ImGui::SameLine();
		if (ImGui::Button("~##Loop")) {
			app.queue.Looping();
		}

		UINT8 max = 200;
		UINT8 min = 0;

		ImGui::SetNextItemWidth(200.0f);
		ImGui::SliderScalar("##Volume", ImGuiDataType_U8, (UINT8*) app.player.GetVolume(), &min, &max);

		ImGui::Text(app.player.GetCurrentMusic().GetName().c_str());
		ImGui::SameLine();

		ma_uint64 totalSeconds = app.player.GetCurrentMusic().GetDuration();
		ma_uint64 currentSeconds = app.player.GetCurrentMusicTime();

		ImGui::Text("%02d : %02d / %02d : %02d", (int)(currentSeconds / 60), (int)(currentSeconds % 60), (int)(totalSeconds / 60), (int)(totalSeconds % 60));


		ImGui::End();
	}
}

#endif