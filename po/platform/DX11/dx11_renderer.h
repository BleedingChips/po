#pragma once
#include "dx11_form.h"
#include "../../frame/define.h"
namespace PO
{
	namespace Platform
	{
		namespace Dx11
		{
			struct dx11_renderer
			{
				dx11_form_interface dfi;
				time_calculator tc;
				float all_time = 0.0;
			public:
				dx11_renderer(dx11_form_interface& adi)  : dfi(adi)
				{
				}
				void tick(time_point tp) 
				{
					duration da;
					if (tc.tick(tp, da))
					{
						
						all_time += da.count();
						float a = abs(sin(all_time * 0.001f));
						float a2 = abs(sin(all_time * 0.001f * 2));
						float a3 = abs(sin(all_time * 0.001f * 3));
						float color[4] = { a,a2,a3,1.0f };
						dfi.clean_chain(a, a2, a3);
						dfi.swap_chain();
					}
				}
			};

			struct dx11_renderer_interface 
			{
				dx11_renderer_interface(dx11_renderer&) {}
			};

		}
	}
	
}