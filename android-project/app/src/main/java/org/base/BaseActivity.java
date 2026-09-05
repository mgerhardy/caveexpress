package org.base;

import java.util.Locale;

import org.libsdl.app.SDLActivity;

import android.app.AlertDialog;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;

/**
 * SDL activity used by CaveExpress / CavePacker.
 *
 * Play Games / Analytics hooks stay as no-ops so debug and CI APKs build
 * without the old bundled Play Services library.
 */
public abstract class BaseActivity extends SDLActivity {
	public abstract String getName();

	@Override
	protected String[] getLibraries() {
		return new String[] { getName() };
	}

	@Override
	protected void onDestroy() {
		doOnDestory();
		super.onDestroy();
		// Needed on some devices so the process does not resume on power button.
		System.exit(0);
	}

	protected void doOnDestory() {
	}

	public static void openPlayStore(String appName) {
		SDLActivity.openURL("market://details?id=" + appName);
	}

	public static void minimize() {
		if (mSingleton != null) {
			mSingleton.moveTaskToBack(true);
		}
	}

	static void alert(final String message) {
		final AlertDialog.Builder bld = new AlertDialog.Builder(mSingleton);
		bld.setMessage(message);
		bld.setNeutralButton("OK", null);
		Log.d(getBaseActivity().getName(), "Showing alert dialog: " + message);
		bld.create().show();
	}

	private static boolean isPackageInstalled(String packageName) {
		try {
			mSingleton.getPackageManager().getPackageInfo(packageName, 0);
			return true;
		} catch (NameNotFoundException ignore) {
		}
		return false;
	}

	static boolean isPlayStoreInstalled() {
		return isPackageInstalled("com.android.vending");
	}

	private static BaseActivity getBaseActivity() {
		return (BaseActivity) getContext();
	}

	static boolean isSmallScreen() {
		DisplayMetrics m = new DisplayMetrics();
		getBaseActivity().getWindowManager().getDefaultDisplay().getMetrics(m);
		boolean small = m.heightPixels < 720;
		Log.v(getBaseActivity().getName(), "resolution " + m.widthPixels + "x" + m.heightPixels + ", density: "
				+ m.density + ", small: " + small);
		return small;
	}

	static boolean track(String hitType, String screenName) {
		return getBaseActivity().doTrack(hitType, screenName);
	}

	protected boolean doTrack(String hitType, String screenName) {
		return false;
	}

	static String getLocale() {
		Locale current = getContext().getResources().getConfiguration().locale;
		Log.v(getBaseActivity().getName(), "locale: " + current);
		return current.getDisplayLanguage();
	}

	static boolean persisterInit() {
		return false;
	}

	static boolean persisterDisconnect() {
		return false;
	}

	static void achievementUnlocked(String id, boolean increment) {
		getBaseActivity().doAchievementUnlocked(id, increment);
	}

	static boolean persisterConnect() {
		return false;
	}

	protected void doAchievementUnlocked(String id, boolean increment) {
		Log.v(getName(), "achievement " + id + " (play services disabled)");
	}

	static byte[] loadGameState() {
		return null;
	}

	static void saveGameState(byte[] bytes) {
	}

	static void addPointsToLeaderBoard(String leaderBoardId, int points) {
	}

	static void showAchievements() {
	}

	static void showLeaderBoard(String leaderBoardId) {
	}

	@Override
	protected void onCreate(final Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		final View content = mSurface;
		if (content != null) {
			content.setKeepScreenOn(true);
		}
	}

	public static native void onPersisterConnectFailed();

	public static native void onPersisterConnectSuccess();

	public static native void onPersisterDisconnect();

	public static native boolean isDebug();

	public static native boolean isTrackingOptOut();

	public static native boolean isHD();
}
