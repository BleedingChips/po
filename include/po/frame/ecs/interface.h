#pragma once
#include <typeindex>
#include "../../tool/tool.h"
#include "../../tool/intrusive_ptr.h"
#include <array>
#include <map>
#include <shared_mutex>
#include <set>
namespace PO::ECS
{

	struct TypeLayout
	{
		size_t hash_code;
		size_t size;
		size_t align;
		const char* name;
		~TypeLayout() = default;
		template<typename Type> static const TypeLayout& create() noexcept {
			static TypeLayout type{typeid(Type).hash_code(), sizeof(Type), alignof(Type), typeid(Type).name()};
			return type;
		}
		bool operator<(const TypeLayout& r) const noexcept
		{
			if (hash_code < r.hash_code)
				return true;
			else if (hash_code == r.hash_code)
			{
				if (size < r.size)
					return true;
				else if (size == r.size)
				{
					if (align < r.align)
						return true;
				}
			}
			return false;
		}
		bool operator<=(const TypeLayout& r) const noexcept
		{
			return (*this) == r || (*this) < r;
		}
		bool operator==(const TypeLayout& type) const noexcept
		{
			return hash_code == type.hash_code && size == type.size && align == type.align;
		}
	};

	struct Context;

	namespace Implement
	{

		enum class RWProperty
		{
			Read,
			Write
		};

		struct RWPropertyTuple
		{
			std::map<TypeLayout, Implement::RWProperty> components;
			std::map<TypeLayout, Implement::RWProperty> gobal_components;
			std::map<TypeLayout, Implement::RWProperty> systems;
			std::set<TypeLayout> events;
		};

		template<typename ...AT> struct ComponentInfoExtractor
		{
			void operator()(std::map<TypeLayout, RWProperty>& result) {}
		};

		template<typename T, typename ...AT> struct ComponentInfoExtractor<T, AT...>
		{
			void operator()(std::map<TypeLayout, RWProperty>& result) {
				if constexpr (std::is_const_v<T>)
					result.insert({ TypeLayout::create<T>(), RWProperty::Read });
				else
					result[TypeLayout::create<T>()] = RWProperty::Write;
				ComponentInfoExtractor<AT...>{}(result);
			}
		};

		template<typename ...Require> struct SystemStorage;
		template<typename Type> struct FilterAndEventAndSystem;
		struct ComponentPoolInterface;
	}

	namespace Implement
	{

		struct TypeGroup;
		struct StorageBlock;

		struct EntityInterface
		{
			virtual void add_ref() const noexcept = 0;
			virtual void sub_ref() const noexcept = 0;
			virtual void* owner() const noexcept = 0;
			virtual void read(TypeGroup*&, StorageBlock*&, size_t& index) const noexcept = 0;
			virtual void set(TypeGroup*, StorageBlock*, size_t index) const noexcept = 0;
			virtual bool have(const TypeLayout*, size_t index) const noexcept = 0;
		};

		using EntityInterfacePtr = Tool::intrusive_ptr<EntityInterface>;
	}

	struct Entity
	{
		operator bool() const noexcept { return m_imp; }
		template<typename ...Type> bool have() const noexcept
		{
			assert(m_imp);
			std::array<TypeLayout, sizeof(...Type)> infos = {TypeLayout::create<Type>()...};
			return m_imp->have(infos.data(), infos.size());
		}
		Entity(const Entity&) = default;
		Entity(Entity&&) = default;
		Entity() = default;
		Entity& operator=(const Entity&) = default;
		Entity& operator=(Entity&&) = default;
	private:
		Entity(Implement::EntityInterfacePtr ptr) : m_imp(std::move(ptr)) {}
		Implement::EntityInterfacePtr m_imp;

		friend struct EntityWrapper;
		template<typename ...CompT> friend struct EntityFilter;
		friend struct Context;
	};

	struct EntityWrapper
	{
		operator bool() const noexcept { return m_imp != nullptr; }
		operator Entity() const noexcept { return Entity{ m_imp }; }
		EntityWrapper(Implement::EntityInterface* ptr = nullptr) : m_imp(ptr) {}
		template<typename Type> bool have() const noexcept
		{
			Implement::ComponentMemoryPageDesc* desc; size_t index;
			m_imp->read(TypeLayout::create<Type>(), desc, index);
			return desc != nullptr;
		}
	private:
		Implement::EntityInterface* m_imp;
	};
}

namespace PO::ECS
{
	namespace Implement
	{
		struct ComponentPoolReadWrapper
		{
			ComponentMemoryPageDesc* page = nullptr;
			size_t component_size = 0;
			size_t available_count = 0;
			PO::ECS::Implement::EntityInterface** entitys;
			void* components;
			size_t total_count;
			bool reach_first()
			{
				assert(page != nullptr);
				assert(entitys != nullptr);
				if (*entitys == nullptr)
					return find_next();
				return true;
			}
			bool find_next() noexcept
			{
				assert(page != nullptr);
				if (*entitys != nullptr)
				{
					if (available_count >= 1)
						--available_count;
				}
				while (available_count > 0)
				{
					entitys = entitys + 1;
					components = static_cast<std::byte*>(components) + component_size;
					if ((*entitys) != nullptr)
						return true;
				}
				return false;
			}
			bool operator==(const ComponentPoolReadWrapper& c) const noexcept
			{
				return page == c.page && entitys == c.entitys;
			}
		};


		struct ComponentPoolInterface
		{
			template<typename CompT, typename ...Parameter> CompT& construction_component(EntityInterface* owner, Parameter&& ...pa);
			virtual bool lock(ComponentPoolReadWrapper& wrapper, size_t count, const TypeLayout* layout, uint64_t* version, size_t mutex_size, void* mutex) = 0;
			virtual void next(ComponentPoolReadWrapper& wrapper) = 0;
			virtual void unlock(size_t count, size_t mutex_size, void* mutex) noexcept = 0;
			virtual void construct_component(const TypeLayout& layout, void(*constructor)(void*,void*), void* data, EntityInterface*, void(*deconstructor)(void*) noexcept, void(*mover)(void*, void*)) = 0;
			virtual bool deconstruct_component(EntityInterface*, const TypeLayout& layout) noexcept = 0;
		};

