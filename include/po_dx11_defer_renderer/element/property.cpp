#include "property.h"
namespace PO
{
	namespace Dx11
	{
		namespace Property
		{
			renderer_3d::renderer_3d() : property_interface(typeid(renderer_3d)) {}
			void renderer_3d::init(creator& c) {
				cb = c.create_constant_buffer_with_size(sizeof(buffer_type), true);
			}
			void renderer_3d::update(pipeline& c) {
				using namespace DirectX;
				buffer_type bt;
				bt.world_to_screen = XMMatrixMultiply(XMLoadFloat4x4(&view), XMLoadFloat4x4(&projection));
				bt.screen_to_world = XMMatrixInverse(nullptr, bt.world_to_screen);
				if (!c.write_constant_buffer(cb, [&](void* data) {
					*static_cast<buffer_type*>(data) = bt;
				})) __debugbreak();
			}

			transfer_3d_static::transfer_3d_static() : property_interface(typeid(transfer_3d_static)) {}

			void transfer_3d_static::init(creator& c)
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
		}
	}
}