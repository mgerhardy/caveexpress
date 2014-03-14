#pragma once

class IRunnable {
public:
	IRunnable ()
	{
	}
	virtual ~IRunnable ()
	{
	}
	virtual int run () = 0;
};
