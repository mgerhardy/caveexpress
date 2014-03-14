#pragma once

template<class T> class Function {
};

template<class Res, class Obj, class ... ArgTypes>
class Function<Res (Obj*, ArgTypes...)> // Parsing the function type
{
	union Pointers // An union to hold different kind of pointers
	{
		Res (*func) (Obj*, ArgTypes...); // void (*)(Funny*, int)
		Res (Obj::*mem_func) (ArgTypes...); // void (Funny::*)(int)
	};
	typedef Res Callback (Pointers&, Obj&, ArgTypes...);

	Pointers ptrs;
	Callback* callback;

	static Res call_func (Pointers& ptrs, Obj& obj, ArgTypes ... args)
	{
		return (*ptrs.func)(&obj, args...); // void (*)(Funny*, int)
	}
	static Res call_mem_func (Pointers& ptrs, Obj& obj, ArgTypes ... args)
	{
		return (obj.*(ptrs.mem_func))(args...); // void (Funny::*)(int)
	}

public:

	Function () :
			callback(0)
	{
	}
	Function (Res (*func) (Obj*, ArgTypes...)) // void (*)(Funny*, int)
	{
		ptrs.func = func;
		callback = &call_func;
	}
	Function (Res (Obj::*mem_func) (ArgTypes...)) // void (Funny::*)(int)
	{
		ptrs.mem_func = mem_func;
		callback = &call_mem_func;
	}
	Function (const Function& function)
	{
		ptrs = function.ptrs;
		callback = function.callback;
	}
	Function& operator= (const Function& function)
	{
		ptrs = function.ptrs;
		callback = function.callback;
		return *this;
	}
	Res operator() (Obj& obj, ArgTypes ... args)
	{
		return (*callback)(ptrs, obj, args...);
	}
};
