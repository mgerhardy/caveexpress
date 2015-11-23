#pragma once

#include "common/Common.h"
#include "common/String.h"
#include <map>
#include <vector>
#include <functional>

class ICommand {
public:
	typedef std::vector<std::string> Args;

	virtual ~ICommand() {
	}

	virtual void run(const Args& args) = 0;

	virtual void operator()(const Args& args) {
		run(args);
	}

	template<typename Completer>
	void setCompleter(Completer func) {
		_completerFunc = func;
	}

	void complete(const std::string& input, std::vector<std::string>& matches) const {
		if (!_completerFunc)
			return;
		_completerFunc(input, matches);
	}

private:
	std::function<void(const std::string&, std::vector<std::string>&)> _completerFunc;
};

class CommandBindVoid : public ICommand {
public:
	template<typename Func>
	CommandBindVoid(Func func) :
			_func(func) {
	}

	void run(const Args& args) override {
		_func();
	}

private:
	std::function<void()> _func;
};

class CommandBindString : public ICommand {
public:
	template<typename Func>
	CommandBindString(Func func) :
			_func(func) {
	}

	void run(const Args& args) override {
		_func(args.front());
	}

private:
	std::function<void(const std::string&)> _func;
};

class CommandBindArgs : public ICommand {
public:
	template<typename Func>
	CommandBindArgs(Func func) :
			_func(func) {
	}

	void run(const Args& args) override {
		_func(args);
	}

private:
	std::function<void(const Args&)> _func;
};

typedef std::shared_ptr<ICommand> CommandPtr;

#define bindFunction(clazz, method) std::bind(&clazz::method, this, std::placeholders::_1)
#define bindFunctionVoid(clazz, method) std::bind(&clazz::method, this)
