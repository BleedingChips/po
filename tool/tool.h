#pragma once
#include "tmp.h"
#include <exception>
#include <string>
#include <functional>
#include <unordered_map>
#include <array>
namespace PO
{
	namespace Tool
	{
		namespace Error
		{
			struct tool_exeception : public std::exception
			{
				std::string scription;
			public:
				tool_exeception(std::string&& s) : scription(std::move(s)) {}
				tool_exeception(const std::string& s) : scription(s) {}
				tool_exeception(const tool_exeception&) = default;
				tool_exeception(tool_exeception&&) = default;
				tool_exeception() {}
				operator std::string& () { return scription; }
				operator const std::string& () const { return scription; }
				operator const char*() const { return scription.c_str(); }
				const char* what() const override { return *this; }
			};
		}

		//TODO - add a is_nothrow_callable
		class at_scope_exit
		{
			std::function<void(void) noexcept> func;
		public:
			template<typename T>
			at_scope_exit(T&& t) : func(std::forward<T>(t)) {}
			~at_scope_exit() 
			{
				if (func)
					func();
			}
		};

		/* statement_if */
		namespace Implement
		{
			template<bool> struct statement_if_struct
			{
				template<typename F, typename P, typename ...AT> static decltype(auto) run(F&& t, P&& p, AT&&... at) { return std::forward<F>(t)(std::forward<AT>(at)...); }
			};

			template<> struct statement_if_struct<false>
			{
				template<typename T, typename P, typename ...AT> static decltype(auto) run(T&& t, P&& p, AT&&... at) { return std::forward<P>(p)(std::forward<AT>(at)...); }
			};

			template<bool s, typename T = int> struct statement_if_implement
			{
				T t;
				template<typename ...AT> decltype(auto) operator()(AT&&... at) { return t(std::forward<AT>(at)...); }
				template<bool other_s, typename K> statement_if_implement& elseif_(K&& k) {
					return *this;
				}
				template<typename K> statement_if_implement& else_(K&& k) {
					return *this;
				}
			};

			template<typename T> struct statement_if_implement<false, T>
			{
				T t;
				template<typename ...AT> decltype(auto) operator()(AT&&... at) { }
				template<bool other_s, typename K> decltype(auto) elseif_(K&& k) {
					return statement_if_implement<other_s, K&&>{std::forward<K>(k)};
				}
				template<typename K> decltype(auto) else_(K&& k) {
					return statement_if_implement<true, K&&>{std::forward<K>(k)};
				}
			};
		}

		template<bool s, typename T, typename P, typename ...AK> decltype(auto) statement_if(T&& t, P&& p, AK&& ...ak) { return Implement::statement_if_struct<s>::run(std::forward<T>(t), std::forward<P>(p), std::forward<AK>(ak)...); }
		template<bool s, typename T> decltype(auto) statement_if(T&& t) { return Implement::statement_if_implement<s, T&&>{std::forward<T>(t)}; }


		/* adapter */
		template<typename ...T> struct adapter
		{
			template<typename K>  adapter(K&& k) {}
		};

		template<typename P, typename ...T> struct adapter<P, T...> : adapter<T...>
		{
			P data;
			template<typename K>  adapter(K&& k) : adapter<T...>(std::forward<K>(k)), data(std::forward<K>(k)) {}
			template<typename D>
			decltype(auto) get()
			{
				static_assert(Tmp::is_one_of<D, P, T...>::value, "what you get is not exist");
				return Tool::statement_if< std::is_same<D, P>::value >
					(
						[](auto& it) { return it.adapter<P, T...>::data;  },
						[](auto& it) { return it.adapter<T...>::template get<D>(); },
						*this
						);
			}
		};

