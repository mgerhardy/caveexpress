package org.caveexpress;

import org.base.game.BaseGameAdsActivity;

/**
 * Activity for the free version of the game with payment support from its super
 * class and ads
 */
public class CaveExpress extends BaseGameAdsActivity {
	@Override
	protected String getPublicKey() {
		return "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4KxbS8KykHzthKhIkWcfbGeJsUAApOtLH5t4jWxLy2j6ISpL72w66a6cEjB+YC8hJSPQ8TjvAKCHAqSmi23F1f9eUiuJ8WhMysESfJuyOdCXdQ4swVsKDLdwcR7tu69X5yEoJd0eHJ0TaEu5YXd8FPg6VRzxIf9H+NlDyeDGIai332kwlbyaPtyNGym8BnDpKh1TwC1bEwNYbZiL+CBe/1B+ZxxdTkGb/0/Sp4y+TpjD42vhQp9VzeiEeJqYlP6c1A0Dh1guKPEIAiowmyjm2rKCya85EKC4IUl9/QgyO/BGZFG13Em/JIr9rFoGygZ3igYLlWuWeUfJUTmgcBJchwIDAQAB";
	}

	@Override
	public String getName() {
		return "caveexpress";
	}

	@Override
	protected String getTrackerId() {
		return "UA-54860800-3";
	}
}
