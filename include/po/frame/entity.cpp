#include "entity.h"
namespace PO::ECSFramework
{
	namespace Implement
	{
		bool entity_implement::check_exist(std::type_index ti) const noexcept
		{
			return component_set.find(ti) != component_set.end();
		}

		void entity_implement::insert(Implement::component_holder_ptr ptr) noexcept
		{
			auto& ite = component_set[ptr->id()];
			if (ite)
				ite->clear();
			ite = std::move(ptr);
		}

		Implement::component_holder_ptr entity_implement::get_component(std::type_index ti) noexcept
		{
			auto ite = component_set.find(ti);
			if (ite != component_set.end())
				return ite->second;
			else
				return {};
		}

		Implement::component_holder_ptr entity_implement::get_component(std::type_index ti) const noexcept
		{
			auto ite = component_set.find(ti);
			if (ite != component_set.end())
				return ite->second;
			else
				return {};
		}
	}

	
}