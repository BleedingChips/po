#include "context_implement.h"
#include <atomic>
#include <limits>
#include <iostream>
#undef max
namespace PO::ECSFramework
{

	// component_map **************************************************
	namespace Implement
	{
		void component_map::insert(component_holder c)
		{
			assert(c.componenet && *c.componenet);
			assert(c.entity && *c.entity);
			auto id = c.componenet->id();
			auto ite = all_component_map.find(id);
			if (ite == all_component_map.end())
			{
				ite = all_component_map.insert({ id, element{} }).first;
			}
			auto& ref = ite->second;
			ref.accosiate_filter.erase(std::remove_if(ref.accosiate_filter.begin(), ref.accosiate_filter.end(), [&](Tool::intrusive_ptr<pre_filter_storage_interface>& wp) {
				if (wp && *wp)
				{
					wp->insert(c.entity);
					return false;
				}
				return true;
			}), ref.accosiate_filter.end());
			ref.ptr.push_back(std::move(c));
		}

		void component_map::reflesh(std::type_index ti)
		{
			auto ite = all_component_map.find(ti);
			if (ite != all_component_map.end())
			{
				auto& ref = ite->second;
				ref.ptr.erase(std::remove_if(ref.ptr.begin(), ref.ptr.end(), [](component_holder& ch) {
					return !(ch.componenet && *ch.componenet);
				}), ref.ptr.end());
			}
		}

		void component_map::insert(Tool::intrusive_ptr<pre_filter_storage_interface> filter)
		{
			assert(filter && *filter);
			auto info = filter->needed();
			decltype(all_component_map)::iterator min_ite = all_component_map.end();
			for (std::size_t i = 0; i < info.size(); ++i)
			{
				auto ite = all_component_map.find(info[i]);
				if (ite == all_component_map.end())
				{
					min_ite = all_component_map.insert({ info[i] ,element{} }).first;
					min_ite->second.accosiate_filter.push_back(filter);
				}
				else {
					ite->second.accosiate_filter.push_back(filter);
					if (min_ite == all_component_map.end())
						min_ite = ite;
					else if (ite->second.ptr.size() < min_ite->second.ptr.size())
						min_ite = ite;
				}
			}
			if (min_ite != all_component_map.end())
			{
				auto& ref = min_ite->second.ptr;
				ref.erase(std::remove_if(ref.begin(), ref.end(), [&](component_holder& ch) {
					if (ch.componenet && *ch.componenet)
					{
						filter->insert(ch.entity);
						return false;
					}
					return true;
				}), ref.end());
			}
		}

		void component_map::update_temporary(Tool::intrusive_ptr<pre_filter_storage_interface> filter)
		{
			assert(filter);
			auto info = filter->needed();
			decltype(all_component_map)::iterator min_ite = all_component_map.end();
			for (std::size_t i = 0; i < info.size(); ++i)
			{
				auto ite = all_component_map.find(info[i]);
				if (ite == all_component_map.end())
				{
					min_ite = all_component_map.end();
					break;
				}
				else {
					if (min_ite == all_component_map.end())
						min_ite = ite;
					else if (ite->second.ptr.size() < min_ite->second.ptr.size())
						min_ite = ite;
				}
			}
			if (min_ite != all_component_map.end())
			{
				auto& ref = min_ite->second.ptr;
				ref.erase(std::remove_if(ref.begin(), ref.end(), [&](component_holder& ch) {
					if (ch.componenet && *ch.componenet)
					{
						filter->insert(ch.entity);
						return false;
					}
					return true;
				}), ref.end());
			}
		}
	}


	//context_temporary *****************************
	namespace Implement
	{
		void context_temporary::insert_component(component_ptr cp, entity_ptr ep)
		{
			context_event.push_back(component_holder{ std::move(cp), std::move(ep) });
		}

		void context_temporary::insert_system(system_ptr sp)
		{
			context_event.push_back(std::move(sp));
		}

		void context_temporary::insert_singleton_component(Implement::singleton_component_ptr scp)
		{
			context_event.push_back(std::move(scp));
		}

		void context_temporary::destory_entity(entity_ptr e)
		{
			context_event.push_back(std::move(e));
		}

		void context_temporary::destory_component(entity_ptr e, std::type_index ti)
		{
			context_event.push_back(component_destory{ std::move(e), ti });
		}

		void context_temporary::destory_singleton_component(std::type_index ti)
		{
			context_event.push_back(singleton_component_destory{ ti });
		}

