#pragma once
#include "vertex_factory.h"
namespace PO
{
	namespace Dx11
	{
		struct cube_vertex_factor : vertex_factory
		{
			input_assember_stage ia;
			UINT draw_index;
			void init(creator& i);
			virtual const input_assember_stage& get_ias() const { return ia; }
			virtual UINT get_index_count() const { return draw_index; }
		};
	}
}