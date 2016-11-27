#pragma once
#include "tmp.h"
#include <exception>
#include <string>
#include <functional>
#include <unordered_map>
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
			template<template<typename...> class less_ope, template<typename ...> class equal_ope, template<typename...> class ope, typename default_op, size_t s, size_t o, typename ...input>
			struct static_mapping_implement
			{
				template<typename T, typename ...AP>
				decltype(auto) operator()(T&& t, AP&&... ap) const 
				{
					using pick_type = TmpCall::call<TmpCall::in_t<input...>, TmpCall::pick_single_t<(s + o) / 2>, TmpCall::self_t>;
					if (equal_ope<pick_type>{}(std::forward<T>(t)))
					{
						return ope<pick_type>{}(std::forward<AP>(ap)...);
					}
					else if (less_ope<pick_type>{}(std::forward<T>(t)))
					{
						return static_mapping_implement<less_ope, equal_ope, ope, default_op, s, (s + o) / 2, input...>{}(std::forward<T>(t), std::forward<AP>(ap)...);
					}
					else
					{
						return static_mapping_implement<less_ope, equal_ope, ope, default_op, (s + o) / 2 + 1, o, input...>{}(std::forward<T>(t), std::forward<AP>(ap)...);
					}
				}
			};

			template<template<typename...> class less_ope, template<typename ...> class equal_ope, template<typename...> class ope, typename default_op, size_t s, typename ...input>
			struct static_mapping_implement<less_ope, equal_ope, ope, default_op, s, s, input...>
			{
				template<typename T, typename ...AP>
				decltype(auto) operator()(T&& t, AP&&... ap) const
				{
					return default_op{}(std::forward<AP>(ap)...);
				}
			};
		}

		template<template<typename...> class less_ope, template<typename ...> class equal_ope, template<typename...> class ope, typename default_op>
		struct make_static_mapping
		{
			template<typename ...input> struct in
			{
				using type = Implement::static_mapping_implement<less_ope, equal_ope, ope, default_op, 0, sizeof...(input), input...>;
			};
		};

		template<template<typename...> class less_ope, template<typename ...> class equal_ope, template<typename...> class ope, typename default_op, typename ...input>
		using static_mapping_t = Implement::static_mapping_implement<less_ope, equal_ope, ope, default_op, 0, sizeof...(input), input...>;



		/* variant */
		namespace Implement
		{
			template<typename O> struct variant_dest_ope
			{
				using type = typename O::original_t;
				void operator() (char* da) noexcept
				{
					reinterpret_cast<type*>(da)->~type();
				}
			};

			template<typename O> struct variant_cons_ope
			{
				using type = typename O::original_t;
				template<typename AT>
				void operator() (char* da, AT&& at)
				{
					new (da) type(std::forward<AT>(at).cast<type>());
				}
			};

			template<typename O> struct variant_equal_ope_ope
			{
				using type = typename O::original_t;
				template<typename AT>
				void operator() (char* da, AT&& at)
				{
					reinterpret_cast<type*>(da)->operator=( std::forward<AT>(at).cast<type>());
				}
			};

			struct default_ope
			{
				template<typename ...AT>
				void operator() (AT&& ...)
				{
					throw Error::tool_exeception("unmatch mapping");
				}
			};

			template<typename T> struct variant_less_ope
			{
				bool operator()(int i) { return i < T::label_t::value; }
			};

			template<typename T> struct variant_equal_ope
			{
				bool operator()(int i) 
				{ 
					int now = T::label_t::value;
					return now == i;
				}
			};
		}

		template<typename ...T> class variant
		{
			static_assert(!Tmp::is_repeat<T...>::value, "type of variant can not repeat");

			using dest_mapping =
				TmpCall::call< TmpCall::in_t<T...>, TmpCall::label_serial_t<1>, make_static_mapping<
					Implement::variant_less_ope, Implement::variant_equal_ope, Implement::variant_dest_ope, Implement::default_ope
				> >;

			using cons_mapping =
				TmpCall::call< TmpCall::in_t<T...>, TmpCall::label_serial_t<1>, make_static_mapping<
					Implement::variant_less_ope, Implement::variant_equal_ope, Implement::variant_cons_ope, Implement::default_ope
				> >;

			using equal_mapping =
				TmpCall::call< TmpCall::in_t<T...>, TmpCall::label_serial_t<1>, make_static_mapping<
				Implement::variant_less_ope, Implement::variant_equal_ope, Implement::variant_equal_ope_ope, Implement::default_ope
				> >;

			using size_index = 
				TmpCall::call<
					TmpCall::in_t<T...>, TmpCall::label_size_t, TmpCall::replace_by_label_t,
					TmpCall::combine_t<Tmp::bigger_value>, TmpCall::self_t
				>;

			template<typename ...in> struct construction_type
			{
				using type = TmpCall::call<
					TmpCall::in_t<T...>, TmpCall::localizer_t<1, Tmp::instant<std::is_constructible, in...>::template front_in_t>, TmpCall::sperate_value_i<Tmp::set_i>,
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

			char data[size_index::value];

			std::conditional_t<
				(sizeof...(T)+1 <= 255),
				uint8_t,
				std::conditional_t<
				(sizeof...(T)+1 <= 65535),
				uint16_t,
				std::conditional_t<
				(sizeof...(T)+1 <= 4294967295),
				uint32_t,
				uint64_t
				>
				>
			> index;

		public:
			operator bool() const noexcept { return index != 0; }

			template<typename K, typename = std::enable_if_t<std::is_same<std::remove_const_t<std::remove_reference_t<K>>, variant>::value >>
			variant(K&& v) : index(v.index)
			{
				if (index != 0)
					cons_mapping{}(index, data, std::forward<K>(v));
			}

			template<typename ...K>
			variant(K&& ...t) : index(
				construction_type<K...>::type::value
			)
			{
				static_assert(construction_type<K...>::type::value != 0, "not avalible type construct form this");
				statement_if<construction_type<K...>::type::value != 0>
					(
						[&, this](auto ind, auto&& ...input)
				{
					using type = TmpCall::call<TmpCall::in_t<T...>, TmpCall::pick_single_t<decltype(ind)::value - 1>, TmpCall::self_t>;
					new (data) type{ std::forward<decltype(input)&&>(input)... };
				}
						)(
							typename construction_type<K...>::type{}, std::forward<K>(t)...
							);
			}
			variant() : index(0) { }

			size_t get_index() const { return index; }

			~variant()
			{
				if( index!=0 )
					dest_mapping()(index, data);
			}

			template<typename K, typename = std::enable_if_t<std::is_same<std::remove_const_t<std::remove_reference_t<K>>, variant>::value >>
			variant& operator= (K&& k)
			{
				using pure_type = std::remove_reference_t<std::remove_const_t<K>>;
				statement_if<std::is_same<pure_type, variant>::value>
					(
						[this](auto&& op) 
				{
					if (index != 0)
					{
						if (index == op.index)
						{
							equal_mapping{}(index, data, std::forward<decltype(op)&&>(op));
							return;
						}

						dest_mapping{}(index, data);
						index = 0;
					}
					if (op.index != 0)
					{
						cons_mapping{}(op.index, data, std::forward<decltype(op) && >(op));
						index = op.index;
					}
					return;
				},
						[this](auto&& op) 
				{
					
					using type = typename construction_type<decltype(op)&&>::type;
					static_assert(type::value != 0, "variant can not cast to unbinded type.");
					using target_type = TmpCall::call<TmpCall::in_t<T...>, TmpCall::pick_single_t<type::value - 1>, TmpCall::self_t>;
					if (index == type::value)
					{
						reinterpret_cast<target_type*>(data)->operator=(std::forward<decltype(op) && >(op));
						return ;
					}
					else {
						dest_mapping()(index, data);
						new (data) target_type(std::forward<decltype(op) && >(op));
						index = type::value;
						return;
					}
				},
						std::forward<K>(k)
						);
				return *this;
			}

			template<typename P> bool able_cast() const noexcept
			{
				static_assert(Tmp::is_one_of<P, T...>::value, "variant can not cast to unbinded type.");
				return index == same_type<P>::type::value;
			}

			template<typename P> P& cast() &
			{
				static_assert(Tmp::is_one_of<P, T...>::value, "variant can not cast to unbinded type.");
				constexpr size_t value = same_type<P>::type::value;
				if (value != index)
				{
					throw Error::tool_exeception("variant now are not this type");
				}
				return *reinterpret_cast<P*>(data);
			}

			
			template<typename P> const P& cast() const&
			{
				static_assert(Tmp::is_one_of<P, T...>::value, "variant can not cast to unbinded type.");
				constexpr size_t value = same_type<P>::type::value;
				if (value != index)
				{
					throw Error::tool_exeception("variant now are not this type");
				}
				return *reinterpret_cast<const P*>(data);
			}

			template<typename P> P&& cast() &&
			{
				static_assert(Tmp::is_one_of<P, T...>::value, "variant can not cast to unbinded type.");
				constexpr size_t value = same_type<P>::type::value;
				if (value != index)
				{
					throw Error::tool_exeception("variant now are not this type");
				}
				return std::move(*reinterpret_cast<P*>(data));
			}


			template<typename P> P* cast_pointer() noexcept
			{
				static_assert(Tmp::is_one_of<P, T...>::value, "variant can not cast to unbinded type.");
				constexpr size_t value = same_type<P>::type::value;
				if (value != index)
				{
					return nullptr;
				}
				return reinterpret_cast<P*>(data);
			}

			template<typename P> const P* cast_pointer() const noexcept
			{
				static_assert(Tmp::is_one_of<P, T...>::value, "variant can not cast to unbinded type.");
				constexpr size_t value = same_type<P>::type::value;
				if (value != index)
				{
					return nullptr;
				}
				return reinterpret_cast<const P*>(data);
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
		};

		template<typename T> class optional : variant<T>
		{
		public:
			T& operator*() { return cast<T>(); }
			T* operator->() noexcept { return cast_pointer<T>(); }
			const T& operator*() const { return cast<T>(); }
			const T* operator->() const noexcept { return cast_pointer<T>(); }
			operator bool() const noexcept { return able_cast<T>(); }

			template<typename K>
			optional& operator= (K&& ap) { variant<T>::operator=(std::forward<K>(ap)); return *this; }

			using variant<T>::variant;

			T& value_or(T& or_data) &
			{
				if (able_cast<T>())
					return cast<T>();
				else
					return or_data;
			}

			const T& value_or(const T& or_data) const
			{
				if (able_cast<T>())
					return cast<T>();
				else
					return or_data;
			}

			T&& value_or(T&& or_data) &&
			{
				if (able_cast<T>())
					return cast<T>();
				else
					return std::move(or_data);
			}
		};

		template<typename state_type, typename fun> struct enum_state_machina
		{
			std::unordered_map<state_type, std::unordered_map<state_type, std::function<fun>>> data;

		};

	}
}