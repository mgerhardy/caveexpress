#include "GooglePlayPersister.h"
#include "common/CommandSystem.h"
#include "common/ConfigManager.h"
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


GooglePlayPersister::GooglePlayPersister(IGameStatePersister* delegate) :
		IGameStatePersister(), _delegate(delegate)
#ifdef GOOGLEPLAY_ACTIVE
		,
		_env(nullptr), _cls(nullptr), _loadGameState(nullptr), _saveGameState(nullptr),
		_persisterInit(nullptr), _persisterConnect(nullptr), _persisterDisconnect(nullptr),
		_showLeaderBoard(nullptr), _showAchievements(nullptr), _addPointsToLeaderBoard(nullptr)
#endif
{
	Commands.registerCommand("googleplay-connect", bindFunction(GooglePlayPersister, connect));
	Commands.registerCommand("googleplay-disconnect", bindFunction(GooglePlayPersister, disconnect));
	Commands.registerCommand("googleplay-upload", bindFunction(GooglePlayPersister, upload));
	Commands.registerCommand("googleplay-download", bindFunction(GooglePlayPersister, download));
	Commands.registerCommand("googleplay-showachievements", bindFunction(GooglePlayPersister, showAchievements));
	Commands.registerCommand("googleplay-showleaderboard", bindFunction(GooglePlayPersister, showLeaderBoard));
}

GooglePlayPersister::~GooglePlayPersister() {
#ifdef GOOGLEPLAY_ACTIVE
	if (_env) {
		if (_cls)
			_env->DeleteGlobalRef(_cls);
		_cls = nullptr;
	}

	_env = nullptr;
#endif
	delete _delegate;
}

void GooglePlayPersister::showAchievements() {
#ifdef GOOGLEPLAY_ACTIVE
	if (_env == nullptr) {
		error(LOG_SYSTEM, "GoolePlayPersister::connect() failed for the google play persister - no env pointer");
		return;
	}
	_env->CallStaticVoidMethod(_cls, _showAchievements);
#endif
}

void GooglePlayPersister::showLeaderBoard(const std::string& boardId) {
#ifdef GOOGLEPLAY_ACTIVE
	if (_env == nullptr) {
		error(LOG_SYSTEM, "GoolePlayPersister::showLeaderBoard() failed for the google play persister - no env pointer");
		return;
	}
	GPLocalReferenceHolder refs;
	if (!refs.init(_env)) {
		error(LOG_SYSTEM, "GoolePlayPersister::showLeaderBoard(): could not init the ref holder");
		return;
	}

	jstring str = _env->NewStringUTF(boardId.c_str());
	_env->CallStaticVoidMethod(_cls, _showLeaderBoard, str);
	_env->DeleteLocalRef(str);
#endif
}

void GooglePlayPersister::upload() {
#ifdef GOOGLEPLAY_ACTIVE
	if (_env == nullptr) {
		error(LOG_SYSTEM, "GoolePlayPersister::upload() failed for the google play persister - no env pointer");
		return;
	}
	GPLocalReferenceHolder refs;
	if (!refs.init(_env)) {
		error(LOG_SYSTEM, "GoolePlayPersister::upload(): could not init the ref holder");
		return;
	}

#if 0
	jbyteArray data = _env->NewByteArray(fileSize);
	_env->SetByteArrayRegion(jBuff, 0, fileSize, (jbyte*) file->getBuffer());
	_env->CallStaticMethod(_cls, _saveGameState, data);
#endif
#endif
}

void GooglePlayPersister::download() {
#ifdef GOOGLEPLAY_ACTIVE
	if (_env == nullptr) {
		error(LOG_SYSTEM, "GoolePlayPersister::download() failed for the google play persister - no env pointer");
		return;
	}
	GPLocalReferenceHolder refs;
	if (!refs.init(_env)) {
		error(LOG_SYSTEM, "GoolePlayPersister::download(): could not init the ref holder");
		return;
	}
#if 0
	jbyteArray data = reinterpret_cast<jbyteArray>(_env->CallStaticMethod(_cls, _loadGameState));
#endif
#endif
}

void GooglePlayPersister::connect() {
#ifdef GOOGLEPLAY_ACTIVE
	if (_env == nullptr) {
		error(LOG_SYSTEM, "GoolePlayPersister::connect() failed for the google play persister - no env pointer");
		return;
	}
	info(LOG_SYSTEM, "GoolePlayPersister::connect()");
	_env->CallStaticVoidMethod(_cls, _persisterConnect);
#endif
}

