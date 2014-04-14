package org.base.game;

import org.base.BaseActivity;

import android.content.Intent;
import android.os.Bundle;

import com.google.android.gms.common.api.GoogleApiClient;
// import com.google.analytics.tracking.android.EasyTracker;
// import com.google.analytics.tracking.android.Fields;
// import com.google.analytics.tracking.android.Tracker;

/**
 * Base activity class with google play payment support
 */
public abstract class BaseGameActivity extends BaseActivity implements GameHelper.GameHelperListener {
	// The game helper object. This class is mainly a wrapper around this
	// object.
	protected GameHelper gameHelper;

	// We expose these constants here because we don't want users of this class
	// to have to know about GameHelper at all.
	public static final int CLIENT_GAMES = GameHelper.CLIENT_GAMES;
	public static final int CLIENT_APPSTATE = GameHelper.CLIENT_APPSTATE;
	public static final int CLIENT_PLUS = GameHelper.CLIENT_PLUS;
	public static final int CLIENT_ALL = GameHelper.CLIENT_ALL;

	// Requested clients. By default, that's just the games client.
	protected int requestedClients = CLIENT_GAMES;

	/** Constructs a BaseGameActivity with default client (GamesClient). */
	protected BaseGameActivity() {
		gameHelper = new GameHelper(this, requestedClients);
	}

	/**
	 * Constructs a BaseGameActivity with the requested clients.
	 * 
	 * @param requestedClients
	 *            The requested clients (a combination of CLIENT_GAMES,
	 *            CLIENT_PLUS and CLIENT_APPSTATE).
	 */
	public BaseGameActivity(int requestedClients) {
		setRequestedClients(requestedClients);
		gameHelper = new GameHelper(this, requestedClients);
	}

	protected GoogleApiClient getApiClient() {
		return gameHelper.getApiClient();
	}

	/**
	 * Sets the requested clients. The preferred way to set the requested
	 * clients is via the constructor, but this method is available if for some
	 * reason your code cannot do this in the constructor. This must be called
	 * before onCreate in order to have any effect. If called after onCreate,
	 * this method is a no-op.
	 * 
	 * @param requestedClients
	 *            A combination of the flags CLIENT_GAMES, CLIENT_PLUS and
	 *            CLIENT_APPSTATE, or CLIENT_ALL to request all available
	 *            clients.
	 * @param additionalScopes
	 *            . Scopes that should also be requested when the auth request
	 *            is made.
	 */
	protected void setRequestedClients(int requestedClients) {
		this.requestedClients = requestedClients;
	}

	@Override
	public void onStart() {
		super.onStart();
		gameHelper.onStart(this);
		// EasyTracker.getInstance(this).activityStart(this);
	}

	@Override
	public void onStop() {
		super.onStop();
		gameHelper.onStop();
		// EasyTracker.getInstance(this).activityStop(this);
	}

	@Override
	protected void onActivityResult(int request, int response, Intent data) {
		super.onActivityResult(request, response, data);
		gameHelper.onActivityResult(request, response, data);
	}

	protected boolean isSignedIn() {
		return gameHelper.isSignedIn();
	}

	protected void beginUserInitiatedSignIn() {
		gameHelper.beginUserInitiatedSignIn();
	}

	protected void signOut() {
		gameHelper.signOut();
	}

	protected String getInvitationId() {
		return gameHelper.getInvitationId();
	}

	protected boolean hasSignInError() {
		return gameHelper.hasSignInError();
	}

	protected GameHelper.SignInFailureReason getSignInError() {
		return gameHelper.getSignInError();
	}

	@Override
	protected void onCreate(final Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		gameHelper = new GameHelper(this, requestedClients);
		if (isDebug()) {
			gameHelper.enableDebugLog(true);
		}
		gameHelper.setup(this);
	}

	@Override
	public void onSignInFailed() {
	}

	@Override
	public void onSignInSucceeded() {
	}
}
