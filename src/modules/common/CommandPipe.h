#pragma once

#include <string>

/**
 * Optional named-pipe command channel (same idea as vengi's app::Pipe).
 *
 * Enable with cvar @c app_pipe (true). Opens a FIFO under the game write path
 * (e.g. ~/.local/share/caveexpress/input.pipe) and executes each newline-
 * terminated line via the command system (@c set, @c ui_push, …).
 *
 * Example:
 * @code
 * ./caveexpress -set app_pipe true
 * echo 'set fullscreen false' > ~/.local/share/caveexpress/input.pipe
 * echo 'screenshot pipe-test' > ~/.local/share/caveexpress/input.pipe
 * @endcode
 */
class CommandPipe {
private:
#if defined(_WIN32)
	void *_pipe; // HANDLE, INVALID_HANDLE_VALUE when closed
	void *_connectEvent = nullptr;
	bool _connected = false;
#elif !defined(EMSCRIPTEN)
	int _fd = -1;
#endif
	std::string _pipePath;
	std::string _pending;
	bool _enabled = false;

	void executeLine(const std::string& line);
	void consume(const char *data, size_t len);

public:
	CommandPipe();
	~CommandPipe();

	CommandPipe(const CommandPipe&) = delete;
	CommandPipe& operator=(const CommandPipe&) = delete;

	/** Open the pipe when @c app_pipe is true. Safe to call once after config init. */
	bool init();
	void shutdown();
	/** Non-blocking read; execute complete lines as commands. */
	void update();

	bool isEnabled() const { return _enabled; }
	const std::string& getPath() const { return _pipePath; }
};
