package org.cavepacker;

import java.util.List;

import org.base.BaseActivity;

import android.os.Bundle;
import android.os.Handler;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
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
public class CavePacker extends BaseActivity {
	private static final int SDLViewID = 1;
	private static final int AdViewID = 2;

	protected AdView adview = null;
	protected RelativeLayout layout;
	protected InterstitialAd interstitial;

	private static final class HideAddsRunnable implements Runnable {
		private final AdView adview;
		private final RelativeLayout layout;
		private final BaseActivity activity;

		public HideAddsRunnable(BaseActivity activity, AdView adview, RelativeLayout layout) {
			this.activity = activity;
			this.adview = adview;
			this.layout = layout;
		}

		@Override
		public void run() {
			layout.removeView(adview);
			Log.v(activity.getName(), "Hiding ads");
		}
	}

	private static final class AddsShowRunnable implements Runnable {
		private final AdView adview;
		private final RelativeLayout layout;
		private final BaseActivity activity;

		public AddsShowRunnable(BaseActivity activity, AdView adview, RelativeLayout layout) {
			this.activity = activity;
			this.adview = adview;
			this.layout = layout;
		}

		@Override
		public void run() {
			final RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(
					RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT);

			params.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
			params.addRule(RelativeLayout.CENTER_VERTICAL);

			layout.addView(adview, params);
			Log.v(activity.getName(), "Showing ads");
		}
	}

	/**
	 * Your application's public key, encoded in base64. This is used for
	 * verification of purchase signatures. You can find your app's
	 * base64-encoded public key in your application's page on Google Play
	 * Developer Console. Note that this is NOT your "developer public key".
	 */
	@Override
	protected String getPublicKey() {
		return "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAh3ajm+zj1dmcZUU17kxEJVTxSX1ljDy5dA6/GX0fonk7lSchsv9ieyd0sJ9uRk3nJEArfCMV4fSWz2zZZJc3DUODyWH5UtU5YyAFO2/qpjywqNM1GG2Iir1fmO0ckhYwmuLCUz4rJByhlJGfGSxoYo6uanYFkBPZ3b6sRcx4PrBp47jHF/OoIsAAppwXEoD14kh596npAa3CQurXThGhuaFbgYRqiVNplwQj4BjncyfFroLC5NW/k3zEa/cp0Dj6XKV5z1PzEjX7/Q8X7Rw21CS0aMnlhsVbaWKe2D8T+zjaI4pmq2JHOh904CE+3rpCWrZPkI86RwLfovahfbfWkwIDAQAB";
	}

	private final class InterstitialRunnable implements Runnable {
		private Boolean retVal = null;

		@Override
		public void run() {
			if (!interstitial.isLoaded()) {
				Log.v(getName(), "don't show fullscreen ads");
				retVal = false;
				return;
			}
			Log.v(getName(), "show fullscreen ads");
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

	@Override
	public void setContentView(View view) {
		layout = new RelativeLayout(this);
		view.setId(SDLViewID);
		layout.addView(view);
		super.setContentView(layout);
	}

	@Override
	protected void onCreate(final Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		interstitial = new InterstitialAd(this);
		interstitial.setAdUnitId("ca-app-pub-5370378935428600/3242099978");
		interstitial.setAdListener(new AdListener() {
			@Override
			public void onAdClosed() {
				reloadInterstitial();
			};
		});
		reloadInterstitial();

		adview = new AdView(mSingleton);
		adview.setAdUnitId("ca-app-pub-5370378935428600/7187948375");
		adview.setAdSize(getAdSize());
		adview.setId(AdViewID);
		adview.setAdListener(new AdListener() {
			@Override
			public void onAdFailedToLoad(int errorCode) {
				super.onAdFailedToLoad(errorCode);
				if (errorCode == AdRequest.ERROR_CODE_NO_FILL) {
					Log.v(getName(), "Failed to load the ad: No fill");
				} else if (errorCode == AdRequest.ERROR_CODE_INVALID_REQUEST) {
					Log.v(getName(), "Failed to load the ad: Invalid request");
				} else if (errorCode == AdRequest.ERROR_CODE_INTERNAL_ERROR) {
					Log.v(getName(), "Failed to load the ad: Internal error");
				} else if (errorCode == AdRequest.ERROR_CODE_NETWORK_ERROR) {
					Log.v(getName(), "Failed to load the ad: Network errors");
				} else {
					Log.v(getName(), "Failed to load the ad");
				}
			}

			@Override
			public void onAdClosed() {
				super.onAdClosed();
				reloadAd();
			}

			@Override
			public void onAdLoaded() {
				super.onAdLoaded();
				Log.v(getName(), "Loaded an ad");
			}
		});
		reloadAd();
	}

	private void reloadAd() {
		adview.loadAd(new AdRequest.Builder().build());
	}

	private void reloadInterstitial() {
		interstitial.loadAd(new AdRequest.Builder().build());
	}

	private AdSize getAdSize() {
		return AdSize.MEDIUM_RECTANGLE;
	}

	@Override
	protected void onPause() {
		adview.pause();
		super.onPause();
	}

	@Override
	protected void onResume() {
		super.onResume();
		adview.resume();
	}

	@Override
	protected void onDestroy() {
		adview.destroy();
		super.onDestroy();
	}

	@Override
	protected boolean doShowFullscreenAds() {
		Handler mainHandler = new Handler(getContext().getMainLooper());
		InterstitialRunnable r = new InterstitialRunnable();
		mainHandler.post(r);
		return r.getReturn();
	}

	@Override
	protected void doShowAds() {
		Handler mainHandler = new Handler(getContext().getMainLooper());
		mainHandler.post(new AddsShowRunnable(this, adview, layout));
	}

	@Override
	protected void doHideAds() {
		Handler mainHandler = new Handler(getContext().getMainLooper());
		mainHandler.post(new HideAddsRunnable(this, adview, layout));
	}

	@Override
	protected void addSkus(List<String> moreSkus) {
		super.addSkus(moreSkus);
		moreSkus.add("autosolve");
	}

	@Override
	public String getName() {
		return "cavepacker";
	}
}
