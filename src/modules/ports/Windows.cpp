#include "Windows.h"
#include "common/Logger.h"
#include "common/Application.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <fcntl.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#include <mmsystem.h>
#include <SDL_messagebox.h>
#include <iterator>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

Windows::Windows ()
{
}

Windows::~Windows ()
{
}

bool Windows::mkdir (const std::string& directory)
{
	if (directory.empty())
		return false;
	const bool bCD = CreateDirectory(directory.c_str(), nullptr);
	if (bCD)
		return true;

	DWORD dwLastError = GetLastError();
	switch (dwLastError) {
	case ERROR_ALREADY_EXISTS:
		return true;
	case ERROR_PATH_NOT_FOUND: {
		std::size_t sep = directory.rfind('/');
		if (sep == std::string::npos)
			sep = directory.rfind('\\');
		if (sep == std::string::npos)
			return false;
		const std::string szPrev = directory.substr(0, sep);
		if (mkdir(szPrev)) {
			const bool bSuccess = CreateDirectory(directory.c_str(), nullptr) != 0;
			if (bSuccess)
				return true;
			return GetLastError() == ERROR_ALREADY_EXISTS;
		}
		return false;
	}
	default:
		return false;
	}
}

std::string Windows::getCurrentUser ()
{
	char userName[1024];
	unsigned long size = sizeof(userName);

	if (!GetUserName(userName, &size))
		strcpy(userName, "unknown");

	if (userName[0] == '\0') {
		strcpy(userName, "unknown");
	}
	return std::string(userName);
}

std::string Windows::getCurrentWorkingDir ()
{
	char cwd[256];

	if (_getcwd(cwd, sizeof(cwd) - 1) == nullptr) {
		return "";
	}
	cwd[sizeof(cwd) - 1] = '\0';

	return std::string(cwd) + "/";
}

std::string Windows::normalizePath (const std::string& path)
{
	String tmp = path;
	return tmp.toLower().replaceAll("\\", "/");
}

std::string Windows::getDatabaseDirectory ()
{
	return getHomeDirectory();
}

typedef HRESULT (WINAPI *GetFolderPath_t) (HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags, LPTSTR pszPath);

std::string Windows::getHomeDirectory ()
{
	TCHAR pathBuf[MAX_PATH];
	std::string path;
	HMODULE shfolder;

	shfolder = LoadLibrary("shfolder.dll");
	if (shfolder == nullptr) {
		error(LOG_SYSTEM, "Unable to load SHFolder.dll");
		return "";
	}

	GetFolderPath_t getFolderPath = (GetFolderPath_t) GetProcAddress(shfolder, "SHGetFolderPathA");
	if (getFolderPath == nullptr) {
		error(LOG_SYSTEM, "Unable to find SHGetFolderPathA in SHFolder.dll");
		FreeLibrary(shfolder);
		return "";
	}

	if (!SUCCEEDED(getFolderPath(nullptr, CSIDL_APPDATA | CSIDL_FLAG_CREATE, nullptr, 0, pathBuf))) {
		error(LOG_SYSTEM, "Unable to detect CSIDL_APPDATA");
		FreeLibrary(shfolder);
		return "";
	}

	path.append(pathBuf).append("\\" + Singleton<Application>::getInstance().getName() + "\\");
	FreeLibrary(shfolder);

	if (!mkdir(path)) {
		error(LOG_SYSTEM, String::format("Unable to create directory \"%s\"", path.c_str()));
		return "";
	}
	return path;
}

DirectoryEntries Windows::listDirectory (const std::string& basedir, const std::string& subdir)
{
	DirectoryEntries entities;
	std::string search(basedir + "\\");
	int findhandle;
	struct _finddata_t findinfo;

	if (!subdir.empty()) {
		search.append(subdir).append("\\");
	} else {
		search.append("*");
	}

	findhandle = _findfirst(search.c_str(), &findinfo);
	if (findhandle == -1)
		return entities;

	do {
		if (findinfo.attrib & _A_SUBDIR) {
			if (strcmp(findinfo.name, ".") && strcmp(findinfo.name, "..")) {
				std::string newsubdirs;
				if (!subdir.empty())
					newsubdirs.append(subdir).append("\\");
				newsubdirs.append(findinfo.name);
				DirectoryEntries subDirEnts = listDirectory(basedir, newsubdirs);
				entities.insert(entities.end(), subDirEnts.begin(), subDirEnts.end());
			}
		} else {
			std::string filename(subdir);
			if (!filename.empty())
				filename.append("\\");
			filename.append(findinfo.name);
			entities.push_back(filename);
		}
	} while (_findnext(findhandle, &findinfo) != -1);

	_findclose(findhandle);

	return entities;
}

void Windows::exit (const std::string& reason, int errorCode)
{
	if (errorCode != 0) {
		logError(reason);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", reason.c_str(), nullptr);
	} else {
		logOutput(reason);
	}
	ExitProcess(errorCode);
}

int Windows::openURL (const std::string& url, bool) const
{
	ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
	return 0;
}

int Windows::exec (const std::string& command, std::vector<std::string>& arguments) const
{
	std::stringstream ss;
	ss << command << " ";
	std::copy(arguments.begin(), arguments.end(), std::ostream_iterator<std::string>(ss, " "));

	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInfo;

	if (!CreateProcess(nullptr, (LPSTR) ss.str().c_str(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, NULL, nullptr,
			&startupInfo, &processInfo)) {
		return -1;
	}

	WaitForSingleObject(processInfo.hProcess, INFINITE);

	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);

	return 0;
}
