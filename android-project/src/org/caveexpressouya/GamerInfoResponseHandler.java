package org.caveexpressouya;

import org.base.BaseActivity;

import tv.ouya.console.api.CancelIgnoringOuyaResponseListener;
import tv.ouya.console.api.GamerInfo;
import android.os.Bundle;
import android.util.Log;

final class GamerInfoResponseHandler extends CancelIgnoringOuyaResponseListener<GamerInfo> {
	private String uuid;
	private String username;

	@Override
	public void onSuccess(GamerInfo result) {
		uuid = result.getUuid();
		Log.d(BaseActivity.NAME, "UUID is: " + uuid);
		username = result.getUsername();
		Log.d(BaseActivity.NAME, "Username is: " + username);
	}

	@Override
	public void onFailure(int errorCode, String errorMessage, Bundle errorBundle) {
		Log.e(BaseActivity.NAME, errorMessage);
	}

	public String getUsername() {
		return username;
	}

	public String getUuid() {
		return uuid;
	}
}
