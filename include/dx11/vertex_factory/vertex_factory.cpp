#include "vertex_factory.h"
namespace PO
{
	namespace Dx11
	{

	}
}
PO::Dx11::pipeline& operator<<(PO::Dx11::pipeline& pl, const PO::Dx11::vertex_factory& vf)
{
	pl << vf.vs << vf.rs << vf.get_ias();
	pl.draw_index(vf.get_index_count(), 0, 0);
	return pl;
}