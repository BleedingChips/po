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

	namespace Implement
	{

		class vision
		{
			size_t vision_number = 0;
		public:
			void update() { vision_number++; }
			bool different(const vision& t) {
				if (vision_number != t.vision_number)
					return (vision_number = t.vision_number, true);
				return false;
			}
		};

		struct context_component_holder
		{
			component_holder_ptr h_ptr;
			entity_implement_ptr e_ptr;
			context_component_holder(component_holder_ptr c, entity_implement_ptr e) : h_ptr(std::move(c)), e_ptr(std::move(e)) {}
		};

		struct context_system_holder
		{
			system_holder_ptr s_ptr;
		};
	}

	namespace Implement
	{
		struct context_temporary : public context
		{
			std::vector<context_component_holder> temporary_component_context_holder;
			std::vector<system_holder_ptr> temporary_system_context_holder;
			virtual void insert(component_holder_ptr chp, entity_implement_ptr e) { temporary_component_context_holder.push_back(context_component_holder{ std::move(chp), std::move(e) }); };
			virtual void insert(system_holder_ptr csh) { temporary_system_context_holder.push_back(system_holder_ptr{ std::move(csh) }); };
			using context::context;
		};

		class component_map
		{
			std::unordered_map<std::type_index, std::pair<Implement::vision, std::vector<Implement::context_component_holder>>> map_holder;
			decltype(map_holder)::iterator find_min_type_index(type_index_view tiv) noexcept;
		public:
			void insert(Implement::vision, Implement::context_component_holder);
			void update(Implement::vision, Implement::system_holder_ptr);
			void reflesh(Implement::vision);
		};

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
			Implement::system_holder_ptr ptr;
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
			Implement::system_holder_ptr pop_one(bool& finish);
			void finish_operating(std::type_index ti);
		public:
			bool reflesh_unavalible_map();
			bool update_waitting_list();
			void insert(Implement::system_holder_ptr);
			void insert(Implement::context_component_holder);
			void insert_singleton(Implement::context_component_holder, Implement::entity_implement_ptr);

			bool execute_one(context& c);
			bool execute_one_other_thread(context& c, bool& finish);
		};
	}

	class context_implement : public Implement::context_interface
	{

		object_pool pool;
		Platform::thread_pool threads;
		Platform::asynchronous_affairs addairs;
		std::atomic_bool avalible;
		Implement::vision reflesh_vision;
		std::chrono::milliseconds duration_ms;
		Implement::component_map all_component;
		std::vector<Implement::context_component_holder> tem_component_buffer;
		PO::Tool::scope_lock<std::vector<Implement::context_component_holder>> temporary_component_holder;

		Implement::system_map all_system;
		PO::Tool::scope_lock<std::vector<Implement::system_holder_ptr>> temporary_system_holder;
		decltype(temporary_system_holder)::type temporary_system_holder_buffer;

		std::mutex waitting_list_mutex;
		std::set<std::type_index> waitting_list_start;
		std::set<std::type_index> waitting_list;

		entity singleton_entity;

		virtual Implement::component_holder_ptr allocate_component(std::type_index, size_t type, size_t aligna, void*& component_out) override;
		virtual Implement::system_holder_ptr allocate_system(std::type_index, size_t type, size_t aligna, void*& system_out) override;

		bool thread_execute();

	public:

		void close_context() noexcept override { avalible = false; }
		void set_duration(std::chrono::milliseconds ms) { duration_ms = ms; }

		context_implement();
		void load_form_context(Implement::context_temporary& c);
		virtual entity create_entity() { return entity{ Implement::entity_implement_ptr{ pool.allocate<Implement::entity_implement>() } }; };

		void loop();

		template<typename fun> void create(fun&& f)
		{
			Implement::context_temporary ct(*this);
			f(ct);
			load_form_context(ct);
		}

	};
}