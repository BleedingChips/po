#include "dx11_renderer.h"
namespace PO
{
	namespace Dx11
	{
		void simple_renderer::init(value_table& vt)
		{
			creator::init(*vt.get<Win32::com_ptr<ID3D11Device>>());
			pipeline::init(*vt.get<Win32::com_ptr<ID3D11DeviceContext>>());
			auto swap = *vt.get<Win32::com_ptr<IDXGISwapChain>>();
			HRESULT re = (swap.get())->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&back_buffer.ptr);
			if (!SUCCEEDED(re)) throw re;
			vp.fill_texture(0, back_buffer);
			om << cast_render_target_view(back_buffer)[0];
		}

		proxy simple_renderer::mapping(std::type_index ti, adapter_interface& ai)
		{
			if (ti == typeid(simple_renderer))
				return make_proxy<simple_renderer>(ai, *this);
			return {};
		}


	}
}