#include "engine/common/TimeManager.h"
#include "tests/TimeManagerTest.h"

class TimeManagerTestCallback {
public:
	TimeManagerTestCallback() :
			callbackNoParamCalled(false), callbackWithParamCalled(false), callbackParamValue(
					0.0f) {
	}

	bool callbackNoParamCalled;
	bool callbackWithParamCalled;
	float callbackParamValue;

	void timeoutNoParamCallback() {
		callbackNoParamCalled = true;
	}

	void timeoutWithParamCallback(void* param) {
		callbackWithParamCalled = true;
		callbackParamValue = *(float*) param;
	}
};

static TimeManagerTestCallback timeManagerTestCallback;

TEST(TimeManagerTest, testTimeoutNoParam) {
	TimeManager time;
	timeManagerTestCallback.callbackNoParamCalled = false;
	time.setTimeout(1, &timeManagerTestCallback,
			&TimeManagerTestCallback::timeoutNoParamCallback);
	time.update(1);
	ASSERT_TRUE(timeManagerTestCallback.callbackNoParamCalled);
}

TEST(TimeManagerTest, testTimeoutWithParam) {
	TimeManager time;
	timeManagerTestCallback.callbackWithParamCalled = false;
	float param = 5.50f;
	time.setTimeout(1, &timeManagerTestCallback,
			&TimeManagerTestCallback::timeoutWithParamCallback, (void*) &param);
	time.update(1);
	ASSERT_TRUE(timeManagerTestCallback.callbackWithParamCalled);
	ASSERT_EQ(timeManagerTestCallback.callbackParamValue, param);
}
