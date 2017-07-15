#include "property.h"
namespace PO
{
	namespace Dx11
	{
		namespace Property
		{
			renderer_3d::renderer_3d() : property_interface(typeid(renderer_3d)) {}
			void renderer_3d::push(creator& c) {
				//aligned_storage<DirectX::XMMATRIX, DirectX::XMMATRIX, float, float> as;
				//cb = c.create_constant_buffer_with_size(sizeof(buffer_type), true);
			}
			void renderer_3d::update(pipeline& c) {
				using namespace DirectX;

				// world_to_screen; screen_to_world; screen_rate; time;
				aligned_storage<DirectX::XMMATRIX, DirectX::XMMATRIX, float, float> as;


				buffer_type bt;
				as.get<0>() = XMMatrixMultiply(XMLoadFloat4x4(&view), XMLoadFloat4x4(&projection));
				as.get<1>() = XMMatrixInverse(nullptr, as.get<0>());
				as.get<2>() = 1024.0f / 768.0f;
				as.get<3>() = time;
				if (!cb)
				{
					cb = c.get_creator().create_constant_buffer(&as, true);
				}else
				if (!c.write_constant_buffer(cb, [&](void* data) {
					*static_cast<std::decay_t<decltype(as)>*>(data) = as;
				})) __debugbreak();
			}

			transfer_3d_static::transfer_3d_static() : property_interface(typeid(transfer_3d_static)) {}

			void transfer_3d_static::push(creator& c)
			{
				cb = c.create_constant_buffer_with_size(sizeof(buffer_type), true);
			}

			void transfer_3d_static::update(pipeline& c)
			{
				using namespace DirectX;
				buffer_type bt;
				bt.local_to_world = XMLoadFloat4x4(&local_to_world);
				bt.world_to_local = XMMatrixInverse(nullptr, bt.local_to_world);
				if(!c.write_constant_buffer(cb, [&](void* data) {
					*static_cast<buffer_type*>(data) = bt;
				})) __debugbreak();
			}

			transfer_2d_static::transfer_2d_static() : property_interface(typeid(transfer_2d_static)) {}
			void transfer_2d_static::update(pipeline& p)
			{

				PO::Dx::aligned_storage<bool, bool, float2, float3, float4> pb{ adapt_screen , true, shift_xy, roll_and_center, scale_and_center };

				if (!cb)
					cb = p.get_creator().create_constant_buffer(&pb, true);
				else if (change)
				{
					p.write_constant_buffer(cb, [&](void* data) {
						*static_cast<PO::Dx::aligned_storage<bool, bool, float2, float3, float4>*>(data) = pb;
					});
					change = false;
				}

			}

		}
	}
}