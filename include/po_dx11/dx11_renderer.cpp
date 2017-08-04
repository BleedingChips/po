#include "dx11_renderer.h"
namespace PO
{
	namespace Dx11
	{
		simple_renderer::simple_renderer(value_table& vt) : 
			pipeline(vt.get<std::shared_ptr<pipeline_implement>>(), vt.get<Win32::com_ptr<ID3D11Device>>()),
			back_buffer(vt.get<tex2>())
		{
			vp.fill_texture(0, back_buffer);
			om << cast_render_target_view(back_buffer)[0];
		}
		/*
		proxy simple_renderer::mapping(std::type_index ti, adapter_interface& ai)
		{
			if (ti == typeid(simple_renderer))
				return make_proxy<simple_renderer>(ai, *this);
			return {};
		}*/


	}
}