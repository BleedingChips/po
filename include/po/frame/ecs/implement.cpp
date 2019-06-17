#include "implement.h"
#include "..//..//platform/platform.h"
namespace PO::ECS
{

	void ContextImplement::loop()
	{
		size_t platform_thread_count = Platform::platform_info::instance().cpu_count() * 2 + 2;
		size_t reserved = (m_thread_reserved > platform_thread_count) ? 0 : (platform_thread_count - m_thread_reserved);
		std::vector<std::thread> mulity_thread(reserved);
		m_available = true;
		for (auto& ite : mulity_thread)
			ite = std::thread(&append_execute_function, this);
		auto last_tick = std::chrono::system_clock::now();
		auto target_duration = m_target_duration;
		m_last_duration = target_duration;
		try {
			while (m_available)
			{
				component_pool.update();
				gobal_component_pool.update();
				event_pool.update();
				if (system_pool.update())
				{
					while (system_pool.asynchro_apply_system(this) != Implement::SystemPool::ApplyResult::AllDone)
						std::this_thread::yield();
					system_pool.synchro_apply_template_system(this);
					auto current_tick = std::chrono::system_clock::now();
					auto dura = std::chrono::duration_cast<std::chrono::milliseconds>(current_tick - last_tick);
					if (dura < target_duration)
					{
						std::this_thread::sleep_for(target_duration - dura);
						current_tick = std::chrono::system_clock::now();
					}
					m_last_duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_tick - last_tick);
					last_tick = current_tick;
				}
				else
					m_available = false;
			}
			system_pool.clean_all();
			event_pool.clean_all();
			gobal_component_pool.clean_all();
			component_pool.clean_all();
			for (auto& ite : mulity_thread)
				ite.join();
		}
		catch (...)
		{
			m_available = false;
			for (auto& ite : mulity_thread)
				ite.join();
			system_pool.clean_all();
			event_pool.clean_all();
			gobal_component_pool.clean_all();
			component_pool.clean_all();
			throw;
		}
		
	}

	ContextImplement::ContextImplement() noexcept : m_available(false), m_thread_reserved(0), m_target_duration(16), m_last_duration(duration_ms{ 0 }),
		allocator(20), component_pool(allocator), gobal_component_pool(), event_pool(allocator), system_pool()
	{

	}

	void ContextImplement::exit() noexcept
	{
		m_available = false;
	}

	void ContextImplement::append_execute_function(ContextImplement* con) noexcept
	{
		while (con->m_available)
		{
			while (true)
			{
				PO::ECS::Implement::SystemPool::ApplyResult result = con->system_pool.asynchro_apply_system(con, false);
				if (result != Implement::SystemPool::ApplyResult::Applied)
					break;
				else
					std::this_thread::yield();
			}
			std::this_thread::sleep_for(ECS::duration_ms{1});
		}
	}

	ContextImplement::operator Implement::ComponentPoolInterface* () { return &component_pool; }
	ContextImplement::operator Implement::GobalComponentPoolInterface* () { return &gobal_component_pool; }
	ContextImplement::operator Implement::EventPoolInterface* () { return &event_pool; }
	ContextImplement::operator Implement::SystemPoolInterface* () { return &system_pool; }
	Implement::EntityInterfacePtr ContextImplement::create_entity_imp() { return Implement::EntityImp::create_one(); }

	float ContextImplement::duration_s() const noexcept { 
		duration_ms tem = m_last_duration;
		return tem.count() / 1000.0f;
	}

}