		template<typename CompT, typename ...Parameter> auto ComponentPoolInterface::construction_component(EntityInterface* owner, Parameter&& ...pa) -> CompT&
		{
			CompT* result = nullptr;
			auto pa_tuple = std::forward_as_tuple(result, std::forward<Parameter>(pa)...);
			construct_component(TypeLayout::create<CompT>(), [](void* adress, void* para) {
				auto& ref = *static_cast<decltype(pa_tuple)*>(para);
				using Type = CompT;
				std::apply([&](auto& ref, auto && ...at) { ref = new (adress) Type{ std::forward<decltype(at)&&>(at)... }; },ref);
				}, & pa_tuple, owner, [](void* in) noexcept { static_cast<CompT*>(in)->~CompT(); });
			return *result;
		}
	}
	template<typename ...CompT> struct EntityFilter;
	namespace Implement
	{
		template<typename T> struct TypePropertyDetector {
			using type = std::remove_reference_t<std::remove_cv_t<T>>;
			static constexpr bool value = std::is_same_v<T, type> || std::is_same_v<T, std::add_const_t<type>>;
		};

		template<size_t index, size_t total_index> struct ComponentPointerTranstlate
		{
			template<typename Tuple>
			void operator()(Tuple& output, std::array<void*, total_index>& input) noexcept
			{
				using target_type = std::remove_reference_t<decltype(std::get<index>(output))>;
				static_assert(std::is_pointer_v<target_type>);
				static_assert(index < total_index);
				auto byte = input[index];
				assert(byte != nullptr);
				std::get<index>(output) = reinterpret_cast<target_type>(byte);
				ComponentPointerTranstlate<index + 1, total_index>{}(output, input);
			}
		};

		template<size_t total_index> struct ComponentPointerTranstlate<total_index, total_index>
		{
			template<typename Tuple>
			void operator()(Tuple& output, std::array<void*, total_index>& input) noexcept {}
		};

		template<typename ...CompT> struct EntityFilterImp
		{
			template<typename Func> bool operator<<(Func&& f);
			EntityFilterImp(EntityFilterImp&&) = default;
		private:
			template<typename Func, typename Tuple, size_t ...index>
			void apply(Func&& f, Tuple&& t, std::index_sequence<index...>);
			EntityFilterImp(EntityInterfacePtr ptr) : m_ptr(std::move(ptr)) {};
			EntityInterfacePtr m_ptr;
			template<typename ...CompT> friend struct EntityFilter;
		};

		template<typename ...CompT> template<typename Func> bool EntityFilterImp<CompT...>::operator<<(Func&& f)
		{
			if (m_ptr)
			{
				std::array<void*, sizeof...(CompT)> result;
				if (m_ptr->read_tuple<CompT...>(result))
				{
					std::tuple<std::add_pointer_t<CompT>...> m_storage;
					ComponentPointerTranstlate<0, sizeof...(CompT)>{}(m_storage, result);
					apply(std::forward<Func>(f), m_storage, std::index_sequence_for<CompT...>{});
					return true;
				}
			}
			return false;
		}

		template<typename ...CompT> template<typename Func, typename Tuple, size_t ...index>
		void EntityFilterImp<CompT...>::apply(Func&& f, Tuple&& t, std::index_sequence<index...>)
		{
			std::forward<Func>(f)(*std::get<index>(t)...);
		}

		template<typename ...CompT> struct ComponentTypeInfo
		{
			static const std::array<TypeLayout, sizeof...(CompT)>& info() noexcept { return m_info; }
		private:
			static const std::array<TypeLayout, sizeof...(CompT)> m_info;
		};

		template<typename ...CompT> const std::array<TypeLayout, sizeof...(CompT)> ComponentTypeInfo<CompT...>::m_info = {
			TypeLayout::create<CompT>()...
		};
	}

	template<typename ...CompT> struct EntityFilter
	{
		static_assert(Tmp::bool_and<true, Implement::TypePropertyDetector<CompT>::value...>::value, "EntityFilter only accept Type and const Type!");
		Implement::EntityFilterImp<CompT...> operator()(Entity enity) const;
	private:
		EntityFilter(Implement::ComponentPoolInterface* pool) noexcept : m_pool(pool) { assert(pool != nullptr); }
		bool lock() noexcept;
		void unlock() noexcept;
		static void export_rw_info(Implement::RWPropertyTuple& tuple) noexcept { Implement::ComponentInfoExtractor<CompT...>{}(tuple.components); }

		Implement::ComponentPoolInterface* m_pool = nullptr;
		std::array<std::byte, sizeof(std::shared_lock<std::shared_mutex>) * 2 * sizeof...(CompT)> m_mutex;

		template<typename ...Require> friend struct Implement::SystemStorage;
		template<typename Require> friend struct Implement::FilterAndEventAndSystem;
	};

	template<typename ...CompT> Implement::EntityFilterImp<CompT...> EntityFilter<CompT...>::operator()(Entity enity) const
	{
		return Implement::EntityFilterImp<CompT...>{std::move(enity.m_imp)};
	}

	template<typename ...CompT> bool EntityFilter<CompT...>::lock() noexcept
	{
		Implement::ComponentPoolReadWrapper wrapper;
		auto& info = Implement::ComponentTypeInfo<CompT...>::info();
		std::array<uint64_t, sizeof...(CompT)> version;
		m_pool->lock(wrapper, sizeof...(CompT), info.data(), version.data(), sizeof(std::shared_lock<std::shared_mutex>) * 2, m_mutex.data());
		return true;
	}

