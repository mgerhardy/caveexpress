#pragma once

#include "engine/common/Common.h"
#include "engine/common/ports/ISystem.h"

class Windows: public ISystem {
public:
	Windows ();
	virtual ~Windows ();

	std::string getCurrentWorkingDir () override;
	std::string getCurrentUser () override;
	std::string getHomeDirectory () override;
	std::string getDatabaseDirectory () override;
	std::string normalizePath (const std::string& path) override;
	void exit (const std::string& reason, int errorCode) override;
	bool mkdir (const std::string& directory) override;
	DirectoryEntries listDirectory (const std::string& basedir, const std::string& subdir = "") override;
	int openURL (const std::string& url) const override;
	int exec (const std::string& command, std::vector<std::string>& arguments) const override;
};
