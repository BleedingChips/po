#include "context.h"
#include <atomic>
#include <limits>
#undef max
namespace PO::ECSFramework
{
	enum class SequenceResult
	{
		UNDEFINE,
		ERROR_RESULT,
		FIRST,
		SECOND,
		NOT_CARE
	};


	SequenceResult calculate_sequence(SystemExecutionSequence S1, SystemExecutionSequence S2)
	{
		switch (S1)
		{
		case SystemExecutionSequence::UNDEFINE:
			switch (S2)
			{
			case PO::ECSFramework::SystemExecutionSequence::UNDEFINE:
				return SequenceResult::UNDEFINE;
			case PO::ECSFramework::SystemExecutionSequence::BEFORE:
				return SequenceResult::SECOND;
			case PO::ECSFramework::SystemExecutionSequence::AFTER:
				return SequenceResult::FIRST;
			case PO::ECSFramework::SystemExecutionSequence::NOT_CARE:
				return SequenceResult::NOT_CARE;
			default:
				return SequenceResult::ERROR_RESULT;
			}
		case SystemExecutionSequence::BEFORE:
			switch (S2)
			{
			case PO::ECSFramework::SystemExecutionSequence::UNDEFINE:
			case PO::ECSFramework::SystemExecutionSequence::NOT_CARE:
			case PO::ECSFramework::SystemExecutionSequence::AFTER:
				return SequenceResult::FIRST;
			case PO::ECSFramework::SystemExecutionSequence::BEFORE:
			default:
				return SequenceResult::ERROR_RESULT;
			};
		case SystemExecutionSequence::AFTER:
			switch (S2)
			{
			case PO::ECSFramework::SystemExecutionSequence::UNDEFINE:
			case PO::ECSFramework::SystemExecutionSequence::NOT_CARE:
			case PO::ECSFramework::SystemExecutionSequence::BEFORE:
				return SequenceResult::SECOND;
			case PO::ECSFramework::SystemExecutionSequence::AFTER:
			default:
				return SequenceResult::ERROR_RESULT;
			};
		case SystemExecutionSequence::NOT_CARE:
			switch (S2)
			{
			case PO::ECSFramework::SystemExecutionSequence::UNDEFINE:
			case PO::ECSFramework::SystemExecutionSequence::NOT_CARE:
				return SequenceResult::NOT_CARE;
			case PO::ECSFramework::SystemExecutionSequence::BEFORE:
				return SequenceResult::SECOND;
			case PO::ECSFramework::SystemExecutionSequence::AFTER:
				return SequenceResult::FIRST;
			default:
				return SequenceResult::ERROR_RESULT;
			};
		default:
			return SequenceResult::ERROR_RESULT;
		};
	}

