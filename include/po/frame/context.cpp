#include "context.h"
namespace PO::ECSFramework
{
	// component **************************************************
	namespace Implement
	{
		singleton_component_ref::singleton_component_ref(std::type_index ti, void(*d)(void*) noexcept) : defined_id(ti), deleter(d)
		{

		}
		void singleton_component_ref::destory() noexcept
		{
			if (avalible)
			{
				(*deleter)(data);
				avalible = false;
			}
		}
		singleton_component_ref::~singleton_component_ref()
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

		void entity_implement::destory_all_component(std::set<std::type_index>& s)
		{
			for (auto& ite : component_set)
			{
				if (ite.second && *ite.second)
				{
					ite.second->destory();
					s.insert(ite.first);
				}
			}
			component_set.clear();
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
			for (auto& ite : component_set)
			{
				if (ite.second && *ite.second)
					ite.second->destory();
			}
			component_set.clear();
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
		void system_ref::destory() noexcept { ptr->~system_interface(); ptr = nullptr; }
		system_ref::~system_ref() { 
			destory(); 
		}
	}
}