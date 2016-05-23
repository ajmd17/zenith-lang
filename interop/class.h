#ifndef __CLASS_H__
#define __CLASS_H__

#include "../runtime/value.h"

#include <string>

namespace zenith
{
	/* Wrapper for native objects */
	class NativeObjectBase
	{
	public:
		virtual ValuePtr &getProperty(const std::string &name) = NULL;
		virtual void setProperty(const std::string &name, ValuePtr val) = NULL;
	};

	/* An instance of a native class */
	template <typename T>
	class NativeObject : public NativeObjectBase
	{
	private:
		T obj; // The actual native object
		ValuePtrMap properties;
	public:
		T &getObject()
		{
			return obj;
		}

		template <typename P>
		void bindProperty(const std::string &propName, P ptr)
		{
			ValuePtr prop = std::make_shared<Value>();
			prop->setData(&(obj.*ptr));
			properties[propName] = prop;
		}

		ValuePtr &getProperty(const std::string &name)
		{
			if (properties.find(name) == properties.end())
			{
				std::string errMsg = "Property \"" + name + "\" does not exist!";
				cout << errMsg << "\n";
				throw std::runtime_error(std::string(errMsg));
			}

			return properties.at(name);
		}

		void setProperty(const std::string &name, ValuePtr val)
		{
			if (properties.find(name) == properties.end())
			{
				std::string errMsg = "Property \"" + name + "\" does not exist!";
				cout << errMsg << "\n";
				throw std::runtime_error(std::string(errMsg));
			}

			properties.at(name) = val;
		}
	};

	class NativePropertyBase
	{
	public:
		virtual void bind(NativeObjectBase &nativeObj) = NULL;
	};

	template <typename C, typename D>
	class NativeProperty : public NativePropertyBase
	{
	private:
		D C::*dataMemberPointer;
		std::string propertyName;
	public:
		NativeProperty(const std::string &propName, D C::*dataMem)
		{
			propertyName = propName;
			dataMemberPointer = dataMem;
		}

		void bind(NativeObjectBase &nativeObj)
		{
			NativeObject<C> *conv = dynamic_cast<NativeObject<C>*>(&nativeObj);
			conv->bindProperty(propertyName, dataMemberPointer);
		}
	};

	class NativeClassBase
	{
	public:
		virtual ValuePtr createInstance() = NULL;
	};

	/* Defines a native class type. */
	template <typename C>
	class NativeClass : public NativeClassBase
	{
	private:
		std::vector<std::shared_ptr<NativePropertyBase>> propertyDefs;
	public:
		/* Create an instance of this class type. */
		ValuePtr createInstance()
		{
			ValuePtr val = std::make_shared<Value>();

			auto nativeObjectPtr = std::make_shared<NativeObject<C>>();
			auto nativeObjectBase = std::static_pointer_cast<NativeObjectBase>(nativeObjectPtr);
			val->setData(nativeObjectBase, true);

			for (int i = 0; i < propertyDefs.size(); i++)
				propertyDefs[i]->bind(*nativeObjectBase.get());

			return val;
		}

		template <typename D>
		NativeClass<C> *bindDataMember(const std::string &name, D C::*ptr)
		{
			auto propertyPtr = std::make_shared<NativeProperty<C, D>>(name, ptr);
			propertyDefs.push_back(std::static_pointer_cast<NativePropertyBase>(propertyPtr));

			return this;
		}
	};

	/* A type that has been defined in a script */
	class ScriptedObject
	{
	private:
		ValuePtrMap properties;
	public:
		ScriptedObject() { }

		ValuePtrMap &getProperties() { return properties; }

		void addProperty(const std::string &name, ValuePtr &object)
		{
			properties[name] = object;
		}

		ValuePtr &getProperty(const std::string &name)
		{
			return properties.at(name);
		}
	};
}

#endif