	template<typename ...CompT> void EntityFilter<CompT...>::unlock() noexcept
	{
		m_pool->unlock(sizeof...(CompT), sizeof(std::shared_lock<std::shared_mutex>) * 2, m_mutex.data());
	}

	template<typename ...CompT> struct Filter
	{
		static_assert(Tmp::bool_and<true, Implement::TypePropertyDetector<CompT>::value...>::value, "Filter only accept Type and const Type!");
		struct iterator
		{
			EntityWrapper entity() const noexcept { return std::get<0>(*m_ite); }
			std::tuple<CompT&...>& components() noexcept { return std::get<1>(*m_ite); }
			std::tuple<EntityWrapper, std::tuple<CompT& ...>> operator*() noexcept { return *m_ite; }
			bool operator==(const iterator& i) const noexcept { return m_ite == i.m_ite; }
			bool operator!=(const iterator& i) const noexcept { return !((*this) == i); }
			iterator& operator++() noexcept { 
				++m_ite; 
				return *this; 
			}
		private:
			typename std::vector<std::tuple<EntityWrapper, std::tuple<CompT&...>>>::iterator m_ite;
			template<typename ...CompT> friend struct Filter;
		};
		iterator begin() noexcept { iterator tem; tem.m_ite = m_all_component.begin(); return tem; }
		iterator end() noexcept { iterator tem; tem.m_ite = m_all_component.end(); return tem; }
		size_t count() const noexcept { return m_all_component.size(); }
	protected:
		Filter(Implement::ComponentPoolInterface* pool) noexcept : m_pool(pool) { assert(pool != nullptr); }
		bool lock() noexcept;
		void unlock() noexcept;
		static void export_rw_info(Implement::RWPropertyTuple& tuple) noexcept { Implement::ComponentInfoExtractor<CompT...>{}(tuple.components); }

		std::vector<std::tuple<EntityWrapper, std::tuple<CompT&...>>> m_all_component;
		std::array<uint64_t, sizeof...(CompT)> m_version;
		std::array<std::byte, sizeof(sizeof(std::shared_lock<std::shared_mutex>)) * 2 * sizeof...(CompT)> m_mutex;
		Implement::ComponentPoolInterface* m_pool = nullptr;

		template<typename ...Require> friend struct Implement::SystemStorage;
		template<typename Require> friend struct Implement::FilterAndEventAndSystem;
	};

	template<typename ...CompT> bool Filter<CompT...>::lock() noexcept
	{
		Implement::ComponentPoolReadWrapper wrapper;
		auto& info = Implement::ComponentTypeInfo<CompT...>::info();
		if (m_pool->lock(wrapper, sizeof...(CompT), info.data(), m_version.data(), sizeof(std::shared_lock<std::shared_mutex>) * 2, m_mutex.data()))
		{
			m_all_component.clear();
			while (wrapper.page != nullptr)
			{
				if (wrapper.reach_first())
				{
					std::array<void*, sizeof...(CompT)> pointer;
					if ((*wrapper.entitys)->read_tuple<CompT...>(pointer))
					{
						std::tuple<std::add_pointer_t<CompT>...> storage;
						Implement::ComponentPointerTranstlate<0, sizeof...(CompT)>{}(storage, pointer);
						auto result = std::apply([](auto* ...ptr) {return std::forward_as_tuple(*ptr...); }, storage);
						m_all_component.push_back({ *wrapper.entitys, result });
					}
					if (!wrapper.find_next())
						m_pool->next(wrapper);
				}else
					m_pool->next(wrapper);
			}
		}
		return true;
	}

	template<typename ...CompT> void Filter<CompT...>::unlock() noexcept
	{
		m_pool->unlock(sizeof...(CompT), sizeof(std::shared_lock<std::shared_mutex>) * 2, m_mutex.data());
	}

	template<typename CompT> struct Filter<CompT>
	{
		static_assert(Implement::TypePropertyDetector<CompT>::value, "Filter only accept Type and const Type!");
		struct iterator
		{
			EntityWrapper entity() const noexcept { return *m_wrapper.entitys; }
			CompT& components() noexcept { return *static_cast<CompT*>(m_wrapper.components); }
			std::tuple<EntityWrapper, CompT&> operator*() noexcept { return { entity(), components()}; }
			bool operator==(const iterator& i) const noexcept { return m_wrapper == i.m_wrapper; }
			bool operator!=(const iterator& i) const noexcept { return !((*this) == i); }
			iterator& operator++() noexcept {
				while (m_wrapper.page != nullptr)
				{
					if (!m_wrapper.find_next())
					{
						m_pool->next(m_wrapper);
						if (m_wrapper.page != nullptr)
						{
							while(m_wrapper.page != nullptr && !m_wrapper.reach_first())
								m_pool->next(m_wrapper);
							break;
						}
					}
					else
						break;
				}
				return *this;
			}
		private:
			Implement::ComponentPoolReadWrapper m_wrapper;
			Implement::ComponentPoolInterface* m_pool = nullptr;
			template<typename ...CompT> friend struct Filter;
		};
		iterator begin() const noexcept;
		iterator end() const noexcept;
		size_t count() const noexcept { return m_wrapper.total_count; }
	protected:
		bool lock() noexcept;
		void unlock() noexcept;
		Filter(Implement::ComponentPoolInterface* in) noexcept : m_pool(in) {}
		static void export_rw_info(Implement::RWPropertyTuple& tuple) noexcept { Implement::ComponentInfoExtractor<CompT>{}(tuple.components); }
		Implement::ComponentPoolInterface* m_pool = nullptr;
		std::array<std::byte, sizeof(std::shared_lock<std::shared_mutex>) * 2> m_mutex;
		Implement::ComponentPoolReadWrapper m_wrapper;
		template<typename ...Require> friend struct Implement::SystemStorage;
		template<typename Require> friend struct Implement::FilterAndEventAndSystem;
	};

	template<typename CompT> auto Filter<CompT>::begin() const noexcept ->iterator
	{
		iterator result;
		result.m_wrapper = m_wrapper;
		result.m_pool = m_pool;
		return result;
	}

