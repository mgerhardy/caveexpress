package org.caveexpressouya;

import java.io.InputStream;

import org.base.BaseActivity;

import tv.ouya.console.api.OuyaFacade;
import android.os.Bundle;
import android.util.Log;

public class CaveExpress extends BaseActivity {
	private final GamerInfoResponseHandler gamerInfoListener = new GamerInfoResponseHandler();
	private final PurchaseHandler purchaseListener = new PurchaseHandler();

	@Override
	protected String getPublicKey() {
		return "";
	}

	@Override
	protected void configureBilling() {
	}

	@Override
	protected void doDestroyBilling() {
	}

	@Override
	protected void doShowAds() {
	}

	@Override
	protected void doHideAds() {
	}

	@Override
	protected boolean doBuyItem(String id) {
		// Product product = new Product(id);
		// purchaseListener.requestPurchase(product);
		// TODO:
		return false;
	}

	@Override
	protected boolean doHasItem(String id) {
		return true;
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		// moreSkus.add("foobar");

		// paymentEntries.add(new PaymentEntry(skuDetails.getSku(),
		// skuDetails.getDescription(), skuDetails.getPrice()));

		try {
			InputStream is = getResources().openRawResource(R.raw.key);
			purchaseListener.init(is);
		} catch (Exception e) {
			Log.e(NAME, "Unable to open raw encryption key", e);
		}

		OuyaFacade.getInstance().requestGamerInfo(gamerInfoListener);

		// Init the controller
		// OuyaController.init(context);
	}

	@Override
	protected boolean doShowFullscreenAds() {
		return false;
	}
}
