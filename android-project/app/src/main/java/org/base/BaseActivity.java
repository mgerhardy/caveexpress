package org.base;

import java.lang.reflect.Field;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

import org.libsdl.app.SDLActivity;

import android.app.AlertDialog;
import android.content.Intent;
import android.content.IntentSender.SendIntentException;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.drive.Drive;
import com.google.android.gms.games.Games;
import com.google.android.gms.games.snapshot.Snapshot;
import com.google.android.gms.games.snapshot.SnapshotMetadataChange;
import com.google.android.gms.games.snapshot.Snapshots.OpenSnapshotResult;
import com.google.android.gms.plus.Plus;

/**
 * Activity class without google play support
 */
public abstract class BaseActivity extends SDLActivity implements GoogleApiClient.ConnectionCallbacks,
		GoogleApiClient.OnConnectionFailedListener {
	private static final String GAMESTATE = "gamestate";

	protected GoogleApiClient googleApiClient;

	// Request code used to invoke sign in user interactions.
	protected static final int RC_SIGN_IN = 9001;

	/**
	 * @return The name of the package of the game. This is also used for
	 *         reflection calls to not import the auto generated R class.
	 */
	public abstract String getName();

	@Override
	protected String[] getLibraries() {
		return new String[] { getName() };
	}

	@Override
	protected void onCreate(final Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		googleApiClient = new GoogleApiClient.Builder(this).addConnectionCallbacks(this)
				.addOnConnectionFailedListener(this).addApi(Plus.API).addScope(Plus.SCOPE_PLUS_LOGIN).addApi(Games.API)
				.addScope(Drive.SCOPE_FILE).addScope(Games.SCOPE_GAMES).build();
	}

	private boolean resolvingError;

	@Override
	protected void onDestroy() {
		doOnDestory();
		super.onDestroy();
		// this is nasty, but needed on e.g. the S3 to not restart the game on
		// hitting the power button
		System.exit(0);
	}

	protected void doOnDestory() {
	}

	public static void openPlayStore(String appName) {
		openURL("market://details?id=" + appName);
	}

	public static void openURL(String url) {
		Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
		mSingleton.startActivity(browserIntent);
	}

	public static void minimize() {
		Intent main = new Intent(Intent.ACTION_MAIN);
		main.addCategory(Intent.CATEGORY_HOME);
		main.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		getBaseActivity().startActivity(main);
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
		boolean small = /* m.widthPixels < 1280 || */m.heightPixels < 720;
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
		return getBaseActivity().doPersisterInit();
	}

	static boolean persisterDisconnect() {
		return getBaseActivity().doPersisterDisconnect();
	}

	static void achievementUnlocked(String id, boolean increment) {
		getBaseActivity().doAchievementUnlocked(id, increment);
	}

	static boolean persisterConnect() {
		return getBaseActivity().doPersisterConnect();
	}

	@Override
	protected void onStop() {
		super.onStop();
		if (googleApiClient.isConnected()) {
			googleApiClient.disconnect();
		}
	}

	@Override
	public void onConnectionFailed(ConnectionResult result) {
		if (resolvingError)
			return;
		Log.e(getName(), "google play api: connection failed with " + result.toString());
		if (!result.hasResolution()) {
			onPersisterConnectFailed();
		} else {
			try {
				resolvingError = true;
				result.startResolutionForResult(this, RC_SIGN_IN);
			} catch (SendIntentException e) {
				// Try connecting again
				Log.d(getName(), "SendIntentException, so connecting again.");
				doPersisterConnect();
			}
		}
	}

	@Override
	public void onConnected(Bundle bundle) {
		Log.v(getName(), "google play api: connected");
		onPersisterConnectSuccess();
		resolvingError = false;
	}

	@Override
	public void onConnectionSuspended(int cause) {
		if (!googleApiClient.isConnected())
			return;
		googleApiClient.disconnect();
	}

	protected boolean doPersisterDisconnect() {
		resolvingError = false;
		if (!googleApiClient.isConnected())
			return false;
		googleApiClient.disconnect();
		onPersisterDisconnect();
		return true;
	}

	protected boolean doPersisterConnect() {
		if (googleApiClient.isConnected() || googleApiClient.isConnecting()) {
			Log.v(getName(), "google play api: already connected");
			return false;
		}
		Log.v(getName(), "google play api: connect()");
		googleApiClient.connect();
		return true;
	}

	protected boolean doPersisterInit() {
		return googleApiClient != null;
	}

	protected String getResourceString(String id) {
		try {
			String className = "org." + getName() + ".R";
			Class<?> resourceIds = Class.forName(className);
			if (resourceIds == null) {
				Log.e(getName(), "Could not get the class for " + className);
				return null;
			}
			Class<?>[] innerClasses = resourceIds.getDeclaredClasses();
			for (Class<?> innerClass : innerClasses) {
				if ("string".equals(innerClass.getSimpleName())) {
					Field resourceIdField = innerClass.getDeclaredField(id);
					if (resourceIdField == null) {
						Log.e(getName(), "Could not get the field for " + id + " in " + className);
						return null;
					}
					int resourceId = resourceIdField.getInt(null);
					Log.v(getName(), "Got value " + resourceId + " for " + id + " in " + className);
					return getString(resourceId);
				}
			}
		} catch (Exception e) {
			Log.e(getName(), e.getMessage(), e);
		}
		return null;
	}

	protected void doAchievementUnlocked(String id, boolean increment) {
		if (googleApiClient == null || !googleApiClient.isConnected()) {
			Log.v(getName(), "google play achievement " + id + " can't get unlocked - not connected");
			return;
		}
		String resourceString = getResourceString(id);
		if (resourceString == null) {
			Log.v(getName(), "google play achievement " + id + " wasn't found in the resource file");
			return;
		}
		Log.v(getName(), "google play achievement " + id + " resolved to " + resourceString);
		if (increment) {
			Games.Achievements.increment(googleApiClient, resourceString, 1);
			Log.v(getName(), "google play achievement " + id + " count");
		} else {
			Games.Achievements.unlock(googleApiClient, resourceString);
			Log.v(getName(), "google play achievement " + id + " unlock");
		}
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		if (requestCode == RC_SIGN_IN && resultCode == RESULT_OK) {
			doPersisterConnect();
		}
	}

	static byte[] loadGameState() {
		return getBaseActivity().doLoadGameState();
	}

	public byte[] doLoadGameState() {
		Snapshot snapshot = getSnapshot();
		return snapshot.readFully();
	}

	protected Snapshot getSnapshot() {
		OpenSnapshotResult result = Games.Snapshots.open(googleApiClient, GAMESTATE, true).await();
		Snapshot snapshot = result.getSnapshot();
		return snapshot;
	}

	/**
	 * Gets a screenshot to use with snapshots. Note that in practice you
	 * probably do not want to use this approach because tablet screen sizes can
	 * become pretty large and because the image will contain any UI and layout
	 * surrounding the area of interest.
	 */
	protected Bitmap getScreenShot() {
		Bitmap coverImage;
		View view = mSurface;
		try {
			view.setDrawingCacheEnabled(true);
			Bitmap base = view.getDrawingCache();
			coverImage = base.copy(base.getConfig(), false /* isMutable */);
		} catch (Exception ex) {
			Log.i(getName(), "Failed to create screenshot", ex);
			coverImage = null;
		} finally {
			view.setDrawingCacheEnabled(false);
		}
		return coverImage;
	}

	static void saveGameState(byte[] bytes) {
		getBaseActivity().doSaveGameState(bytes);
	}

	public void doSaveGameState(byte[] bytes) {
		Snapshot snapshot = getSnapshot();
		snapshot.writeBytes(bytes);
		SnapshotMetadataChange.Builder builder = new SnapshotMetadataChange.Builder();
		builder.setCoverImage(getScreenShot());
		builder.setDescription(getName());
		SnapshotMetadataChange metadataChange = builder.build();
		Games.Snapshots.commitAndClose(googleApiClient, snapshot, metadataChange);
	}

	static void addPointsToLeaderBoard(String leaderBoardId, int points) {
		getBaseActivity().doAddPointsToLeaderBoards(leaderBoardId, points);
	}

	protected void doAddPointsToLeaderBoards(String leaderBoardId, int points) {
		Games.Leaderboards.submitScore(googleApiClient, leaderBoardId, points);
	}

	static void showAchievements() {
		getBaseActivity().doShowAchievements();
	}

	protected void doShowAchievements() {
		startActivityForResult(Games.Achievements.getAchievementsIntent(googleApiClient), 1337);
	}

	static void showLeaderBoard(String leaderBoardId) {
		getBaseActivity().doShowLeaderBoard(leaderBoardId);
	}

	protected void doShowLeaderBoard(String leaderBoardId) {
		startActivityForResult(Games.Leaderboards.getLeaderboardIntent(googleApiClient, leaderBoardId), 42);
	}

	public static native void onPersisterConnectFailed();

	public static native void onPersisterConnectSuccess();

	public static native void onPersisterDisconnect();

	public static native boolean isDebug();

	public static native boolean isTrackingOptOut();

	public static native boolean isHD();
}