		void context_temporary::destory_system(std::type_index ti)
		{
			context_event.push_back(system_destory{ ti });
		}

		void context_temporary::insert_temporary_system(system_ptr pt)
		{
			context_event.push_back(temporary_system{ std::move(pt) });
		}
	}
	
	// system map **********************************
	enum class SequenceResult
	{
		UNDEFINE,
		ERROR_RESULT,
		FIRST,
		SECOND,
		NOT_CARE
	};

	bool have(Implement::type_viewer view, std::type_index ti)
	{
		if (view.size() == 0)
			return false;
		if (view[0] > ti || view[view.size() - 1] < ti)
			return false;
		std::size_t Start = 0;
		std::size_t End = view.size();
		for (; Start != End; )
		{
			std::size_t C = (Start + End) / 2;
			if (view[C] == ti)
				return true;
			else if (view[C] < ti)
			{
				Start = C + 1;
			}
			else
			{
				End = C;
			}
		}
		return false;
	}

	bool is_collided(Implement::type_viewer ti1, Implement::type_viewer ti2)
	{
		if (ti1.size() != 0 && ti2.size() != 0 && ti1[ti1.size() - 1] >= ti2[0] && ti1[0] <= ti2[ti2.size() - 1])
		{
			for (std::size_t index = 0; index <= ti1.size(); ++index)
			{
				if (have(ti2, ti1[index]))
					return true;
			}
		}
		return false;
	}



	SequenceResult calculate_sequence(SystemSequence S1, SystemSequence S2)
	{
		switch (S1)
		{
		case SystemSequence::UNDEFINE:
			switch (S2)
			{
			case PO::ECSFramework::SystemSequence::UNDEFINE:
				return SequenceResult::UNDEFINE;
			case PO::ECSFramework::SystemSequence::BEFORE:
				return SequenceResult::SECOND;
			case PO::ECSFramework::SystemSequence::AFTER:
				return SequenceResult::FIRST;
			case PO::ECSFramework::SystemSequence::NOT_CARE:
				return SequenceResult::NOT_CARE;
			default:
				return SequenceResult::ERROR_RESULT;
			}
		case SystemSequence::BEFORE:
			switch (S2)
			{
			case PO::ECSFramework::SystemSequence::UNDEFINE:
			case PO::ECSFramework::SystemSequence::NOT_CARE:
			case PO::ECSFramework::SystemSequence::AFTER:
				return SequenceResult::FIRST;
			case PO::ECSFramework::SystemSequence::BEFORE:
			default:
				return SequenceResult::ERROR_RESULT;
			};
		case SystemSequence::AFTER:
			switch (S2)
			{
			case PO::ECSFramework::SystemSequence::UNDEFINE:
			case PO::ECSFramework::SystemSequence::NOT_CARE:
			case PO::ECSFramework::SystemSequence::BEFORE:
				return SequenceResult::SECOND;
			case PO::ECSFramework::SystemSequence::AFTER:
			default:
				return SequenceResult::ERROR_RESULT;
			};
		case SystemSequence::NOT_CARE:
			switch (S2)
			{
			case PO::ECSFramework::SystemSequence::UNDEFINE:
			case PO::ECSFramework::SystemSequence::NOT_CARE:
				return SequenceResult::NOT_CARE;
			case PO::ECSFramework::SystemSequence::BEFORE:
				return SequenceResult::SECOND;
			case PO::ECSFramework::SystemSequence::AFTER:
				return SequenceResult::FIRST;
			default:
				return SequenceResult::ERROR_RESULT;
			};
		default:
			return SequenceResult::ERROR_RESULT;
		};
	}

	void insert_sequence(SequenceResult SR, Implement::system_relationship_iterator_t& s1, Implement::system_relationship_iterator_t& s2)
	{
		switch (SR)
		{
		case PO::ECSFramework::SequenceResult::FIRST:
			s1->second.before.insert({ s2->first, s2 });
			s2->second.after.insert({ s1->first, s1 });
			break;
		case PO::ECSFramework::SequenceResult::SECOND:
			s2->second.before.insert({ s1->first, s1 });
			s1->second.after.insert({ s2->first, s2 });
			break;
		case PO::ECSFramework::SequenceResult::NOT_CARE:
			s2->second.mutex.insert({ s1->first, s1 });
			s1->second.mutex.insert({ s2->first, s2 });
			break;
		case PO::ECSFramework::SequenceResult::UNDEFINE:
			s2->second.undefine.insert({ s1->first, s1 });
			s1->second.undefine.insert({ s2->first, s2 });
		default:
			throw Error::system_dependence_confuse(s1->first, s2->first, "dependence_confuse");
			break;
		}
	}