	template<typename CompT> auto Filter<CompT>::end() const noexcept->iterator
	{
		iterator result;
		result.m_wrapper = Implement::ComponentPoolReadWrapper{nullptr,sizeof(CompT), 0, nullptr, nullptr, 0};
		result.m_pool = m_pool;
		return result;
	}

	template<typename CompT> bool Filter<CompT>::lock() noexcept
	{
		auto& info = Implement::ComponentTypeInfo<CompT>::info();
		uint64_t version = 0;
		m_pool->lock(m_wrapper, 1, info.data(), &version, sizeof(std::shared_lock<std::shared_mutex>) * 2, m_mutex.data());
		while (m_wrapper.page != nullptr && !m_wrapper.reach_first())
			m_pool->next(m_wrapper);
		return true;
	}

	template<typename CompT> void Filter<CompT>::unlock() noexcept
	{
		m_pool->unlock(1, sizeof(std::shared_lock<std::shared_mutex>) * 2, m_mutex.data());
	}

	namespace Implement
	{
		struct EventPoolMemoryDescription;

		struct EventPoolWriteWrapperInterface
		{
			virtual void construct_event(void(*construct)(void*, void*), void* para, void(*deconstruct)(void*)noexcept) = 0;
		};

		struct EventPoolReadResult
		{
			EventPoolMemoryDescription* page = nullptr;
			size_t count = 0;
			void* components = nullptr;
			bool operator==(const EventPoolReadResult& epr) const noexcept
			{
				return page == epr.page && count == epr.count;
			}
			bool operator!=(const EventPoolReadResult& epr) const noexcept
			{
				return !(*this == epr);
			}
		};

		struct EventPoolInterface
		{
			virtual void read_lock(const TypeLayout& layout, EventPoolReadResult&, size_t mutex_size, void* mutex) noexcept = 0;
			virtual void read_next_page(EventPoolReadResult& result) noexcept = 0;
			virtual void read_unlock(size_t mutex_size, void* mutex) noexcept = 0;
			virtual EventPoolWriteWrapperInterface* write_lock(const TypeLayout& layout, size_t mutex_size, void* mutex) noexcept = 0;
			virtual void write_unlock(EventPoolWriteWrapperInterface*, size_t mutex_size, void* mutex) noexcept = 0;
		};
	}

	template<typename EventT> struct EventProvider
	{
		operator bool() const noexcept { return m_ref != nullptr; }
		template<typename ...Parameter> void push(Parameter&& ...pa);
	private:
		EventProvider(Implement::EventPoolInterface* pool) noexcept : m_pool(pool){}
		bool lock() noexcept;
		void unlock() noexcept;
		static void export_rw_info(Implement::RWPropertyTuple& tuple) noexcept {
			tuple.events.insert(TypeLayout::create<EventT>());
		}
		Implement::EventPoolWriteWrapperInterface* m_ref = nullptr;
		Implement::EventPoolInterface* m_pool = nullptr;
		std::array<std::byte, sizeof(std::lock_guard<std::mutex>) * 2> m_mutex;
		template<typename ...Require> friend struct Implement::SystemStorage;
		template<typename Require> friend struct Implement::FilterAndEventAndSystem;
	};

	template<typename EventT> template<typename ...Parameter> void EventProvider<EventT>::push(Parameter&& ...pa)
	{
		assert(m_ref != nullptr);
		auto pa_tuple = std::forward_as_tuple(std::forward<Parameter>(pa)...);
		m_ref->construct_event(
			[](void* adress, void* para) {
				auto& po = *static_cast<decltype(pa_tuple)*>(para);
				std::apply([&](auto && ...at) {
					new (adress) EventT{ std::forward<decltype(at) &&>(at)... };
					}, po);
			}, &pa_tuple, [](void* in) noexcept { reinterpret_cast<EventT*>(in)->~EventT(); });
	}

	template<typename EventT> bool EventProvider<EventT>::lock() noexcept
	{
		m_ref = m_pool->write_lock(TypeLayout::create<EventT>(), sizeof(std::lock_guard<std::mutex>) * 2, m_mutex.data());
		return m_ref != nullptr;
		return true;
	}

	template<typename EventT> void EventProvider<EventT>::unlock() noexcept
	{
		m_pool->write_unlock(m_ref, sizeof(std::lock_guard<std::mutex>) * 2, m_mutex.data());
		m_ref = nullptr;
	}

	template<typename EventT> struct EventViewer
	{
		struct iterator
		{
			const EventT& operator*() const noexcept{ return *static_cast<const EventT*>(m_wrapper.components); }
			iterator operator++() noexcept
			{
				if (m_wrapper.page != nullptr)
				{
					if (m_wrapper.count == 1)
						m_pool->read_next_page(m_wrapper);
					else {
						--m_wrapper.count;
						m_wrapper.components = static_cast<std::byte*>(m_wrapper.components) + sizeof(EventT);
					}
				}
				return *this;
			}
			bool operator== (const iterator& i) const noexcept { return m_wrapper == i.m_wrapper; }
			bool operator!= (const iterator& i) const noexcept { return !(m_wrapper == i.m_wrapper); }
		private:
			Implement::EventPoolReadResult m_wrapper;
			Implement::EventPoolInterface* m_pool;
			template<typename EventT> friend struct EventViewer;
		};
		iterator begin() noexcept;
		iterator end() noexcept;
	private:
		EventViewer(Implement::EventPoolInterface* pool) noexcept : m_pool(pool){}
		bool lock() noexcept;
		void unlock() noexcept;
		static void export_rw_info(Implement::RWPropertyTuple& tuple) noexcept {}
		Implement::EventPoolReadResult m_wrapper;
		Implement::EventPoolInterface* m_pool = nullptr;
		std::array<std::byte, sizeof(std::shared_lock<std::shared_mutex>) * 2> m_mutex;
		template<typename ...Require> friend struct Implement::SystemStorage;
		template<typename Require> friend struct Implement::FilterAndEventAndSystem;
	};

