#include "simple_renderer.h"
namespace PO
{
	namespace Dx11
	{
		void simple_renderer::init()
		{
			vp.fill_texture(0, back_buffer);
			om << cast_render_target_view(back_buffer)[0];
		}
	}
}