	namespace Implement
	{
		bool implicit_after_t::is_include(graph_time_t ia) const noexcept
		{
			for (auto& ite : v)
			{
				if (ite.is_include(ia)) return true;
			}
			return false;
		}

		void implicit_after_t::include(graph_time_t gtt)
		{
			for (auto ite = v.begin(); ite != v.end(); ++ite)
			{
				if (ite->is_include(gtt)) {
					return;
				}
				else if (gtt.is_include(*ite))
				{
					*ite = gtt;
					auto ite_f = ite;
					if (ite_f != v.begin())
					{
						do {
							--ite_f;
							if (ite->is_include(*ite_f))
								*ite_f = {};
							else if (ite->reach == ite_f->finish + 1)
							{
								ite->reach = ite_f->reach;
								*ite_f = {};
							}
							else
								break;
						} while (ite_f != v.begin());
					}
					auto ite_b = ite;
					for (++ite_b; ite_b != v.end(); ++ite_b)
					{
						if (ite->is_include(*ite_b))
							*ite_b = {};
						else if (ite_b->reach == ite->finish + 1)
						{
							ite->finish = ite->finish;
							*ite_b = {};
						}
						else
							break;
					}
					v.erase(std::remove_if(ite_f, v.end(), [](graph_time_t& gt) {return gt == graph_time_t{}; }), v.end());
					return;
				}
				else if (ite->finish + 1 == gtt.reach)
				{
					ite->finish = gtt.finish;
					auto ite_b = ite;
					for (++ite_b; ite_b != v.end(); ++ite_b)
					{
						if (ite->is_include(*ite_b))
							*ite_b = {};
						else if (ite_b->reach == ite->finish + 1)
						{
							ite->finish = ite->finish;
							*ite_b = {};
						}
						else
							break;
					}
					v.erase(std::remove_if(ite, v.end(), [](graph_time_t& gt) {return gt == graph_time_t{}; }), v.end());
					return;
				}
				else if (ite->reach == gtt.finish + 1)
				{
					ite->reach = gtt.reach;
					auto ite_f = ite;
					if (ite_f != v.begin())
					{
						do {
							--ite_f;
							if (ite->is_include(*ite_f))
								*ite_f = {};
							else if (ite->reach == ite_f->finish + 1)
							{
								ite->reach = ite_f->reach;
								*ite_f = {};
							}
							else
								break;
						} while (ite_f != v.begin());
					}
					v.erase(std::remove_if(ite_f, v.end(), [](graph_time_t& gt) {return gt == graph_time_t{}; }), v.end());
					return;
				}
				else if (ite->reach > gtt.finish)
				{
					v.insert(ite, gtt);
					return;
				}
			}
			v.push_back(gtt);
		}

		void implicit_after_t::include(const implicit_after_t& ia)
		{
			for (auto& ite : ia.v)
				include(ite);
		}

		void implicit_after_t::clear()
		{
			v.clear();
		}

		void implicit_after_t::unclude(graph_time_t gtt)
		{
			v.erase(std::remove_if(v.begin(), v.end(), [=](graph_time_t& gtt2) {
				if (gtt.is_include(gtt2)) return true;
				return false;
			}), v.end());
		}

		void system_map::remove_relation(system_relationship_iterator_t ite)
		{
			assert(ite != systems.end());

			for (auto& ite2 : ite->second.mutex)
				ite2.second->second.mutex.erase(ite->first);
			ite->second.mutex.clear();

			for (auto& ite2 : ite->second.after)
				ite2.second->second.before.erase(ite->first);
			ite->second.after.clear();

			for (auto& ite2 : ite->second.before)
				ite2.second->second.after.erase(ite->first);
			ite->second.before.clear();

			for (auto& ite2 : ite->second.undefine)
				ite2.second->second.undefine.erase(ite->first);
			ite->second.undefine.clear();
		}

		bool system_map::reflesh_unavalible_map()
		{
			std::lock_guard<decltype(mutex)> lg(mutex);
			for (auto ite1 = systems.begin(); ite1 != systems.end();)
			{
				auto& ptr = ite1->second;
				if (ptr.ptr && *(ptr.ptr))
					++ite1;
				else {
					system_need_update = true;
					remove_relation(ite1);
					systems.erase(ite1++);
				}
			}
			return !systems.empty();
		}

