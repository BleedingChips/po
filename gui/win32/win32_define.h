#pragma once
#include <Windows.h>
#include <string>
#include "../../frame/error.h"
//#define LINE_INFO ( _FILENAME#_LINE_ )
namespace PO
{
	namespace Win32
	{
		inline std::u16string HRESULT_to_u16string(HRESULT re)
		{
			static thread_local char16_t buffer[1024 * 2];
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, re, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<wchar_t*>(buffer), 0, NULL);
			return std::u16string(buffer);
		}

		namespace Error
		{
			inline void fail_throw(HRESULT re)
			{
				if (!SUCCEEDED(re))
				{
					throw PO::Error::po_error(HRESULT_to_u16string(re));
				}
			}

			template<typename T>
			void fail_throw(HRESULT re, T&& t)
			{
				if (!SUCCEEDED(re))
				{
					Tool::statement_if<std::is_same<decltype(t()), void>>
						(
							[re](auto&& y)
					{
						y();
						throw PO::Error::po_error(HRESULT_to_u16string(re));
					},
							[re](auto&& y)
					{
						throw PO::Error::po_error(HRESULT_to_u16string(re) + y());
					},
						std::forward<T>(t)
						);
				}
			}
		}

		template<typename T>
		struct com_obj
		{
			T* data;
		public:
			operator bool() const { return data != nullptr; }
			com_obj(T* da) : data(da) {}
			com_obj(com_obj&& co) : data(co.data) { co.data = nullptr; }
			~com_obj()
			{
				if (data != nullptr)
					data->Release();
			}
			void clear()
			{
				if (data != nullptr)
				{
					data->Release();
					data = nullptr;
				}
			}
			operator T*() { return data; }
			operator T**() { clear(); return data; }
		};

		template<typename T, typename K = std::allocator<T>> struct com_vector
		{
			std::vector<T*, K> ptr;
			UINT size() const { return static_cast<UINT>(ptr.size()); }
			T*const* data() const { return ptr.data(); }
			T** data() { return ptr.data(); }
			bool empty() const { return ptr.empty(); }
			void set(size_t solt, T* da)
			{
				if (ptr.size() <= solt)
					ptr.insert(ptr.end(), solt + 1 - ptr.size(), nullptr);
				auto& p = ptr[solt];
				if (p != nullptr) p->Release();
				p = da;
				if (p != nullptr) p->AddRef();
			}
			void clear() { for (auto ui : ptr) if (ui != nullptr) ui->Release(); ptr.clear(); }
			~com_vector() { for (auto ui : ptr) if (ui != nullptr) ui->Release(); }
			com_vector() {}
			com_vector(const com_vector& dre)
			{
				ptr = dre.ptr;
				for (auto ui : ptr)
					if (ui != nullptr) ui->AddRef();
			}
			com_vector& operator=(const com_vector& dra) {
				clear(); ptr = dre.ptr;
				for (auto ui : ptr)
					if (ui != nullptr) ui->AddRef();
				return *this;
			}
			com_vector& operator=(const com_vector&& dra) {
				clear(); ptr = std::move(dre.ptr);
				return *this;
			}
			auto begin() { return ptr.begin(); }
			auto end() { return ptr.end(); }
			decltype(auto) operator[](size_t s) { return ptr[s]; }
		};

		template<typename T> struct com_ptr
		{
			T* ptr;
			com_ptr(const com_ptr& p) : ptr(p.ptr) { if (ptr != nullptr) ptr->AddRef(); }
			com_ptr(com_ptr&& p) : ptr(p.ptr) { p.ptr = nullptr; }
			com_ptr() : ptr(nullptr) {}
			operator T* () { return ptr; }
			operator T*() const { return ptr; }
			operator bool() const { return ptr != nullptr; }
			T** adress() { return &ptr; }
			const T** adress() const { return &ptr; }
			com_ptr& operator= (com_ptr&& c) { com_ptr tem(std::move(c)); if (ptr != nullptr) ptr->Release(); ptr = tem.ptr; tem.ptr = nullptr; return *this; };
			com_ptr& operator= (const com_ptr& c) { com_ptr tem(c); if (ptr != nullptr) ptr->Release(); ptr = tem.ptr; tem.ptr = nullptr; return *this; };
			com_ptr& operator= (const T* p) { if (ptr != nullptr) ptr->Release(); ptr = p; }
			T& operator*() { return *ptr; }
			const T& operator*() const { return *ptr; }
			T* operator->() { return ptr; }
			const T* operator->() const { return ptr; }
		};
	}

}
