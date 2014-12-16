package org.caveexpressouya;

import org.base.BaseActivity;

import tv.ouya.console.api.CancelIgnoringOuyaResponseListener;
import tv.ouya.console.api.GamerInfo;
import android.os.Bundle;
import android.util.Log;

final class GamerInfoResponseHandler extends CancelIgnoringOuyaResponseListener<GamerInfo> {
	private String uuid;
	private String username;
	private final BaseActivity baseActivity;

	GamerInfoResponseHandler(BaseActivity baseActivity) {
		this.baseActivity = baseActivity;
	}

	@Override
	public void onSuccess(GamerInfo result) {
		uuid = result.getUuid();
		Log.d(baseActivity.getName(), "UUID is: " + uuid);
		username = result.getUsername();
		Log.d(baseActivity.getName(), "Username is: " + username);
	}

	@Override
	public void onFailure(int errorCode, String errorMessage, Bundle errorBundle) {
		Log.e(baseActivity.getName(), errorMessage);
	}

	public String getUsername() {
		return username;
	}

	public String getUuid() {
		return uuid;
	}
}
