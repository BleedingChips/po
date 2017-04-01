#pragma once
#include "../../../frame/frame.h"
#include <DirectXMath.h>
namespace PO
{
	namespace Dx11
	{
		struct view_matrix
		{

			bool front_state = false;
			bool back_state = false;
			bool up_state = false;
			bool down_state = false;
			bool left_state = false;
			bool right_state = false;

			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT3 front;
			DirectX::XMFLOAT3 up;

			DirectX::XMFLOAT4X4 view;
			DirectX::XMFLOAT4X4 projection;

			float speed;

		public:
			Respond capture_event(event& e);
			void tick(duration da);
			DirectX::XMFLOAT4X4 get_shader_proj() const;
			DirectX::XMFLOAT4X4 get_proj() const;
			view_matrix();
		};
	}
}