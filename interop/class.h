#ifndef __ZENITH_INTEROP_CLASS_H__
#define __ZENITH_INTEROP_CLASS_H__

#include <string>

#include "../runtime/experimental/object.h"

namespace zenith
{
	namespace runtime
	{
		/* Wrapper for native objects */
		class NativeObjectBase
		{
		public:
			virtual ObjectPtr &getProperty(const std::string &name) = 0;
			virtual void setProperty(const std::string &name, ObjectPtr val) = 0;
		};

		/* An instance of a native class */
		template <typename T>
		class NativeObject : public NativeObjectBase
		{
		private:
			T obj; // The actual native object
			std::map<std::string, ObjectPtr> properties;

		public:
			T &getObject()
			{
				return obj;
			}

			template <typename P>
			void bindProperty(const std::string &propName, P ptr)
			{
				auto prop = std::make_shared<Object>();
				prop->any.assign(&(obj.*ptr));
				properties[propName] = prop;
			}

			ObjectPtr &getProperty(const std::string &name)
			{
				if (properties.find(name) == properties.end())
				{
					std::string errMsg = "Property \"" + name + "\" does not exist!";
					std::cout << errMsg << "\n";
					throw std::runtime_error(std::string(errMsg));
				}

				return properties.at(name);
			}

			void setProperty(const std::string &name, ObjectPtr val)
			{
				if (properties.find(name) == properties.end())
				{
					std::string errMsg = "Property \"" + name + "\" does not exist!";
					std::cout << errMsg << "\n";
					throw std::runtime_error(std::string(errMsg));
				}

				properties.at(name) = val;
			}
		};

		class NativePropertyBase
		{
		public:
			virtual void bind(NativeObjectBase &nativeObj) = 0;
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
			virtual ObjectPtr createInstance() = 0;
		};

		/* Defines a native class type. */
		template <typename C>
		class NativeClass : public NativeClassBase
		{
		private:
			std::vector<std::shared_ptr<NativePropertyBase>> propertyDefs;
		public:
			/* Create an instance of this class type. */
			ObjectPtr createInstance()
			{
				auto val = std::make_shared<Object>();

				auto nativeObjectPtr = std::make_shared<NativeObject<C>>();
				auto nativeObjectBase = std::static_pointer_cast<NativeObjectBase>(nativeObjectPtr);
				val->any.assign(nativeObjectBase);
				val->setNative(true);

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
	}
}

#endif
