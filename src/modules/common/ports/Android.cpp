#include "Android.h"
#include "common/Log.h"
#include "common/Config.h"
#include "common/System.h"
#include "common/ConfigManager.h"
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
			Log::error(LOG_COMMON, "Failed to allocate enough JVM local references");
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

Android::Android () :
		Unix(), _env(nullptr), _cls(nullptr), _assetManager(nullptr),
		_openURL(nullptr), _track(nullptr), _buyItem(nullptr),
		_isSmallScreen(nullptr), _minimize(nullptr), _externalState(0) {
}

void Android::init() {
	LocalReferenceHolder refs;

	JNIEnv *env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
	if (env == nullptr) {
		Log::info(LOG_COMMON, "could not init the jni env");
		return;
	}
	if (!refs.init(env)) {
		Log::info(LOG_COMMON, "could not init the ref holder");
		return;
	}

	jobject activity = static_cast<jobject>(SDL_AndroidGetActivity());
	jclass cls = env->GetObjectClass(activity);
	// context = SDLActivity.getContext();
	jmethodID mid = env->GetStaticMethodID(cls, "getContext", "()Landroid/content/Context;");
	if (mid == nullptr) {
		Log::error(LOG_COMMON, "error getting getContext");
		return;
	}

	jobject context = env->CallStaticObjectMethod(cls, mid);
	if (context == nullptr) {
		Log::error(LOG_COMMON, "error calling getContext");
		return;
	}

	// assetManager = context.getAssets();
	jclass assetManagerClass = env->GetObjectClass(context);
	mid = env->GetMethodID(assetManagerClass, "getAssets", "()Landroid/content/res/AssetManager;");
	if (mid == nullptr) {
		Log::error(LOG_COMMON, "error getting getAssets");
		return;
	}
	jobject assetManager = env->CallObjectMethod(context, mid);
	if (assetManager == nullptr) {
		Log::error(LOG_COMMON, "error calling getAssets");
		return;
	}

	_env = env;
	_cls = reinterpret_cast<jclass>(_env->NewGlobalRef(cls));
	_assetManager = reinterpret_cast<jobject>(_env->NewGlobalRef(assetManager));

	_openURL = _env->GetStaticMethodID(_cls, "openURL", "(Ljava/lang/String;)V");
	_track = env->GetStaticMethodID(_cls, "track", "(Ljava/lang/String;Ljava/lang/String;)Z");
	_achievementUnlocked = env->GetStaticMethodID(_cls, "achievementUnlocked", "(Ljava/lang/String;Z)V");
	_isSmallScreen = env->GetStaticMethodID(_cls, "isSmallScreen", "()Z");
	_minimize = env->GetStaticMethodID(_cls, "minimize", "()V");
	_getLocale = env->GetStaticMethodID(_cls, "getLocale", "()Ljava/lang/String;");

	if (_openURL == 0) {
		Log::error(LOG_COMMON, "error getting openURL()");
	}
	if (_minimize == 0) {
		Log::error(LOG_COMMON, "error getting minimize()");
	}
	if (_track == 0) {
		Log::error(LOG_COMMON, "error getting track()");
	}
	if (_isSmallScreen == 0) {
		Log::error(LOG_COMMON, "error getting isSmallScreen()");
	}
	if (_getLocale == 0) {
		Log::error(LOG_COMMON, "error getting getLocale()");
	}
	if (_achievementUnlocked == 0) {
		Log::error(LOG_COMMON, "error getting achievementUnlocked()");
	}

	Log::info(LOG_COMMON, "Running on: [%s] [%s] [%s] [%s] [%s] SDK:%s ABI:%s",
			getSystemProperty("ro.product.manufacturer").c_str(),
			getSystemProperty("ro.product.model").c_str(),
			getSystemProperty("ro.product.brand").c_str(),
			getSystemProperty("ro.build.fingerprint").c_str(),
			getSystemProperty("ro.build.display.id").c_str(),
			getSystemProperty("ro.build.version.sdk").c_str(),
			getSystemProperty("ro.product.cpu.abi").c_str());

	Log::info(LOG_COMMON, "internal storage path: %s", SDL_AndroidGetInternalStoragePath());
	_externalState = SDL_AndroidGetExternalStorageState();
	if (_externalState)
		Log::info(LOG_COMMON, "external storage path: %s", SDL_AndroidGetExternalStoragePath());
	else
		Log::info(LOG_COMMON, "no external storage path with write support");

	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");

	const bool smallScreen = isSmallScreen(nullptr);
	if (smallScreen) {
		Log::info(LOG_COMMON, "running on a small screen");
	} else {
		Log::info(LOG_COMMON, "running on a large screen");
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

std::string Android::getSystemProperty (const char *name) const
{
	char value[PROP_VALUE_MAX];
	const int len = __system_property_get(name, value);
	return std::string(value, len);
}

// Test for an exception and call SDL_SetError with its detail if one occurs
bool Android::testException ()
{
	if (!LocalReferenceHolder::IsActive()) {
		Log::error(LOG_COMMON, "failed to test exceptions, the local ref holder is not active");
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
			Log::error(LOG_COMMON, "%s: %s", exceptionNameUTF8, exceptionMessageUTF8);
			_env->ReleaseStringUTFChars(exceptionMessage, exceptionMessageUTF8);
		} else {
			Log::error(LOG_COMMON, "%s", exceptionNameUTF8);
		}

		_env->ReleaseStringUTFChars(exceptionName, exceptionNameUTF8);

		return true;
	}

	return false;
}

bool Android::hasMouseOrFinger ()
{
	return true;
}

DirectoryEntries Android::listDirectory (const std::string& basedir, const std::string& subdir)
{
	if (basedir[0] == '/')
		return Unix::listDirectory(basedir, subdir);

	DirectoryEntries entries;

	LocalReferenceHolder refs;

	if (_env == nullptr || !refs.init(_env)) {
		Log::error(LOG_COMMON, "error while listing directory entries");
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
		Log::error(LOG_COMMON, "error getting list");
		return entries;
	}
	jobjectArray list = reinterpret_cast<jobjectArray>(_env->CallObjectMethod(_assetManager, mid,
			_env->NewStringUTF(modified.c_str())));
	if (testException() || list == nullptr) {
		Log::error(LOG_COMMON, "error calling list");
		return entries;
	}
	jsize n = _env->GetArrayLength(list);

	Log::debug(LOG_COMMON, "list %s with %i elements", modified.c_str(), (int)n);

	for (int i = 0; i < n; ++i) {
		jstring str = reinterpret_cast<jstring>(_env->GetObjectArrayElement(list, i));
		if (testException())
			break;
		const char* cstr = _env->GetStringUTFChars(str, 0);
		entries.push_back(cstr);
		Log::debug(LOG_COMMON, "%s", cstr);
		_env->ReleaseStringUTFChars(str, cstr);
		_env->DeleteLocalRef(str);
	}

	testException();

	return entries;
}

void Android::achievementUnlocked (const std::string& id, bool increment)
{
	if (_achievementUnlocked == 0) {
		Log::error(LOG_COMMON, "failed to unlock achievement");
		return;
	}

	LocalReferenceHolder refs;

	if (_env == nullptr || !refs.init(_env)) {
		Log::error(LOG_COMMON, "error while calling achievementUnlocked");
		return;
	}

	jstring idJavaStr = _env->NewStringUTF(id.c_str());
	jboolean incrementBool = (jboolean)increment;
	_env->CallStaticVoidMethod(_cls, _achievementUnlocked, idJavaStr, incrementBool);
	_env->DeleteLocalRef(idJavaStr);

	testException();
}

bool Android::hasAchievement (const std::string& id)
{
	return true;
}

bool Android::track (const std::string& hitType, const std::string& screenName)
{
	LocalReferenceHolder refs;

	if (_env == nullptr || !refs.init(_env)) {
		Log::error(LOG_COMMON, "error while calling track");
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
		Log::error(LOG_COMMON, "error while getting the locale");
		return "";
	}

	jstring locale = reinterpret_cast<jstring>(_env->CallObjectMethod(_cls, _getLocale));
	if (testException() || locale == nullptr) {
		Log::error(LOG_COMMON, "error calling getLocale()");
		return "";
	}
	const char *js = _env->GetStringUTFChars(locale, nullptr);
	const std::string cs(js);
	_env->ReleaseStringUTFChars(locale, js);
	return cs;
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
	return false;
}

int Android::openURL (const std::string& url, bool) const
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
	return true;
}

