#pragma once
#include "../../renderer.h"
#include "dx11_frame.h"
namespace PO
{
	namespace Dx11
	{
		struct simple_renderer : creator, pipeline
		{

			tex2 back_buffer;
			output_merge_stage om;
			viewports vp;
			void clear_back_buffer(const std::array<float, 4>& bc) {
				clear_render_target(om, bc);
			}
			simple_renderer(value_table& vt);

			proxy mapping(std::type_index, adapter_interface& ai);

			void init(value_table& );

			void pre_tick(duration da)
			{
				*this << vp;
			}
		};
	}
}
