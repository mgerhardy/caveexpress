#include <SDL.h>

void CGameCenter_ShowLeaderBoard (void)
{
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);

	SDL_Window* window = SDL_GetFocusWindow();
	if (!SDL_GetWindowWMInfo(window, &info))
		return;

	UIWindow* uiWindow = (UIWindow*) info.info.uikit.window;
	UIView* view = [uiWindow.subviews objectAtIndex:0];
	id nextResponder = [view nextResponder];
	if ([nextResponder isKindOfClass:[UIViewController class]]) {
		UIViewController* viewController = (UIViewController*) nextResponder;

		// You can do some terrifying things with Objective-C
		//NSString* className = [NSString stringWithUTF8String:viewName];

		GKLeaderboardViewController *leaderboardController =
		[[GKLeaderboardViewController alloc] init];
		if (leaderboardController != NULL) {
			//leaderboardController.category = self.currentLeaderBoard;
			leaderboardController.timeScope = GKLeaderboardTimeScopeAllTime;
			leaderboardController.leaderboardDelegate = pGameCenterManager;

			[viewController presentModalViewController:
			leaderboardController animated:YES];
		}
	}
}
