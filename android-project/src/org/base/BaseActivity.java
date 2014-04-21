package org.base;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import org.PaymentEntry;
import org.base.util.IabHelper;
import org.base.util.IabResult;
import org.base.util.Inventory;
import org.base.util.Purchase;
import org.base.util.SkuDetails;
import org.libsdl.app.SDLActivity;

import android.app.AlertDialog;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
// import com.google.analytics.tracking.android.EasyTracker;
// import com.google.analytics.tracking.android.Fields;
// import com.google.analytics.tracking.android.Tracker;

/**
 * Base activity class with google play payment support
 */
public abstract class BaseActivity extends SDLActivity {
	public static final String NAME = "caveexpress";

	// (arbitrary) request code for the purchase flow
	protected static final int RC_REQUEST = 10001;

	private static Map<String, Purchase> paymentIds = new ConcurrentHashMap<String, Purchase>();
	protected static boolean noInAppBilling = false;
	protected static IabHelper inAppBillingHelper;
	protected static List<String> moreSkus = new ArrayList<String>();
	protected static List<PaymentEntry> paymentEntries = new ArrayList<PaymentEntry>();

	/**
	 * Your application's public key, encoded in base64. This is used for
	 * verification of purchase signatures. You can find your app's
	 * base64-encoded public key in your application's page on Google Play
	 * Developer Console. Note that this is NOT your "developer public key".
	 */
	protected String getPublicKey() {
		return "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4KxbS8KykHzthKhIkWcfbGeJsUAApOtLH5t4jWxLy2j6ISpL72w66a6cEjB+YC8hJSPQ8TjvAKCHAqSmi23F1f9eUiuJ8WhMysESfJuyOdCXdQ4swVsKDLdwcR7tu69X5yEoJd0eHJ0TaEu5YXd8FPg6VRzxIf9H+NlDyeDGIai332kwlbyaPtyNGym8BnDpKh1TwC1bEwNYbZiL+CBe/1B+ZxxdTkGb/0/Sp4y+TpjD42vhQp9VzeiEeJqYlP6c1A0Dh1guKPEIAiowmyjm2rKCya85EKC4IUl9/QgyO/BGZFG13Em/JIr9rFoGygZ3igYLlWuWeUfJUTmgcBJchwIDAQAB";
	}

	private final class InAppBillingSetupFinishedListener implements IabHelper.OnIabSetupFinishedListener {
		@Override
		public void onIabSetupFinished(final IabResult result) {
			if (!result.isSuccess()) {
				noInAppBilling = true;
				Log.e(NAME, "Problem setting up In-app Billing: " + result);
			} else {
				Log.v(NAME, "App billing setup OK, querying inventory.");

				moreSkus.clear();
				moreSkus.add("adfree");
				moreSkus.add("unlockall");
				// for (int i = 0; i < 20; i++) {
				// moreSkus.add("campaign" + i);
				// }

				inAppBillingHelper.queryInventoryAsync(true, moreSkus, mGotInventoryListener);
			}
		}
	}

	@Override
	public void onStart() {
		super.onStart();
		// EasyTracker.getInstance(this).activityStart(this);
	}

	@Override
	public void onStop() {
		super.onStop();
		// EasyTracker.getInstance(this).activityStop(this);
	}

	@Override
	protected void onCreate(final Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		final DisplayMetrics dm = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(dm);
		configureBilling();
	}

	protected void configureBilling() {
		inAppBillingHelper = new IabHelper(this, getPublicKey());
		inAppBillingHelper.startSetup(new InAppBillingSetupFinishedListener());
		inAppBillingHelper.enableDebugLogging(isDebug(), NAME);
	}

