#pragma once
#include "dx11_define.h"
namespace PO
{
	namespace Dx11
	{

		namespace Buffer
		{
			class index
			{
				buffer_ptr data;
			public:
				operator bool() const {return data.}
				index() {}
			};
		}




		template<typename T> Buffer::buffer_ptr create_index(T* t, size_t index_size)
		{
			D3D11_BUFFER_DESC tem
			{
				sizeof(T) * index_size,

			};
		}
	}
}