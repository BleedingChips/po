#pragma once
#include "interface.h"
namespace PO
{
	namespace Dx11
	{
		namespace Property
		{
			struct renderer_3d : public property_interface
			{
				float4x4 projection;
				float4x4 view;

				struct buffer_type
				{
					alignas(16) DirectX::XMMATRIX world_to_screen;
					alignas(16) DirectX::XMMATRIX screen_to_world;
				};

				constant_buffer cb;
				renderer_3d();
				void init(creator& c);
				void update(pipeline& c);
			};

			struct transfer_3d_static : public property_interface
			{
				struct buffer_type
				{
					alignas(16) DirectX::XMMATRIX local_to_world;
					alignas(16) DirectX::XMMATRIX world_to_local;
				};
				float4x4 local_to_world;

				constant_buffer cb;
				transfer_3d_static();
				void init(creator& c);
				void update(pipeline& c);
			};
		}
	}
}