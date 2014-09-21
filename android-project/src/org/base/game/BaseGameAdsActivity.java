package org.base.game;

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
 * Base activity class with ads
 */
public abstract class BaseGameAdsActivity extends BaseGameActivity {
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
		interstitial.setAdUnitId("ca-app-pub-5370378935428600/4094881177");
		interstitial.setAdListener(new AdListener() {
			@Override
			public void onAdClosed() {
				reloadInterstitial();
			};
		});
		reloadInterstitial();

		adview = new AdView(mSingleton);
		adview.setAdUnitId("ca-app-pub-5370378935428600/2618147974");
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
}
