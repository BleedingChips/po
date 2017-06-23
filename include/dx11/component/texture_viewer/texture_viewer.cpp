#include "texture_viewer.h"
#include "../../dx11_vertex.h"
#include <vector>

struct verdex_index 
{
	const char* operator()() { return "INDEX"; }
};

std::vector<uint32_t> poi = { 0, 1, 2, 3 };
using vertex_type = PO::Dx11::layout_type<
	PO::Dx11::syntax<verdex_index, 0, decltype(poi)::value_type>
	>;


namespace PO
{
	namespace Dx11
	{
		void texture_viewer::init(creator& c)
		{
			is << c.create_vertex(poi, vertex_type{})[0];

		}

		void texture_viewer::tick(const unordered_access_view& uav, pipeline& p)
		{

		}
	}
}