		void system_map::insert(system_ptr shp)
		{
			assert(shp && *shp);

			auto id1 = shp->id();
			auto info1 = shp->info();
			auto layout1 = shp->layout;

			std::lock_guard<decltype(mutex)> lg(mutex);
			system_need_update = true;


			auto ite = systems.find(id1);
			if (ite != systems.end())
			{
				remove_relation(ite);
				ite->second.ptr = std::move(shp);
			}
			else {
				ite = systems.insert({ id1, system_relationship{ std::move(shp) } }).first;
			}
			auto& ref1 = ite->second;
			// calculate relation ship between each system
			for (auto ite2 = systems.begin(); ite2 != systems.end(); ++ite2)
			{
				if (ite2 == ite) continue;
				auto& ref2 = ite2->second;
				assert(ref2.ptr && *ref2.ptr);
				auto info2 = ref2.ptr->info();
				SystemLayout layout2 = ref2.ptr->layout;
				auto id2 = ref2.ptr->id();
				
				bool write_conflict = is_collided(info1.entity_write, info2.entity_write) 
					|| is_collided(info1.singleton_write, info2.singleton_write) 
					|| is_collided(info1.event_write, info2.event_write);
				bool e1_rw_conflict = is_collided(info1.entity_write, info2.entity_read)
					|| is_collided(info1.singleton_write, info2.singleton_read)
					|| is_collided(info1.event_write, info2.event_read);
				bool e2_rw_conflict = is_collided(info1.entity_read, info2.entity_write)
					|| is_collided(info1.singleton_read, info2.singleton_write)
					|| is_collided(info1.event_read, info2.event_write);
				
				if (write_conflict || e1_rw_conflict || e2_rw_conflict)
				{
					SequenceResult result = (layout1 == layout2 ? SequenceResult::UNDEFINE : (layout1 < layout2 ? SequenceResult::FIRST : SequenceResult::SECOND));
					if (result == SequenceResult::UNDEFINE)
					{
						result = calculate_sequence(ref1.ptr->check_sequence(id2), ref2.ptr->check_sequence(id1));
						if (result == SequenceResult::UNDEFINE || result == SequenceResult::NOT_CARE)
						{
							if(write_conflict && e1_rw_conflict && !e2_rw_conflict)
								result = SequenceResult::FIRST;
							else if (write_conflict && e2_rw_conflict && !e1_rw_conflict)
								result = SequenceResult::SECOND;
							else if (e1_rw_conflict && !e2_rw_conflict)
								result = SequenceResult::FIRST;
							else if (e2_rw_conflict && !e1_rw_conflict)
								result = SequenceResult::SECOND;
						}
					}
					insert_sequence(result, ite, ite2);
				}
			}
		}