	template<typename EventT> bool EventViewer<EventT>::lock() noexcept
	{
		m_pool->read_lock(TypeLayout::create<EventT>(), m_wrapper, sizeof(std::shared_lock<std::shared_mutex>) * 2, m_mutex.data());
		return true;
	}

	template<typename EventT> void EventViewer<EventT>::unlock() noexcept
	{
		m_pool->read_unlock(sizeof(std::shared_lock<std::shared_mutex>) * 2, m_mutex.data());
	}

	template<typename EventT> auto EventViewer<EventT>::begin() noexcept -> iterator
	{
		iterator result;
		result.m_wrapper = m_wrapper;
		result.m_pool = m_pool;
		return result;
	}

	template<typename EventT> auto EventViewer<EventT>::end() noexcept -> iterator
	{
		iterator result;
		result.m_wrapper = Implement::EventPoolReadResult{};
		result.m_pool = m_pool;
		return result;
	}


	namespace Implement
	{

		template<typename Type> struct GobalComponentImp;
		template<typename CompT> using GobalComponentImpPtr = Tool::intrusive_ptr<GobalComponentImp<CompT>>;

		struct GobalComponentInterface
		{
			virtual ~GobalComponentInterface() = default;
			virtual const TypeLayout& layout() const noexcept = 0;
			template<typename T> std::remove_reference_t<T>* get_adress() {
				assert(layout() == TypeLayout::create<T>());
				return static_cast<std::remove_reference_t<T>*>(get_adress_imp());
			}
			virtual void add_ref() const noexcept = 0;
			virtual void sub_ref() const noexcept = 0;
		private:
			virtual void* get_adress_imp() = 0;
		};

		template<typename Type> struct GobalComponentImp : GobalComponentInterface
		{
			operator Type& () { return m_storage; }
			const TypeLayout& layout() const noexcept { return m_layout; }
			virtual void add_ref() const noexcept override { m_ref.add_ref(); }
			virtual void sub_ref() const noexcept override {
				if (m_ref.sub_ref())
					delete this;
			}
			void* get_adress_imp() { return &m_storage; }
			template<typename ...Parameter>
			static Tool::intrusive_ptr<GobalComponentImp> create(Parameter&& ... para) { return new GobalComponentImp{ std::forward<Parameter>(para)... }; }
		private:
			template<typename ...Parameter> GobalComponentImp(Parameter&& ...);
			mutable Tool::atomic_reference_count m_ref;
			const TypeLayout& m_layout;
			Type m_storage;
		};

		template<typename Type> template<typename ...Parameter> GobalComponentImp<Type>::GobalComponentImp(Parameter&& ... para)
			: m_layout(TypeLayout::create<Type>()), m_storage(std::forward<Parameter>(para)...)
		{}

		using GobalComponentInterfacePtr = Tool::intrusive_ptr<GobalComponentInterface>;

		struct GobalComponentPoolInterface
		{
			template<typename Type>
			std::remove_reference_t<Type>* find() noexcept;
			virtual void regedit_gobal_component(GobalComponentInterface*) noexcept = 0;
			virtual void destory_gobal_component(const TypeLayout&) = 0;
		private:
			virtual GobalComponentInterface* find_imp(const TypeLayout& layout) const noexcept = 0;
		};

		template<typename Type> std::remove_reference_t<Type>* GobalComponentPoolInterface::find() noexcept
		{
			using PureType = std::remove_const_t<std::remove_reference_t<Type>>;
			auto re = find_imp(TypeLayout::create<PureType>());
			if (re)
				return re->get_adress<Type>();
			return nullptr;
		}
	}

	enum class TickPriority
	{
		HighHigh = 0,
		High = 1,
		Normal = 2,
		Low = 3,
		LowLow = 4,
	};

	enum class TickOrder
	{
		Undefine = 0,
		Mutex = 1,
		Before = 2,
		After = 3,
	};

	namespace Implement
	{
		struct SystemInterface
		{
			virtual void* data() noexcept = 0;
			virtual const TypeLayout& layout() const noexcept = 0;
			virtual void apply(Context*) noexcept = 0;
			virtual void add_ref() const noexcept = 0;
			virtual void sub_ref() const noexcept = 0;
			virtual TickPriority tick_layout() = 0;
			virtual TickPriority tick_priority() = 0;
			virtual TickOrder tick_order(const TypeLayout&) = 0;
			virtual void rw_property(const TypeLayout*& storage, const RWProperty*& property, const size_t*& count) const noexcept = 0;
		};

		using SystemInterfacePtr = Tool::intrusive_ptr<SystemInterface>;

		struct TemplateSystemInterface
		{
			virtual void apply(Context*) noexcept = 0;
			virtual void add_ref() const noexcept = 0;
			virtual void sub_ref() const noexcept = 0;
		};

		using TemplateSystemInterfacePtr = Tool::intrusive_ptr<TemplateSystemInterface>;

		struct SystemPoolInterface
		{
			virtual SystemInterface* find_system(const TypeLayout& ti) noexcept = 0;
			virtual void regedit_system(SystemInterface*) noexcept = 0;
			virtual void destory_system(const TypeLayout& id) noexcept = 0;
			virtual void regedit_template_system(TemplateSystemInterface*) noexcept = 0;
		};

	}

	template<typename Type>
	struct SystemWrapper
	{
		static_assert(Implement::TypePropertyDetector<Type>::value, "SystemWrapper only accept Type and const Type!");
		Type* operator->() noexcept { return static_cast<Type*>(m_resource->data()); }
	private:
		SystemWrapper(Implement::SystemPoolInterface* pool) noexcept : m_pool(pool) {}
		static void export_rw_info(Implement::RWPropertyTuple& tuple) noexcept;
		bool lock() noexcept;
		void unlock()  noexcept { m_resource.reset(); }
		Implement::SystemInterfacePtr m_resource;
		Implement::SystemPoolInterface* m_pool;
		template<typename ...Require> friend struct Implement::SystemStorage;
		template<typename Require> friend struct Implement::FilterAndEventAndSystem;
	};

