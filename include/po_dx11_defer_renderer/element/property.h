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
				float screen_rate;

				struct buffer_type
				{
					alignas(16) DirectX::XMMATRIX world_to_screen;
					alignas(16) DirectX::XMMATRIX screen_to_world;
					alignas(16) float screen_rate;
				};

				constant_buffer cb;
				renderer_3d();
				void push(creator& c) override;
				void update(pipeline& c) override;
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
				void push(creator& c) override;
				void update(pipeline& c) override;
			};

			class transfer_2d_static : public property_interface
			{
				constant_buffer cb;
				bool adapt_screen = false;
				float2 shift_xy = float2{ 0.0, 0.0 };
				float3 roll_and_center = float3{ 0.0, 0.0, 0.0 };
				float4 scale_and_center = float4{ 1.0, 1.0, 0.0, 0.0 };
				bool change = false;
			public:
				const constant_buffer& get_buffer() const { return cb; }
				void set_shift(float2 d) { shift_xy = d; change = true; }
				void set_scale(float2 r, float2 center = {0.0, 0.0}) { scale_and_center = float4{ r.x, r.y, center.x, center.y }; change = true; }
				void set_adapt_screen(bool s = true) { adapt_screen = s; change = true; }
				transfer_2d_static();
				virtual void update(pipeline& p) override;
			};
		}
	}
}