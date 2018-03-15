#pragma once
#include "entity.h"
#include "requirement_detection.h"
namespace PO::ECSFramework
{
	enum class SystemLayout
	{
		PRE_UPDATE = 0,
		UPDATE = 1,
		POS_UPDATE = 2
	};

	inline bool operator<(SystemLayout SL, SystemLayout SL2) { return static_cast<size_t>(SL) < static_cast<size_t>(SL2); }
	inline bool operator==(SystemLayout SL, SystemLayout SL2) { return static_cast<size_t>(SL) == static_cast<size_t>(SL2); }

	enum class SystemExecutionSequence
	{
		UNDEFINE,
		BEFORE,
		AFTER,
		NOT_CARE
	};

	struct system_res : Implement::base_res
	{
		SystemExecutionSequence check_sequence(std::type_index ti) const noexcept { return SystemExecutionSequence::UNDEFINE; }
		SystemLayout layout() const noexcept { return SystemLayout::UPDATE; }
		void start(context&) {}
		void end(context&) {}
	};
	
	namespace Implement
	{
		struct context_interface;

		// requirement_storage =============================
		template<typename storage_type> struct system_requirement_storage 
		{
			component_holder_ptr storage;
			bool storage_form_entity(entity_implement_ptr& EIP) noexcept { return storage = EIP->get_component(typeid(storage_type)); }
			bool is_avalible() const noexcept { return storage && storage->is_avalible(); }
			storage_type get() noexcept { return storage->cast<std::decay_t<storage_type>>();}
			operator storage_type() { return get(); }
		};

		template<> struct system_requirement_storage<entity&>
		{
			entity storage;
			bool storage_form_entity(entity_implement_ptr& EIP) noexcept { return storage.ptr = EIP; }
			bool is_avalible() noexcept { return storage; }
			entity& get() noexcept { return storage; }
			operator entity&() { return get(); }
		};

		template<> struct system_requirement_storage<const entity&>
		{
			entity storage;
			bool storage_form_entity(entity_implement_ptr& EIP) noexcept { return storage.ptr = EIP; }
			bool is_avalible() noexcept { return storage; }
			const entity& get() noexcept { return storage; }
			operator const entity&() { return get(); }
		};

		// requirement_storage_tuple =============================
		template<typename index, typename ...requirement_sequence> struct system_requiremenet_storage_tuple;
		template<size_t ...i, typename ...requirement_sequence>
		struct system_requiremenet_storage_tuple<std::integer_sequence<size_t, i...>, requirement_sequence...>
		{
			using storage_type = std::tuple<system_requirement_storage<requirement_sequence>...>;
			//using storage_type = std::tuple<system_requirement_storage<typename requirement_sequence::type>...>;
			storage_type storage;
			bool is_all_avalible() const noexcept
			{ 
				return is_all_true(true, std::get<i>(storage).is_avalible()...);
			}
			bool load_form_entity(entity_implement_ptr& EIP) noexcept
			{
				return EIP && EIP->is_avalible() && is_all_true(std::get<i>(storage).storage_form_entity(EIP)...);
			}
		};

		template<typename index, typename ...storage_type> bool insert_component_vector(std::vector<std::pair<entity_implement_ptr, system_requiremenet_storage_tuple<index, storage_type...>>>& v, entity_implement_ptr& e)
		{
			assert(e && e->is_avalible());
			typename std::decay_t<decltype(v)>::value_type temporary;
			temporary.first = e;
			if (temporary.second.load_form_entity(e))
			{
				v.push_back(temporary);
				return true;
			}
			return false;
		}

		// requirement_rw_requirement_info =============================
		struct system_rw_requirement_info
		{
			const type_index_view entity_read;
			const type_index_view entity_write;
			const type_index_view singleton_read;
			const type_index_view singleton_write;
			const std::type_index trigger;
		};

		template<typename er, typename ew, typename sr, typename sw, typename t> struct system_rw_requirement_info_initializer;
		template<typename ...er, typename ...ew, typename ...sr, typename ...sw, typename t> 
		struct system_rw_requirement_info_initializer<ctl<er...>, ctl<ew...>, ctl<sr...>, ctl<sw...>, t>
		{
			type_array<er...> entity_read;
			type_array<ew...> entity_write;
			type_array<sr...> single_read;
			type_array<sw...> single_write;
			system_rw_requirement_info view() const noexcept { return  system_rw_requirement_info{ entity_read.view(), entity_write.view(), single_read.view(), single_write.view(),typeid(t) }; }
		};

		template<typename er, typename ew, typename sr, typename sw, typename t>
		const system_rw_requirement_info& system_rw_requirement_info_instance()
		{
			static system_rw_requirement_info_initializer<er, ew, sr, sw, t> init;
			static system_rw_requirement_info info(init.view());
			return info;
		}

