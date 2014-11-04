package org.cavepacker;

import java.util.List;

import org.base.game.BaseGameAdsActivity;

/**
 * Activity for the free version of the game with payment support from its super
 * class and ads
 */
public class CavePacker extends BaseGameAdsActivity {
	@Override
	protected String getPublicKey() {
		return "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAh3ajm+zj1dmcZUU17kxEJVTxSX1ljDy5dA6/GX0fonk7lSchsv9ieyd0sJ9uRk3nJEArfCMV4fSWz2zZZJc3DUODyWH5UtU5YyAFO2/qpjywqNM1GG2Iir1fmO0ckhYwmuLCUz4rJByhlJGfGSxoYo6uanYFkBPZ3b6sRcx4PrBp47jHF/OoIsAAppwXEoD14kh596npAa3CQurXThGhuaFbgYRqiVNplwQj4BjncyfFroLC5NW/k3zEa/cp0Dj6XKV5z1PzEjX7/Q8X7Rw21CS0aMnlhsVbaWKe2D8T+zjaI4pmq2JHOh904CE+3rpCWrZPkI86RwLfovahfbfWkwIDAQAB";
	}

	@Override
	protected String getAdUnitId() {
		return "ca-app-pub-5370378935428600/7187948375";
	}

	@Override
	protected String getInterstitialAdUnitId() {
		return "ca-app-pub-5370378935428600/3242099978";
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

	@Override
	protected String getTrackerId() {
		return "UA-54860800-2";
	}
}