	bool context_implement::thread_execute()
	{
		Implement::context_temporary ct(*this);
		bool finish;
		if (all_system.execute_one_other_thread(ct, finish))
		{
			return false;
		}
		else {
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

	context_implement::context_implement() : avalible(true) {
		singleton_entity = create_entity();
		size_t process_count = Platform::platform_info_instance().cpu_count();
		process_count -= 1;
		process_count = process_count * 2 + 1;
		for (size_t i = 0; i < process_count; ++i)
		{
			threads.create_thread([this]() {return thread_execute(); });
		}
	}

	Implement::component_holder_ptr context_implement::allocate_component(std::type_index ti, size_t type, size_t aligna, void*& component_out)
	{
		void* data = pool.allocate(ti, type + sizeof(Implement::component_holder) + 
			(aligna == alignof(Implement::component_holder) ? 0 : aligna - 1),
			alignof(Implement::component_holder));
		Implement::component_holder_ptr tem = new(data) Implement::component_holder{};
		void* component_start = static_cast<std::byte*>(data) + sizeof(Implement::component_holder);
		size_t space;
		std::align(aligna, type, component_start, space);
		component_out = component_start;
		return tem;
	}

	Implement::system_holder_ptr context_implement::allocate_system(std::type_index ti, size_t type, size_t aligna, void*& system_out)
	{
		void* data = new std::byte[sizeof(Implement::system_holder) + type + (aligna == alignof(nullptr_t) ? 0 : aligna -1)];
		Implement::system_holder_ptr tem = new(data) Implement::system_holder{};
		void* system_start = static_cast<std::byte*>(data) + sizeof(Implement::system_holder);
		size_t space;
		std::align(aligna, type, system_start, space);
		system_out = system_start;
		return tem;
	}

	void context_implement::load_form_context(Implement::context_temporary& c)
	{
		if (!c.temporary_component_context_holder.empty())
		{
			temporary_component_holder.lock([&](decltype(temporary_component_holder)::type& t) {
				t.insert(t.end(),
					std::move_iterator<typename decltype(temporary_component_holder)::type::iterator>(c.temporary_component_context_holder.begin()),
					std::move_iterator<typename decltype(temporary_component_holder)::type::iterator>(c.temporary_component_context_holder.end())
				);
			});
			c.temporary_component_context_holder.clear();
		}
		if (!c.temporary_system_context_holder.empty())
		{
			temporary_system_holder.lock([&](decltype(temporary_system_holder)::type& t) {
				t.insert(t.end(),
					std::move_iterator<typename decltype(temporary_system_holder)::type::iterator>(c.temporary_system_context_holder.begin()),
					std::move_iterator<typename decltype(temporary_system_holder)::type::iterator>(c.temporary_system_context_holder.end())
				);
			});
			c.temporary_system_context_holder.clear();
		}
	}

	template<typename T>
	void for_each_component(Implement::vision vi, std::pair<Implement::vision, std::vector<Implement::context_component_holder>>& target_com, T&& t)
	{
		if (target_com.first.different(vi))
		{
			auto& ref = target_com.second;
			ref.erase(
				std::remove_if(ref.begin(), ref.end(), [&](Implement::context_component_holder& cch)->bool {
				if (cch.e_ptr && cch.e_ptr->is_avalible() && cch.h_ptr && cch.h_ptr->is_avalible())
				{
					std::forward<T>(t)(cch.e_ptr);
					return false;
				}
				return true;
			}),
				ref.end()
				);
		}
		else {
			for (auto& ite : target_com.second)
			{
				std::forward<T>(t)(ite.e_ptr);
			}
		}
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
		void component_map::insert(Implement::vision v, context_component_holder cch)
		{
			assert(cch.e_ptr && cch.h_ptr && cch.h_ptr->is_avalible());
			if (cch.e_ptr->is_avalible())
			{
				auto id = cch.h_ptr->id();
				auto ite = map_holder.find(id);
				if (ite == map_holder.end())
				{
					auto result = map_holder.insert({ id, typename decltype(map_holder)::value_type::second_type{} });
					result.first->second.first.different(v);
					result.first->second.second.emplace_back(std::move(cch));
				}
				else
					ite->second.second.emplace_back(std::move(cch));
			}
		}

		decltype(component_map::map_holder)::iterator component_map::find_min_type_index(type_index_view tiv) noexcept
		{
			size_t view_type = tiv.size();
			if (view_type == 0) return map_holder.end();
			decltype(map_holder)::iterator min_result = map_holder.end();
			size_t min_index_size = std::numeric_limits<size_t>::max();
			for (size_t i = 0; i < view_type; ++i)
			{
				auto ite = map_holder.find(tiv[i]);
				if (ite != map_holder.end())
				{
					size_t component_size = ite->second.second.size();
					if (component_size < min_index_size)
					{
						min_index_size = component_size;
						min_result = ite;
					}
				}else
					return map_holder.end();
			}
			return min_result;
		}

		void component_map::update(Implement::vision v, Implement::system_holder_ptr shp)
		{
			assert(shp && shp->is_avalible());
			auto info = shp->info();
			if (info.trigger == typeid(void))
			{
				auto ite = find_min_type_index(info.first);
				if (ite != map_holder.end())
					for_each_component(v, ite->second, [&](entity_implement_ptr& eip) {shp->insert_first(eip); });
				auto ite2 = find_min_type_index(info.second);
				if (ite2 != map_holder.end())
					for_each_component(v, ite2->second, [&](entity_implement_ptr& eip) {shp->insert_second(eip); });
			}
		}

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
				if (ptr.ptr && ptr.ptr->is_avalible())
					++ite1;
				else {
					system_need_update = true;
					remove_relation(ite1);
					systems.erase(ite1++);
				}
			}
			return systems.empty();
		}

		void system_map::insert(Implement::system_holder_ptr shp)
		{
			assert(shp && shp->is_avalible());

			auto id1 = shp->id();
			auto info1 = shp->rw_info();
			auto layout1 = shp->layout();

			std::lock_guard<decltype(mutex)> lg(mutex);
			system_need_update = true;
			
			
			auto ite = systems.find(id1);
			if (ite != systems.end())
			{
				remove_relation(ite);
				ite->second.ptr = std::move(shp);
			}
			else {
				ite = systems.insert({ id1, system_relationship{std::move(shp)} }).first;
			}
			auto& ref1 = ite->second;
			// 计算各system之间的关系
			for (auto ite2 = systems.begin(); ite2 != systems.end(); ++ite2)
			{
				if (ite2 == ite) continue;
				auto& ref2 = ite2->second;
				assert(ref2.ptr && ref2.ptr->is_avalible());
				auto info2 = ref2.ptr->rw_info();
				SystemLayout layout2 = ref2.ptr->layout();
				auto id2 = ref2.ptr->id();

				bool write_conflict = info1.entity_write.is_collided(info2.entity_write) || info1.singleton_write.is_collided(info2.singleton_write);
				bool e1_rw_conflict = info1.entity_write.is_collided(info2.entity_read) || info1.singleton_write.is_collided(info2.singleton_read);
				bool e2_rw_conflict = info2.entity_write.is_collided(info1.entity_read) || info2.singleton_write.is_collided(info1.singleton_read);
				bool e1_trigger_conflict = info2.singleton_write.have(info1.trigger);
				bool e2_trigger_conflict = info1.singleton_write.have(info2.trigger);
				// 0 : 相等， 1 : 1 在 2 之前 2 : 2 在1 之前。
				SequenceResult layout_result = (layout1 == layout2 ? SequenceResult::UNDEFINE : (layout1 < layout2 ? SequenceResult::FIRST : SequenceResult::SECOND));
				if (write_conflict || e1_rw_conflict || e2_rw_conflict || e1_trigger_conflict || e2_trigger_conflict)
				{
					SequenceResult result = calculate_sequence(ref1.ptr->check_sequence(id2), ref2.ptr->check_sequence(id1));
					if (result == SequenceResult::UNDEFINE || result == SequenceResult::NOT_CARE)
					{
						if (e1_trigger_conflict && !e2_trigger_conflict)
							insert_sequence(SequenceResult::SECOND, ite, ite2);
						else if (e2_trigger_conflict && !e1_trigger_conflict)
							insert_sequence(SequenceResult::FIRST, ite, ite2);
						else if (e1_rw_conflict && !e2_rw_conflict)
							insert_sequence(SequenceResult::FIRST, ite, ite2);
						else if (e2_rw_conflict && !e1_rw_conflict)
							insert_sequence(SequenceResult::SECOND, ite, ite2);
						else
							insert_sequence(result, ite, ite2);
					}
					else {
						insert_sequence(result, ite, ite2);
					}
				}
			}
		}

		void system_map::insert(Implement::context_component_holder cch)
		{
			assert(!cch.e_ptr || (cch.e_ptr && cch.e_ptr->is_avalible()));
			auto id = cch.h_ptr->id();
			std::lock_guard<decltype(mutex)> lg(mutex);
			for (auto& ite : systems)
			{
				assert(ite.second.ptr && ite.second.ptr->is_avalible());
				auto info = ite.second.ptr->info();
				if (info.trigger != typeid(void))
				{
					if (info.first.have(id))
						ite.second.ptr->insert_first(cch.e_ptr);
					if (info.second.have(id))
						ite.second.ptr->insert_second(cch.e_ptr);
				}
			}
		}

		void system_map::insert_singleton(Implement::context_component_holder cch, Implement::entity_implement_ptr e)
		{
			assert(cch.h_ptr && cch.h_ptr->is_avalible());
			assert(!cch.e_ptr);
			assert(e && e->is_avalible());
			auto id = cch.h_ptr->id();
			std::lock_guard<decltype(mutex)> lg(mutex);
			for (auto& ite : systems)
			{
				assert(ite.second.ptr && ite.second.ptr->is_avalible());
				auto info = ite.second.ptr->info();
				if (info.trigger == id)
					ite.second.ptr->insert_trigger(cch.h_ptr);
				if (info.singleton.have(id))
					ite.second.ptr->insert_singleton(e);
			}
		}

		bool system_map::update_waitting_list()
		{
			std::lock_guard<decltype(mutex)> lg(mutex);
			if (system_need_update)
			{
				start_system_temporary.clear();
				//先清空
				for (auto ite = systems.begin(); ite != systems.end(); ++ite)
				{
					assert(ite->second.ptr && ite->second.ptr->is_avalible());
					ite->second.state = SystemOperatorState::READY;
					ite->second.simplify_after = ite->second.after;
					ite->second.simplify_before.clear();
					ite->second.simplify_mutex = ite->second.mutex;
					ite->second.implicit_after.clear();
					ite->second.graph_time = graph_time_t{};
					if (ite->second.before.empty())
						start_system_temporary.push_back(ite);
				}
				//深度优先，加上时间戳
				search.clear();
				size_t step = 1;
				for (auto ite : start_system_temporary)
				{
					//入栈
					search.push_back({ ite, (ite)->second.after.begin() });
					ite->second.state = SystemOperatorState::OPERATING;
					ite->second.graph_time.reach = step++;
					while (!search.empty())
					{
						auto& ite2 = *search.rbegin();
						if ((ite2.first)->second.after.end() == ite2.second)
						{
							//查找所有子节点
							(ite2.first)->second.state = SystemOperatorState::FINISH;
							ite2.first->second.graph_time.finish = step++;
							auto& sys_ref = ite2.first->second;
							//合并依赖，去除重复依赖
							for (auto ite_s = sys_ref.simplify_after.begin(); ite_s != sys_ref.simplify_after.end();)
							{
								auto& sys_ref1 = ite_s->second->second;
								auto ite_e = ite_s;
								++ite_e;
								bool do_not_include = true;
								while (ite_e != sys_ref.simplify_after.end())
								{
									auto& sys_ref2 = ite_e->second->second;
									if (sys_ref1.graph_time.is_include(sys_ref2.graph_time)|| sys_ref1.implicit_after.is_include(sys_ref2.graph_time))
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
							if(ite3->second->second.state == SystemOperatorState::OPERATING)
								//有环
								throw Error::system_dependence_circle(ite3->first, "system dependence as a circle");
							else if (ite3->second->second.state == SystemOperatorState::READY)
							{
								ite3->second->second.state = SystemOperatorState::OPERATING;
								ite3->second->second.graph_time.reach = step++;
								search.push_back({ ite3->second, (*ite3).second->second.after.begin() });
								auto size = search.size();
							}
							//遇上终止点，不管了。
						}
					}
				}
				for (auto ite = systems.begin(); ite != systems.end(); ++ite)
				{
					//重置
					auto& ref = ite->second;
					ref.state = SystemOperatorState::READY;
					//查找未定义对
					for (auto& ite2 : ref.undefine)
					{
						auto& ref2 = ite2.second->second;
						if (
							!(ref.graph_time.is_include(ref2.graph_time) || ref2.graph_time.is_include(ref.graph_time) ||
							ref.implicit_after.is_include(ref2.graph_time) || ref2.implicit_after.is_include(ref.graph_time))
							)
						{
							throw Error::system_dependence_confuse(ite->first, ite2.first, "system is dependence buy did not set");
						}
					}
					//查找互斥对
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
					//依赖链补全。
					for (auto& ite2 : ref.simplify_after)
						ite2.second->second.simplify_before.insert({ ite->first, ite });
				}

				start_system_temporary.clear();
				for(auto ite = systems.begin(); ite != systems.end(); ++ite)
					if (ite->second.simplify_after.empty())
						start_system_temporary.push_back(ite);

				system_need_update = false;
			}
			else {
				for (auto& ite : systems)
					//标志位重置
					ite.second.state = SystemOperatorState::READY;
			}
			for (auto & ite : start_system_temporary)
			{
				waitting_list.insert({ ite->first, ite });
			}
			finished_system = 0;
			return true;
		}

		Implement::system_holder_ptr system_map::pop_one(bool& finish)
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

		bool system_map::execute_one_other_thread(context& c, bool& finish)
		{
			Implement::system_holder_ptr ptr;
			if (mutex.try_lock())
			{
				std::unique_lock<decltype(mutex)> u(mutex, std::adopt_lock);
				ptr = pop_one(finish);
			}
			if (ptr)
			{
				assert(ptr->is_avalible());
				ptr->call(c);
				std::lock_guard<decltype(mutex)> ld(mutex);
				finish_operating(ptr->id());
				return true;
			}
			return false;
		}


		bool system_map::execute_one(context& c)
		{
			bool finish;
			Implement::system_holder_ptr ptr;
			{
				std::lock_guard<decltype(mutex)> ld(mutex);
				ptr = pop_one(finish);
			}
			if (finish) return false;
			if (ptr)
			{
				assert(ptr->is_avalible());
				ptr->call(c);
				std::lock_guard<decltype(mutex)> ld(mutex);
				finish_operating(ptr->id());
			}
			return true;
		}

	}

	void context_implement::loop()
	{
		while (avalible)
		{
			all_system.reflesh_unavalible_map();

			temporary_component_holder.lock([this](decltype(temporary_component_holder)::type& t) {
				std::swap(t, tem_component_buffer);
			});
			if (!tem_component_buffer.empty())
			{
				for (auto& ite : tem_component_buffer)
				{
					if (ite.h_ptr)
					{
						if (ite.e_ptr)
						{
							if (ite.e_ptr->is_avalible())
							{
								ite.e_ptr->insert(ite.h_ptr);
								all_system.insert(ite);
								all_component.insert(reflesh_vision, ite);
							}
						}
						else
						{
							singleton_entity.ptr->insert(ite.h_ptr);
							all_system.insert_singleton(ite, singleton_entity.ptr);
						}
					}
				}
				tem_component_buffer.clear();
			}

			temporary_system_holder.lock([this](decltype(temporary_system_holder)::type& t) {
				std::swap(t, temporary_system_holder_buffer);
			});
			if (!temporary_system_holder_buffer.empty())
			{
				for (auto& ite : temporary_system_holder_buffer)
				{
					if (ite && ite->is_avalible())
					{
						all_component.update(reflesh_vision, ite);
						auto info = ite->info();
						if (info.trigger != typeid(void))
						{
							auto ite2 = singleton_entity.ptr->get_component(info.trigger);
							ite->insert_trigger(ite2);
						}
						ite->insert_singleton(singleton_entity.ptr);
						all_system.insert(std::move(ite));
					}
				}
				temporary_system_holder_buffer.clear();
			}

			all_system.update_waitting_list();

			reflesh_vision.update();
			threads.notity_all();
			Implement::context_temporary ct(*this);
			while (all_system.execute_one(ct))
			{
				std::this_thread::yield();
				//std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			load_form_context(ct);
			if (duration_ms != std::chrono::milliseconds{ 0 })
				std::this_thread::sleep_for(duration_ms);
		}
	}
}