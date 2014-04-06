package org.caveexpressouya;

import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.security.GeneralSecurityException;
import java.security.KeyFactory;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.spec.X509EncodedKeySpec;
import java.util.HashMap;
import java.util.Map;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

import org.base.BaseActivity;
import org.json.JSONException;
import org.json.JSONObject;

import tv.ouya.console.api.CancelIgnoringOuyaResponseListener;
import tv.ouya.console.api.OuyaEncryptionHelper;
import tv.ouya.console.api.OuyaErrorCodes;
import tv.ouya.console.api.OuyaFacade;
import tv.ouya.console.api.Product;
import tv.ouya.console.api.Purchasable;
import android.os.Bundle;
import android.util.Base64;
import android.util.Log;

final class PurchaseHandler extends CancelIgnoringOuyaResponseListener<String> {
	private PublicKey publicKey;
	private final Map<String, Product> outstandingPurchaseRequests = new HashMap<String, Product>();

	@Override
	public void onSuccess(String result) {
		try {
			OuyaEncryptionHelper helper = new OuyaEncryptionHelper();
			JSONObject response = new JSONObject(result);

			String id = helper.decryptPurchaseResponse(response, publicKey);
			Product storedProduct;
			synchronized (outstandingPurchaseRequests) {
				storedProduct = outstandingPurchaseRequests.remove(id);
			}
			if (storedProduct == null) {
				onFailure(OuyaErrorCodes.THROW_DURING_ON_SUCCESS,
						"No purchase outstanding for the given purchase request", Bundle.EMPTY);
				return;
			}

			Log.d(BaseActivity.NAME, "Congrats you bought: " + storedProduct.getName());
		} catch (Exception e) {
			Log.e(BaseActivity.NAME, "Your purchase failed.", e);
		}
	}

	public void requestPurchase(final Product product) throws GeneralSecurityException, UnsupportedEncodingException,
			JSONException {
		SecureRandom sr = SecureRandom.getInstance("SHA1PRNG");

		/*
		 * This is an ID that allows you to associate a successful purchase with
		 * it's original request. The server does nothing with this string
		 * except pass it back to you, so it only needs to be unique within this
		 * instance of your app to allow you to pair responses with requests.
		 */
		String uniqueId = Long.toHexString(sr.nextLong());

		JSONObject purchaseRequest = new JSONObject();
		purchaseRequest.put("uuid", uniqueId);
		purchaseRequest.put("identifier", product.getIdentifier());
		/*
		 * This value is only needed for testing, not setting it results in a
		 * live purchase
		 */
		purchaseRequest.put("testing", "true");
		String purchaseRequestJson = purchaseRequest.toString();

		byte[] keyBytes = new byte[16];
		sr.nextBytes(keyBytes);
		SecretKey key = new SecretKeySpec(keyBytes, "AES");

		byte[] ivBytes = new byte[16];
		sr.nextBytes(ivBytes);
		IvParameterSpec iv = new IvParameterSpec(ivBytes);

		Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding", "BC");
		cipher.init(Cipher.ENCRYPT_MODE, key, iv);
		byte[] payload = cipher.doFinal(purchaseRequestJson.getBytes("UTF-8"));

		cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding", "BC");
		cipher.init(Cipher.ENCRYPT_MODE, publicKey);
		byte[] encryptedKey = cipher.doFinal(keyBytes);

		Purchasable purchasable = new Purchasable(product.getIdentifier(), Base64.encodeToString(encryptedKey,
				Base64.NO_WRAP), Base64.encodeToString(ivBytes, Base64.NO_WRAP), Base64.encodeToString(payload,
				Base64.NO_WRAP));

		synchronized (outstandingPurchaseRequests) {
			outstandingPurchaseRequests.put(uniqueId, product);
		}
		OuyaFacade.getInstance().requestPurchase(purchasable, this);
	}

	public void init(InputStream applicationKeyInputStream) {
		/*
		 * Create a PublicKey object from the key data downloaded from the
		 * developer portal.
		 */
		try {
			/* Read in the key.der file (downloaded from the developer portal) */
			byte[] applicationKey = new byte[applicationKeyInputStream.available()];
			applicationKeyInputStream.read(applicationKey);
			applicationKeyInputStream.close();
			/* Create a public key */
			X509EncodedKeySpec keySpec = new X509EncodedKeySpec(applicationKey);
			KeyFactory keyFactory = KeyFactory.getInstance("RSA");
			publicKey = keyFactory.generatePublic(keySpec);
		} catch (Exception e) {
			Log.e(BaseActivity.NAME, "Unable to create encryption key", e);
		}
	}

	@Override
	public void onFailure(int errorCode, String errorMessage, Bundle errorBundle) {
		Log.e(BaseActivity.NAME, errorMessage);
	}
}
