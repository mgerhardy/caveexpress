package org.caveexpress;

import org.base.BaseActivity;

import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.widget.RelativeLayout;

import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.AdView;
import com.google.android.gms.ads.InterstitialAd;

/**
 * Activity for the free version of the game with payment support from its super
 * class and ads
 */
public class CaveExpress extends BaseActivity {
	protected static AdView adview = null;
	static RelativeLayout layout;

	protected static Handler uiHandler;
	private static final String adUnitId = "a151fe5e535a323";

	private static final int SDLViewID = 1;
	private static final int AdViewID = 2;

	private InterstitialAd interstitial;

	/**
	 * Your application's public key, encoded in base64. This is used for
	 * verification of purchase signatures. You can find your app's
	 * base64-encoded public key in your application's page on Google Play
	 * Developer Console. Note that this is NOT your "developer public key".
	 */
	@Override
	protected String getPublicKey() {
		return "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4KxbS8KykHzthKhIkWcfbGeJsUAApOtLH5t4jWxLy2j6ISpL72w66a6cEjB+YC8hJSPQ8TjvAKCHAqSmi23F1f9eUiuJ8WhMysESfJuyOdCXdQ4swVsKDLdwcR7tu69X5yEoJd0eHJ0TaEu5YXd8FPg6VRzxIf9H+NlDyeDGIai332kwlbyaPtyNGym8BnDpKh1TwC1bEwNYbZiL+CBe/1B+ZxxdTkGb/0/Sp4y+TpjD42vhQp9VzeiEeJqYlP6c1A0Dh1guKPEIAiowmyjm2rKCya85EKC4IUl9/QgyO/BGZFG13Em/JIr9rFoGygZ3igYLlWuWeUfJUTmgcBJchwIDAQAB";
	}

	private final class InterstitialRunnable implements Runnable {
		private Boolean retVal = null;

		@Override
		public void run() {
			if (!interstitial.isLoaded()) {
				Log.v(NAME, "don't show fullscreen ads");
				retVal = false;
				return;
			}
			Log.v(NAME, "show fullscreen ads");
			interstitial.show();
			retVal = true;
		}

		public boolean getReturn() {
			while (retVal == null) {
				try {
					Thread.sleep(1);
				} catch (InterruptedException e) {
				}
			}
			return retVal;
		}
	}

	private static final class HideAddsRunnable implements Runnable {
		@Override
		public void run() {
			if (adview == null) {
				return;
			}

			layout.removeView(adview);
			adview.destroy();
			adview = null;

			Log.v(NAME, "Hiding ads");
		}
	}

	@Override
	public void setContentView(View view) {
		layout = new RelativeLayout(this);
		view.setId(SDLViewID);
		layout.addView(view);
		super.setContentView(layout);
	}

	private static final class AddsShowRunnable implements Runnable {
		private final boolean ontop;

		private AddsShowRunnable(final boolean ontop) {
			this.ontop = ontop;
		}

		@Override
		public void run() {
			if (adview != null) {
				Log.v(NAME, "There is already an ads view");
				return;
			}

			adview = new AdView(mSingleton);
			adview.setAdUnitId(adUnitId);
			adview.setAdSize(AdSize.BANNER);
			adview.setId(AdViewID);
			adview.setAdListener(new AdListener() {
				@Override
				public void onAdFailedToLoad(int errorCode) {
					super.onAdFailedToLoad(errorCode);
					if (errorCode == AdRequest.ERROR_CODE_NO_FILL) {
						// TODO: try to get another one?
					}
					Log.v(NAME, "Failed to load the ad");
				}
			});

			final RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(
					RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT);

			if (ontop) {
				params.addRule(RelativeLayout.ALIGN_PARENT_TOP);
				params.addRule(RelativeLayout.CENTER_HORIZONTAL);
			} else {
				params.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
				params.addRule(RelativeLayout.CENTER_HORIZONTAL);
				params.addRule(RelativeLayout.ABOVE, 1);
			}

			final AdRequest re = new AdRequest.Builder() // no break
					.addTestDevice(AdRequest.DEVICE_ID_EMULATOR) // Emulator
					// .setGender(AdRequest.GENDER_FEMALE) // Demographic
					// .setBirthday(new GregorianCalendar(1985, 1, 1).getTime())
					.build();
			adview.loadAd(re);
			layout.addView(adview, params);
			Log.v(NAME, "Showing ads, ontop:" + ontop);
		}
	}

	@Override
	protected void onCreate(final Bundle savedInstanceState) {
		uiHandler = new Handler();

		super.onCreate(savedInstanceState);

		interstitial = new InterstitialAd(this);
		interstitial.setAdUnitId(adUnitId);
		final AdRequest re = new AdRequest.Builder() // no break
				.addTestDevice(AdRequest.DEVICE_ID_EMULATOR) // Emulator
				// .setGender(AdRequest.GENDER_FEMALE) // Demographic
				// .setBirthday(new GregorianCalendar(1985, 1, 1).getTime())
				.build();
		interstitial.loadAd(re);
	}

	@Override
	protected void onPause() {
		if (adview != null) {
			adview.pause();
		}
		super.onPause();
	}

	@Override
	protected void onResume() {
		super.onResume();
		if (adview != null) {
			adview.resume();
		}
	}

	@Override
	protected void onDestroy() {
		if (adview != null) {
			adview.destroy();
		}
		super.onDestroy();
		System.exit(0);
	}

	@Override
	protected boolean doShowFullscreenAds() {
		Handler mainHandler = new Handler(getContext().getMainLooper());
		InterstitialRunnable myRunnable = new InterstitialRunnable();
		mainHandler.post(myRunnable);
		return myRunnable.getReturn();
	}

	@Override
	protected void doShowAds(final boolean ontop) {
		uiHandler.post(new AddsShowRunnable(ontop));
	}

	@Override
	protected void doHideAds() {
		uiHandler.post(new HideAddsRunnable());
	}
}