void GooglePlayPersister::disconnect() {
#ifdef GOOGLEPLAY_ACTIVE
	if (_env == nullptr) {
		error(LOG_SYSTEM, "GoolePlayPersister::disconnect() failed for the google play persister - no env pointer");
		return;
	}
	info(LOG_SYSTEM, "GoolePlayPersister::disconnect()");
	_env->CallStaticVoidMethod(_cls, _persisterDisconnect);
#endif
}

bool GooglePlayPersister::init() {
	info(LOG_SYSTEM, "GoolePlayPersister::init() initializing...");
#ifdef GOOGLEPLAY_ACTIVE
	GPLocalReferenceHolder refs;

	JNIEnv *env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
	if (env == nullptr) {
		error(LOG_SYSTEM, "GoolePlayPersister::init() failed to init the google play persister - no env pointer");
		return false;
	}
	if (!refs.init(env)) {
		error(LOG_SYSTEM, "GoolePlayPersister::init(): could not init the ref holder");
		return false;
	}

	jobject activity = static_cast<jobject>(SDL_AndroidGetActivity());
	jclass cls = env->GetObjectClass(activity);

	_env = env;
	_cls = reinterpret_cast<jclass>(_env->NewGlobalRef(cls));

	_persisterInit = env->GetStaticMethodID(_cls, "persisterInit", "()Z");
	if (_persisterInit == 0) {
		error(LOG_SYSTEM, "Could not get the jni bindings for persisterInit");
		_env = nullptr;
		return false;
	}
	_persisterConnect = env->GetStaticMethodID(_cls, "persisterConnect", "()Z");
	if (_persisterConnect == 0) {
		error(LOG_SYSTEM, "Could not get the jni bindings for persisterConnect");
		_env = nullptr;
		return false;
	}
	_persisterDisconnect = env->GetStaticMethodID(_cls, "persisterDisconnect", "()Z");
	if (_persisterDisconnect == 0) {
		error(LOG_SYSTEM, "Could not get the jni bindings for persisterDisconnect");
		_env = nullptr;
		return false;
	}

	_saveGameState = _env->GetStaticMethodID(_cls, "saveGameState", "([B)V");
	if (_saveGameState == 0) {
		error(LOG_SYSTEM, "Could not get the jni bindings for saveGameState");
		_env = nullptr;
		return false;
	}

	_loadGameState = _env->GetStaticMethodID(_cls, "loadGameState", "()[B");
	if (_loadGameState == 0) {
		error(LOG_SYSTEM, "Could not get the jni bindings for loadGameState");
		_env = nullptr;
		return false;
	}

	_showLeaderBoard = _env->GetStaticMethodID(_cls, "showLeaderBoard", "(Ljava/lang/String;)V");
	if (_showLeaderBoard == 0) {
		error(LOG_SYSTEM, "Could not get the jni bindings for showLeaderBoard");
		_env = nullptr;
		return false;
	}

	_showAchievements = _env->GetStaticMethodID(_cls, "showAchievements", "()V");
	if (_showAchievements == 0) {
		error(LOG_SYSTEM, "Could not get the jni bindings for showAchievements");
		_env = nullptr;
		return false;
	}

	_addPointsToLeaderBoard  = _env->GetStaticMethodID(_cls, "addPointsToLeaderBoard", "(Ljava/lang/String;I)V");
	if (_addPointsToLeaderBoard == 0) {
		error(LOG_SYSTEM, "Could not get the jni bindings for addPointsToLeaderBoard");
		_env = nullptr;
		return false;
	}

	_env->CallStaticVoidMethod(_cls, _persisterInit);

	info(LOG_SYSTEM, "GoolePlayPersister::init() initialized");
#endif
	_delegate->init();
	if (Config.getConfigVar("googleplaystate")->getBoolValue()) {
		connect();
	}
	return true;
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
	return _delegate->saveCampaign(campaign);
}

bool GooglePlayPersister::loadCampaign(Campaign* campaign) {
#ifdef GOOGLEPLAY_ACTIVE
#endif
	return _delegate->loadCampaign(campaign);
}

bool GooglePlayPersister::reset() {
#ifdef GOOGLEPLAY_ACTIVE
#endif
	return _delegate->reset();
}

bool GooglePlayPersister::resetCampaign(Campaign* campaign) {
#ifdef GOOGLEPLAY_ACTIVE
#endif
	return _delegate->resetCampaign(campaign);
}