		/* static_mapping */
		namespace Implement
		{
			template<typename equal_ope, typename less_ope, size_t s, size_t o, typename ...input>
			struct static_mapping_implement
			{
				template<typename T, typename equal_func, typename default_func>
				decltype(auto) operator()(T&& t, equal_func&& ef, default_func&& df) const
				{
					using pick_type = TmpCall::call<TmpCall::in_t<input...>, TmpCall::pick_single_t<(s + o) / 2>, TmpCall::self_t>;
					if (equal_ope{}(Tmp::itself<pick_type>(), std::forward<T>(t)))
					{
						return ef(Tmp::itself<pick_type>());
					}
					else if (less_ope{}(Tmp::itself<pick_type>(), std::forward<T>(t)))
					{
						return static_mapping_implement<equal_ope, less_ope, s, (s + o) / 2, input...>{}(std::forward<T>(t), std::forward<equal_func>(ef), std::forward<default_func>(df));
					}
					else
					{
						return static_mapping_implement<equal_ope, less_ope, (s + o) / 2 + 1, o, input...>{}(std::forward<T>(t), std::forward<equal_func>(ef), std::forward<default_func>(df));
					}
				}
			};

			template<class equal_ope, class less_ope, size_t s, typename ...input>
			struct static_mapping_implement<equal_ope, less_ope, s, s, input...>
			{
				template<typename T, typename equal_func, typename default_func>
				decltype(auto) operator()(T&& t, equal_func&& ef, default_func&& df) const
				{
					return df();
				}
			};
		}

		template<typename equal_ope, typename less_ope>
		struct make_static_mapping
		{
			template<typename ...input> struct in
			{
				using type = Implement::static_mapping_implement<equal_ope, less_ope, 0, sizeof...(input), input...>;
			};
		};

		template<class equal_ope, class less_ope, typename ...input>
		using static_mapping_t = Implement::static_mapping_implement<less_ope, equal_ope, 0, sizeof...(input), input...>;

		/* variant */
		namespace Implement
		{
			struct variant_type_index_equal
			{
				template<typename T, typename L> bool operator()(T t, L l)
				{
					return Tmp::type_extract_t<T>::label_t::value == l;
				}
			};

			struct variant_type_index_less
			{
				template<typename T, typename L> bool operator()(T t, L l)
				{
					return l < Tmp::type_extract_t<T>::label_t::value;
				}
			};

			template<typename T, typename K, typename = void> struct is_assignable : std::false_type {};
			template<typename T, typename K> struct is_assignable<T,K,std::void_t<decltype( std::declval<T>() = std::declval<K>())>> : std::true_type {};

			template<typename type, typename store>
			void variant_destructor(store& s) noexcept
			{
				reinterpret_cast<type*>(&s)->~type();
			}

			template<typename type, typename store, typename ...AK>
			bool variant_constructor(store& s, AK&& ...ak)
			{
				return statement_if< std::is_constructible<type, AK...>::value>
					(
						[&](auto ptr, auto&& ...pa)
				{
					using type = Tmp::type_extract_t<decltype(ptr)>;
					new (&s) type{ std::forward<decltype(pa)&&>(pa)... };
					return true;
				},
						[&](auto ptr, auto&& ...pa)
				{
						return false;
				},
					Tmp::itself<type>(), std::forward<AK>(ak)...
					);
			}

			template<typename type, typename store, typename K>
			bool variant_assignment(store& s, K&& k)
			{
				return statement_if<std::is_assignable<type, K>::value>
					(
						[&](auto ptr)
				{
					using type = Tmp::type_extract_t<decltype(ptr)>;
					reinterpret_cast<type*>(&s)->operator=(std::forward<K>(k));
					return true;
				},
						[&](auto ptr)
				{
					using type = Tmp::type_extract_t<decltype(ptr)>;
					variant_destructor<type>(s);
					return variant_constructor<type>(s, std::forward<K>(k));
				},
					Tmp::itself<type>()
					);
			}

		}