		bool system_map::update_waitting_list()
		{
			std::lock_guard<decltype(mutex)> lg(mutex);
			if (system_need_update)
			{

				// clear event_pool
				for (auto& ite : event_pool)
				{
					ite.second.erase(
						std::remove_if(ite.second.begin(), ite.second.end(), [](Tool::intrusive_ptr<Implement::event_pool_interface>& p) {
						return !(p && *p);
					}),
						ite.second.end()
					);
				}

				start_system_temporary.clear();
				//clear all state
				for (auto ite = systems.begin(); ite != systems.end(); ++ite)
				{
					assert(ite->second.ptr && *ite->second.ptr);
					ite->second.state = SystemOperatorState::READY;
					ite->second.simplify_after = ite->second.after;
					ite->second.simplify_before.clear();
					ite->second.simplify_mutex = ite->second.mutex;
					ite->second.implicit_after.clear();
					ite->second.graph_time = graph_time_t{};
					if (ite->second.before.empty())
						start_system_temporary.push_back(ite);
				}
				//dsf and add reach time
				search.clear();
				std::size_t step = 1;
				for (auto ite : start_system_temporary)
				{
					//call buffer
					search.push_back({ ite, (ite)->second.after.begin() });
					ite->second.state = SystemOperatorState::OPERATING;
					ite->second.graph_time.reach = step++;
					while (!search.empty())
					{
						auto& ite2 = *search.rbegin();
						if ((ite2.first)->second.after.end() == ite2.second)
						{
							//find all child node
							(ite2.first)->second.state = SystemOperatorState::FINISH;
							ite2.first->second.graph_time.finish = step++;
							auto& sys_ref = ite2.first->second;
							//combine dependence, and remove repeat dependence
							for (auto ite_s = sys_ref.simplify_after.begin(); ite_s != sys_ref.simplify_after.end();)
							{
								auto& sys_ref1 = ite_s->second->second;
								auto ite_e = ite_s;
								++ite_e;
								bool do_not_include = true;
								while (ite_e != sys_ref.simplify_after.end())
								{
									auto& sys_ref2 = ite_e->second->second;
									if (sys_ref1.graph_time.is_include(sys_ref2.graph_time) || sys_ref1.implicit_after.is_include(sys_ref2.graph_time))
										sys_ref.simplify_after.erase(ite_e++);
									else if (sys_ref2.graph_time.is_include(sys_ref1.graph_time) || sys_ref2.implicit_after.is_include(sys_ref1.graph_time))
									{
										do_not_include = false;
										sys_ref.simplify_after.erase(ite_s++);
										break;
									}
									else
										++ite_e;
								}
								if (do_not_include)
									++ite_s;
							}
							for (auto& ite : sys_ref.after)
							{
								sys_ref.implicit_after.include(ite.second->second.graph_time);
								sys_ref.implicit_after.include(ite.second->second.implicit_after);
								sys_ref.implicit_after.unclude(sys_ref.graph_time);
							}
							search.pop_back();
						}
						else {
							auto ite3 = ite2.second++;
							auto& ref = *ite3;
							if (ite3->second->second.state == SystemOperatorState::OPERATING)
								//circle dependence
								throw Error::system_dependence_circle(ite3->first, "system dependence as a circle");
							else if (ite3->second->second.state == SystemOperatorState::READY)
							{
								ite3->second->second.state = SystemOperatorState::OPERATING;
								ite3->second->second.graph_time.reach = step++;
								search.push_back({ ite3->second, (*ite3).second->second.after.begin() });
								auto size = search.size();
							}
							//reach end, do not handled this
						}
					}
				}
				for (auto ite = systems.begin(); ite != systems.end(); ++ite)
				{
					//repeat
					auto& ref = ite->second;
					ref.state = SystemOperatorState::READY;
					//search undefine pair
					for (auto& ite2 : ref.undefine)
					{
						auto& ref2 = ite2.second->second;
						if (
							!(ref.graph_time.is_include(ref2.graph_time) || ref2.graph_time.is_include(ref.graph_time) ||
								ref.implicit_after.is_include(ref2.graph_time) || ref2.implicit_after.is_include(ref.graph_time))
							)
						{
							throw Error::system_dependence_confuse(ite->first, ite2.first, "system is dependence but did not set");
						}
					}
					//search mutex pair
					for (auto ite2 = ref.simplify_mutex.begin(); ite2 != ref.simplify_mutex.end();)
					{
						auto& ref2 = ite2->second->second;
						if (
							ref.graph_time.is_include(ref2.graph_time) || ref2.graph_time.is_include(ref.graph_time) ||
							ref.implicit_after.is_include(ref2.graph_time) || ref2.implicit_after.is_include(ref.graph_time)
							)
						{
							ref.simplify_mutex.erase(ite2);
							ref2.simplify_mutex.erase(ite->first);
						}
					}
					//finish dependence list
					for (auto& ite2 : ref.simplify_after)
						ite2.second->second.simplify_before.insert({ ite->first, ite });
				}

				start_system_temporary.clear();
				for (auto ite = systems.begin(); ite != systems.end(); ++ite)
					if (ite->second.simplify_after.empty())
						start_system_temporary.push_back(ite);

				system_need_update = false;
			}
			else {
				for (auto& ite : systems)
					//clear state
					ite.second.state = SystemOperatorState::READY;
			}
			for (auto & ite : start_system_temporary)
			{
				waitting_list.insert({ ite->first, ite });
			}
			finished_system = 0;
			return true;
		}

		Implement::system_ptr system_map::pop_one(bool& finish)
		{
			if (finished_system == systems.size())
			{
				finish = true;
			}
			else {
				finish = false;
				if (!waitting_list.empty())
				{
					for (auto ite = waitting_list.begin(); ite != waitting_list.end(); ++ite)
					{
						bool ready = true;
						for (auto ite2 : ite->second->second.simplify_mutex)
						{
							if (ite2.second->second.state == SystemOperatorState::OPERATING)
							{
								ready = false;
								break;
							}
						}
						if (ready)
						{
							for (auto ite2 : ite->second->second.simplify_after)
							{
								if (ite2.second->second.state != SystemOperatorState::FINISH)
								{
									ready = false;
									break;
								}
							}
							if (ready)
							{
								auto ptr = ite->second->second.ptr;
								ite->second->second.state = SystemOperatorState::OPERATING;
								for (auto& ite2 : ite->second->second.simplify_before)
									waitting_list.insert({ ite2.first, ite2.second });
								waitting_list.erase(ite);
								return ptr;
							}
						}
					}
				}
			}
			return {};
		}

