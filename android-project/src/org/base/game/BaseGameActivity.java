package org.base.game;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import org.base.BaseActivity;

import android.util.Log;

import com.google.android.gms.analytics.GoogleAnalytics;
import com.google.android.gms.analytics.HitBuilders;
import com.google.android.gms.analytics.Logger;
import com.google.android.gms.analytics.Tracker;

/**
 * Base activity class with google analytics support
 */
public abstract class BaseGameActivity extends BaseActivity {
	public enum TrackerName {
		APP_TRACKER, GLOBAL_TRACKER
	}

	private final Map<TrackerName, Tracker> trackers = new HashMap<TrackerName, Tracker>();

	protected synchronized Tracker getTracker(TrackerName trackerId) {
		if (!trackers.containsKey(trackerId)) {
			GoogleAnalytics analytics = GoogleAnalytics.getInstance(this);
			Tracker t;
			switch (trackerId) {
			case APP_TRACKER:
				t = analytics.newTracker(getTrackerId());
				break;
			case GLOBAL_TRACKER:
				t = analytics.newTracker("UA-54860800-1");
				break;
			default:
				return null;
			}
			boolean trackingOptOut = isTrackingOptOut();
			if (trackingOptOut) {
				analytics.setAppOptOut(true);
				Log.v(getName(), "Tracker opt out");
			}
			if (isDebug()) {
				Log.v(getName(), "Tracker dry run");
				analytics.setDryRun(true);
				analytics.getLogger().setLogLevel(Logger.LogLevel.VERBOSE);
			}
			t.setAnonymizeIp(true);
			t.enableAutoActivityTracking(true);
			t.setAppName(getName());
			t.setSessionTimeout(300L);
			trackers.put(trackerId, t);
		}
		return trackers.get(trackerId);
	}

	protected abstract String getTrackerId();

	@Override
	protected boolean doTrack(String hitType, String screenName) {
		Log.v(getName(), "Track: " + hitType + "=" + screenName);
		Tracker tracker = getTracker(TrackerName.APP_TRACKER);
		if ("activewindow".equals(hitType)) {
			tracker.setScreenName(screenName);
			HitBuilders.AppViewBuilder appViewBuilder = new HitBuilders.AppViewBuilder();
			tracker.send(appViewBuilder.build());
			return true;
		}
		HitBuilders.EventBuilder eventBuilder = new HitBuilders.EventBuilder();
		Map<String, String> params = eventBuilder.setCategory(hitType).setAction(screenName).build();
		tracker.send(params);
		return true;
	}
}
