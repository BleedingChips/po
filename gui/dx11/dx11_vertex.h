#pragma once
#include "dx11_define.h"
namespace PO
{
	namespace Dx11
	{

		template<typename type, size_t i, typename T, size_t ic = 0>
		struct syntax : DXGI::data_format<T>
		{
			static constexpr size_t size = sizeof(T);
			static constexpr size_t index = i;
			static constexpr size_t instance_used = ic;
			static const char* name() { return type{}(); }
		};

		namespace Implement
		{
			template<size_t d, typename T> struct layout_element
			{
				static void create_input_element_desc(D3D11_INPUT_ELEMENT_DESC* v, size_t solt)
				{
					*(v) = D3D11_INPUT_ELEMENT_DESC{ T::name(), static_cast<UINT>(T::index), T::format, static_cast<UINT>(solt), static_cast<UINT>(d),
						(T::instance_used == 0 ? D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_INSTANCE_DATA),
						static_cast<UINT>(T::instance_used)
					};
				}
			};

			template<typename its, typename ...oth> struct final_layout : std::integral_constant<size_t, sizeof...(oth) +1>
			{
				static void create_input_element_desc(D3D11_INPUT_ELEMENT_DESC* v, size_t solt)
				{
					its::create_input_element_desc(v, solt);
					final_layout<oth...>::create_input_element_desc(++v, solt);
				}
			};

			template<typename its> struct final_layout<its> : std::integral_constant<size_t, 1>
			{
				static void create_input_element_desc(D3D11_INPUT_ELEMENT_DESC* v, size_t solt)
				{
					its::create_input_element_desc(v, solt);
				}
			};

			template<typename input, size_t last, typename ...AT> struct make_layout_execute;
			template<typename ...input, size_t last, typename its, typename ...AT> struct make_layout_execute<Tmp::set_t<input...>, last, its, AT...>
			{
				using type = typename make_layout_execute<Tmp::set_t<input..., layout_element<last, its>>, last + its::size, AT...>::type;
			};

			template<typename ...input, size_t last> struct make_layout_execute<Tmp::set_t<input...>, last>
			{
				using type = final_layout<input...>;
			};
		}

		template<typename T, typename ...AT> struct layout_type
		{
			static_assert(!Tmp::is_repeat<T, AT...>::value, "");
			using type = typename Implement::make_layout_execute<Tmp::set_t<>, 0, T, AT...>::type;
			static constexpr size_t value = type::value;
			static void create_input_element_desc(D3D11_INPUT_ELEMENT_DESC* v, size_t solt)
			{
				type::create_input_element_desc(v, solt);
			}
		};

		namespace Components
		{
			template<typename T> struct is_component;
			struct vertex
			{
				static constexpr D3D11_BIND_FLAG bind_flag = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
				struct scription
				{
					size_t element_size;
					void(*layout_creater)(std::vector<D3D11_INPUT_ELEMENT_DESC>& v, size_t slot);
				};
			};
			template<> struct is_component<vertex> :std::true_type {};
		}
		
	}
}