	template<typename Type>
	void SystemWrapper<Type>::export_rw_info(Implement::RWPropertyTuple& tuple) noexcept
	{
		if constexpr (std::is_const_v<Type>)
			tuple.systems.emplace(TypeLayout::create<Type>(), Implement::RWProperty::Read);
		else
			tuple.systems[TypeLayout::create<Type>()] = Implement::RWProperty::Write;
	}

	template<typename Type> bool SystemWrapper<Type>::lock() noexcept
	{
		//Implement::SystemPoolInterface& SI = *in;
		m_resource = m_pool->find_system(TypeLayout::create<Type>());
		return m_resource;
	}


	namespace Implement
	{
		template<typename Type> struct GobalComponentStorage
		{
			bool lock() noexcept;
			void unlock()  noexcept { m_component = nullptr; }
			std::remove_reference_t<Type>* as_pointer()noexcept { return m_component; }
			static void export_rw_info(Implement::RWPropertyTuple& tuple) noexcept
			{
				if constexpr (std::is_const_v<Type> || std::is_same_v<Type, std::remove_reference<Type>>)
					tuple.gobal_components.emplace(TypeLayout::create<Type>(), Implement::RWProperty::Read);
				else
					tuple.gobal_components[TypeLayout::create<Type>()] = Implement::RWProperty::Write;
			}
			GobalComponentStorage(Context* in) noexcept : m_pool(*in) {}
		private:
			using PureType = std::remove_const_t<std::remove_reference_t<Type>>;
			std::remove_reference_t<Type>* m_component;
			Implement::GobalComponentPoolInterface* m_pool;
		};

		template<typename Type> bool GobalComponentStorage<Type>::lock() noexcept
		{
			m_component = m_pool->find<Type>();
			return m_component != nullptr;
		}

		template<typename Type> struct ContextStorage
		{
			bool lock() noexcept { return true; }
			void unlock() {  }
			Context* as_pointer() noexcept { return m_ref; }
			static void export_rw_info(Implement::RWPropertyTuple& tuple) noexcept{}
			ContextStorage(Context* input) noexcept : m_ref(input) {}
		private:
			using PureType = std::remove_const_t<Type>;
			static_assert(std::is_same_v<PureType, Context&>, "System require Parameter Should be \"Context&\" but not \"Context\"");
			Context* m_ref = nullptr;
		};

		template<typename Type> struct FilterAndEventAndSystem
		{
			bool lock() noexcept { return m_storage.lock(); }
			void unlock() noexcept { m_storage.unlock(); }
			std::remove_reference_t<Type>* as_pointer() noexcept { return &m_storage; }
			static void export_rw_info(Implement::RWPropertyTuple& tuple) noexcept { PureType::export_rw_info(tuple); }
			FilterAndEventAndSystem(Context* in) noexcept : m_storage(*in) {}
		private:
			using PureType = std::remove_const_t<std::remove_reference_t<Type>>;
			static_assert(
				std::is_reference_v<Type>,
				"System require Parameter Like Event And Filter should be \'Type&\' or \'const Type&\' bug not \'Type\'"
				);
			PureType m_storage;
		};

		template<typename T> struct IsContext : std::false_type {};
		template<> struct IsContext<Context> : std::true_type {};

		template<typename T> struct IsFilterOrEventOrSystem : std::false_type {};
		template<typename ...T> struct IsFilterOrEventOrSystem<Filter<T...>> : std::true_type {};
		template<typename ...T> struct IsFilterOrEventOrSystem<EntityFilter<T...>> : std::true_type {};
		template<typename T> struct IsFilterOrEventOrSystem<EventProvider<T>> : std::true_type {};
		template<typename T> struct IsFilterOrEventOrSystem<EventViewer<T>> : std::true_type {};
		template<typename T> struct IsFilterOrEventOrSystem<SystemWrapper<T>> : std::true_type {};

		template<size_t index, typename InputType> struct SystemStorageDetectorImp { using Type = GobalComponentStorage<InputType>; };
		template<typename InputType> struct SystemStorageDetectorImp<1, InputType> { using Type = ContextStorage<InputType>; };
		template<typename InputType> struct SystemStorageDetectorImp<2, InputType> { using Type = FilterAndEventAndSystem<InputType>; };

		template<typename InputType> struct SystemStorageDetector
		{
			using PureType = std::remove_const_t<std::remove_reference_t<InputType>>;
			using Type = typename SystemStorageDetectorImp<
				IsContext<PureType>::value ? 1 : (IsFilterOrEventOrSystem<PureType>::value ? 2 : 0),
				InputType
			>::Type;
		};

		template<size_t start, size_t end> struct SystemStorageImp
		{
			template<typename Tuple>
			static bool lock(Tuple& tuple) noexcept
			{
				bool re = std::get<start>(tuple).lock();
				bool re2 = SystemStorageImp<start + 1, end>::lock(tuple);
				return re && re2;
			}
			template<typename Tuple>
			static void unlock(Tuple& tuple) noexcept
			{
				std::get<start>(tuple).unlock();
				SystemStorageImp<start + 1, end>::unlock(tuple);
			}
		};

		template<size_t end> struct SystemStorageImp<end, end>
		{
			template<typename Tuple>
			static bool lock(Tuple& tuple) noexcept
			{ return true; }
			template<typename Tuple>
			static void unlock(Tuple& tuple) noexcept {}
		};

		template<typename ...Require>
		struct SystemRWInfo
		{
			SystemRWInfo();
			void rw_property(const TypeLayout*& layout, const RWProperty*& property, const size_t*& index) const noexcept
			{
				layout = m_rw_type.data(); property = m_rw_property.data(); index = m_index.data();
			}
		private:
			std::vector<TypeLayout> m_rw_type;
			std::vector<RWProperty> m_rw_property;
			std::array<size_t, 4> m_index;
		};

