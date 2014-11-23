#include "GooglePlayPersister.h"
#include <SDL_system.h>

#ifdef GOOGLEPLAY_ACTIVE
class GPLocalReferenceHolder {
private:
	static int s_active;

public:
	static bool IsActive ()
	{
		return s_active > 0;
	}

public:
	GPLocalReferenceHolder () :
			m_env(nullptr)
	{
	}
	~GPLocalReferenceHolder ()
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
int GPLocalReferenceHolder::s_active;
#endif


GooglePlayPersister::GooglePlayPersister() :
		IGameStatePersister() {
}

GooglePlayPersister::~GooglePlayPersister() {
#ifdef GOOGLEPLAY_ACTIVE
#endif
}

bool GooglePlayPersister::init() {
#ifdef GOOGLEPLAY_ACTIVE
	GPLocalReferenceHolder refs;

	JNIEnv *env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
	if (env == nullptr) {
		return false;
	}
	if (!refs.init(env)) {
		info(LOG_SYSTEM, "could not init the ref holder");
		return false;
	}

	jobject activity = static_cast<jobject>(SDL_AndroidGetActivity());
	jclass cls = env->GetObjectClass(activity);

	_env = env;
	_cls = reinterpret_cast<jclass>(_env->NewGlobalRef(cls));

	_persisterInit = env->GetStaticMethodID(_cls, "persisterInit", "()Z");
	_persisterConnect = env->GetStaticMethodID(_cls, "persisterConnect", "()Z");
	_persisterDisconnect = env->GetStaticMethodID(_cls, "persisterDisconnect", "()Z");

	_saveCampaign = _env->GetStaticMethodID(_cls, "saveCampaign", "(Lorg/CampaignStub;)V");
	if (_saveCampaign == 0) {
		error(LOG_SYSTEM, "Could not get the jni bindings for saveCampaign");
		_env = nullptr;
		return false;
	}

	_loadCampaign = _env->GetStaticMethodID(_cls, "loadCampaign", "(Ljava/lang/String;)Lorg/CampaignStub;");
	if (_loadCampaign == 0) {
		error(LOG_SYSTEM, "Could not get the jni bindings for loadCampaign");
		_env = nullptr;
		return false;
	}

	return true;
#endif
	return false;
}

// Test for an exception and call SDL_SetError with its detail if one occurs
bool GooglePlayPersister::testException ()
{
#ifdef GOOGLEPLAY_ACTIVE
	if (!GPLocalReferenceHolder::IsActive()) {
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
#endif
	return false;
}

bool GooglePlayPersister::saveCampaign(Campaign* campaign) {
#ifdef GOOGLEPLAY_ACTIVE
	GPLocalReferenceHolder refs;

	if (_env == nullptr || !refs.init(_env)) {
		error(LOG_SYSTEM, "error while saving the campaign");
		return false;
	}

#endif
	return false;
}

bool GooglePlayPersister::loadCampaign(Campaign* campaign) {
#ifdef GOOGLEPLAY_ACTIVE
#endif
	return false;
}

bool GooglePlayPersister::reset() {
#ifdef GOOGLEPLAY_ACTIVE
#endif
	return false;
}

bool GooglePlayPersister::resetCampaign(Campaign* campaign) {
#ifdef GOOGLEPLAY_ACTIVE
#endif
	return false;
}
