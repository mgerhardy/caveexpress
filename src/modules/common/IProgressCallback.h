#pragma once

#include <string>

class IProgressCallback {
public:
	IProgressCallback ()
	{
	}

	virtual ~IProgressCallback ()
	{
	}

	virtual void progressInit (int steps, const std::string& text = "") = 0;
	virtual void progressStep (const std::string& text = "") = 0;
	virtual void progressDone () = 0;
};