		template<typename ...T> class variant
		{
			static_assert(!Tmp::is_repeat<T...>::value, "type of variant can not repeat");
			using void_index = TmpCall::call<TmpCall::in_t<T...>, TmpCall::localizer_t<0, Tmp::instant<std::is_same, void>::template in_t>, TmpCall::bind_i<Tmp::set_i>, TmpCall::self_t>;
			static_assert(std::is_same<void_index, Tmp::set_i<>>::value, "varient can not include void");

			using mapping = TmpCall::call< TmpCall::in_t<T...>, TmpCall::label_serial_t<1>, make_static_mapping< Implement::variant_type_index_equal, Implement::variant_type_index_less > >;

			using size_index = 
				TmpCall::call<
					TmpCall::in_t<T...>, TmpCall::label_size_t, TmpCall::replace_by_label_t,
					TmpCall::combine_t<Tmp::bigger_value>, TmpCall::self_t
				>;

			template<template<typename ...> class relation, typename ...in> struct relation_type
			{
				using type = TmpCall::call<
					TmpCall::in_t<T...>, TmpCall::localizer_t<1, Tmp::instant<relation, in...>::template front_in_t>, TmpCall::sperate_value_i<Tmp::set_i>,
					TmpCall::append_t<Tmp::set_i<0>>, TmpCall::pick_single_t<0>, TmpCall::self_t
				>;
			};

			template<typename in> struct same_type
			{
				using type = TmpCall::call <
					TmpCall::in_t<T...>, TmpCall::localizer_t<1, Tmp::instant<std::is_same, in>::template front_in_t>, TmpCall::sperate_value_i<Tmp::set_i>,
					TmpCall::append_t<Tmp::set_i<0>>, TmpCall::pick_single_t<0>, TmpCall::self_t
				>;
			};

			typename std::aligned_union<1, T...>::type data;

			std::conditional_t< (sizeof...(T)+1 <= 255), uint8_t, std::conditional_t< (sizeof...(T)+1 <= 65535), uint16_t, std::conditional_t< (sizeof...(T)+1 <= 4294967295), uint32_t, uint64_t > > > index;

			template<typename K>
			variant(std::true_type, K&& v) : index(v.index)
			{
				if (v.index != 0)
					mapping{}(
						v.index,
						[&v, this](auto self)
				{
					using type = typename Tmp::type_extract_t<decltype(self)>::original_t;
					if(!Implement::variant_constructor<type>(data, std::forward<K>(v).template cast<type>()))
						throw Error::tool_exeception("this kind of type can not construct form itself");
				},
						[]() { throw Error::tool_exeception("unmatch mapping while construct form const varient&"); }
				);
			}

			template<typename ...K>
			variant(std::false_type, K&& ...k) : index(relation_type<std::is_constructible, K...>::type::value)
			{
				static_assert(relation_type<std::is_constructible, K...>::type::value != 0, "not avalible type construct form this");
				using type = TmpCall::call<TmpCall::in_t<T...>, TmpCall::pick_single_t<relation_type<std::is_constructible, K...>::type::value - 1>, TmpCall::self_t>;
				if (!Implement::variant_constructor<type>(data, std::forward<K>(k)...))
					throw Error::tool_exeception("unmatch mapping while construct form those parameter");
			}


		public:

			operator bool() const noexcept { return index != 0; }

			auto get_index() const noexcept { return index; }

			template<typename P> bool able_cast() const noexcept
			{
				static_assert(Tmp::is_one_of<P, T...>::value, "variant can not cast to unbinded type.");
				return index == same_type<P>::type::value;
			}

			template<typename P> P& cast() &
			{
				if (!this->template able_cast<P>())
					throw Error::tool_exeception("variant now are not this type");
				return *reinterpret_cast<P*>(&data);
			}

			template<typename P> const P& cast() const&
			{
				if (!this->template able_cast<P>())
					throw Error::tool_exeception("variant now are not this type");
				return *reinterpret_cast<const P*>(&data);
			}

			template<typename P> P&& cast() &&
			{
				if (!this->template able_cast<P>())
					throw Error::tool_exeception("variant now are not this type");
				return std::move(*reinterpret_cast<P*>(&data));
			}

			variant() noexcept : index(0){}
			template<typename P, typename ...K> variant(P&& p, K&& ...k) :
				variant(
					std::integral_constant<bool, sizeof...(K) == 0 && std::is_same<std::remove_const_t<std::remove_reference_t<P>>, variant>::value>(),
					std::forward<P>(p), std::forward<K>(k)...
				) {}

			~variant()
			{
				if (index != 0)
					mapping{}(index, [this](auto ptr) { using type = typename Tmp::type_extract_t<decltype(ptr)>::original_t; Implement::variant_destructor<type>(data); }, []() {});
			}

			
			variant& operator= (const variant& v)
			{
				if (index == 0 && v.index == 0) return *this;
				if (index == v.index)
				{
					mapping{}(index, 
						[&, this](auto ptr)
					{
						using type = typename Tmp::type_extract_t<decltype(ptr)>::original_t;
						if(!Implement::variant_assignment<type>(data, v.template cast<type>()))
							throw Error::tool_exeception("unable to assignment");
					},
						[]() {throw Error::tool_exeception("unmatch mapping while call operator="); }
					);
					return *this;
				}
				if (index != 0)
					mapping{}(index, 
						[&, this](auto ptr) 
				{ 
					using type = typename decltype(ptr)::type; 
					Implement::variant_destructor<type>(data);
				}, []() {});
				if(v.index!=0)
					mapping{}(v.index, 
						[&, this](auto ptr) 
				{ 
					using type = typename decltype(ptr)::type;
					Implement::variant_constructor<type>(data, v.template cast<type>());
				}, []() {});
				index = v.index;
				return *this;
			}
			
			variant& operator= (variant&& v)
			{
				if (index == 0 && v.index == 0) return *this;
				if (index == v.index)
				{
					mapping{}(index,
						[&, this](auto ptr)
					{
						using type = typename Tmp::type_extract_t<decltype(ptr)>::original_t;
						if (!Implement::variant_assignment<type>(data, std::move(v).template cast<type>()))
							throw Error::tool_exeception("unable to assignment");
					},
							[]() {throw Error::tool_exeception("unmatch mapping while call operator="); }
					);
						return *this;
				}
				if (index != 0)
					mapping{}(index,
						[&, this](auto ptr)
				{
					using type = typename Tmp::type_extract_t<decltype(ptr)>::original_t;
					Implement::variant_destructor<type>(data);
				}, []() {});
				if (v.index != 0)
					mapping{}(v.index,
						[&, this](auto ptr)
				{
					using type = typename Tmp::type_extract_t<decltype(ptr)>::original_t;
					Implement::variant_constructor<type>(data, std::move(v).template cast<type>());
				}, []() {});
				index = v.index;
				return *this;
			}

			template<typename K, typename = std::enable_if_t<!std::is_same< std::remove_const_t<std::remove_reference_t<K>>, variant>::value>>
			variant& operator= (K&& k)
			{
				using type_ass = typename relation_type<Implement::is_assignable, K>::type;
				using type_con = typename relation_type<std::is_constructible, K>::type;
				static_assert(type_ass::value != 0 || type_con::value !=0, "variant can not cast to unbinded type.");
				
				using type_index = std::conditional_t< type_ass::value != 0, type_ass, type_con>;
				using type = TmpCall::call<TmpCall::in_t<T...>, TmpCall::pick_single_t<type_index::value -1>, TmpCall::self_t>;
				if (index == type_index::value)
				{
					if (!Implement::variant_assignment<type>(data, std::forward<K>(k)))
						throw Error::tool_exeception("unable to assignment");
					return *this;
				}
				if (index != 0)
					mapping{}(index, [this](auto ptr) {Implement::variant_destructor<Tmp::type_extract_t<decltype(ptr)>>(data); }, []() {});
				Implement::variant_constructor<type>(data, std::forward<K>(k));
				index = type_index::value;
				return *this;
			}
		};
		
