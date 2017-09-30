#include "extension.h"
namespace PO
{
	namespace Implement
	{
		extension_interface::~extension_interface() {}
		extension_interface::extension_interface(const std::type_index& t) :id_info(t) {}
	}
}