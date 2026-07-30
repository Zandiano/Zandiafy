#ifndef _FILEHANDLER__
#define _FILEHANDLER__

#include <filesystem>
#include <windows.h>
#include <shobjidl.h>

namespace Files {
	class fileHandler {
	private:
		std::filesystem::path musicPath;

	public:
		void SetMusicPath(std::filesystem::path newPath) {
			musicPath = newPath;
		}

		std::filesystem::path GetMusicPath() {
			return musicPath;
		}

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
	};
}
#endif