void Android::exit (const std::string& reason, int errorCode)
{
	if (errorCode != 0) {
		Log::error(LOG_COMMON, "%s", reason.c_str());
	} else {
		Log::info(LOG_COMMON, "%s", reason.c_str());
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
	Log::debug(LOG_COMMON, "isTrackingOptOut c side");
	// TODO:
	jboolean optout = 0;
	return optout;
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

extern "C" JNIEXPORT void JNICALL Java_org_base_BaseActivity_onPersisterConnectFailed(JNIEnv* env, jclass jcls)
{
	Log::error(LOG_COMMON, "google play connection failed");
	Config.getConfigVar("googleplaystate", "false", true)->setValue("false");
}

extern "C" JNIEXPORT void JNICALL Java_org_base_BaseActivity_onPersisterConnectSuccess(JNIEnv* env, jclass jcls)
{
	Log::info(LOG_COMMON, "google play connection succeeds");
	Config.getConfigVar("googleplaystate", "false", true)->setValue("true");
}

extern "C" JNIEXPORT void JNICALL Java_org_base_BaseActivity_onPersisterDisconnect(JNIEnv* env, jclass jcls)
{
	Log::info(LOG_COMMON, "google play disconnect");
	Config.getConfigVar("googleplaystate", "false", true)->setValue("false");
}
