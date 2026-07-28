#ifndef ZANDIAFY__CPP
#define ZANDIAFY__CPP

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "imgui.h"
#include <string>
#include <vector>
#include <queue>

namespace OBJ {
	class Music {
	private:
		std::string name = "Unknown";
		std::string author = "Unknown";
		UINT64 duration = 0;
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
		void setDuration(UINT64 duration) {
			this->duration = duration;
		}

		std::string GetAuthor() {
			return this->author;
		}
		std::string GetName() {
			return this->name;
		}
		UINT64 GetDuration() {
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
		UINT64 currentTime = 0;
		bool isPlaying;

		ma_engine engine;
		ma_sound currentSound;
	
	public:
		void Play() {
			if (!isPlaying) {
				ma_sound_init_from_file(
					&engine,
					currentMusic.GetPath().c_str(),
					0,
					NULL,
					NULL,
					&currentSound
				);

				ma_sound_start(&currentSound);

				isPlaying = true;
			}
		}

		ma_engine* GetEngine() {
			return &engine;
		}
	};

	class Queue {
	private:
		std::queue<Music> queue;


	public:

	};
}

namespace App {

	class App {
	private:
		OBJ::Player player;
	public:
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

		ImGui::Dummy(ImVec2(0, music_Height - 35));
		ImGui::Text("MUSICA X");
		ImGui::SameLine();
		ImGui::Text("00:00 / 00:00");


		ImGui::End();

		

	}
}

#endif