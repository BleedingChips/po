#pragma once
#include "define.h"
namespace PO
{

	namespace Implement
	{
		struct viewer_interface
		{

		};
	}

	class viewer {
		Implement::viewer_interface& ref;
	public:
		viewer(Implement::viewer_interface& r): ref(r) {}
		viewer(const viewer&) = default;
	};
}