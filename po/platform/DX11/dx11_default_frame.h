#pragma once
#include "dx11_form.h"
#include "dx11_renderer.h"
namespace PO
{
	namespace Assistant
	{
	}

	struct dx11_default_frame
	{
		using form = mod_pair<Platform::Dx11::dx11_form, Platform::Dx11::dx11_form_interface>;
		using renderer = mod_pair<Platform::Dx11::dx11_renderer, Platform::Dx11::dx11_renderer_interface>;
	};
}
