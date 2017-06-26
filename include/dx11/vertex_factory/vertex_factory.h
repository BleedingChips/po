#pragma once
#include "../../dx/movement.h"
#include "../dx11_frame.h"
namespace PO
{
	namespace Dx11
	{
		struct vertex_factory
		{
			//movement_interpolation mi;
			vertex_stage vs;
			raterizer_state rs;
			virtual void init(creator& c);
			virtual const input_assember_stage& get_ias() const = 0;
			virtual UINT get_index_count() const = 0;
		};
	}
}
PO::Dx11::pipeline& operator<<(PO::Dx11::pipeline&, const PO::Dx11::vertex_factory&);
