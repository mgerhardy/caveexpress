#pragma once

#include <map>

#include "common/NonCopyable.h"
#include "common/Pointers.h"

template<class TYPE, class CTX>
class IFactory {
public:
	virtual ~IFactory() {}
	virtual SharedPtr<TYPE> create (const CTX* ctx) const = 0;
};

template<class KEY, class TYPE, class CTX>
class IFactoryRegistry: public NonCopyable {
public:
	typedef std::map<const KEY*, const IFactory<TYPE, CTX>*> FactoryMap;
	typedef typename FactoryMap::const_iterator FactoryMapConstIter;
	typedef typename FactoryMap::iterator FactoryMapIter;
protected:
	FactoryMap _factories;
public:
	bool registerFactory (const KEY& type, const IFactory<TYPE, CTX>& factory)
	{
		FactoryMapConstIter i = _factories.find(&type);
		if (i != _factories.end()) {
			return false;
		}

		_factories[&type] = &factory;
		return true;
	}

	SharedPtr<TYPE> create (const KEY& type, const CTX* ctx = 0) const
	{
		FactoryMapConstIter i = _factories.find(&type);
		if (i == _factories.end()) {
			return SharedPtr<TYPE>();
		}

		const IFactory<TYPE, CTX>* factory = i->second;
		return factory->create(ctx);
	}

	inline const FactoryMap& getFactories () const
	{
		return _factories;
	}
};