		void system_map::finish_operating(std::type_index ti)
		{
			auto ite = systems.find(ti);
			assert(ite != systems.end());
			ite->second.state = SystemOperatorState::FINISH;
			finished_system += 1;
		}

		bool system_map::execute_one_other_thread(context& c, system_update& su, bool& finish)
		{
			Implement::system_ptr ptr;
			if (mutex.try_lock())
			{
				std::unique_lock<decltype(mutex)> u(mutex, std::adopt_lock);
				ptr = pop_one(finish);
			}
			if (ptr)
			{
				assert(*ptr);
				ptr->call(c, su);
				std::lock_guard<decltype(mutex)> ld(mutex);
				finish_operating(ptr->id());
				return true;
			}
			return false;
		}

		void system_map::destory_system(std::type_index id)
		{
			auto ite = systems.find(id);
			if (ite != systems.end())
			{
				remove_relation(ite);
				systems.erase(ite);
				system_need_update = true;
			}
		}

		bool system_map::execute_one(context& c, system_update& su)
		{
			bool finish;
			Implement::system_ptr ptr;
			{
				std::lock_guard<decltype(mutex)> ld(mutex);
				ptr = pop_one(finish);
			}
			if (finish) return false;
			if (ptr)
			{
				assert(*ptr);
				ptr->call(c, su);
				std::lock_guard<decltype(mutex)> ld(mutex);
				finish_operating(ptr->id());
			}
			return true;
		}

		void system_map::insert_event_pool(Tool::intrusive_ptr<event_pool_interface> pool)
		{
			if (pool && *pool)
			{
				auto ite = event_pool.find(pool->id);
				if (ite == event_pool.end())
					ite = event_pool.insert({ pool->id , event_pool_tank{} }).first;
				ite->second.push_back(std::move(pool));
			}
		}

		viewer<const Tool::intrusive_ptr<event_pool_interface>> system_map::get_event_pool(std::type_index id) const noexcept
		{
			auto ite = event_pool.find(id);
			if (ite != event_pool.end())
			{
				return { ite->second.data(), ite->second.size() };
			}
			else {
				return {};
			}
		}
	}

	// context_implement *******************
	bool context_implement::thread_execute()
	{
		Implement::context_temporary ct(*this);
		bool finish;
		if (all_system.execute_one_other_thread(ct, *this, finish))
		{
			load_form_context(ct);
			return false;
		}
		else {
			load_form_context(ct);
			auto fu = addairs.pop_front();
			if (fu)
			{
				fu();
				return false;
			}
			else if (finish)
				return true;
			else
				return false;
		}
	}

	Implement::viewer<const Tool::intrusive_ptr<Implement::event_pool_interface>> context_implement::get_normal_event_pool(std::type_index id) const noexcept
	{
		return all_system.get_event_pool(id);
	}
	Implement::viewer<const Tool::intrusive_ptr<Implement::event_pool_interface>> context_implement::get_temporary_event_pool() const noexcept
	{
		return Implement::viewer<const Tool::intrusive_ptr<Implement::event_pool_interface>>{ temporary_event_pool.data(), temporary_event_pool.size() };
	}

	context_implement::context_implement() : avalible(true) {}

	std::size_t resize_aligna(std::size_t size, std::size_t aligna)
	{
		return (size % aligna == 0) ? size : (size - (size % aligna) + aligna);
	}

	Implement::component_ptr context_implement::allocate_component(std::type_index ti, std::size_t type, std::size_t aligna, void(*deleter)(void*) noexcept)
	{
		assert(type % aligna == 0);
		std::size_t aligna_ref = alignof(Implement::component_ref);
		assert(aligna_ref % aligna == 0 || aligna % aligna_ref == 0);
		if (aligna <= aligna_ref)
		{
			type = resize_aligna(type, aligna_ref);
		}
		else {
			std::size_t mulity = aligna / aligna_ref;
			mulity -= 1;
			type += mulity * aligna_ref;
		}
		void * data = pool.allocate(ti, sizeof(Implement::component_ref) + type, aligna_ref);
		Implement::component_ptr tem = new(data) Implement::component_ref{ ti, deleter };
		tem->data = reinterpret_cast<std::byte*>(data) + sizeof(Implement::component_ref);
		size_t space = sizeof(Implement::component_ref) + type;
		std::align(aligna, aligna, tem->data, space);
		return tem;
	}