		// requirement_requirement_info =============================
		struct system_requirement_info
		{
			const type_index_view first;
			const type_index_view second;
			const type_index_view singleton;
			const std::type_index trigger;
		};

		template<typename first, typename second, typename singleton, typename trigger> struct system_requirement_info_initializer;
		template<typename ...first, typename ...second, typename ...singleton, typename trigger>
		struct system_requirement_info_initializer<ctl<first...>, ctl<second...>, ctl<singleton...>, trigger>
		{
			type_array<first...> first_storage;
			type_array<second...> second_storage;
			type_array<singleton...> singleton_storage;
			system_requirement_info view() const noexcept { return  system_requirement_info{ first_storage.view(), second_storage.view(), singleton_storage.view(), typeid(trigger) }; }
		};

		template<typename first, typename second, typename singleton, typename trigger>
		const system_requirement_info& system_requirement_info_instance()
		{
			static system_requirement_info_initializer<first, second, singleton, trigger> init;
			static system_requirement_info info(init.view());
			return info;
		}


		// singleton_holder ==============================================
		template<typename singleton_type, typename index> struct system_requirement_storage_singleton_holder;
		template<size_t ...i, typename ...singleton_type>
		struct system_requirement_storage_singleton_holder<ctl<singleton_type...>, std::integer_sequence<size_t, i...>>
		{
			system_requiremenet_storage_tuple<std::integer_sequence<size_t, i...>, singleton_type...> storage;
			void insert_singleton(entity_implement_ptr& e) { storage.load_form_entity(e); }
			bool is_singleton_avalible() const noexcept { return storage.is_all_avalible(); }
			template<typename function_t, typename ...other_parameter>
			void singleton_call(function_t&& f, other_parameter&& ...parameter)
			{
				auto& ref = storage.storage;
				std::forward<function_t>(f)(std::forward<other_parameter>(parameter)..., singleton{}, std::get<i>(ref).get()...);
			}
		};

		template<>
		struct system_requirement_storage_singleton_holder<ctl<>, std::integer_sequence<size_t>>
		{
			void insert_singleton(entity_implement_ptr& e) { }
			bool is_singleton_avalible() const noexcept { return true; }
			template<typename function_t, typename ...other_parameter>
			void singleton_call(function_t&& f, other_parameter&& ...parameter) { std::forward<function_t>(f)(std::forward<other_parameter>(parameter)...); }
		};

		// system_requirement_storage_first_second_holder ================================
		template<typename trigger_type, typename first_type, typename first_type_index, typename second_type, typename second_type_index, typename singleton_type, typename singleton_type_index> 
		struct system_requirement_storage_first_second_holder;
		template<typename singleton_type, typename singleton_index> 
		struct system_requirement_storage_first_second_holder<void, ctl<>, std::integer_sequence<size_t>, ctl<>, std::integer_sequence<size_t>, singleton_type, singleton_index>
			: public system_requirement_storage_singleton_holder<singleton_type, singleton_index>
		{

			void insert_first(entity_implement_ptr& e) { assert(false); }

			void insert_second(entity_implement_ptr& e) { assert(false); }
			void insert_trigger(component_holder_ptr chp) { assert(false); }

		protected:
			template<typename function_t, typename ...other_parameter>
			void call(function_t&& f, other_parameter&& ...op)
			{
				if (is_singleton_avalible())
					singleton_call(std::forward<function_t>(f), std::forward<other_parameter>(op)...);
			}
		};
			
		template<typename ...first_type, size_t ...i, typename singleton_type, typename singleton_index>
		struct system_requirement_storage_first_second_holder<void, ctl<first_type...>, std::integer_sequence<size_t, i...>, ctl<>, std::integer_sequence<size_t>, singleton_type, singleton_index>
			: public system_requirement_storage_singleton_holder<singleton_type, singleton_index>
		{
			using storage_type = std::pair<Implement::entity_implement_ptr, system_requiremenet_storage_tuple<std::integer_sequence<size_t, i...>, first_type...>>;
			std::vector<storage_type> storage;

			void insert_first(entity_implement_ptr& e)
			{
				assert(e && e->is_avalible());
				insert_component_vector(storage, e);
			}

			void insert_second(entity_implement_ptr& e) { assert(false); }
			void insert_trigger(component_holder_ptr chp) { assert(false); }

		protected:
			
			template<typename function_t, typename ...other_parameter>
			void call(function_t&& f, other_parameter&& ...op)
			{
				storage.erase(std::remove_if(storage.begin(), storage.end(), [&](storage_type& st) {
					if (st.first && st.first->is_avalible() && st.second.is_all_avalible())
						return false;
					return true;
				}), storage.end());
				
				if (is_singleton_avalible())
				{
					for (auto& ite : storage)
					{
						auto& ref = ite.second.storage;
						singleton_call(std::forward<function_t>(f), std::forward<other_parameter>(op)..., std::get<i>(ref)...);
					}
				}
			}
		};

