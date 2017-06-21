#pragma once
#include "../../renderer.h"
#include "dx11_frame.h"
namespace PO
{
	namespace Dx11
	{
		struct simple_renderer
		{
			Tool::optional<creator> cre;
			Tool::optional<pipeline> pipe;

			operator creator& () { return *cre; }
			operator pipeline& () { return *pipe; }

			tex2 back_buffer;
			output_merge_stage om;
			viewports vp;
			void clear_back_buffer(const std::array<float, 4>& bc) {
				pipe->clear_render_target(om, bc);
			}
			simple_renderer() {}

			proxy mapping(std::type_index, adapter_interface& ai);

			void init(value_table& );

			void pre_tick(duration da)
			{
				*pipe << vp;
			}
		};
	}
}
