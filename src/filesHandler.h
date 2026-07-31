#ifndef _FILEHANDLER__
#define _FILEHANDLER__

#include <filesystem>
#include <windows.h>
#include <shobjidl.h>
#include <iostream>

namespace Files {
	class fileHandler {
	private:
		std::filesystem::path musicPath;
		std::filesystem::path folderPath{ "..\\..\\..\\musics" };

		std::filesystem::path GetFilePath() {
			HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
			PWSTR path = nullptr;

			if (FAILED(hr)) {
				CoUninitialize();
				return {};
			}

			IFileOpenDialog* pFileOpen = nullptr;

			hr = CoCreateInstance(
				CLSID_FileOpenDialog,
				NULL,
				CLSCTX_ALL,
				IID_IFileOpenDialog,
				(void**)&pFileOpen
			);

			if (FAILED(hr)) {
				CoUninitialize();
				return {};
			}

			hr = pFileOpen->Show(NULL);

			if (FAILED(hr)) {
				CoUninitialize();
				pFileOpen->Release();
				return {};
			}

			IShellItem* pItem;

			hr = pFileOpen->GetResult(&pItem);

			if (FAILED(hr)) {
				CoUninitialize();
				pFileOpen->Release();
				pItem->Release();
				return {};
			}

			pItem->GetDisplayName(SIGDN_FILESYSPATH, &path);

			std::wcout << path << std::endl;

			std::filesystem::path filepath(path);

			CoTaskMemFree(path);
			pItem->Release();
			pFileOpen->Release();
			CoUninitialize();

			return filepath;
		}

		void SetMusicPath() {
			musicPath = GetFilePath();
		}

	public:
		void CopyFileToFolder(std::filesystem::path file) {
			if (!std::filesystem::exists(file)) {
				return;
			}

			std::filesystem::copy_file(file, folderPath / file.filename(), std::filesystem::copy_options::overwrite_existing);
		}

		std::filesystem::path GetMusicPath() {
			SetMusicPath();
			std::wcout << musicPath << std::endl;
			return musicPath;
		}

		std::string GetFileName() {
			return musicPath.filename().string();
		}
	};
}
#endif