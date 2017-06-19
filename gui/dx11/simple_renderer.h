#pragma once
#include "../../po.h"
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
				pipeline::clear_render_target(om, bc);
			}
			template<typename form> simple_renderer(form& f) : creator(f.get_Dx11_device()), pipeline(f.get_Dx11_context()) {
				HRESULT re = f.get_swap_chain()->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&back_buffer.ptr);
				if (!SUCCEEDED(re)) throw re;
				init();
			}
			void pre_tick(duration da)
			{
				*this << vp;
			}
			void init();
		};
	}
}
