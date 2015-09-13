#include "IOS.h"
#include "common/Application.h"

IOS::IOS () :
		Unix()
{
}

IOS::~IOS ()
{
}

std::string IOS::getHomeDirectory ()
{
	return "";
}

void IOS::showAds (bool show)
{
	// TODO:
}

int IOS::openURL (const std::string& url, bool) const
{
	// TODO: move in mm file
#if 0
	@autoreleasepool
	{
		UIApplication *app = [UIApplication sharedApplication];
		NSURL *nsurl = [NSURL URLWithString:@(url.c_str())];

		if ([app canOpenURL:nsurl]) {
			const bool success = [app openURL:nsurl];
			return success ? 0 : -1;
		}
		return -1;
	}
#endif

	const std::string cmd = "open " + url;
	system(cmd.c_str());
	return 0;
}
