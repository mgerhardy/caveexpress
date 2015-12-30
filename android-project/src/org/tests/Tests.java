package org.caveexpress;

import org.base.game.BaseGameActivity;

public class Tests extends BaseGameActivity {
	@Override
	protected String getPublicKey() {
		return "none";
	}

	@Override
	protected void doShowAds() {
	}

	@Override
	protected void doHideAds() {
	}

	@Override
	protected boolean doShowFullscreenAds() {
		return false;
	}

	@Override
	public String getName() {
		return "tests";
	}

	@Override
	protected String getTrackerId() {
		return "none";
	}
}
