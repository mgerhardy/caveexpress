#include "Android.h"
#include "engine/common/Logger.h"
#include "engine/common/Version.h"
#include "engine/common/Payment.h"
#include "engine/common/Config.h"
#include "engine/common/System.h"
#include <SDL_config.h>
#include <SDL_assert.h>
#include <SDL_main.h>
#include <SDL_system.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/system_properties.h>

/**
 * http://developer.android.com/guide/practices/design/jni.html
 */

class LocalReferenceHolder {
private:
	static int s_active;

public:
	static bool IsActive ()
	{
		return s_active > 0;
	}

public:
	LocalReferenceHolder () :
			m_env(nullptr)
	{
	}
	~LocalReferenceHolder ()
	{
		if (m_env) {
			m_env->PopLocalFrame(nullptr);
			--s_active;
		}
	}

	bool init (JNIEnv *env, jint capacity = 16)
	{
		if (env->PushLocalFrame(capacity) < 0) {
			error(LOG_SYSTEM, "Failed to allocate enough JVM local references");
			return false;
		}
		++s_active;
		m_env = env;
		return true;
	}

protected:
	JNIEnv *m_env;
};
int LocalReferenceHolder::s_active;

#define _ANDROID_LOG(prio, fmt, args...) __android_log_print(prio, APPFULLNAME, fmt, ## args)
#define _logDebug(fmt, args...) _ANDROID_LOG(ANDROID_LOG_DEBUG, fmt, ##args)
#define _logInfo(fmt, args...) _ANDROID_LOG(ANDROID_LOG_INFO, fmt, ##args)
#define _logWarn(fmt, args...) _ANDROID_LOG(ANDROID_LOG_WARN, fmt, ##args)
#define _logError(fmt, args...) _ANDROID_LOG(ANDROID_LOG_ERROR, fmt, ##args)

Android::Android () :
		Unix(), _env(nullptr), _cls(nullptr), _assetManager(nullptr), _showAds(nullptr), _hideAds(nullptr),
		_showFullscreenAds(nullptr), _openURL(nullptr), _buyItem(nullptr), _hasItem(nullptr), _isOUYA(nullptr),
		_isSmallScreen(nullptr), _minimize(nullptr), _getPaymentEntries(nullptr), _externalState(0) {
}

