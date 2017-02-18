#pragma once
#include "dx11_define.h"
namespace PO
{
	namespace Dx11
	{

		template<size_t i, typename T>
		struct position : Implement::data_format<T>
		{
			static constexpr const char* name = "POSITION";
			static constexpr size_t size = sizeof(T);
			static constexpr size_t index = i;
		};

		template<size_t i, typename T>
		struct diffuse : Implement::data_format<T>
		{
			static constexpr const char* name = "DIFFUSE";
			static constexpr size_t size = sizeof(T);
			static constexpr size_t index = i;
		};

		template<size_t i, typename T>
		struct texcoord : Implement::data_format<T>
		{
			static constexpr const char* name = "TEXCOORD";
			static constexpr size_t size = sizeof(T);
			static constexpr size_t index = i;
		};

		namespace Implement
		{
			template<size_t d, typename T> struct layout_element
			{
				static void create_input_element_desc(std::vector<D3D11_INPUT_ELEMENT_DESC>& v, size_t& last, size_t solt, size_t instant_size)
				{
					size_t last_size = last + sizeof(T);
					last = last_size;
					v.push_back(D3D11_INPUT_ELEMENT_DESC{ T::name, static_cast<UINT>(T::index), T::format, static_cast<UINT>(solt), static_cast<UINT>(last_size),
						(instant_size == 0 ? D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_INSTANCE_DATA),
						instant_size
					});
				}
				/*
				const D3D11_INPUT_ELEMENT_DESC& operator() (size_t input_slot = 0)
				{
				static const D3D11_INPUT_ELEMENT_DESC format{ T::name, static_cast<UINT>(T::index), T::format, static_cast<UINT>(input_slot), static_cast<UINT>(d),  D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,  0 };
				return format;
				}
				*/
			};
			//template<size_t d, typename T> const D3D11_INPUT_ELEMENT_DESC layout_element<d, T>::format{ T::name, static_cast<UINT>(T::index), T::format, 0, static_cast<UINT>(d),  D3D11_INPUT_PER_VERTEX_DATA,  0 };


			template<typename ...T> class final_layout
			{
			public:
				static void create_input_element_desc(std::vector<D3D11_INPUT_ELEMENT_DESC>& v, size_t solt, size_t instant_size)
				{
					size_t last = 0;
					(T::create_input_element_desc(v, last, solt, instant_size)...);
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
			static void create_input_element_desc(std::vector<D3D11_INPUT_ELEMENT_DESC>& v, size_t solt)
			{
				type::create_input_element_desc(v, solt, 0);
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