		/*
		template<typename ...first_type, typename ...second_type, typename singleton_type>
		struct system_requirement_storage_first_second_holder<void, ctl<first_type...>, ctl<second_type...>, singleton_type>
			: public system_requirement_storage_first_second_holder_base,
			public system_requirement_storage_singleton_holder<singleton_type>
		{
			using storage_type_first = std::pair<entity_ptr_const, system_requiremenet_storage_tuple<first_type...>>;
			using storage_type_second = std::pair<entity_ptr_const, system_requiremenet_storage_tuple<second_type...>>;
			std::vector<storage_type_first> storage_first;
			std::vector<storage_type_second> storage_second;
			bool insert_first(entity_ptr& e) { return insert_component_vector(storage, e); }
			bool insert_second(entity_ptr& e) { return insert_component_vector(storage, e); }
		protected:
			template<bool result, typename function_t, typename ...other_parameter>
			void call(function_t&& f, other_parameter&& ...op)
			{
				storage_second.erase(
					std::remove_if(storage_second.begin(), storage_second.end(), [](storage_type_second& sts) {
					return !(sts.first && sts.first->is_living() && sts.second.is_all_living());
				})
					, storage_second.end());

				storage_first.erase(
					std::remove_if(storage_first.begin(), storage_first.end(), [](storage_type_second& sts) {
					return !(sts.first && sts.first->is_living() && sts.second.is_all_living());
				})
					, storage_first.end());

				if (is_singleton_avalible())
				{
					for (auto& ite : storage_first)
					{
						for (auto& ite2 : storage_second)
						{
							if (st.first != ite2.first)
								singleton_call(
									std::forward<function_t>(f), 
									std::forward<other_parameter>(op)..., 
									std::get<first_type::value>(ite.second.storage).get()...,
									PO::ECSFramework::other_entity{},
									std::get<second_type::value>(ite2.second.storage).get()...,
									std::forward<other_parameter>(op)...
								);
						}
					}
				}
			}
		};
		*/

		template<typename trigger_type, typename first_type, typename second_type, typename single_type> struct system_requirement_storage_first_second_holder_wrapper;

		template<typename trigger_type, typename ...first_type, typename ...second_type, typename ...single_type> 
		struct system_requirement_storage_first_second_holder_wrapper<trigger_type, ctl<first_type...>, ctl<second_type...>, ctl<single_type...>> :
			system_requirement_storage_first_second_holder<trigger_type,
			ctl<first_type...>, decltype(std::make_integer_sequence<size_t, sizeof...(first_type)>()),
			ctl<second_type...>, decltype(std::make_integer_sequence<size_t, sizeof...(second_type)>()),
			ctl<single_type...>, decltype(std::make_integer_sequence<size_t, sizeof...(single_type)>())
			>
		{};

		template<bool have_context, typename trigger_type, typename first_index, typename second_type, typename singleton_type> 
		struct system_requirement_storage_holder
			: system_requirement_storage_first_second_holder_wrapper<trigger_type, first_index, second_type, singleton_type>
		{
			template<typename function_t>
			void operator()(function_t&& f, context& c)
			{
				system_requirement_storage_first_second_holder_wrapper<trigger_type, first_index, second_type, singleton_type>::call(std::forward<function_t>(f), c);
			}
		};

		template<typename trigger_type, typename first_index_type, typename second_type, typename singleton_type> 
		struct system_requirement_storage_holder<false, trigger_type, first_index_type, second_type, singleton_type>
			: system_requirement_storage_first_second_holder_wrapper<trigger_type, first_index_type, second_type, singleton_type>
		{
			template<typename function_t>
			void operator()(function_t&& f, context& c)
			{
				system_requirement_storage_first_second_holder_wrapper<trigger_type, first_index_type, second_type, singleton_type>::call(std::forward<function_t>(f));
			}
		};

		

		struct system_inside_ptr : public ptr_base
		{
			virtual bool is_avalible() const noexcept = 0;
			virtual void insert_first(entity_implement_ptr&) = 0;
			virtual void insert_second(entity_implement_ptr&) = 0;
			virtual void insert_singleton(entity_implement_ptr&) = 0;
			virtual void insert_trigger(component_holder_ptr) = 0;
			virtual void call(context& c) = 0;
			virtual const system_rw_requirement_info& rw_info() const noexcept = 0;
			virtual const system_requirement_info& info() const noexcept = 0;
			virtual SystemLayout layout() const noexcept = 0;
			virtual SystemExecutionSequence check_sequence(std::type_index) const noexcept = 0;
		};

