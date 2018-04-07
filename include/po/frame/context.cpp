#include "context.h"
namespace PO::ECSFramework
{
	// component **************************************************
	namespace Implement
	{
		component_ref::component_ref(std::type_index ti, void(*d)(void*) noexcept) : defined_id(ti), deleter(d)
		{

		}
		void component_ref::destory() noexcept
		{
			if (avalible)
			{
				(*deleter)(data);
				avalible = false;
			}
		}
		component_ref::~component_ref()
		{
			destory();
		}
	}

	// entity **************************************************
	namespace Implement
	{
		bool entity_implement::insert(component_ptr cp) noexcept
		{
			auto ite = component_set.find(cp->id());
			if (ite != component_set.end())
			{
				ite->second->destory();
				ite->second = std::move(cp);
				return true;
			}
			else {
				auto id = cp->id();
				component_set.insert({ id, std::move(cp) });
				return false;
			}
		}

		bool entity_implement::check_exist(std::type_index ti) const noexcept
		{
			return component_set.find(ti) != component_set.end();
		}

		bool entity_implement::destory_component(std::type_index id) noexcept
		{
			auto ite = component_set.find(id);
			if (ite == component_set.end())
				return false;
			if (ite->second && *ite->second)
				ite->second->destory();
			component_set.erase(ite);
			return true;
		}

		component_ptr entity_implement::get_component(std::type_index ti) const noexcept
		{
			auto ite = component_set.find(ti);
			if (ite != component_set.end())
			{
				return ite->second;
			}
			else {
				return {};
			}
		}

		bool entity_implement::reflesh() noexcept
		{
			for (auto ite = component_set.begin(); ite != component_set.end();)
			{
				if (!ite->second || !*ite->second)
					component_set.erase(ite++);
				else
					++ite;
			}
			return component_set.empty();
		}

		entity_implement::~entity_implement()
		{
			assert(component_set.empty());
		}

		void entity_ref::destory() noexcept
		{
			if (avalible)
			{
				shift()->~entity_implement();
				avalible = false;
			}
		}

		entity_ref::~entity_ref()
		{
			destory();
		}
	}

	
	// system **************************************************
	namespace Implement
	{
		void system_ref::destory() { ptr->~system_interface(); }
	}
}