#pragma once

#include "common/Config.h"
#include "common/Common.h"
#include "ISystem.h"
#include <string>
#include <set>

class Unix: public ISystem {
protected:
	std::string _user;

public:
	Unix ();
	virtual ~Unix ();

	virtual std::string getCurrentUser () override;

	virtual std::string normalizePath (const std::string& path) override;

	virtual void exit (const std::string& reason, int errorCode) override;

	virtual bool mkdir (const std::string& directory) override;

	virtual DirectoryEntries listDirectory (const std::string& basedir, const std::string& subdir = "") override;

	virtual int openURL (const std::string& url, bool newWindow) const override;

	virtual int exec (const std::string& command, std::vector<std::string>& arguments) const override;

	virtual void backtrace (const char *errorMessage) override;

	virtual std::string getRateURL (const std::string& packageName) const override;

#ifdef DEBUG
public:
	virtual int getScreenPadding () override;
#endif
};