		template<> class variant<>
		{
		public:
			variant() {}
			variant(const variant&) {}
			~variant() {}
			size_t get_index() const { return 0; }
			operator bool() const noexcept { return false; }
			variant& operator=(variant&&) = default;
			variant& operator=(const variant&) = default;
		};

		template<typename T> class optional : variant<T>
		{
		public:
			T& operator*() { return variant<T>::template cast<T>(); }
			const T& operator*() const { return variant<T>::template cast<T>(); }
			const T* operator->() const noexcept { return variant<T>::template cast_pointer<T>(); }
			operator bool() const noexcept { return variant<T>::operator bool(); }

			template<typename K>
			optional& operator= (K&& ap) { variant<T>::operator=(std::forward<K>(ap)); return *this; }

			using variant<T>::variant;

			T& value_or(T& or_data) &
			{
				if (variant<T>::template able_cast<T>())
					return variant<T>::template cast<T>();
				else
					return or_data;
			}

			const T& value_or(const T& or_data) const
			{
				if (variant<T>::template able_cast<T>())
					return variant<T>::template cast<T>();
				else
					return or_data;
			}

			T&& value_or(T&& or_data) &&
			{
				if (variant<T>::template able_cast<T>())
					return variant<T>::template cast<T>();
				else
					return std::move(or_data);
			}
		};