		template<typename ...Type> struct SystemRWInfoImp { void operator()(Implement::RWPropertyTuple& tuple) {} };
		template<typename Cur, typename ...Type> struct SystemRWInfoImp<Cur, Type...> { void operator()(Implement::RWPropertyTuple& tuple) {
			SystemStorageDetector<Cur>::Type::export_rw_info(tuple);
			SystemRWInfoImp<Type...>{}(tuple);
		} };

		template<typename ...Require> SystemRWInfo<Require...>::SystemRWInfo()
		{
			Implement::RWPropertyTuple tuple;
			SystemRWInfoImp<Require...>{}(tuple);
			m_index[0] = tuple.systems.size();
			m_index[1] = tuple.gobal_components.size();
			m_index[2] = tuple.components.size();
			m_index[3] = tuple.events.size();
			size_t total_size = m_index[0] + m_index[1] + m_index[2] + m_index[3];
			m_rw_type.reserve(total_size);
			m_rw_property.reserve(total_size);
			for (auto& ite : tuple.systems)
			{
				m_rw_type.push_back(ite.first);
				m_rw_property.push_back(ite.second);
			}
			for (auto& ite : tuple.gobal_components)
			{
				m_rw_type.push_back(ite.first);
				m_rw_property.push_back(ite.second);
			}
			for (auto& ite : tuple.components)
			{
				m_rw_type.push_back(ite.first);
				m_rw_property.push_back(ite.second);
			}
			for (auto& ite : tuple.events)
			{
				m_rw_type.push_back(ite);
				m_rw_property.push_back(RWProperty::Write);
			}
		}

		template<size_t index> struct CopyContext
		{
			auto operator()(Context* in) { return std::tuple_cat(CopyContext<index - 1>{}(in), std::make_tuple(in)); }
		};

		template<> struct CopyContext<0>
		{
			std::tuple<> operator()(Context* in) { return std::tuple<>{}; }
		};

		template<typename ...Requires> struct SystemStorage
		{
			template<typename System>
			void apply(System& sys, Context* con) noexcept
			{
				if (SystemStorageImp<0, sizeof...(Requires)>::lock(m_storage))
					std::apply([&](auto && ... ai) { sys(*ai.as_pointer()...); }, m_storage);
				SystemStorageImp<0, sizeof...(Requires)>::unlock(m_storage);
			}
			static SystemRWInfo<Requires...> rw_info;
			SystemStorage(Context* in) : m_storage(CopyContext<sizeof...(Requires)>{}(in)) {}
		private:
			std::tuple<typename SystemStorageDetector<Requires>::Type...> m_storage;
		};

		template<typename ...Requires> SystemRWInfo<Requires...> SystemStorage<Requires...>::rw_info;
	}

	namespace Implement
	{

		template<typename SystemT, typename = std::void_t<>>
		struct TickLayoutDetector
		{
			TickPriority operator()(SystemT& sys) const { return TickPriority::Normal; }
		};

		template<typename SystemT>
		struct TickLayoutDetector<SystemT, std::void_t<std::enable_if_t<std::is_same_v<typename Tmp::function_type_extractor<decltype(&SystemT::tick_layout)>::pure_type, TickPriority()>>>>
		{
			TickPriority operator()(SystemT& sys) const { return sys.tick_layout(); }
		};

		template<typename SystemT, typename = std::void_t<>>
		struct TickPriorityDetector
		{
			TickPriority operator()(SystemT& sys) const { return TickPriority::Normal; }
		};

		template<typename SystemT>
		struct TickPriorityDetector<SystemT, std::void_t<std::enable_if_t<std::is_same_v<typename Tmp::function_type_extractor<decltype(&SystemT::tick_priority)>::pure_type, TickPriority()>>>>
		{
			TickPriority operator()(SystemT& sys) const { return sys.tick_priority(); }
		};

		template<typename SystemT, typename = std::void_t<>>
		struct TickOrderDetector
		{
			TickOrder operator()(SystemT& sys, const TypeLayout& id) const { return TickOrder::Undefine; }
		};

		template<typename SystemT>
		struct TickOrderDetector<SystemT, std::enable_if_t<std::is_same_v<typename Tmp::function_type_extractor<decltype(&SystemT::tick_order)>::pure_type, TickOrder(const TypeLayout&)>>>
		{
			TickOrder operator()(SystemT& sys, const TypeLayout& id) const { return sys.tick_order(id); }
		};

		template<typename SystemT, typename = std::void_t<>>
		struct PreTickDetector
		{
			void operator()(SystemT& sys, Context& c) const {}
		};

		template<typename SystemT>
		struct PreTickDetector<SystemT, std::enable_if_t<std::is_same_v<typename Tmp::function_type_extractor<decltype(&SystemT::pre_tick)>::pure_type, void(Context&)>>>
		{
			void operator()(SystemT& sys, Context& c) const { sys.pre_tick(c); }
		};

		template<typename SystemT, typename = std::void_t<>>
		struct PosTickDetector
		{
			void operator()(SystemT& sys, Context& c) const {}
		};

		template<typename SystemT>
		struct PosTickDetector<SystemT, std::enable_if_t<std::is_same_v<typename Tmp::function_type_extractor<decltype(&SystemT::pos_tick)>::pure_type, void(Context&)>>>
		{
			void operator()(SystemT& sys, Context& c) const { sys.pos_tick(c); }
		};

