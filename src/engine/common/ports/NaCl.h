#pragma once

#include "engine/common/Config.h"
#include "engine/common/Common.h"
#include "engine/common/ports/ISystem.h"
#include <string>
#include <set>

class NaCl: public ISystem {
protected:
	std::string _user;

	void mountDir(const std::string& dir);

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

	virtual int openURL (const std::string& url) const override;

	virtual int exec (const std::string& command, std::vector<std::string>& arguments) const override;

	virtual void logError (const std::string& error) const override;

	virtual void logOutput (const std::string& string) const override;
};