		template<typename implement_t> struct system_implement :
			system_inside_ptr, implement_t
		{
			using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<implement_t>>::type>;
			using true_type = std::decay_t<typename funtype::template out<system_requirement_analyzer>>;
			using trigger_type = typename system_trigger_define_detector_implement<implement_t>::type;
			using first_type = typename true_type::first;
			using second_type = typename true_type::second;
			using singleton_type = typename true_type::singleton;
			using entity_rw = typename link_t<first_type, second_type>::template outpacket<collect_read_write>;
			using singleton_rw = typename singleton_type::template outpacket<collect_read_write>;

			//system_type_holder<typename true_type::write_type, typename true_type::read_type, typename true_type::all_type> holder;

			static_assert(!(true_type::second::size != 0 && true_type::first::size == 0), "requirement form first entity is 0 but form second entity is not 0, why not use set requirement form firrst?");
			static_assert(!(true_type::second::size == 0 && true_type::have_other_entity), "need component form second entity, but no requirement");
			static_assert(!(true_type::singleton::size == 0 && true_type::have_singleton), "need singleton component, but no requirement");

			system_requirement_storage_holder<true_type::need_context, void, first_type, second_type, singleton_type> storage;

			virtual void insert_first(entity_implement_ptr& e) override { storage.insert_first(e); }
			virtual void insert_second(entity_implement_ptr& e) override { storage.insert_second(e); }
			virtual void insert_singleton(entity_implement_ptr& e) override { storage.insert_singleton(e); }
			virtual void insert_trigger(component_holder_ptr chp) override { storage.insert_trigger(std::move(chp)); }

			virtual void call(context& c) override { 
				start(c);
				storage(*static_cast<implement_t*>(this), c);
				end(c);
			}
			virtual bool is_avalible() const noexcept override { return implement_t::is_avalible(); }
			//virtual bool is_living() const noexcept override { return true_type::need_; }

			template<typename ...construction_para>
			system_implement(construction_para&& ...cp) : implement_t(std::forward<construction_para>(cp)...) {}
			virtual void* data() noexcept override { return static_cast<implement_t*>(this); }
			virtual const void* data() const noexcept override { return static_cast<const implement_t*>(this); }
			virtual bool is_same_id(std::type_index ti) const noexcept override { return ti == typeid(implement_t); }
			virtual std::type_index id() const noexcept override { return typeid(implement_t); }
			virtual SystemLayout layout() const noexcept override { return implement_t::layout(); }
			virtual SystemExecutionSequence check_sequence(std::type_index ti) const noexcept { return implement_t::check_sequence(ti); };
			virtual const system_requirement_info& info() const noexcept override { 
				return system_requirement_info_instance<first_type, second_type, singleton_type, trigger_type>();
			}
			virtual const system_rw_requirement_info& rw_info() const noexcept {
				return system_rw_requirement_info_instance<typename entity_rw::read, typename entity_rw::write, typename singleton_rw::read, typename singleton_rw::write, trigger_type>();
			}
		};

		template<typename implement> 
		struct normal_deleter
		{
			~normal_deleter()
			{
				std::byte* ptr = reinterpret_cast<std::byte*>(static_cast<implement*>(this));
				delete [] ptr;
			}
		};

		struct system_holder :
			normal_deleter<system_holder>
		{
			ecs_unique_ptr<system_inside_ptr> ptr;
			mutable Tool::atomic_reference_count count;
			void add_ref() const noexcept { count.add_ref(); }
			bool sub_ref() const noexcept { return count.sub_ref(); }
			bool is_avalible() const noexcept { return ptr && ptr->is_avalible(); }
			void call(context& c) { ptr->call(c); }
			SystemLayout layout() const noexcept { return ptr->layout(); }
			SystemExecutionSequence check_sequence(std::type_index ti) const noexcept { return ptr->check_sequence(ti); }
			const system_rw_requirement_info& rw_info() const noexcept { return ptr->rw_info(); }
			const system_requirement_info& info() const noexcept { return ptr->info(); }
			void insert_first(entity_implement_ptr& e) { ptr->insert_first(e); }
			void insert_second(entity_implement_ptr& e) { ptr->insert_second(e); }
			void insert_singleton(entity_implement_ptr& e) { ptr->insert_singleton(e); }
			void insert_trigger(component_holder_ptr chp) { ptr->insert_trigger(std::move(chp)); }
			std::type_index id() { return ptr->id(); }
		};

		using system_holder_ptr = ecs_intrusive_ptr<system_holder>;

	}
}