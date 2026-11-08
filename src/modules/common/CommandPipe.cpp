#include "CommandPipe.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__ANDROID__) || defined(EMSCRIPTEN) || defined(__IPHONEOS__)
/* no fifo APIs used */
#else
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "common/Application.h"
#include "common/CommandSystem.h"
#include "common/ConfigManager.h"
#include "common/FileSystem.h"
#include "common/Log.h"
#include "common/Singleton.h"
#include "common/String.h"

CommandPipe::CommandPipe()
#if defined(_WIN32)
	: _pipe(INVALID_HANDLE_VALUE), _connectEvent(nullptr), _connected(false), _enabled(false)
#elif defined(__ANDROID__) || defined(EMSCRIPTEN) || defined(__IPHONEOS__)
	: _enabled(false)
#else
	: _fd(-1), _enabled(false)
#endif
{
}

CommandPipe::~CommandPipe()
{
	shutdown();
}

void CommandPipe::executeLine(const std::string& line)
{
	const std::string trimmed = string::trim(line);
	if (trimmed.empty() || trimmed[0] == '#')
		return;
	Log::info(LOG_COMMON, "pipe: %s", trimmed.c_str());
	Commands.executeCommandLine(trimmed);
}

void CommandPipe::consume(const char *data, size_t len)
{
	_pending.append(data, len);
	size_t start = 0;
	for (;;) {
		const size_t nl = _pending.find('\n', start);
		if (nl == std::string::npos) {
			if (start > 0)
				_pending.erase(0, start);
			break;
		}
		std::string line = _pending.substr(start, nl - start);
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		executeLine(line);
		start = nl + 1;
		if (start >= _pending.size()) {
			_pending.clear();
			break;
		}
	}
}

bool CommandPipe::init()
{
	shutdown();

	const ConfigVarPtr& var = Config.getConfigVar("app_pipe", "false", true);
	if (!var->getBoolValue()) {
		_enabled = false;
		return true;
	}

#if defined(__ANDROID__) || defined(EMSCRIPTEN) || defined(__IPHONEOS__)
	Log::warn(LOG_COMMON, "app_pipe is not supported on this platform");
	_enabled = false;
	return true;
#elif defined(_WIN32)
	_pipePath = "\\\\.\\pipe\\" + Singleton<Application>::getInstance().getName() + "-input";
	_pipe = CreateNamedPipeA(_pipePath.c_str(), PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 1024, 1024, 0, NULL);
	if (_pipe == INVALID_HANDLE_VALUE) {
		Log::error(LOG_COMMON, "Failed to create named pipe %s (%lu)", _pipePath.c_str(),
				(unsigned long)GetLastError());
		return false;
	}
	_connectEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
	if (_connectEvent == NULL) {
		CloseHandle((HANDLE)_pipe);
		_pipe = INVALID_HANDLE_VALUE;
		return false;
	}
	OVERLAPPED ov;
	ZeroMemory(&ov, sizeof(ov));
	ov.hEvent = (HANDLE)_connectEvent;
	const BOOL ok = ConnectNamedPipe((HANDLE)_pipe, &ov);
	if (!ok) {
		const DWORD err = GetLastError();
		if (err == ERROR_IO_PENDING) {
			_connected = false;
		} else if (err == ERROR_PIPE_CONNECTED) {
			_connected = true;
			SetEvent((HANDLE)_connectEvent);
		} else {
			Log::error(LOG_COMMON, "ConnectNamedPipe failed (%lu)", (unsigned long)err);
			shutdown();
			return false;
		}
	}
	_enabled = true;
	Log::info(LOG_COMMON, "Opened command pipe %s", _pipePath.c_str());
	return true;
#else
	_pipePath = FS.getAbsoluteWritePath() + "input.pipe";
	if (mkfifo(_pipePath.c_str(), 0666) == -1 && errno != EEXIST) {
		Log::error(LOG_COMMON, "Failed to create named pipe %s: %s", _pipePath.c_str(), strerror(errno));
		return false;
	}
	// O_RDWR avoids blocking for a writer when opening a FIFO for read.
	_fd = open(_pipePath.c_str(), O_RDWR | O_NONBLOCK);
	if (_fd == -1) {
		Log::error(LOG_COMMON, "Failed to open named pipe %s: %s", _pipePath.c_str(), strerror(errno));
		return false;
	}
	_enabled = true;
	Log::info(LOG_COMMON, "Opened command pipe %s (echo 'set fullscreen false' > that path)", _pipePath.c_str());
	return true;
#endif
}

void CommandPipe::shutdown()
{
#if defined(_WIN32)
	if (_pipe != INVALID_HANDLE_VALUE) {
		CloseHandle((HANDLE)_pipe);
		_pipe = INVALID_HANDLE_VALUE;
	}
	if (_connectEvent != NULL) {
		CloseHandle((HANDLE)_connectEvent);
		_connectEvent = NULL;
	}
	_connected = false;
#elif defined(__ANDROID__) || defined(EMSCRIPTEN) || defined(__IPHONEOS__)
	/* nothing */
#else
	if (_fd != -1) {
		close(_fd);
		_fd = -1;
	}
	if (!_pipePath.empty()) {
		unlink(_pipePath.c_str());
		_pipePath.clear();
	}
#endif
	_pending.clear();
	_enabled = false;
}

void CommandPipe::update()
{
	if (!_enabled)
		return;

#if defined(_WIN32)
	if (_pipe == INVALID_HANDLE_VALUE)
		return;
	if (!_connected) {
		if (_connectEvent == NULL)
			return;
		if (WaitForSingleObject((HANDLE)_connectEvent, 0) != WAIT_OBJECT_0)
			return;
		_connected = true;
	}
	DWORD bytesAvailable = 0;
	if (!PeekNamedPipe((HANDLE)_pipe, NULL, 0, NULL, &bytesAvailable, NULL)) {
		if (GetLastError() == ERROR_BROKEN_PIPE) {
			DisconnectNamedPipe((HANDLE)_pipe);
			ResetEvent((HANDLE)_connectEvent);
			OVERLAPPED ov;
			ZeroMemory(&ov, sizeof(ov));
			ov.hEvent = (HANDLE)_connectEvent;
			ConnectNamedPipe((HANDLE)_pipe, &ov);
			_connected = false;
		}
		return;
	}
	if (bytesAvailable == 0)
		return;
	char buffer[1024];
	DWORD bytesRead = 0;
	if (!ReadFile((HANDLE)_pipe, buffer, sizeof(buffer), &bytesRead, NULL) || bytesRead == 0)
		return;
	consume(buffer, (size_t)bytesRead);
#elif defined(__ANDROID__) || defined(EMSCRIPTEN) || defined(__IPHONEOS__)
	/* unsupported */
#else
	if (_fd == -1)
		return;
	char buffer[1024];
	for (;;) {
		const ssize_t bytes = ::read(_fd, buffer, sizeof(buffer));
		if (bytes == -1) {
			if (errno != EAGAIN && errno != EWOULDBLOCK)
				Log::error(LOG_COMMON, "command pipe read failed: %s", strerror(errno));
			break;
		}
		if (bytes == 0)
			break;
		consume(buffer, (size_t)bytes);
	}
#endif
}