	Implement::system_ptr context_implement::allocate_system(std::type_index ti, std::size_t type, std::size_t aligna)
	{
		assert(type % aligna == 0);
		std::size_t aligna_ref = alignof(Implement::system_ref);
		assert(aligna_ref % aligna == 0 || aligna % aligna_ref == 0);
		assert(aligna_ref == alignof(void*));
		if (aligna <= aligna_ref)
		{
			type = resize_aligna(type, aligna_ref);
		}
		else {
			std::size_t mulity = aligna / aligna_ref;
			mulity -= 1;
			type += mulity * aligna_ref;
		}
		std::byte* data = new std::byte[sizeof(Implement::system_ref) + type];
		Implement::system_ptr tem = new(data) Implement::system_ref{};
		void* data_start = data + sizeof(Implement::system_ref);
		std::size_t space = sizeof(Implement::system_ref) + type;
		std::align(aligna, aligna, data_start, space);
		tem->ptr = reinterpret_cast<Implement::system_interface*>(data_start);
		return tem;
	}

	Implement::singleton_component_ptr context_implement::allocate_singleton_component(std::type_index ti, std::size_t type, std::size_t aligna, void(*deleter)(void*) noexcept)
	{
		assert(type % aligna == 0);
		std::size_t aligna_ref = alignof(Implement::singleton_component_ref);
		assert(aligna_ref % aligna == 0 || aligna % aligna_ref == 0);
		std::size_t da = alignof(void*);
		assert(aligna_ref == da);
		if (aligna <= aligna_ref)
		{
			type = resize_aligna(type, aligna_ref);
		}
		else {
			std::size_t mulity = aligna / aligna_ref;
			mulity -= 1;
			type += mulity * aligna_ref;
		}
		std::byte* data = new std::byte[sizeof(Implement::singleton_component_ref) + type];
		Implement::singleton_component_ptr tem = new(data) Implement::singleton_component_ref{ti, deleter};
		void* data_start = data + sizeof(Implement::singleton_component_ref);
		std::size_t space = type;
		std::align(aligna, aligna, data_start, space);
		tem->data = data_start;
		return tem;
	}

	template<typename T> void insert_vertex(T& t, Tool::scope_lock<T>& it)
	{
		if (!t.empty())
		{
			it.lock([&](T& iit) {
				iit.insert(iit.end(),
					std::move_iterator<typename T::iterator>(t.begin()),
					std::move_iterator<typename T::iterator>(t.end())
				);
			});
			t.clear();
		}
	}


	void context_implement::load_form_context(Implement::context_temporary& c)
	{
		insert_vertex(c.context_event, context_event);
	}

	entity context_implement::create_entity()
	{
		assert(alignof(Implement::entity_ref) == alignof(Implement::entity_implement));
		void* allocated_data = pool.allocate(typeid(Implement::entity_ref), sizeof(Implement::entity_ref) + sizeof(Implement::entity_implement), alignof(Implement::entity_ref));
		std::byte* da = reinterpret_cast<std::byte*>(allocated_data);
		new (da + sizeof(Implement::entity_ref)) Implement::entity_implement{};
		Implement::entity_ptr ptr = new(da) Implement::entity_ref{};
		return entity{ ptr };
	}

	void context_implement::insert_filter(Tool::intrusive_ptr<Implement::pre_filter_storage_interface> ptr)
	{
		assert(ptr && *ptr);
		all_component.insert(std::move(ptr));
	}

	void context_implement::insert_event_pool(Tool::intrusive_ptr<Implement::event_pool_interface> ptr)
	{
		assert(ptr && *ptr);
		all_system.insert_event_pool(std::move(ptr));
	}

	/*
	void context_implement::set_filter(std::shared_ptr<Implement::pre_filter_storage_interface> wp)
	{
		if (wp)
			all_component.insert(std::move(wp));
	}
	*/

	Implement::singleton_component_ptr context_implement::get_singleton_component(std::type_index ti) const noexcept
	{
		auto ite = singleton_component.find(ti);
		if (ite != singleton_component.end())
			return ite->second;
		return {};
	}