void Android::init() {
	LocalReferenceHolder refs;

	JNIEnv *env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
	if (env == nullptr) {
		info(LOG_SYSTEM, "could not init the jni env");
		return;
	}
	if (!refs.init(env)) {
		info(LOG_SYSTEM, "could not init the ref holder");
		return;
	}

	jobject activity = static_cast<jobject>(SDL_AndroidGetActivity());
	jclass cls = env->GetObjectClass(activity);
	// context = SDLActivity.getContext();
	jmethodID mid = env->GetStaticMethodID(cls, "getContext", "()Landroid/content/Context;");
	if (mid == nullptr) {
		error(LOG_SYSTEM, "error getting getContext");
		return;
	}

	jobject context = env->CallStaticObjectMethod(cls, mid);
	if (context == nullptr) {
		error(LOG_SYSTEM, "error calling getContext");
		return;
	}

	// assetManager = context.getAssets();
	jclass assetManagerClass = env->GetObjectClass(context);
	mid = env->GetMethodID(assetManagerClass, "getAssets", "()Landroid/content/res/AssetManager;");
	if (mid == nullptr) {
		error(LOG_SYSTEM, "error getting getAssets");
		return;
	}
	jobject assetManager = env->CallObjectMethod(context, mid);
	if (assetManager == nullptr) {
		error(LOG_SYSTEM, "error calling getAssets");
		return;
	}

	_env = env;
	_cls = reinterpret_cast<jclass>(_env->NewGlobalRef(cls));
	_assetManager = reinterpret_cast<jobject>(_env->NewGlobalRef(assetManager));

	_showAds = _env->GetStaticMethodID(_cls, "showAds", "()V");
	_hideAds = _env->GetStaticMethodID(_cls, "hideAds", "()V");
	_showFullscreenAds = _env->GetStaticMethodID(_cls, "showFullscreenAds", "()Z");
	_openURL = _env->GetStaticMethodID(_cls, "openURL", "(Ljava/lang/String;)V");
	_buyItem = env->GetStaticMethodID(_cls, "buyItem", "(Ljava/lang/String;)Z");
	_hasItem = env->GetStaticMethodID(_cls, "hasItem", "(Ljava/lang/String;)Z");
	_track = env->GetStaticMethodID(_cls, "track", "(Ljava/lang/String;Ljava/lang/String;)Z");
	_isOUYA = env->GetStaticMethodID(_cls, "isOUYA", "()Z");
	_isSmallScreen = env->GetStaticMethodID(_cls, "isSmallScreen", "()Z");
	_minimize = env->GetStaticMethodID(_cls, "minimize", "()V");
	_getPaymentEntries = env->GetStaticMethodID(_cls, "getPaymentEntries", "()[Lorg/PaymentEntry;");
	_getLocale = env->GetStaticMethodID(_cls, "getLocale", "()Ljava/lang/String;");

	if (_showAds == 0) {
		error(LOG_SYSTEM, "error getting showAds()");
	}
	if (_hideAds == 0) {
		error(LOG_SYSTEM, "error getting hideAds()");
	}
	if (_showFullscreenAds == 0) {
		error(LOG_SYSTEM, "error getting showFullscreenAds()");
	}
	if (_openURL == 0) {
		error(LOG_SYSTEM, "error getting openURL()");
	}
	if (_buyItem == 0) {
		error(LOG_SYSTEM, "error getting buyItem()");
	}
	if (_minimize == 0) {
		error(LOG_SYSTEM, "error getting minimize()");
	}
	if (_track == 0) {
		error(LOG_SYSTEM, "error getting track()");
	}
	if (_hasItem == 0) {
		error(LOG_SYSTEM, "error getting hasItem()");
	}
	if (_isOUYA == 0) {
		error(LOG_SYSTEM, "error getting isOUYA()");
	}
	if (_isSmallScreen == 0) {
		error(LOG_SYSTEM, "error getting isSmallScreen()");
	}
	if (_getPaymentEntries == 0) {
		error(LOG_SYSTEM, "error getting getPaymentEntries()");
	}
	if (_getLocale == 0) {
		error(LOG_SYSTEM, "error getting getLocale()");
	}

	info(LOG_SYSTEM, String::format("Running on: [%s] [%s] [%s] [%s] [%s] SDK:%s ABI:%s",
			getSystemProperty("ro.product.manufacturer").c_str(),
			getSystemProperty("ro.product.model").c_str(),
			getSystemProperty("ro.product.brand").c_str(),
			getSystemProperty("ro.build.fingerprint").c_str(),
			getSystemProperty("ro.build.display.id").c_str(),
			getSystemProperty("ro.build.version.sdk").c_str(),
			getSystemProperty("ro.product.cpu.abi").c_str()));

	info(LOG_SYSTEM, String::format("internal storage path: %s", SDL_AndroidGetInternalStoragePath()));
	_externalState = SDL_AndroidGetExternalStorageState();
	if (_externalState)
		info(LOG_SYSTEM, String::format("external storage path: %s", SDL_AndroidGetExternalStoragePath()));
	else
		info(LOG_SYSTEM, "no external storage path with write support");

	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");

	const bool smallScreen = isSmallScreen(nullptr);
	if (smallScreen) {
		info(LOG_SYSTEM, "running on a small screen");
	} else {
		info(LOG_SYSTEM, "running on a large screen");
	}
}

Android::~Android ()
{
	if (_env) {
		if (_assetManager)
			_env->DeleteGlobalRef(_assetManager);
		_assetManager = nullptr;
		if (_cls)
			_env->DeleteGlobalRef(_cls);
		_cls = nullptr;
	}

	_env = nullptr;
}

