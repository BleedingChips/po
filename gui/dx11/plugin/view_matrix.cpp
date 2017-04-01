#include "view_matrix.h"


float state_to_index(bool o, bool i)
{
	if (o != i)
	{
		if (o) return 1.0;
		return -1.0;
	}
	return 0.0;
}


namespace PO
{
	namespace Dx11
	{
		view_matrix::view_matrix() : position(0.0,0.0,-1.0), front(0.0,0.0,-1.0), up(0.0, 1.0, 0.0)
		{
			using namespace DirectX;
			XMStoreFloat4x4(&view, XMMatrixIdentity());
			auto poi = XMLoadFloat3(&position);
			auto fro = XMLoadFloat3(&front);
			auto u = XMLoadFloat3(&up);
			XMStoreFloat4x4(&view, XMMatrixLookAtLH(poi, fro + poi, u));
			speed = 1.0f;
			XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(3.1415926f / 2.0f, 1024.0f/768.0f, 0.01f, 1000.0f));
		}

		Respond view_matrix::capture_event(event& e)
		{
			if (e.is_key())
			{
				bool button_state = (e.key.button_state == ButtonState::BS_DOWN);
				switch (e.key.value)
				{
				case KeyValue::K_W:
					front_state = button_state;
					break;
				case KeyValue::K_S:
					back_state = button_state;
					break;
				case KeyValue::K_A:
					left_state = button_state;
					break;
				case KeyValue::K_D:
					right_state = button_state;
					break;
				case KeyValue::K_R:
					up_state = button_state;
					break;
				case KeyValue::K_F:
					down_state = button_state;
					break;
				default:
					return Respond::Pass;
					break;
				}
				return Respond::Truncation;
			}
			return Respond::Pass;
		}

		void view_matrix::tick(duration da)
		{
			using namespace DirectX;
			if (right_state != left_state || up_state != down_state || front_state != back_state)
			{
				float s = speed * da.count() / 1000.0f;
				XMFLOAT3 ver_f{
					state_to_index(left_state, right_state),
					state_to_index(down_state, up_state),
					state_to_index(back_state, front_state)
				};
				XMVECTOR ver = XMLoadFloat3(&ver_f);
				ver = XMVector3Normalize(ver) * s;
				XMMATRIX m = XMLoadFloat4x4(&view);
				m = XMMatrixMultiply(m, XMMatrixTranslationFromVector(ver));
				XMStoreFloat4x4(&view, m);
			}
		}

		DirectX::XMFLOAT4X4 view_matrix::get_shader_proj() const
		{
			using namespace DirectX;
			XMFLOAT4X4 tem;
			XMStoreFloat4x4(&tem, XMMatrixMultiply(XMLoadFloat4x4(&view), XMLoadFloat4x4(&projection)));
			return tem;
		}

		DirectX::XMFLOAT4X4 view_matrix::get_proj() const
		{
			using namespace DirectX;
			XMFLOAT4X4 tem;
			XMStoreFloat4x4(&tem, XMMatrixMultiply(XMLoadFloat4x4(&view), XMLoadFloat4x4(&projection)));
			return tem;
		}

	}
}