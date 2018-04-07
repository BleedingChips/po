#pragma once
#include "context.h"
namespace PO::ECSFramework
{
	namespace Error
	{
		struct context_logic_error : std::logic_error
		{
			using std::logic_error::logic_error;
		};

		struct system_dependence_circle : context_logic_error
		{
			std::type_index start_link;
			system_dependence_circle(std::type_index ti, const char* string) : start_link(ti), context_logic_error(string) {}
		};

		struct system_dependence_confuse : context_logic_error
		{
			std::type_index system1;
			std::type_index system2;
			system_dependence_confuse(std::type_index ti, std::type_index t2, char* string) : system1(ti), system2(t2), context_logic_error(string) {}
		};
	}

	// component_map *************************************
	namespace Implement
	{
		struct component_holder
		{
			component_ptr componenet;
			entity_ptr entity;
		};

		struct component_map
		{
			struct element
			{
				std::vector<component_holder> ptr;
				std::vector<std::weak_ptr<filter_storage_interface>> accosiate_filter;
			};

			std::unordered_map<std::type_index, element> all_component_map;
			void insert(component_holder c);
			void reflesh(std::type_index ti);
			void insert(std::shared_ptr<filter_storage_interface> filter);
		};
	}

	// component_temporary *****************************
	namespace Implement
	{
		struct context_temporary : public context
		{
			std::vector<component_holder> init_component_list;
			std::vector<system_ptr> init_system_list;
			std::vector<std::pair<entity_ptr, std::type_index>> destory_component_list;
			std::vector<entity_ptr> destory_entity_list;
			std::vector<std::type_index> destory_singleton_component_list;
			std::vector<std::type_index> destory_system_list;
			using context::context;

			virtual void insert(component_ptr, entity_ptr) override;
			virtual void insert(system_ptr) override;
			virtual void destory(entity_ptr) override;
			virtual void destory_component(entity_ptr, std::type_index) override;
			virtual void destory_singleton_component(std::type_index) override;
			virtual void destory_system(std::type_index) override;
		};
	}

	// system map **********************************
	namespace Implement
	{

		struct system_relationship;

		using system_relationship_map_t = std::unordered_map<std::type_index, system_relationship>;
		using system_relationship_iterator_t = typename system_relationship_map_t::iterator;

		enum class SystemOperatorState
		{
			READY,
			OPERATING,
			FINISH
		};

		struct graph_time_t
		{
			size_t reach = 0;
			size_t finish = 0;
			operator bool() const noexcept { return reach != 0 && finish != 0; }
			bool is_left(graph_time_t gt) const noexcept {
				return finish < gt.reach;
			}
			bool is_right(graph_time_t gt) const noexcept {
				return reach > gt.finish;
			}
			int is_include(graph_time_t gt) const noexcept {
				return reach <= gt.reach && finish >= gt.finish;
			}
		};

		class implicit_after_t
		{
			std::vector<graph_time_t> v;
			friend std::ostream& operator<<(std::ostream& s, const implicit_after_t& gt);
		public:
			bool is_include(graph_time_t ia) const noexcept;
			void include(graph_time_t gtt);
			void include(const implicit_after_t& ia);
			void unclude(graph_time_t gtt);
			void clear();
		};

		struct system_relationship
		{
			system_ptr ptr;
			SystemOperatorState state = SystemOperatorState::READY;

			std::map<std::type_index, system_relationship_iterator_t> simplify_before;
			std::map<std::type_index, system_relationship_iterator_t> simplify_after;
			std::map<std::type_index, system_relationship_iterator_t> simplify_mutex;
			graph_time_t graph_time;
			implicit_after_t implicit_after;

			std::map<std::type_index, system_relationship_iterator_t> mutex;
			std::map<std::type_index, system_relationship_iterator_t> before;
			std::map<std::type_index, system_relationship_iterator_t> after;
			std::map<std::type_index, system_relationship_iterator_t> undefine;
		};

		class system_map
		{
			std::mutex mutex;
			bool system_need_update = false;
			system_relationship_map_t systems;
			size_t finished_system = 0;

			std::vector<std::pair<system_relationship_iterator_t, typename std::map<std::type_index, system_relationship_iterator_t>::iterator>> search;

			std::vector<system_relationship_iterator_t> start_system_temporary;
			std::map<std::type_index, system_relationship_iterator_t> waitting_list;

			void remove_relation(system_relationship_iterator_t ite);
			Implement::system_ptr pop_one(bool& finish);
			void finish_operating(std::type_index ti);
		public:
			bool reflesh_unavalible_map();
			bool update_waitting_list();
			void insert(system_ptr);
			void destory_system(std::type_index id);
			bool execute_one(context& c);
			bool execute_one_other_thread(context& c, bool& finish);
		};

		

	}

	// component *******************
	class context_implement : public Implement::context_interface
	{
		object_pool pool;
		//Platform::thread_pool threads;
		Platform::asynchronous_affairs addairs;
		std::atomic_bool avalible;
		std::chrono::milliseconds duration_ms;

		Tool::scope_lock<std::vector<Implement::component_holder>> init_component_list;
		Tool::scope_lock<std::vector<Implement::system_ptr>> init_system_list;
		Tool::scope_lock<std::vector<std::pair<Implement::entity_ptr, std::type_index>>> destory_component_list;
		Tool::scope_lock<std::vector<Implement::entity_ptr>> destory_entity_list;
		Tool::scope_lock<std::vector<std::type_index>> destory_singleton_component_list;
		Tool::scope_lock<std::vector<std::type_index>> destory_system_list;

		Implement::component_map all_component;
		Implement::system_map all_system;
		

		std::mutex waitting_list_mutex;

		std::map<std::type_index, Implement::component_ptr> singleton_component;

		virtual Implement::component_ptr allocate_component(std::type_index, size_t type, size_t aligna, void(*deleter)(void*) noexcept) override;
		virtual Implement::system_ptr allocate_system(std::type_index, size_t type, size_t aligna) override;

		virtual void set_filter(std::shared_ptr<Implement::filter_storage_interface>) override;
		virtual Implement::component_ptr get_singleton_component(std::type_index) noexcept override;

		bool thread_execute();
		size_t thread_reserved = 0;

	public:

		void set_thread_reserved(size_t count) { thread_reserved = count; }
		void close_context() noexcept override { avalible = false; }
		void set_duration(std::chrono::milliseconds ms) { duration_ms = ms; }

		context_implement();
		void load_form_context(Implement::context_temporary& c);
		virtual entity create_entity() override;

		void loop();

		template<typename fun> void init(fun&& f)
		{
			Implement::context_temporary ct(*this);
			f(ct);
			load_form_context(ct);
		}

	};
}