std::string Android::getHomeDirectory ()
{
	std::string path;
	if (_externalState & SDL_ANDROID_EXTERNAL_STORAGE_WRITE)
		path = SDL_AndroidGetExternalStoragePath();
	else
		path = SDL_AndroidGetInternalStoragePath();
	return path + "/";
}

String Android::getSystemProperty (const char *name) const
{
	char value[PROP_VALUE_MAX];
	const int len = __system_property_get(name, value);
	return String(value, len);
}

// Test for an exception and call SDL_SetError with its detail if one occurs
bool Android::testException ()
{
	if (!LocalReferenceHolder::IsActive()) {
		error(LOG_SYSTEM, "failed to test exceptions, the local ref holder is not active");
	}

	jthrowable exception = _env->ExceptionOccurred();
	if (exception != nullptr) {
		jmethodID mid;

		// Until this happens most JNI operations have undefined behaviour
		_env->ExceptionClear();

		jclass exceptionClass = _env->GetObjectClass(exception);
		jclass classClass = _env->FindClass("java/lang/Class");

		mid = _env->GetMethodID(classClass, "getName", "()Ljava/lang/String;");
		jstring exceptionName = reinterpret_cast<jstring>(_env->CallObjectMethod(exceptionClass, mid));
		const char* exceptionNameUTF8 = _env->GetStringUTFChars(exceptionName, 0);

		mid = _env->GetMethodID(exceptionClass, "getMessage", "()Ljava/lang/String;");
		jstring exceptionMessage = reinterpret_cast<jstring>(_env->CallObjectMethod(exception, mid));

		if (exceptionMessage != nullptr) {
			const char* exceptionMessageUTF8 = _env->GetStringUTFChars(exceptionMessage, 0);
			error(LOG_SYSTEM, String::format("%s: %s", exceptionNameUTF8, exceptionMessageUTF8));
			_env->ReleaseStringUTFChars(exceptionMessage, exceptionMessageUTF8);
		} else {
			error(LOG_SYSTEM, String::format("%s", exceptionNameUTF8));
		}

		_env->ReleaseStringUTFChars(exceptionName, exceptionNameUTF8);

		return true;
	}

	return false;
}

std::string Android::getDatabaseDirectory ()
{
	// data/data/your.game.id/databases
	return getHomeDirectory();
}

bool Android::hasMouseOrFinger ()
{
	return !isOUYA();
}

DirectoryEntries Android::listDirectory (const std::string& basedir, const std::string& subdir)
{
	if (basedir[0] == '/')
		return Unix::listDirectory(basedir, subdir);

	DirectoryEntries entries;

	LocalReferenceHolder refs;

	if (_env == nullptr || !refs.init(_env)) {
		error(LOG_SYSTEM, "error while listing directory entries");
		return entries;
	}

	std::string modified = basedir;
	const size_t l = modified.size() - 1;
	if (modified[l] == '/')
		modified.erase(l);

	// String[] files = assetManager.list(<path>);
	jclass assetManagerClass = _env->GetObjectClass(_assetManager);
	jmethodID mid = _env->GetMethodID(assetManagerClass, "list", "(Ljava/lang/String;)[Ljava/lang/String;");
	if (testException() || mid == nullptr) {
		error(LOG_SYSTEM, "error getting list");
		return entries;
	}
	jobjectArray list = reinterpret_cast<jobjectArray>(_env->CallObjectMethod(_assetManager, mid,
			_env->NewStringUTF(modified.c_str())));
	if (testException() || list == nullptr) {
		error(LOG_SYSTEM, "error calling list");
		return entries;
	}
	jsize n = _env->GetArrayLength(list);

	debug(LOG_SYSTEM, "list " + modified + " with " + string::toString((int)n) + " elements");

	for (int i = 0; i < n; ++i) {
		jstring str = reinterpret_cast<jstring>(_env->GetObjectArrayElement(list, i));
		if (testException())
			break;
		const char* cstr = _env->GetStringUTFChars(str, 0);
		entries.push_back(cstr);
		debug(LOG_SYSTEM, String::format("%s", cstr));
		_env->ReleaseStringUTFChars(str, cstr);
		_env->DeleteLocalRef(str);
	}

	testException();

	return entries;
}

