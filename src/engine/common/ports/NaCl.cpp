#include "NaCl.h"
#include "engine/common/Version.h"
#include <SDL.h>
#include "engine/common/Config.h"
#include "engine/common/ConfigManager.h"
#include "engine/common/String.h"
#include "engine/common/System.h"
#include "engine/common/DateUtil.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

namespace {
static const std::string ROOT = "/";
}

NaCl::NaCl() :
		ISystem()
{
}

NaCl::~NaCl ()
{
}

std::string NaCl::getCurrentWorkingDir ()
{
	return ROOT;
}

std::string NaCl::getCurrentUser ()
{
	return "NaCl";
}

std::string NaCl::getDatabaseDirectory ()
{
	return getHomeDirectory();
}

std::string NaCl::getHomeDirectory ()
{
	const std::string dir = ROOT + APPNAME + "/";
	if (!mkdir(dir))
		return ROOT;
	return dir;
}

std::string NaCl::normalizePath (const std::string& path)
{
	return path;
}

bool NaCl::mkdir (const std::string& directory)
{
	if (directory.empty())
		return false;
	std::string s = directory;
	if (s[s.size() - 1] != '/') {
		// force trailing / so we can handle everything in loop
		s += '/';
	}

	size_t pre = 0, pos;
	while ((pos = s.find_first_of('/', pre)) != std::string::npos) {
		const std::string dir = s.substr(0, pos++);
		pre = pos;
		if (dir.empty())
			continue; // if leading / first time is 0 length
		const char *dirc = dir.c_str();
		const int retVal = ::mkdir(dirc, 0777);
		if (retVal == -1 && errno != EEXIST) {
			return false;
		}
	}
	return true;
}

DirectoryEntries NaCl::listDirectory (const std::string& basedir, const std::string& subdir)
{
	DirectoryEntries entities;
	std::string search(basedir + "/");
	DIR *directory;
	struct dirent *d;
	struct stat st;

	if (!subdir.empty())
		search.append(subdir).append("/");

	if ((directory = ::opendir(search.c_str())) == nullptr)
		return entities;

	while ((d = ::readdir(directory)) != nullptr) {
		std::string name(search);
		name.append("/").append(d->d_name);
		if (::stat(name.c_str(), &st) == -1)
			continue;
		if (st.st_mode & S_IFDIR) {
			if (strcmp(d->d_name, ".") && strcmp(d->d_name, "..")) {
				std::string newsubdirs;
				if (!subdir.empty())
					newsubdirs.append(subdir).append("/");
				newsubdirs.append(d->d_name);
				const DirectoryEntries& subDirEnts = listDirectory(basedir, newsubdirs);
				entities.insert(entities.end(), subDirEnts.begin(), subDirEnts.end());
			}
		} else {
			std::string filename(subdir);
			if (!filename.empty())
				filename.append("/");
			filename.append(d->d_name);
			entities.push_back(filename);
		}
	}

	::closedir(directory);

	return entities;
}

void NaCl::exit (const std::string& reason, int errorCode)
{
	if (errorCode != 0) {
		logError(reason);
		backtrace(reason.c_str());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", reason.c_str(), nullptr);
	} else {
		logOutput(reason);
	}

#ifdef DEBUG
	SDL_TriggerBreakpoint();
#endif
	::exit(errorCode);
}

int NaCl::openURL (const std::string& url) const
{
	return -1;
}

int NaCl::exec (const std::string& command, std::vector<std::string>& arguments) const
{
	return -1;
}
