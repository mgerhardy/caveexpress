package org.base;

import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

import org.PaymentEntry;
import org.base.util.IabHelper;
import org.base.util.IabResult;
import org.base.util.Inventory;
import org.base.util.Purchase;
import org.base.util.SkuDetails;
import org.libsdl.app.SDLActivity;

import android.app.AlertDialog;
import android.content.Intent;
import android.content.IntentSender.SendIntentException;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.Uri;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.drive.Drive;
import com.google.android.gms.games.Games;
import com.google.android.gms.plus.Plus;

/**
 * Activity class without google play support
 */
public abstract class BaseActivity extends SDLActivity implements GoogleApiClient.ConnectionCallbacks,
		GoogleApiClient.OnConnectionFailedListener {
	protected GoogleApiClient googleApiClient;

	// (arbitrary) request code for the purchase flow
	protected static final int RC_REQUEST = 10001;
	// Request code used to invoke sign in user interactions.
	protected static final int RC_SIGN_IN = 9001;

	protected Map<String, Purchase> paymentIds = new ConcurrentHashMap<String, Purchase>();
	protected List<String> moreSkus = new CopyOnWriteArrayList<String>();
	protected List<PaymentEntry> paymentEntries = new CopyOnWriteArrayList<PaymentEntry>();
	protected boolean noInAppBilling = false;
	protected IabHelper inAppBillingHelper;

	/**
	 * Your application's public key, encoded in base64. This is used for
	 * verification of purchase signatures. You can find your app's
	 * base64-encoded public key in your application's page on Google Play
	 * Developer Console. Note that this is NOT your "developer public key".
	 */
	protected abstract String getPublicKey();

	public abstract String getName();

	@Override
	protected String[] getLibraries() {
		return new String[] { "SDL2", "SDL2_image", "SDL2_mixer", "SDL2_net", "main" };
	}

	private final class InAppBillingSetupFinishedListener implements IabHelper.OnIabSetupFinishedListener {
		@Override
		public void onIabSetupFinished(final IabResult result) {
			if (!result.isSuccess()) {
				noInAppBilling = true;
				Log.e(getName(), "Problem setting up In-app Billing: " + result);
			} else {
				Log.v(getName(), "App billing setup OK, querying inventory.");

				moreSkus.clear();

				addSkus(moreSkus);
				// for (int i = 0; i < 20; i++) {
				// moreSkus.add("campaign" + i);
				// }

				inAppBillingHelper.queryInventoryAsync(true, moreSkus, mGotInventoryListener);
			}
		}
	}

	protected void addSkus(List<String> moreSkus) {
		if (!isHD()) {
			moreSkus.add("adfree");
		}
		moreSkus.add("unlockall");
	}

	@Override
	protected void onCreate(final Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		configureBilling();

		googleApiClient = new GoogleApiClient.Builder(this).addConnectionCallbacks(this)
				.addOnConnectionFailedListener(this).addApi(Plus.API).addScope(Plus.SCOPE_PLUS_LOGIN).addApi(Games.API)
				.addScope(Drive.SCOPE_FILE).addScope(Games.SCOPE_GAMES).build();
	}

	protected void configureBilling() {
		inAppBillingHelper = new IabHelper(this, getPublicKey());
		inAppBillingHelper.startSetup(new InAppBillingSetupFinishedListener());
		inAppBillingHelper.enableDebugLogging(isDebug(), getName());
	}

	/**
	 * Listener that's called when we finish querying the items and
	 * subscriptions we own
	 */
	private final IabHelper.QueryInventoryFinishedListener mGotInventoryListener = new IabHelper.QueryInventoryFinishedListener() {
		@Override
		public void onQueryInventoryFinished(final IabResult result, final Inventory inventory) {
			if (result.isFailure()) {
				Log.e(getName(), "Failed to query inventory: " + result);
				return;
			}

			Log.d(getName(), "Query inventory was successful.");

			/**
			 * Check for items we own. Notice that for each purchase, we check
			 * the developer payload to see if it's correct!
			 */
			List<Purchase> allPurchases = inventory.getAllPurchases();
			for (Purchase purchase : allPurchases) {
				final String orig = getName() + purchase.getSku();
				final String payload = purchase.getDeveloperPayload();
				if (payload.equals(orig)) {
					paymentIds.put(purchase.getSku(), purchase);
					Log.v(getName(), "Users has bought: " + purchase.getSku());
				} else {
					Log.v(getName(), purchase.getSku() + " is invalid.");
				}
			}
			Log.d(getName(), "more SKUs: " + moreSkus.size());
			for (String sku : moreSkus) {
				SkuDetails skuDetails = inventory.getSkuDetails(sku);
				if (skuDetails == null) {
					Log.v(getName(), "Could not get details for sku: " + sku);
					continue;
				}
				paymentEntries.add(new PaymentEntry(skuDetails.getSku(), skuDetails.getDescription(), skuDetails
						.getPrice()));
			}

			Log.d(getName(), "user purchased " + allPurchases.size() + " items.");
			onPaymentDone();
		}
	};

	private final IabHelper.OnIabPurchaseFinishedListener purchaseFinishedListener = new IabHelper.OnIabPurchaseFinishedListener() {
		@Override
		public void onIabPurchaseFinished(final IabResult result, final Purchase purchase) {
			Log.v(getName(), "Purchase finished: " + result + ", purchase: " + purchase);
			if (result.isFailure()) {
				Log.e(getName(), "Failed purchase!");
				return;
			}
			if (!verifyDeveloperPayload(purchase, purchase.getSku())) {
				Log.e(getName(), "Error purchasing. Authenticity verification failed.");
				return;
			}

			paymentIds.put(purchase.getSku(), purchase);

			Log.v(getName(), "Purchase successful.");
		}
	};

	private boolean resolvingError;

	protected static boolean verifyDeveloperPayload(final Purchase p, String sku) {
		final String orig = getBaseActivity().getName() + sku;
		final String payload = p.getDeveloperPayload();
		return payload.equals(orig);
	}

	@Override
	protected void onDestroy() {
		doOnDestory();
		super.onDestroy();
		// this is nasty, but needed on e.g. the S3 to not restart the game on
		// hitting the power button
		System.exit(0);
	}

	protected void doOnDestory() {
		doDestroyBilling();
	}

	protected void doDestroyBilling() {
		// killing billing services
		if (inAppBillingHelper != null)
			inAppBillingHelper.dispose();
		inAppBillingHelper = null;
	}

	public static void showAds() {
		getBaseActivity().doShowAds();
	}

	protected abstract void doShowAds();

	public static boolean showFullscreenAds() {
		return getBaseActivity().doShowFullscreenAds();
	}

	protected abstract boolean doShowFullscreenAds();

	public static void hideAds() {
		getBaseActivity().doHideAds();
	}

	protected abstract void doHideAds();

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

	static boolean buyItem(String id) {
		return getBaseActivity().doBuyItem(id);
	}

	protected boolean doBuyItem(String id) {
		if (noInAppBilling) {
			Log.v(getName(), "Purchase procedure not available");
			return false;
		}

		Log.v(getName(), "Purchase procedure started");
		final String payload = getName() + id;

		inAppBillingHelper.launchPurchaseFlow(mSingleton, id, RC_REQUEST, purchaseFinishedListener, payload);
		return true;
	}

	static void alert(final String message) {
		final AlertDialog.Builder bld = new AlertDialog.Builder(mSingleton);
		bld.setMessage(message);
		bld.setNeutralButton("OK", null);
		Log.d(getBaseActivity().getName(), "Showing alert dialog: " + message);
		bld.create().show();
	}

	static boolean hasItem(String id) {
		return getBaseActivity().doHasItem(id);
	}

	protected boolean doHasItem(String id) {
		return paymentIds.containsKey(id);
	}

	static boolean isOUYA() {
		try {
			mSingleton.getPackageManager().getPackageInfo("tv.ouya", 0);
			return true;
		} catch (NameNotFoundException ignore) {
		}
		return false;
	}

	static PaymentEntry[] getPaymentEntries() {
		return getBaseActivity().paymentEntries.toArray(new PaymentEntry[] {});
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

	static void achievementUnlocked(String id) {
		getBaseActivity().doAchievementUnlocked(id);
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
	}

	@Override
	public void onConnectionSuspended(int cause) {
		if (!googleApiClient.isConnected())
			return;
		googleApiClient.disconnect();
	}

	protected boolean doPersisterDisconnect() {
		if (!googleApiClient.isConnected())
			return false;
		googleApiClient.disconnect();
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

	protected void doAchievementUnlocked(String id) {
		if (!googleApiClient.isConnected())
			return;
		Games.Achievements.unlock(googleApiClient, id);
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		if (requestCode == RC_SIGN_IN && resultCode == RESULT_OK) {
			doPersisterConnect();
		}
	}

	public static native void onPersisterConnectFailed();

	public static native void onPersisterConnectSuccess();

	public static native void onPaymentDone();

	public static native boolean isDebug();

	public static native boolean isTrackingOptOut();

	public static native boolean isHD();
}