bool Android::showFullscreenAds ()
{
	const bool retVal = _env->CallStaticBooleanMethod(_cls, _showFullscreenAds);
	return retVal;
}

void Android::showAds (bool show)
{
	if (_showAds == 0 || _hideAds == 0)
		return;

	if (show) {
		_env->CallStaticVoidMethod(_cls, _showAds);
	} else {
		_env->CallStaticVoidMethod(_cls, _hideAds);
	}
}

void Android::achievementUnlocked (const std::string& id)
{
}

bool Android::hasAchievement (const std::string& id)
{
	return false;
}

bool Android::track (const std::string& hitType, const std::string& screenName)
{
	LocalReferenceHolder refs;

	if (_env == nullptr || !refs.init(_env)) {
		error(LOG_SYSTEM, "error while calling track");
		return false;
	}

	jstring hitTypeJavaStr = _env->NewStringUTF(hitType.c_str());
	jstring screenNameJavaStr = _env->NewStringUTF(screenName.c_str());
	const bool retVal = _env->CallStaticBooleanMethod(_cls, _track, hitTypeJavaStr, screenNameJavaStr);
	_env->DeleteLocalRef(hitTypeJavaStr);
	_env->DeleteLocalRef(screenNameJavaStr);

	testException();

	return retVal;
}

std::string Android::getLanguage ()
{
	LocalReferenceHolder refs;

	if (_env == nullptr || !refs.init(_env)) {
		error(LOG_SYSTEM, "error while getting the payment entries");
		return "";
	}

	jstring locale = reinterpret_cast<jstring>(_env->CallObjectMethod(_cls, _getLocale));
	if (testException() || locale == nullptr) {
		error(LOG_SYSTEM, "error calling getLocale()");
		return "";
	}
	const char *js = _env->GetStringUTFChars(locale, nullptr);
	const std::string cs(js);
	_env->ReleaseStringUTFChars(locale, js);
	return cs;
}

void Android::getPaymentEntries (std::vector<PaymentEntry>& entries)
{
	LocalReferenceHolder refs;

	if (_env == nullptr || !refs.init(_env)) {
		error(LOG_SYSTEM, "error while getting the payment entries");
		return;
	}

	jobjectArray list = reinterpret_cast<jobjectArray>(_env->CallObjectMethod(_cls, _getPaymentEntries));
	if (testException() || list == nullptr) {
		error(LOG_SYSTEM, "error calling getPaymentEntries()");
		return;
	}

	jsize n = _env->GetArrayLength(list);
	info(LOG_SYSTEM, "payment items found: " + string::toString((int)n));

	// same order as the paymententry ctor!
	const char *ids[] = { "name", "id", "price", nullptr };
	std::vector<std::string> memberValues;

	for (int i = 0; i < n; ++i) {
		jobject entryObj = reinterpret_cast<jobject>(_env->GetObjectArrayElement(list, i));
		if (testException())
			break;
		jclass entryClass = _env->GetObjectClass(entryObj);
		if (entryClass == nullptr) {
			error(LOG_SYSTEM, "could not get entry object class");
			break;
		}

		memberValues.clear();
		for (const char **memberName = ids; *memberName != nullptr; ++memberName) {
			jfieldID fieldId = _env->GetFieldID(entryClass, *memberName, "Ljava/lang/String;");
			jstring str = reinterpret_cast<jstring>(_env->GetObjectField(entryObj, fieldId));
			const char* cstr = _env->GetStringUTFChars(str, 0);
			const std::string memberValue = cstr;
			memberValues.push_back(memberValue);
			info(LOG_SYSTEM, memberValue);
			_env->ReleaseStringUTFChars(str, cstr);
			_env->DeleteLocalRef(str);
		}

		entries.push_back(PaymentEntry(memberValues[0], memberValues[1], memberValues[2]));
	}

	testException();

	// TODO: get this data from the play store
	//entries.push_back(PaymentEntry("Remove ads", "adfree", "1.79"));
	//entries.push_back(PaymentEntry("Unlock all maps and campaigns", "unlockall", "1.50"));
}

