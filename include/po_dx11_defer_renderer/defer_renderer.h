#pragma once
#include "../po_dx11/dx11_frame.h"
#include "../po/renderer.h"
#include "../po_dx11/dx11_renderer.h"
#include "defer_element.h"
#include <typeindex>
namespace PO
{
	namespace Dx11
	{
		struct defer_renderer : simple_renderer
		{
			float near_plane;
			float far_plane;
			float xy_rate;
			float view_angel;

			defer_renderer();

			tex2 depth_stencial;
			tex2 color_bufer;
			tex2 liner_z;
			output_merge_stage oms;

			interface_storage all_interface;
			defer_element_implement_storage all_element;

			float4x4 view;
			float4x4 projection;

			defer_element merga;
			defer_element linearize_z;

			depth_stencil_state dss_defer;
			depth_stencil_state dss_transparent;
			depth_stencil_state dss_post;

			float time;

			proxy mapping(std::type_index, adapter_interface& ai);

			void init(value_table&);

			void pre_tick(duration);
			void pos_tick(duration);

			template<typename F> decltype(auto) make_compute(defer_element& dm, F&& f) { return all_interface.make_compute(*this, dm, f); }
			template<typename F> decltype(auto) make_material(defer_element& dm, F&& f) { return all_interface.make_material(*this, dm, f); }
			template<typename F> decltype(auto) make_geometry_and_placement(defer_element& dm, F&& f) { return all_interface.make_geometry_and_placement(*this, dm, f); }

			void push_element(const defer_element& de) { all_element.insert(de); }

			template<typename T>
			defer_renderer& operator<<(const T& t) { simple_renderer::operator<<(t); return *this; }

			defer_renderer& operator<<(const defer_element& t) { push_element(t); return *this; }

			bool check(const defer_element& t) { return all_element.check(t); }

			decltype(auto) lack_acceptance(const defer_element& t) const {
				return all_element.check_acceptance(t);
			}

			/*
			template<typename T> decltype(auto) find(T t) { return storage_inter.find(t, *this);}
			void push_element(const element& ptr);

			std::set<std::type_index> check(const element& ptr) { return storage_elemnt.check(ptr); }
			*/
		};
	}
}
