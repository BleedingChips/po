#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include "../po_dx/dx_type.h"
namespace PO
{
	namespace Dx11
	{

		template<typename syntax_t, size_t i, typename store_type, size_t ic = 0, bool force_instance = false>
		struct syntax : DXGI::data_format<store_type>
		{
			static constexpr size_t align = alignof(store_type);
			static constexpr size_t size = sizeof(store_type);
			static constexpr size_t index = i;
			static constexpr D3D11_INPUT_CLASSIFICATION buffer_type = ((!force_instance && ic == 0) ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA);
			static constexpr size_t instance_used = ic;
			static const char* name() { return syntax_t{}(); }
		};

		template<typename T, typename ...AT> struct layout_type;

		namespace Implement
		{
			template<size_t d, typename T> struct layout_element
			{
				using type = T;
				D3D11_INPUT_ELEMENT_DESC operator()(size_t solt) const
				{
					return D3D11_INPUT_ELEMENT_DESC{
						T::name(), static_cast<UINT>(T::index), T::format, static_cast<UINT>(solt), static_cast<UINT>(d),
						T::buffer_type,
						static_cast<UINT>(T::instance_used)
					};
				}
			};

			template<typename its, typename ...oth> struct final_layout
			{
				static_assert(!Tmp::is_repeat<typename its::type, typename oth::type...>::value, "layout can not contain same element");

				std::vector<D3D11_INPUT_ELEMENT_DESC> operator()(size_t solt) const
				{
					std::vector<D3D11_INPUT_ELEMENT_DESC> buffer =
					{
						its{}(solt), oth{}(solt)...
					};
					return buffer;
				}
			};

			template<typename input, size_t last, typename ...AT> struct make_layout_execute;
			template<typename ...input, size_t last, typename its, typename ...AT> struct make_layout_execute<std::tuple<input...>, last, its, AT...>
			{
				using type = typename make_layout_execute<
					std::tuple<input..., layout_element<last, its>>,
					((last % its::align) == 0 ? (last + its::size) : (last + its::align - (last %its::align))),


					//((( last + its::size ) % 4) == 0 ? (last + its::size) : ((last /4 +1 )*4)),
					AT...
				>::type;
			};

			template<typename ...input, size_t last, typename ...its, typename ...AT> struct make_layout_execute<std::tuple<input...>, last, layout_type<its...>, AT...>
			{
				using type = typename make_layout_execute <
					std::tuple<input...>, last, its..., AT...
				>::type;
			};

			template<typename ...input, size_t last> struct make_layout_execute<std::tuple<input...>, last>
			{
				using type = final_layout<input...>;
			};

		}

		template<typename T, typename ...AT> struct layout_type
		{
			using type = typename Implement::make_layout_execute<std::tuple<>, 0, T, AT...>::type;
			std::vector<D3D11_INPUT_ELEMENT_DESC> operator ()(size_t solt = 0) const
			{
				return type{}(solt);
			}
			operator std::vector<D3D11_INPUT_ELEMENT_DESC> () const
			{
				return type{}(0);
			}
		};

		//template<typename T, >
	}
}