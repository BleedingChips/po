#pragma once
#include "define.h"
namespace PO
{

	namespace Implement
	{
		class viewer_interface
		{

		};
	}

	class viewer {
		Tool::completeness_ref ref;
		Implement::viewer_interface& interface;
	public:
		viewer(Tool::completeness_ref r, Implement::viewer_interface& in) : ref(std::move(r)), interface(in) {}
		viewer(const viewer& v) : ref(v.ref), interface(v.interface) {}
	};
}