bool Android::hasItem (const std::string& id)
{
	LocalReferenceHolder refs;

	if (_env == nullptr || !refs.init(_env)) {
		error(LOG_SYSTEM, "error while getting the payment entries");
		return false;
	}

	jstring str = _env->NewStringUTF(id.c_str());
	const bool retVal = _env->CallStaticBooleanMethod(_cls, _hasItem, str);
	_env->DeleteLocalRef(str);

	testException();

	return retVal;
}

bool Android::buyItem (const std::string& id)
{
	LocalReferenceHolder refs;

	if (_env == nullptr || !refs.init(_env)) {
		error(LOG_SYSTEM, "error while getting the payment entries");
		return false;
	}

	jstring str = _env->NewStringUTF(id.c_str());
	const bool retVal = _env->CallStaticBooleanMethod(_cls, _buyItem, str);
	_env->DeleteLocalRef(str);

	testException();

	return retVal;
}

bool Android::wantCursor ()
{
	return false;
}

bool Android::quit ()
{
	_env->CallStaticBooleanMethod(_cls, _minimize);
	return true;
}

bool Android::isSmallScreen (IFrontend*)
{
	return _env->CallStaticBooleanMethod(_cls, _isSmallScreen);
}

bool Android::supportFocusChange ()
{
	return isOUYA();
}

bool Android::supportPayment ()
{
	// TODO: odk
	if (isOUYA())
		return false;
	return true;
}

int Android::getScreenPadding()
{
	if (isOUYA()) {
		return 110;
	}

	return Unix::getScreenPadding();
}

void Android::notifyPaymentLoaded ()
{
}

bool Android::isOUYA () const
{
    return _env->CallStaticBooleanMethod(_cls, _isOUYA);
}

int Android::openURL (const std::string& url) const
{
	if (_openURL == 0)
		return -1;

	jstring str = _env->NewStringUTF(url.c_str());
	_env->CallStaticVoidMethod(_cls, _openURL, str);
	_env->DeleteLocalRef(str);
	return 0;
}

bool Android::hasTouch () const
{
	return !isOUYA();
}

void Android::exit (const std::string& reason, int errorCode)
{
	if (errorCode != 0) {
		logError(reason);
	} else {
		logOutput(reason);
	}

	if (_env) {
		_env->DeleteGlobalRef(_assetManager);
		_env->DeleteGlobalRef(_cls);
		_env = nullptr;
	}

	::exit(errorCode);
}

void Android::tick (uint32_t deltaTime)
{
}

int Android::getAdHeight() const
{
	// AdSize.BANNER = 320x50
	return 50;
}

extern "C" JNIEXPORT void JNICALL Java_org_base_BaseActivity_onPaymentDone(JNIEnv* env, jclass jcls)
{
	info(LOG_SYSTEM, "onPaymentDone c side");
	Android& s = static_cast<Android&>(getSystem());
	s.notifyPaymentLoaded();
}

extern "C" JNIEXPORT jboolean JNICALL Java_org_base_BaseActivity_isDebug(JNIEnv* env, jclass jcls)
{
	jboolean debug;
#ifdef DEBUG
	debug = 1;
#else
	debug = 0;
#endif
	return debug;
}

extern "C" JNIEXPORT jboolean JNICALL Java_org_base_BaseActivity_isTrackingOptOut(JNIEnv* env, jclass jcls)
{
	info(LOG_SYSTEM, "isTrackingOptOut c side");
	// TODO:
	jboolean optout = 0;
	return output;
}

extern "C" JNIEXPORT jboolean JNICALL Java_org_base_BaseActivity_isHD(JNIEnv* env, jclass jcls)
{
	jboolean hd;
#ifdef HD_VERSION
	hd = 1;
#else
	hd = 0;
#endif
	return hd;
}
