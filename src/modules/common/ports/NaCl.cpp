#include "NaCl.h"
#include <SDL.h>
#include "common/Config.h"
#include "common/ConfigManager.h"
#include "common/String.h"
#include "common/System.h"
#include "common/DateUtil.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/mount.h>
#include <nacl_io/nacl_io.h>

namespace {
static const std::string ROOT = "/";
static const std::string HOMEDIR = "/persistent/";
}

NaCl::NaCl() :
		ISystem()
{
	mountDir("", HOMEDIR, "html5fs", "type=PERSISTENT,expected_size=1048576");
}

NaCl::~NaCl ()
{
}

void NaCl::mountDir(const std::string& src, const std::string& target, const std::string& filesystem, const std::string& filesystemParams)
{
	const int retVal = mount(src.c_str(), target.c_str(), filesystem.c_str(), 0, filesystemParams.c_str());
	logOutput("mounting dir: '" + src + "' as target '" + target + "' with return value: " + string::toString(retVal) + "\n");
	if (retVal == -1)
		perror("mountDir failed");
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
	if (!mkdir(HOMEDIR))
		logError("Failed to create the homedir at " + HOMEDIR);
	return HOMEDIR;
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

int NaCl::openURL (const std::string& url, bool newWindow) const
{
	return -1;
}

int NaCl::exec (const std::string& command, std::vector<std::string>& arguments) const
{
	return -1;
}
