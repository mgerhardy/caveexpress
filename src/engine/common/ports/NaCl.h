#pragma once

#include "engine/common/Config.h"
#include "engine/common/Common.h"
#include "engine/common/ports/ISystem.h"
#include <string>
#include <set>

class NaCl: public ISystem {
protected:
	std::string _user;

	void mountDir(const std::string& src, const std::string& target, const std::string& filesystem = "httpfs", const std::string& filesystemParams = "");

public:
	NaCl ();
	virtual ~NaCl ();

	// ISystem implementation
	virtual std::string getCurrentWorkingDir () override;

	virtual std::string getCurrentUser () override;

	virtual std::string getHomeDirectory () override;

	virtual std::string getDatabaseDirectory () override;

	virtual std::string normalizePath (const std::string& path) override;

	virtual void exit (const std::string& reason, int errorCode) override;

	virtual bool mkdir (const std::string& directory) override;

	virtual DirectoryEntries listDirectory (const std::string& basedir, const std::string& subdir = "") override;

	virtual int openURL (const std::string& url, bool newWindow) const override;

	virtual int exec (const std::string& command, std::vector<std::string>& arguments) const override;
};
