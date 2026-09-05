package org.base.game;

import org.base.BaseActivity;

import android.util.Log;

/**
 * Game activity with optional analytics. Tracking is a no-op unless a subclass
 * overrides {@link #doTrack(String, String)}.
 */
public abstract class BaseGameActivity extends BaseActivity {
	protected abstract String getTrackerId();

	@Override
	protected boolean doTrack(String hitType, String screenName) {
		Log.v(getName(), "Track: " + hitType + "=" + screenName);
		return true;
	}
}
