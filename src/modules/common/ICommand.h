#pragma once

#include "common/Common.h"
#include "common/String.h"
#include <map>
#include <vector>
#include <functional>

class ICommand {
protected:
	std::function<void(const std::string&, std::vector<std::string>&)> _completerFunc;
public:
	ICommand()
	{
	}

	virtual ~ICommand ()
	{
	}

	typedef std::vector<std::string> Args;

	virtual void run (const Args& args) = 0;

	virtual void operator() (const Args& args) {
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
};

template<class Class>
class ConstCommandFunctor: public ICommand {
private:
	void (Class::*_functionPtrStr) (const std::string& argument) const;
	void (Class::*_functionPtrNoParam) () const;
	Class* _object;

public:
	ConstCommandFunctor (Class* object, void(Class::*functionPtrStr) (const std::string& argument) const)
	{
		_object = object;
		_functionPtrStr = functionPtrStr;
		_functionPtrNoParam = nullptr;
	}

	ConstCommandFunctor (Class* object, void(Class::*functionPtrNoParam) () const)
	{
		_object = object;
		_functionPtrStr = nullptr;
		_functionPtrNoParam = functionPtrNoParam;
	}

	void run (const Args& args) override
	{
		if (_functionPtrStr) {
			(_object->*_functionPtrStr)(args.empty() ? "" : *args.begin());
		} else if (_functionPtrNoParam) {
			(_object->*_functionPtrNoParam)();
		}
	}
};

template<class Class>
class CommandFunctor: public ICommand {
private:
	void (Class::*_functionPtr) (const Args& args);
	void (Class::*_functionPtrStr) (const std::string& argument);
	void (Class::*_functionPtrNoParam) ();
	Class* _object;

public:
	CommandFunctor (Class* object, void(Class::*functionPtr) (const Args& args))
	{
		_object = object;
		_functionPtr = functionPtr;
		_functionPtrStr = nullptr;
		_functionPtrNoParam = nullptr;
	}

	CommandFunctor (Class* object, void(Class::*functionPtrStr) (const std::string& argument))
	{
		_object = object;
		_functionPtr = nullptr;
		_functionPtrStr = functionPtrStr;
		_functionPtrNoParam = nullptr;
	}

	CommandFunctor (Class* object, void(Class::*functionPtrNoParam) ())
	{
		_object = object;
		_functionPtrStr = nullptr;
		_functionPtr = nullptr;
		_functionPtrNoParam = functionPtrNoParam;
	}

	virtual void operator() (const std::string& argument)
	{
		(*_object.*_functionPtrStr)(argument);
	}

	virtual void run (const Args& args) override
	{
		if (_functionPtrStr) {
			if (args.empty())
				(*_object.*_functionPtrStr)("");
			else
				(*_object.*_functionPtrStr)((*args.begin()));
		} else if (_functionPtrNoParam) {
			(*_object.*_functionPtrNoParam)();
		} else {
			(*_object.*_functionPtr)(args);
		}
	}
};

typedef std::shared_ptr<ICommand> CommandPtr;

#define bindFunction(className, method) new CommandFunctor<className>(this, &className::method)