		namespace Implement
		{
			struct any_interface
			{
				const std::type_info& info;
				virtual ~any_interface() {}
				any_interface(const std::type_info& ti) :info(ti) {}
			};

			template<typename T> struct any_implement_without_aligned : public any_interface, public T
			{
				template<typename ...AT> any_implement_without_aligned(AT&& ...at) : any_interface(typeid(T)), T(std::forward<AT>(at)...) {}
			};

			template<typename T>
			struct alignas(std::alignment_of<T>::value) any_implement_aligned : public any_interface, public T
			{
				template<typename ...AT> any_implement_aligned(AT&& ...at) : any_interface(typeid(T)), T(std::forward<AT>(at)...) {}
			};

			template<typename T> using any_implement = typename std::conditional_t< std::alignment_of<T>::value <= 4, Tmp::instant<any_implement_without_aligned>, Tmp::instant<any_implement_aligned>>::template in_t<T>;
		}

		//to do in c++ 17 : may be change to std::aligned_alloc or operator new (size_t ,.aligned)

		class any
		{
			void* buffer = nullptr;
			size_t buffer_size = 0;
			Implement::any_interface* pointer = nullptr;

			template<typename T> auto realloc()
			{
				if (pointer != nullptr)
					pointer->~any_interface();
				size_t require_size = std::alignment_of<T>::value + sizeof(T) - 1;
				if (require_size > buffer_size)
				{
					std::free(buffer);
					buffer = std::malloc(require_size);
					if (buffer == nullptr)
						throw std::bad_alloc{};
					buffer_size = require_size;
				}
				T* ptr = reinterpret_cast<T*>((reinterpret_cast<uintptr_t>(buffer) + std::alignment_of<T>::value - 1) & ~(std::alignment_of<T>::value - 1));
				return ptr;
			}

		public:
			operator bool() const noexcept {
				return pointer != nullptr;
			}
			~any()
			{
				if (pointer != nullptr)
					pointer->~any_interface();
				std::free(buffer);
			}
			any() {}
			any(any&& a) : buffer(a.buffer), buffer_size(a.buffer_size), pointer(a.pointer)
			{
				a.buffer = nullptr;
				a.pointer = nullptr;
				a.buffer_size = 0;
			}
			template<typename T, typename ...AT>
			any(Tmp::itself<T>, AT&&... at)
			{
				using any_type = Implement::any_implement<Tmp::pure_type<T>>;
				any_type* ptr = realloc<any_type>();
				new (ptr) any_type{ std::forward<AT>(at)... };
				pointer = ptr;
			}
			any(const any&) = delete;

			template<typename T> any(T&& t)
			{
				using any_type = Implement::any_implement<std::remove_const_t<std::remove_reference_t<T>>>;
				any_type* ptr = realloc<any_type>();
				new (ptr) any_type{ std::forward<T>(t) };
				pointer = ptr;
			}

