#include "dx11_define.h"
#include "../../frame/define.h"
#include <fstream>
namespace PO
{
	namespace Dx11
	{

		HRESULT vertex_const::create_buffer(const Implement::resource& re, void* da, size_t t)
		{
			D3D11_BUFFER_DESC tem_b{
				static_cast<UINT>(t),
				D3D11_USAGE::D3D11_USAGE_IMMUTABLE,
				static_cast<UINT>(D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER),
				static_cast<UINT>(0x0L),
				static_cast<UINT>(0x0L),
				0
			};
			D3D11_SUBRESOURCE_DATA tem_s{ da, 0, 0 };
			return re->CreateBuffer(&tem_b, &tem_s, &data);
		}

		HRESULT shader_v::load_binary(const Implement::resource& re, Implement::data da)
		{
			shader = nullptr;
			da = std::move(da);
			return re->CreateVertexShader(da, da.size(), nullptr, &shader);
		}
	}
}