#ifndef ZANDIAFY__CPP
#define ZANDIAFY__CPP

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "imgui.h"
#include <string>
#include <vector>
#include <iostream>

namespace OBJ {
	class Music {
	private:
		std::string name = "Unknown";
		std::string author = "Unknown";
		ma_uint64 duration = 0;
		UINT64 id;
		std::string path = "";


	public:

		Music(std::string name, std::string author, std::string path) {
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
		void SetDuration(UINT64 duration) {
			this->duration = duration;
		}
		void setDuration(ma_uint64 duration) {
			this->duration = duration;
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
		std::string GetPath() {
			return this->path;
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
		Playlist playlist = Playlist("Default", "Unknown");
		Music currentMusic = Music("Unknown", "Unknown", "");
		
		ma_engine engine = { NULL };
		ma_sound currentSound = { NULL };

		ma_uint64 currentTime = 0;
		ma_uint64 cursor = 0;
		
		ma_uint64 currentLength = 0;
		ma_uint64 lengthInFrames = 0;

		bool isPlaying = false;
		UINT8 volume = 100;

		float sampleRate = 0;

	public:
		void Play() {
			if (!isPlaying) {
				if (ma_sound_start(&currentSound) != MA_SUCCESS) {
					std::cout << "Failed to start sound: " << currentMusic.GetPath() << std::endl;
					return;
				}

				std::cout << "Playing -> " << currentMusic.GetName() << " by " << currentMusic.GetAuthor() << std::endl;
			}
		}

		void Stop() {
			if (isPlaying) {
				if (ma_sound_stop(&currentSound) != MA_SUCCESS) {
					std::cout << "Failed to stop sound: " << currentMusic.GetPath() << std::endl;
					return;
				}
				std::cout << "Stopped -> " << currentMusic.GetName() << " by " << currentMusic.GetAuthor() << std::endl;
			}
		}

		void UpdateAudio() {
			sampleRate = ma_engine_get_sample_rate(&engine);
			ma_sound_get_cursor_in_pcm_frames(&currentSound, &cursor);
			currentTime = cursor / sampleRate;
			isPlaying = ma_sound_is_playing(&currentSound);

			if (!isPlaying) {
				currentTime = 0;
				currentMusic = Music("NO MUSIC", "Zandiano", "");
			}

			ma_engine_set_volume(&engine, volume / 100.0f);
		}

		void SetCurrentMusic(Music music) {
			ma_sound_uninit(&currentSound);
			this->currentMusic = music;

			if (ma_sound_init_from_file(&engine, currentMusic.GetPath().c_str(), 0, NULL, NULL, &currentSound) != MA_SUCCESS) {
				std::cout << "Failed to load sound: " << currentMusic.GetPath() << std::endl;
			}

			ma_sound_get_length_in_pcm_frames(&currentSound, &lengthInFrames);
			currentLength = lengthInFrames / sampleRate;
			this->currentMusic.setDuration(currentLength);
		}

		Music GetCurrentMusic() {
			return this->currentMusic;
		}

		ma_uint64 GetCurrentMusicTime() {
			return this->currentTime;
		}

		UINT8* GetVolume() {
			return &this->volume;
		}

		ma_engine* GetEngine() {
			return &engine;
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

	public:
		void QueueMusic(Music music) {
			if (tail == nullptr) {
				tail = new QueueItem{ music, nullptr, nullptr };
				head = tail;
				current = tail;

				std::cout << "FQueued-> " << music.GetName() << std::endl;
				return;
			}
			
			QueueItem *newItem = new QueueItem{ music, nullptr, head };
			head = newItem;

			std::cout << "Queued-> " << music.GetName() << std::endl;
		}

		void RemoveMusic() {
			if (!current->next) {
				std::cout << "No music to remove" << std::endl;
				return;
			}

			if (current->prev) {
				current->prev->next = current;
				if (head == current) {
					head == current->next;
				}
			}
			if (current->next) {
				current->next->prev = current->prev;
				
				if (tail == current) {
					tail == current->next;
				}
			}

			std::cout << "Removed-> " << current->music.GetAuthor() << std::endl;

			QueueItem *temp = current->next;
			delete current;
			current = temp;
		}

		void QueueMusicNow(Music music) {

		}
	};
}

namespace App {

	class App {
	private:
	public:
		OBJ::Player player;

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

		ImGui::End();


		float music_Width = ImGui::GetIO().DisplaySize.x / 3;
		float music_Height = ImGui::GetIO().DisplaySize.y / 3;

		float music_PosX = music_Width * 2 + 2;
		float music_PosY = music_Height * 2 - 1;

		ImGui::SetNextWindowPos(ImVec2(music_PosX, music_PosY));
		ImGui::SetNextWindowSize(ImVec2(music_Width, music_Height + 2));
		ImGui::Begin("MUSICA", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		
		ImDrawList* draw = ImGui::GetWindowDrawList();

		ImVec2 music_circleCenter = ImVec2(music_PosX + music_Width / 2, music_PosY + music_Height / 2);
		draw->AddCircleFilled(music_circleCenter, 100.0f, IM_COL32(255, 0, 0, 255));
		draw->AddCircleFilled(music_circleCenter, 40.0f, IM_COL32(50, 50, 50, 100));

		ImGui::Dummy(ImVec2(0, music_Height - 80));

		if (ImGui::Button("Play")) {
			OBJ::Music test = OBJ::Music("20th Century Schizoid Man", "King Crimson", "..\\..\\..\\musics\\20thCenturySchizoidMan.mp3");
			app.player.SetCurrentMusic(test);
			app.player.Play();
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop")) {
			app.player.Stop();
		}
		
		ImGui::SetNextItemWidth(200.0f);
		ImGui::SliderInt("##Volume", (int*)app.player.GetVolume(), 0, 200);

		ImGui::Text(app.player.GetCurrentMusic().GetName().c_str());
		ImGui::SameLine();

		ma_uint64 totalSeconds = app.player.GetCurrentMusic().GetDuration();
		ma_uint64 currentSeconds = app.player.GetCurrentMusicTime();

		ImGui::Text("%02d : %02d / %02d : %02d", (int)(currentSeconds / 60), (int)(currentSeconds % 60), (int)(totalSeconds / 60), (int)(totalSeconds % 60));

		app.player.UpdateAudio();

		ImGui::End();

		

	}
}

#endif