	void context_implement::loop()
	{
		avalible = true;
		Platform::thread_pool threads;
		std::size_t process_count = Platform::platform_info_instance().cpu_count();
		process_count -= 1;
		process_count = process_count * 2;

		if (thread_reserved > process_count)
			process_count = 0;
		else
			process_count = process_count - thread_reserved;

		for (std::size_t i = 0; i < process_count + 1; ++i)
			threads.create_thread([this]() {return thread_execute(); });

		struct context_event_handler
		{
			context_implement* context;
			std::set<std::type_index>& destory_component;
			std::vector<Implement::system_ptr>& temporary_system_buffer;
			void operator()(Implement::component_holder& ref)
			{
				assert(ref.entity && * ref.entity);
				if (ref.entity->insert(ref.componenet))
					destory_component.insert(ref.componenet->id());
				context->all_component.insert(std::move(ref));
			}
			void operator()(Implement::singleton_component_ptr& ref) {
				auto id = ref->id();
				auto ite2 = context->singleton_component.find(id);
				if (ite2 != context->singleton_component.end())
				{
					ite2->second->destory();
					ite2->second = std::move(ref);
				}
				else
					context->singleton_component.insert({ id, std::move(ref) });
			}
			void operator()(Implement::system_ptr& ref) {
				ref->init(*context);
				context->all_system.insert(std::move(ref));
			}
			void operator()(Implement::component_destory& ref) {
				if (ref.entity->destory_component(ref.id))
					destory_component.insert(ref.id);
			}
			void operator()(Implement::entity_ptr& ref) {
				if (ref && *ref)
					ref->destory_all_component(destory_component);
			}
			void operator()(Implement::singleton_component_destory& ref) {
				std::type_index id = ref.id;
				auto ite2 = context->singleton_component.find(id);
				if (ite2 != context->singleton_component.end())
				{
					if (ite2->second && *ite2->second)
						ite2->second->destory();
					context->singleton_component.erase(ite2);
				}
			}
			void operator()(Implement::system_destory& cp) {
				std::type_index id = cp.id;
				context->all_system.destory_system(id);
			}
			void operator()(Implement::temporary_system& cp) {
				temporary_system_buffer.push_back(std::move(cp.ptr));
			}
		};
		std::set<std::type_index> destory_component;
		std::vector<Implement::system_ptr> temporary_system;
		bool ready = false;
		auto last_tick = std::chrono::system_clock::now();
		Implement::context_temporary ct(*this);
		while (avalible)
		{
			context_event.lock([&, this](decltype(context_event)::type& t) {
				for (auto& ite : t)
					std::visit(context_event_handler{this, destory_component, temporary_system}, ite);
				t.clear();
			});

			for (auto& ite : destory_component)
				all_component.reflesh(ite);
			destory_component.clear();

			if (!temporary_system.empty())
			{
				struct temporary_initializer : public Implement::system_initializer
				{
					Implement::component_map& cm;
					std::vector<Tool::intrusive_ptr<Implement::event_pool_interface>>& pool;
					virtual void insert_filter(Tool::intrusive_ptr<Implement::pre_filter_storage_interface> ptr) override
					{
						cm.update_temporary(std::move(ptr));
					}
					virtual void insert_event_pool(Tool::intrusive_ptr<Implement::event_pool_interface> ptr) override
					{
						pool.push_back(std::move(ptr));
					}
					temporary_initializer(Implement::component_map& r, std::vector<Tool::intrusive_ptr<Implement::event_pool_interface>>& p)
						: cm(r), pool(p) {}
				};
				temporary_initializer temporary{ all_component, temporary_event_pool };
				for (auto& ite : temporary_system)
				{
					ite->init(temporary);
					ite->call(ct, *this);
				}
			}

			avalible = all_system.reflesh_unavalible_map();
			if (avalible)
			{
				all_system.update_waitting_list();

				if (ready)
				{
					auto current_tick = std::chrono::system_clock::now();
					auto dura = std::chrono::duration_cast<std::chrono::milliseconds>(current_tick - last_tick);
#ifdef _DEBUG
					std::cout << "frame count (ms):" << dura.count() << std::endl;
#endif // _DEBUG
					if(dura < duration_ms)
						std::this_thread::sleep_for(duration_ms - dura);
					last_tick = std::chrono::system_clock::now();
				}
				else
					ready = true;

				threads.notity_all();
				while (all_system.execute_one(ct, *this))
				{
					std::this_thread::yield();
				}
				load_form_context(ct);
				temporary_system.clear();
				temporary_event_pool.clear();
			}
		}
	}
}