		template<typename Type> struct SystemImplement : SystemInterface
		{
			virtual void* data() noexcept { return &m_storage; }
			using PureType = std::remove_const_t<std::remove_reference_t<Type>>;
			using ParameterType = Tmp::function_type_extractor<PureType>;
			using AppendType = typename ParameterType::template extract_parameter<SystemStorage>;
			virtual void apply(Context* con) noexcept override { 
				PreTickDetector<Type>{}(m_storage, *con);
				m_append_storgae.apply(m_storage, con); 
				PosTickDetector<Type>{}(m_storage, *con);
			}
			virtual void add_ref() const noexcept { m_ref.add_ref(); };
			virtual void sub_ref() const noexcept {
				if (m_ref.sub_ref())
					m_deconstructor(this);
			}
			virtual const TypeLayout& layout() const noexcept override { return TypeLayout::create<Type>(); }
			template<typename ...Parameter> SystemImplement(Context* in, void (*deconstructor)(const SystemImplement<Type>*) noexcept, Parameter&& ... para);
			virtual TickPriority tick_layout() override { return TickLayoutDetector<PureType>{}(m_storage); }
			virtual TickPriority tick_priority() override { return TickPriorityDetector<PureType>{}(m_storage); }
			virtual TickOrder tick_order(const TypeLayout& layout) override { return TickOrderDetector<PureType>{}(m_storage, layout); };
			virtual void rw_property(const TypeLayout*& storage, const RWProperty*& property, const size_t*& count) const noexcept override { 
				AppendType::rw_info.rw_property(storage, property, count);
			}
			operator std::remove_reference_t<Type>& () noexcept { return m_storage; }
		private:
			//virtual std::type_index id() const noexcept { return typeid(Type); }
			mutable Tool::atomic_reference_count m_ref;
			PureType m_storage;
			AppendType m_append_storgae;
			void (*m_deconstructor)(const SystemImplement<Type>*) noexcept;
		};

		template<typename Type> template<typename ...Parameter>
		SystemImplement<Type>::SystemImplement(Context* in, void (*deconstructor)(const SystemImplement<Type>*) noexcept, Parameter&& ... para)
			: m_storage(std::forward<Parameter>(para)...), m_append_storgae(in), m_deconstructor(deconstructor)
		{
			assert(m_deconstructor != nullptr);
		}

	}

	struct Context
	{
		Entity create_entity() { return Entity{ create_entity_imp() }; }
		template<typename CompT, typename ...Parameter> std::remove_reference_t<std::remove_const_t<CompT>>& create_component(Entity entity, Parameter&& ...p);
		template<typename CompT, typename ...Parameter> std::remove_reference_t<std::remove_const_t<CompT>>& create_gobal_component(Parameter&& ...p);
		template<typename SystemT, typename ...Parameter> std::remove_reference_t<std::remove_const_t<SystemT>>& create_system(Parameter&& ...p);
		template<typename SystemT> void create_system(SystemT&& p);
		template<typename SystemT> void destory_system();
		template<typename CompT> bool destory_component(Entity entity);
		template<typename CompT> void destory_gobal_component();
		void destory_entity(Entity entity) { assert(entity); entity.m_imp->remove_all(*this->operator PO::ECS::Implement::ComponentPoolInterface *()); }
		virtual void exit() noexcept = 0;
		virtual float duration_s() const noexcept = 0;
	private:
		template<typename CompT> friend struct Implement::FilterAndEventAndSystem;
		template<typename CompT> friend struct Implement::GobalComponentStorage;
		template<typename CompT> friend struct Implement::ContextStorage;
		virtual operator Implement::ComponentPoolInterface* () = 0;
		virtual operator Implement::GobalComponentPoolInterface* () = 0;
		virtual operator Implement::EventPoolInterface* () = 0;
		virtual operator Implement::SystemPoolInterface* () = 0;
		virtual Implement::EntityInterfacePtr create_entity_imp() = 0;
	};

	template<typename CompT> void Context::destory_gobal_component()
	{
		Implement::GobalComponentPoolInterface* GPI = *this;
		GPI->destory_gobal_component(TypeLayout::create<CompT>());
	}

	template<typename CompT, typename ...Parameter> std::remove_reference_t<std::remove_const_t<CompT>>& Context::create_component(Entity entity, Parameter&& ...p)
	{
		Implement::ComponentPoolInterface* cp = *this;
		assert(entity);
		return cp->construction_component<CompT>(entity.m_imp, std::forward<Parameter>(p)...);
	}

	template<typename CompT> bool Context::destory_component(Entity entity)
	{
		Implement::ComponentPoolInterface* cp = *this;
		assert(entity);
		return cp->deconstruct_component(entity.m_imp, TypeLayout::create<CompT>());
	}

	template<typename CompT, typename ...Parameter> std::remove_reference_t<std::remove_const_t<CompT>>& Context::create_gobal_component(Parameter&& ...p)
	{
		Implement::GobalComponentPoolInterface* cp = *this;
		auto result = Implement::GobalComponentImp<CompT>::create(std::forward<Parameter>(p)...);
		cp->regedit_gobal_component(result);
		return *result;
	}

	template<typename SystemT, typename ...Parameter> std::remove_reference_t<std::remove_const_t<SystemT>>& Context::create_system(Parameter&& ...p)
	{
		Implement::SystemPoolInterface* SI = *this;
		Tool::intrusive_ptr<Implement::SystemImplement<SystemT>> ptr = new Implement::SystemImplement<SystemT>{this, [](const Implement::SystemImplement<SystemT> * in) noexcept {delete in; }, std::forward<Parameter>(p)... };
		SI->regedit_system(ptr);
		return *ptr;
	}

	template<typename SystemT> void Context::create_system(SystemT&& p)
	{
		Implement::SystemPoolInterface* SI = *this;
		Tool::intrusive_ptr<Implement::SystemImplement<SystemT>> ptr = new Implement::SystemImplement<SystemT>{this, [](const Implement::SystemImplement<SystemT> * in) noexcept {delete in; }, std::forward<SystemT>(p) };
		SI->regedit_system(ptr);
	}

	template<typename SystemT> void Context::destory_system()
	{
		Implement::SystemPoolInterface* SI = *this;
		SI->destory_system(TypeLayout::create<SystemT>());
	}

}