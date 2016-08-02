#pragma once
#include <typeinfo>
#include <typeindex>

namespace PO {
	class any {
		struct handle {
			virtual handle* copy() const = 0;
			virtual ~handle() {}
		};
		template<typename T>
		struct holder:public handle {
			T data;
			virtual handle* copy() const {
				return new holder<T>(data);
			}
			template<typename I>
			holder(I&& t) :data(std::forward<I>(t)) {}
		};
		handle* data;
	public:		
		~any() { delete data; }
		any() :data(nullptr) {}
		any(any&& m) :data(m.data) { m.data = nullptr; }
		any(const any& m) :data(m.data == nullptr ? m.data : m.data->copy()) {}
		template<typename T> any(T&& t) : data(new holder<typename std::remove_reference<T>::type>(std::forward<T>(t))) { }
		template<size_t i> any(const char(&p)[i]) : data(new holder<std::string>(std::string(p))) {}
		
		template<typename T> any& operator = (T&& t) 
		{
			delete data;
			data = new holder<typename std::remove_reference<T>::type>(std::forward<T>(t));
			return *this;
		}
		template<typename T> any& operator = (any&& a)
		{
			delete data;
			data = a.data;
			a.data = nullptr;
			return *this;
		}
		template<size_t i> any& operator = (const char (&p)[i] )
		{
			delete data;
			data = new holder<std::string>(std::string(p));
			return *this;
		}
		operator bool() const { return data != nullptr; }
		template<typename T> T& cast()
		{
			holder<T>* tem = dynamic_cast<holder<T>*>(data);
			if (tem == nullptr)
				throw std::bad_cast();
			return tem->data;
		}
		template<typename T> const T& cast() const
		{
			const holder<T>* tem = dynamic_cast<const holder<T>*>(data);
			if (tem == nullptr)
				throw std::bad_cast();
			return tem->data;
		}
	};
}