	/**
	 * Listener that's called when we finish querying the items and
	 * subscriptions we own
	 */
	private final IabHelper.QueryInventoryFinishedListener mGotInventoryListener = new IabHelper.QueryInventoryFinishedListener() {
		@Override
		public void onQueryInventoryFinished(final IabResult result, final Inventory inventory) {
			if (result.isFailure()) {
				Log.e(NAME, "Failed to query inventory: " + result);
				return;
			}

			Log.d(NAME, "Query inventory was successful.");

			/**
			 * Check for items we own. Notice that for each purchase, we check
			 * the developer payload to see if it's correct!
			 */
			List<Purchase> allPurchases = inventory.getAllPurchases();
			for (Purchase purchase : allPurchases) {
				final String orig = NAME + purchase.getSku();
				final String payload = purchase.getDeveloperPayload();
				if (payload.equals(orig)) {
					paymentIds.put(purchase.getSku(), purchase);
					Log.v(NAME, "Users has bought: " + purchase.getSku());
				} else {
					Log.v(NAME, purchase.getSku() + " is invalid.");
				}
			}
			Log.d(NAME, "more SKUs: " + moreSkus.size());
			for (String sku : moreSkus) {
				SkuDetails skuDetails = inventory.getSkuDetails(sku);
				if (skuDetails == null) {
					Log.v(NAME, "Could not get details for sku: " + sku);
					continue;
				}
				paymentEntries.add(new PaymentEntry(skuDetails.getSku(), skuDetails.getDescription(), skuDetails
						.getPrice()));
			}

			Log.d(NAME, "user purchased " + allPurchases.size() + " items.");
			onPaymentDone();
		}
	};

	static IabHelper.OnIabPurchaseFinishedListener purchaseFinishedListener = new IabHelper.OnIabPurchaseFinishedListener() {
		@Override
		public void onIabPurchaseFinished(final IabResult result, final Purchase purchase) {
			Log.v(NAME, "Purchase finished: " + result + ", purchase: " + purchase);
			if (result.isFailure()) {
				Log.e(NAME, "Failed purchase!");
				return;
			}
			if (!verifyDeveloperPayload(purchase, purchase.getSku())) {
				Log.e(NAME, "Error purchasing. Authenticity verification failed.");
				return;
			}

			paymentIds.put(purchase.getSku(), purchase);

			Log.v(NAME, "Purchase successful.");
		}
	};

	protected static boolean verifyDeveloperPayload(final Purchase p, String sku) {
		final String orig = NAME + sku;
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

	public static void showAds(final boolean ontop) {
		((BaseActivity) mSingleton).doShowAds(ontop);
	}

	protected abstract void doShowAds(final boolean ontop);

	public static boolean showFullscreenAds() {
		return ((BaseActivity) mSingleton).doShowFullscreenAds();
	}

	protected abstract boolean doShowFullscreenAds();

	public static void hideAds() {
		((BaseActivity) mSingleton).doHideAds();
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
		((BaseActivity) mSingleton).startActivity(main);
	}

	static boolean buyItem(String id) {
		return ((BaseActivity) mSingleton).doBuyItem(id);
	}

	protected boolean doBuyItem(String id) {
		if (noInAppBilling) {
			Log.v(NAME, "Purchase procedure not available");
			return false;
		}

		Log.v(NAME, "Purchase procedure started");
		final String payload = NAME + id;

		inAppBillingHelper.launchPurchaseFlow(mSingleton, id, RC_REQUEST, purchaseFinishedListener, payload);
		return true;
	}

	static void alert(final String message) {
		final AlertDialog.Builder bld = new AlertDialog.Builder(mSingleton);
		bld.setMessage(message);
		bld.setNeutralButton("OK", null);
		Log.d(NAME, "Showing alert dialog: " + message);
		bld.create().show();
	}

	static boolean hasItem(String id) {
		return ((BaseActivity) mSingleton).doHasItem(id);
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
		return paymentEntries.toArray(new PaymentEntry[] {});
	}

	static boolean isSmallScreen() {
		final Configuration config = getContext().getResources().getConfiguration();
		final int i = config.screenLayout & Configuration.SCREENLAYOUT_SIZE_MASK;
		// small is 320x426 dp units
		// normal is 320x470 dp units
		// large is 480x640 dp units
		// xlarge is 720x960 dp units
		if (i == Configuration.SCREENLAYOUT_SIZE_SMALL || i == Configuration.SCREENLAYOUT_SIZE_NORMAL) {
			return true;
		}
		return false;
	}

	static boolean track(String hitType, String screenName) {
		// final Tracker tracker = EasyTracker.getInstance(mSingleton);
		// final Map<String, String> hitParameters = new HashMap<String,
		// String>();
		// hitParameters.put(Fields.HIT_TYPE, hitType);
		// hitParameters.put(Fields.SCREEN_NAME, screenName);
		// tracker.send(hitParameters);
		return true;
	}

	static String getLocale() {
		Locale current = getContext().getResources().getConfiguration().locale;
		Log.v(NAME, "locale: " + current);
		return current.getDisplayLanguage();
	}

	public static native void onPaymentDone();

	public static native boolean isDebug();
}
