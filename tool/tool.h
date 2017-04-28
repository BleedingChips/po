#pragma once
#include "tmpcall.h"
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
					using pick_type = TmpCall::call<TmpCall::append<input...>, TmpCall::select_index<std::integral_constant<size_t, (s + o) / 2>>, TmpCall::self>;
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
					return Tmp::type_extract_t<T>::label::value == l;
				}
			};

			struct variant_type_index_less
			{
				template<typename T, typename L> bool operator()(T t, L l)
				{
					return l < Tmp::type_extract_t<T>::label::value;
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
			using void_index = TmpCall::call<TmpCall::append<T...>, TmpCall::localizer<TmpCall::make_func<std::is_void>>, TmpCall::self>;
			static_assert(std::is_same<void_index, std::integer_sequence<size_t>>::value, "varient can not include void");

			using mapping = TmpCall::call<TmpCall::append<T...>, TmpCall::label<TmpCall::make_func<TmpCall::label_serial>>, make_static_mapping< Implement::variant_type_index_equal, Implement::variant_type_index_less > >;

			using size_index =
				TmpCall::call <
				TmpCall::append<T...>, TmpCall::label<TmpCall::make_func<TmpCall::label_size>>, TmpCall::replace <TmpCall::make_func<TmpCall::replace_label>> ,
					TmpCall::combine<TmpCall::make_func<Tmp::bigger_value>>, TmpCall::self
				>;

			template<template<typename ...> class relation, typename ...in> struct relation_type
			{
				using type = TmpCall::call<
					TmpCall::append<T...>, TmpCall::localizer<TmpCall::make_func<relation, in...>>, TmpCall::sperate_value, TmpCall::append<std::integral_constant<size_t, sizeof...(T)>>,
					TmpCall::select_index<std::integer_sequence<size_t, 0>>, TmpCall::self
				>;
			};

			template<typename in> struct same_type
			{
				using type = TmpCall::call<
					TmpCall::append<T...>, TmpCall::localizer<TmpCall::make_func<std::is_same, in>>, TmpCall::sperate_value,
					TmpCall::append<std::integral_constant<size_t, sizeof...(T)>>, TmpCall::select_index<std::integer_sequence<size_t, 0>>, TmpCall::self
				>;
			};

			typename std::aligned_union<1, T...>::type data;

			std::conditional_t< (sizeof...(T)+1 <= 255), uint8_t, std::conditional_t< (sizeof...(T)+1 <= 65535), uint16_t, std::conditional_t< (sizeof...(T)+1 <= 4294967295), uint32_t, uint64_t > > > index;

			template<typename K>
			variant(std::true_type, K&& v) : index(v.index)
			{
				if (v.index != sizeof...(T))
					mapping{}(
						v.index,
						[&v, this](auto self)
				{
					using type = typename Tmp::type_extract_t<decltype(self)>::type;
					if(!Implement::variant_constructor<type>(data, std::forward<K>(v).template cast<type>()))
						throw Error::tool_exeception("this kind of type can not construct form itself");
				},
						[]() { throw Error::tool_exeception("unmatch mapping while construct form const varient&"); }
				);
			}

			template<typename ...K>
			variant(std::false_type, K&& ...k) : index(relation_type<std::is_constructible, K...>::type::value)
			{
				static_assert(relation_type<std::is_constructible, K...>::type::value != sizeof...(T), "not avalible type construct form this");
				using type = TmpCall::call<TmpCall::append<T...>, TmpCall::select_index<typename relation_type<std::is_constructible, K...>::type>, TmpCall::self>;
				if (!Implement::variant_constructor<type>(data, std::forward<K>(k)...))
					throw Error::tool_exeception("unmatch mapping while construct form those parameter");
			}


		public:

			operator bool() const noexcept { return index != sizeof...(T); }

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

			variant() noexcept : index(sizeof...(T)){}
			template<typename P, typename ...K> variant(P&& p, K&& ...k) :
				variant(
					std::integral_constant<bool, sizeof...(K) == 0 && std::is_same<std::remove_const_t<std::remove_reference_t<P>>, variant>::value>(),
					std::forward<P>(p), std::forward<K>(k)...
				) {}

			template<typename P, typename ...AT>
			variant(Tmp::itself<P>, AT&&... at) : index(same_type<P>::type::value)
			{
				static_assert(Tmp::is_one_of<P,T...>::value, "not avalible type construct form this");
				if (!Implement::variant_constructor<P>(data, std::forward<AT>(at)...))
					throw Error::tool_exeception("unmatch mapping while construct form those parameter");
			}

			template<typename ...AT>
			variant(Tmp::itself<void>, AT&& ...at) : variant(std::false_type{}, std::forward<AT>(at)...)
			{
				static_assert(Tmp::is_one_of<Tmp::itself<void>, T...>::value, "not avalible type construct form this");
			}

			~variant()
			{
				if (index != 0)
					mapping{}(index, [this](auto ptr) { using type = typename Tmp::type_extract_t<decltype(ptr)>::type; Implement::variant_destructor<type>(data); }, []() {});
			}

			template<typename func>
			void call(func&& f)
			{
				mapping{}(index, [&f, this](auto i) { using type = typename Tmp::type_extract_t<decltype(i)>::type; f(this->cast<type>()); }, []() {});
			}

			template<typename func, typename def>
			void call(func&& f, def&& d)
			{
				mapping{}(index, [&f, this](auto i) { using type = typename Tmp::type_extract_t<decltype(i)>::type; f(this->cast<type>()); }, d);
			}
			
			variant& operator= (const variant& v)
			{
				if (index == sizeof...(T) && v.index == sizeof...(T)) return *this;
				if (index == v.index)
				{
					mapping{}(index, 
						[&, this](auto ptr)
					{
						using type = typename Tmp::type_extract_t<decltype(ptr)>::type;
						if(!Implement::variant_assignment<type>(data, v.template cast<type>()))
							throw Error::tool_exeception("unable to assignment");
					},
						[]() {throw Error::tool_exeception("unmatch mapping while call operator="); }
					);
					return *this;
				}
				if (index != sizeof...(T))
					mapping{}(index, 
						[&, this](auto ptr) 
				{ 
					using type = typename decltype(ptr)::type::type; 
					Implement::variant_destructor<type>(data);
				}, []() {});
				if(v.index!= sizeof...(T))
					mapping{}(v.index, 
						[&, this](auto ptr) 
				{ 
					using type = typename decltype(ptr)::type::type;
					Implement::variant_constructor<type>(data, v.template cast<type>());
				}, []() {});
				index = v.index;
				return *this;
			}
			
			variant& operator= (variant&& v)
			{
				if (index == sizeof...(T) && v.index == sizeof...(T)) return *this;
				if (index == v.index)
				{
					mapping{}(index,
						[&, this](auto ptr)
					{
						using type = typename Tmp::type_extract_t<decltype(ptr)>::type;
						if (!Implement::variant_assignment<type>(data, std::move(v).template cast<type>()))
							throw Error::tool_exeception("unable to assignment");
					},
							[]() {throw Error::tool_exeception("unmatch mapping while call operator="); }
					);
						return *this;
				}
				if (index != sizeof...(T))
					mapping{}(index,
						[&, this](auto ptr)
				{
					using type = typename Tmp::type_extract_t<decltype(ptr)>::type;
					Implement::variant_destructor<type>(data);
				}, []() {});
				if (v.index != sizeof...(T))
					mapping{}(v.index,
						[&, this](auto ptr)
				{
					using type = typename Tmp::type_extract_t<decltype(ptr)>::type;
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
				
				static_assert(type_ass::value != sizeof...(T) || type_con::value != sizeof...(T), "variant can not cast to unbinded type.");
				using type_index = std::conditional_t< type_ass::value != sizeof...(T), type_ass, type_con>;
				using type = TmpCall::call<TmpCall::append<T...>, TmpCall::select_index<std::integer_sequence<size_t, type_index::value>>, TmpCall::self>;
				if (index == type_index::value)
				{
					if (!Implement::variant_assignment<type>(data, std::forward<K>(k)))
						throw Error::tool_exeception("unable to assignment");
					return *this;
				}
				if (index != sizeof...(T))
					mapping{}(index, [this](auto ptr) {Implement::variant_destructor<typename Tmp::type_extract_t<decltype(ptr)>::type>(data); }, []() {__debugbreak(); });
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
			const T* operator->() const noexcept { return &(variant<T>::template cast<T>()); }
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

		template<typename T> using optional_t = std::conditional_t<std::is_void<T>::value,
			Tool::optional<Tmp::itself<void>>,
			Tool::optional<T>
		>;

		template<typename F, typename ...P>
		decltype(auto) return_optional_t(F&& f, P&& ...ap)
		{
			return statement_if<std::is_void<decltype(f(std::forward<P>(ap)...))>::value>
				(
					[](auto& t, auto&& ...io) {t(std::forward<decltype(io) && >(io)...); return Tmp::itself<void>{}; },
					[](auto& t, auto&& ...io) {return t(std::forward<decltype(io) && >(io)...); },
					f, std::forward<P>(ap)...
					);
		};

		namespace Implement
		{
			struct any_store_struct
			{
				std::vector<char> buffer;
				template<typename T> T* alloc()
				{
					buffer.resize(sizeof(T) + alignof(T)- 1, 0);
					return reinterpret_cast<T*>((reinterpret_cast<uintptr_t>(buffer.data()) + alignof(T)- 1) & ~(alignof(T)- 1));
				}
			};

			struct any_interface
			{
				const std::type_info& info;
				virtual ~any_interface() {}
				any_interface(const std::type_info& ti) :info(ti) {}
				any_interface(const any_interface& ai) : info(ai.info) {}
				virtual any_interface* clone(any_store_struct&) const = 0;
			};

			template<typename T>
			struct alignas(alignof(std::decay_t<T>) < 4 ? 0: alignof(std::decay_t<T>)) any_implement : public any_interface 
			{
				
				std::decay_t<T> data;
				template<typename ...AT> any_implement(AT&& ...at) : any_interface(typeid(std::decay_t<T>)), data(std::forward<AT>(at)...) 
				{
					std::cout << typeid(std::decay_t<T>).name() << std::endl;
				}
				any_implement(const any_implement& ai) : any_interface(ai), std::decay_t<T>(ai) {}
				any_interface* clone(any_store_struct& ass) const
				{
					any_implement<std::decay_t<T>>* ptr = ass.alloc<any_implement<std::decay_t<T>>>();
					new (ptr) any_implement<std::decay_t<T>>{data};
					return ptr;
					/*
					return statement_if<std::is_copy_constructible<std::decay_t<T>>::value>
						(
							[this](auto& ass, auto t) {
						using type = typename std::decay_t<decltype(t)>::type;
						any_implement<type>* ptr = ass.alloc<any_implement<type>>();
						new (ptr) any_implement<type>{*this};
						return ptr;
					},
							[](auto& ass, auto t) {return nullptr; },
							ass, Tmp::itself<T>{}
							);
							*/
				}
			};
		}

		//to do in c++ 17 : may be change to std::aligned_alloc or operator new (size_t ,.aligned)

		class any
		{
			Implement::any_store_struct ass;
			Implement::any_interface* pointer = nullptr;
		public:
			operator bool() const noexcept {
				return pointer != nullptr;
			}

			~any()
			{
				if (pointer != nullptr)
					pointer->~any_interface();
			}
			any() {}
			any(any&& a) :ass(std::move(a.ass)), pointer(a.pointer) { a.pointer = nullptr; }
			template<typename T, typename ...AT>
			any(Tmp::itself<T>, AT&&... at)
			{
				static_assert(std::is_copy_constructible<std::decay_t<T>>::value, "any only accept copy constructible class");
				using any_type = Implement::any_implement<Tmp::pure_type<T>>;
				any_type* ptr = ass.alloc<any_type>();
				new (ptr) any_type{ std::forward<AT>(at)... };
				pointer = ptr;
			}

			any(const any& a)
			{
				if (a.pointer != nullptr)
					pointer = a.pointer->clone(ass);
			}

			template<typename T, typename = std::enable_if_t<!std::is_same<std::decay_t<T>, any>::value && std::is_copy_constructible<std::decay_t<T>>::value>> any(T&& t)
			{
				static_assert(std::is_copy_constructible<std::decay_t<T>>::value, "any only accept copy constructible class");
				using any_type = Implement::any_implement<std::decay_t<T>>;
				std::cout << typeid(any_type).name() << std::endl;
				any_type* ptr = ass.alloc<any_type>();
				new (ptr) any_type{ std::forward<T>(t) };
				pointer = ptr;
			}

			template<typename T> void reconstruct(T&& t)
			{
				this->operator=(std::forward<T>(t));
			}

			template<typename T, typename ...AT> void reconstruct(Tmp::itself<T>, AT&& ...at)
			{
				using any_type = Implement::any_implement<std::decay_t<T>>;
				any_type* ptr = realloc<any_type>();
				new (ptr) any_type{ std::forward<AT>(at)... };
				pointer = ptr;
			}

			any& operator=(any&& a)
			{
				any Tem(std::move(a));
				if (pointer != nullptr)
					pointer->~any_interface();
				ass = std::move(Tem.ass);
				pointer = Tem.pointer;
				Tem.pointer = nullptr;
				return *this; 
			}

			any& operator=(const any& a)
			{
				any Tem(a);
				if(pointer!=nullptr)
					pointer->~any_interface();
				ass = std::move(Tem.ass);
				pointer = Tem.pointer;
				Tem.pointer = nullptr;
				return *this;
			}

			template<typename T, typename = std::enable_if_t<!std::is_same<std::decay_t<T>, any>::value>> auto& operator=(T&& t)
			{
				static_assert(std::is_copy_constructible<std::decay_t<T>>::value, "any only accept copy constructible class");
				using any_type = Implement::any_implement<std::decay_t<T>>;
				if (pointer != nullptr)
					pointer->~any_interface();
				any_type* ptr = ass.alloc<any_type>();
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
				return static_cast<Implement::any_implement<T>*>(pointer)->data;
			}

			template<typename T> const T& cast() const& noexcept
			{
				return static_cast<Implement::any_implement<T>*>(pointer)->data;
			}
			template<typename T> T&& cast() && noexcept
			{
				return static_cast<Implement::any_implement<T>*>(pointer)->data;
			}
		};

		template<typename T> struct align_store
		{
			std::aligned_union_t<1, T> data;
			template<typename ...AT> align_store(AT&&... at) { new (data) T(std::forward<AT>(at)...); }
			~align_store() { reinterpret_cast<T*>(data)->~T(); }
			T* operator->() {
				return reinterpret_cast<T*>(data);
			}
			const T* operator->() const {
				return reinterpret_cast<T*>(data);
			}
			T& operator*() {
				return (*reinterpret_cast<T*>(data));
			}
			const T* operator*() const {
				return (*reinterpret_cast<T*>(data));
			}
			operator T&() {
				return (*reinterpret_cast<T*>(data));
			}
			operator const T& () const{
				return (*reinterpret_cast<T*>(data));
			}
		};

		/*
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
		*/

	} 
}