			template<typename T> void reconstruct(T&& t)
			{
				this->operator=(std::forward<T>(t));
			}

			template<typename T, typename ...AT> void reconstruct(Tmp::itself<T>, AT&& ...at)
			{
				using any_type = Implement::any_implement<Tmp::pure_type<T>>;
				any_type* ptr = realloc<any_type>();
				new (ptr) any_type{ std::forward<AT>(at)... };
				pointer = ptr;
			}

			any& operator=(any&& a)
			{
				any Tem(std::move(a));
				if (pointer != nullptr)
					pointer->~any_interface();
				std::free(buffer);
				buffer = Tem.buffer;
				pointer = Tem.pointer;
				buffer_size = Tem.buffer_size;
				Tem.buffer = nullptr;
				Tem.buffer_size = 0;
				Tem.pointer = nullptr;
				return *this; 
			}

			any& operator=(const any&) = delete;
			template<typename T> auto& operator=(T&& t)
			{
				using any_type = Implement::any_implement<std::remove_const_t<std::remove_reference_t<T>>>;
				any_type* ptr = realloc<any_type>();
				new (ptr) any_type{ std::forward<T>(t) };
				pointer = ptr;
				return *this;
			}
			template<typename T> bool able_cast() const noexcept
			{
				if (pointer != nullptr)
					return pointer->info == typeid(T);
				return false;
			}
			template<typename T> T& cast()& noexcept
			{
				return *(static_cast<Implement::any_implement<T>*>(pointer));
			}
			template<typename T> const T& cast() const& noexcept
			{
				return *(static_cast<Implement::any_implement<T>*>(pointer));
			}
			template<typename T> T&& cast() && noexcept
			{
				return *(static_cast<Implement::any_implement<T>*>(pointer));
			}
		};

		template<typename T> class any_interface
		{
			any data;
			Tmp::pure_type<T>* inter = nullptr;
		public:
			any_interface() {}
			operator bool() const noexcept { return data; }
			template<typename K> any_interface(K&& k) : data(std::forward<K>(k)), inter(&data.cast<Tmp::pure_type<K>>())
			{
				static_assert(std::is_base_of<T, Tmp::pure_type<K>>::value, "any_interface<T> can not store the derived class of T");
			}
			template<typename K, typename ...AT> any_interface(Tmp::itself<K> t, AT&& ...at) : data(t, std::forward<AT>(at)...), inter(&data.cast<Tmp::pure_type<K>>())
			{
				static_assert(std::is_base_of<T, Tmp::pure_type<K>>::value, "any_interface<T> can not store the derived class of T");
			}
			any_interface(any_interface&& ai) : data(std::move(ai.data)), inter(ai.inter)
			{
				ai.inter = nullptr;
			}
			template<typename K> any_interface(any_interface<K>&& ai) : data(std::move(ai.data)), inter(ai.inter)
			{
				static_assert(std::is_base_of<T, Tmp::pure_type<K>>::value, "any_interface<T> can not store the derived class of T");
				ai.inter = nullptr;
			}
			Tmp::pure_type<T>* operator->() noexcept { return inter; }
			Tmp::pure_type<T>& operator*() { return *inter; }
			template<typename K, typename ...AT> void reconstruct(Tmp::itself<K> t, AT&& ...at)
			{
				static_assert(std::is_base_of<T, Tmp::pure_type<K>>::value, "any_interface<T> can not store the derived class of T");
				data.reconstruct(t, std::forward<AT>(at)...);
				inter = &data.cast<Tmp::pure_type<K>>();
			}
			template<typename K> void reconstruct(K&& k)
			{
				static_assert(std::is_base_of<T, Tmp::pure_type<K>>::value, "any_interface<T> can not store the derived class of T");
				data.reconstruct(k);
				inter = &data.cast<Tmp::pure_type<K>>();
			}
		};

	} 
}