#pragma once
#include "renderer.h"
namespace PO
{
	namespace Implement
	{

		template<typename extension_t> struct extension_implement;

		class extension_interface
		{
			template<typename T>
			struct check
			{
				using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<T>>::type>;
				static_assert(funtype::size == 1, "only receive one parameter");
				using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
			};

			std::type_index id_info;
		public:
			const std::type_index& id() const { return id_info; }
			virtual void handle_respond(const event& e, viewer& v) = 0;
			virtual void tick(duration da, viewer& v) = 0;
			extension_interface(const std::type_index& id);
			virtual ~extension_interface();
			template<typename T> bool cast(T&& t)
			{
				using type = typename check<T>::true_type;
				if (id_info == typeid(type))
					return (t(static_cast<type&>(static_cast<extension_implement<type>&>(*this))), true);
				return false;
			}
		};

		template<typename extension_t>
		struct extension_implement : extension_interface, extension_t
		{
			virtual void handle_respond(const event& e, viewer& v)
			{
				extension_t::handle_respond(e, v);
			}
			virtual void tick(duration da, viewer& v)
			{
				extension_t::tick(da, v);
			}
			template<typename ...AT>
			extension_implement(value_table& vt, AT&& ... at) : extension_interface(typeid(extension_t)), extension_t(vt, std::forward<AT>(at)...) {}
		};

	}

	template<typename T> struct extension {};
}