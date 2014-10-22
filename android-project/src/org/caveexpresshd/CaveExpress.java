package org.caveexpresshd;

import org.base.game.BaseGameActivity;

/**
 * Activity for the hd version of the game with payment support from its super
 * class and without ads
 */
public class CaveExpress extends BaseGameActivity {
	@Override
	protected String getPublicKey() {
		return "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuEolFRaERcC4MeTAHhjpFonPC3+kHNxFtEZ0fPxS0VO5LLV3/6GjqvSV95WE7ahNX+nxdgj1Z/ZtGMRHX1D2Ha5eaSy1rwIPcA60tvyPHtg9nM0pzbzSqn9dS1EXTqzndcD8D3L9LGOyDGFl2BOU3BouztddmDVkfIJpx7xf8pXChAAlGLqUsx3dZnzC5Wv4xYEXuH6fUxDNEWkM7V4UGF4S6yGqEY3fWzF1zcZfUZTQZLTL7dvqJ/4WouwfBlUMXuA94q+2gONxMoVlrMOEH+ds4WWR7thXv0xtLI2iamAcDDAcuXdL2jy4ZqrLMKfvd9kDkplakubPOeWz0AVpZQIDAQAB";
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
		return "caveexpresshd";
	}

	@Override
	protected String getTrackerId() {
		return "UA-54860800-3";
	}
}