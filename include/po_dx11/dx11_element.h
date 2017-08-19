#pragma once
#include "dx11_frame.h"
#include <typeindex>
#include <set>
#include <memory>
namespace PO
{
	namespace Dx11
	{

		bool add_shader_path(std::type_index ti, const std::u16string& path);
		template<typename T> bool add_shader_path(const std::u16string& s) { return add_shader_path(typeid(T), s); }

		namespace Implement
		{
			class base_interface
			{
				std::type_index id_info;
			public:
				base_interface(std::type_index ti);
				virtual ~base_interface();
				std::type_index id() const { return id_info; }
				template<typename T> bool is() const { return id_info == typeid(T); }
				template<typename T> T& cast() { return static_cast<T&>(*this); }
				template<typename Function> bool cast(Function&& f)
				{
					using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<Function>>::type>;
					static_assert(funtype::size == 1, "only receive one parameter");
					using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
					static_assert(std::is_base_of<base_interface, true_type>::value, "need derived form base_interface.");
					if (is<true_type>())
						return (f(cast<true_type>()), true);
					return false;
				}
			};
		}

		class property_interface : public Implement::base_interface
		{
		protected:
			std::function<void(stage_context& p)> update_function;
		public:
			using Implement::base_interface::base_interface;
			void update(stage_context& p);
		};

		class property_mapping
		{

			std::map<std::type_index, std::shared_ptr<property_interface>> mapping;
			friend struct element_implement;

			template<typename T>
			struct check
			{
				using funtype = Tmp::pick_func<typename Tmp::degenerate_func<Tmp::extract_func_t<T>>::type>;
				static_assert(funtype::size == 1, "only receive one parameter");
				using true_type = std::decay_t<typename funtype::template out<Tmp::itself>::type>;
				static_assert(std::is_base_of<property_interface, true_type>::value, "need derived form property_interface.");
			};

			template<typename T> using check_t = typename check<T>::true_type;


		public:

			bool insert(std::shared_ptr<property_interface> sp);

			void clear();

			template<typename F> bool find_property(F&& f)
			{
				auto ite = mapping.find(typeid(check_t<F>));
				if (ite != mapping.end())
				{
					auto ptr = ite->second;
					return (f(ptr->cast<check_t<F>>()), true);
				}
				return false;
			}

			template<typename F, typename ...AT> void create_property(F&& f, AT&& ... at)
			{
				auto ptr = std::make_shared<check_t<F>>(std::forward<AT>(at)...);
				mapping.insert({ ptr->id(), ptr });
				f(*ptr);
			}

			template<typename F, typename ...AT> bool create_property_if_not_existing(F&& f, AT&& ... at)
			{
				auto ite = mapping.find(typeid(check_t<F>));
				if (ite != mapping.end())
				{
					auto ptr = ite->second;
					f(ptr->cast<check_t<F>>());
					return false;
				}
				else {
					auto ptr = std::make_shared<check_t<F>>(std::forward<AT>(at)...);
					mapping.insert({ ptr->id(), ptr });
					f(*ptr);
					return true;
				}
			}

			template<typename F> property_mapping& operator<<(F&& f) 
			{
				using type = check_t<F>;
				static constexpr bool value = std::is_constructible_v<type>;
				if constexpr (value)
					create_property_if_not_existing(f);
				else
					find_property(f);
				return *this;
			}

			template<typename T> bool have() const { return have(typeid(T)); }
			bool have(std::type_index) const;

			template<typename F> void for_each(F&& func)
			{
				for (auto& f : mapping) func(f);
			}

			template<typename F> void remove_if(F&& func)
			{
				for (auto ite = mapping.begin(); ite != mapping.end();)
				{
					if (func(*ite->second))
						mapping.erase(ite++);
					else
						++ite;
				}
			}

			template<typename F> bool find(std::type_index ti, F&& f)
			{
				auto ite = mapping.find(ti);
				if (ite != mapping.end())
				{
					f(*(ite->second));
					return true;
				}
				return false;
			}
			
			void update(stage_context& p);
		};

		struct property_mapping_list
		{
			property_mapping& pm;

		};

		namespace Implement
		{
			class stage_interface : public base_interface
			{
			public:
				using Implement::base_interface::base_interface;
				using acceptance_t = std::set<std::type_index>;
			protected:
				property_mapping default_mapping;
				virtual bool update(property_interface&, stage_context&);

				template<typename ...AT> struct make_acceptance
				{
					operator const acceptance_t&() const { 
						static const acceptance_t acceptance{ typeid(AT)... };
						return acceptance; 
					}
				};

				static bool check_implmenet(const std::type_index& ti, const property_mapping& pm) { return pm.have(ti); }
				template<typename T, typename ...AT> static bool check_implmenets(const std::type_index& ti, const T& t, const AT& ...at) { return check_implmenet(ti, t) && check_implmenets(ti, at...); }
				static bool check_implmenets(const std::type_index& ti) { return true; }

				bool update_acceptance_implement(stage_context& p, const std::type_index&, property_mapping& pm);
				template<typename T, typename ...AT> bool update_acceptance_implements(pipeline& p, const std::type_index& ti, T& t, AT&... pm)
				{
					return update_acceptance_implement(p, ti, t) && update_acceptance_implements(p, to, pm...);
				}
				bool update_acceptance_implements(stage_context& p, const std::type_index& ti) { return true; }

			public:

				template<typename ...AT> bool update_implement(pipeline& pi, AT& ...pm)
				{
					for (const auto& ite : acceptance())
					{
						update_acceptance_implements(pi, ite, pm..., default_mapping);
					}
				}

				operator const property_mapping& () const { return default_mapping; }

				template<typename ...AT> bool check_acceptance(const AT& ...at)
				{
					for (const auto& ite : acceptance())
						if (!check_implmenets(ite, at..., default_mapping))
							return false;
					return true;
				}

				template<typename ...AT> acceptance_t lack_acceptance(const AT&& ...at)
				{
					acceptance_t temporary;
					for (const auto& ite : acceptance())
						if (!check_implmenets(ite, at..., default_mapping))
							temporary.insert(ite);
					return temporary;
				}

				virtual auto acceptance() const -> const acceptance_t&;
				virtual void init(creator&) = 0;
			};

			class pipeline_interface
			{
				std::type_index type_info;
			public:
				pipeline_interface(const std::type_index& ti);
				const std::type_index& pipeline_id() const { return type_info; }
				virtual ~pipeline_interface();
			};

		}

		class placement_interface : public Implement::stage_interface
		{
			std::u16string path;
		protected:
			friend class geometry_interface;
			vertex_stage stage_vs;
			bool load_vs(std::u16string path, creator& c);
		public:
			using Implement::stage_interface::stage_interface;
			virtual void apply(stage_context&);
			const vertex_stage& vs()const { return stage_vs; }
		};

		class geometry_interface : public Implement::stage_interface
		{
		protected:
			input_assember_stage stage_ia;
			raterizer_state stage_rs;
		public:
			using Implement::stage_interface::stage_interface;
			virtual void apply(stage_context&);
			virtual void draw(stage_context&) = 0;
			const input_assember_stage& ia()const { return stage_ia; }
		};

		class material_interface : public Implement::stage_interface , public Implement::pipeline_interface
		{
			std::u16string path;
		protected:
			pixel_stage stage_ps;
			blend_state stage_bs;
			bool load_ps(std::u16string p, creator& c);
		public:
			material_interface(const std::type_index& material_type, const std::type_index& pipeline_type = typeid(Implement::pipeline_interface));
			virtual void apply(stage_context&);
		};

		class compute_interface : public Implement::stage_interface
		{
			std::u16string path;
		protected:
			compute_stage stage_cs;
			bool load_cs(std::u16string p, creator& c);
		public:
			using Implement::stage_interface::stage_interface;
			virtual void apply(stage_context&);
			virtual void dispath(stage_context& p) = 0;
		};

		namespace Implement
		{

			struct element_compute_implement
			{
				std::shared_ptr<compute_interface> compute_ptr;
				property_mapping mapping;

				operator bool() const { return static_cast<bool>(compute_ptr); }

				void clear_unused_property();
				void clear_property() { mapping.clear(); }
				void clear() { mapping.clear(); compute_ptr.reset(); }

				template<typename ...PropertyMapping> bool dispatch(pipeline& p, PropertyMapping&... property_mapping)
				{
					if (compute_ptr)
						if (compute_ptr->update_implement(p, property_mapping))
							return (compute_ptr->dispath(p), true);
					return false;
				}

				element_compute_implement& operator= (std::shared_ptr<compute_interface> sp) { compute_ptr = std::move(sp); }
				template<typename ...AT> bool check_acceptance(const AT& ... at) const
				{
					return compute_ptr ? compute_ptr->check_acceptance(at..., mapping) : true;
				}
				template<typename ...AT> stage_interface::acceptance_t lack_acceptance(const AT&... at) const
				{
					return compute_ptr ? compute_ptr->lack_acceptance(at...) : stage_interface::acceptance_t{};
				}

				element_compute_implement& operator= (std::shared_ptr<compute_interface> gp) { compute_ptr = std::move(gp); return *this; }
			};

			struct element_implement
			{
				std::shared_ptr<placement_interface> placemenet_ptr;
				std::shared_ptr<geometry_interface> geometry_ptr;
				std::shared_ptr<material_interface> material_ptr;

				input_layout layout;
				property_mapping mapping;

				void clear_unused_property();
				void clear_property() { mapping.clear(); }
				void clear() { mapping.clear(); placemenet_ptr.reset(); geometry_ptr.reset(); material_ptr.reset(); }

				element_implement& operator= (std::shared_ptr<geometry_interface> gp) { geometry_ptr = std::move(gp); return *this; }
				element_implement& operator= (std::shared_ptr<placement_interface> gp) { placemenet_ptr = std::move(gp); return *this; }
				element_implement& operator= (std::shared_ptr<material_interface> gp) { material_ptr = std::move(gp); return *this; }

				void update_layout(creator& c);

				const std::type_index& pipeline_id() const { material_ptr ? material_ptr->pipeline_id() : typeid(void); }

				template<typename ...AT> bool check_acceptance(const AT& ... at) const
				{
					return (placemenet_ptr ? placemenet_ptr->check_acceptance(at..., mapping) : true) &&
						(geometry_ptr ? geometry_ptr->check_acceptance(at..., mapping) : true) &&
						(material_ptr ? material_ptr->check_acceptance(at..., mapping) : true);
				}

				template<typename ...AT> stage_interface::acceptance_t lack_acceptance(const AT&... at) const
				{
					stage_interface::acceptance_t temporary = placemenet_ptr ? placemenet_ptr->lack_acceptance(at...) : stage_interface::acceptance_t{ };
					if (geometry_ptr)
					{
						auto tem = geometry_ptr->lack_acceptance(at...);
						temporary.insert(tem.begin(), tem.end());
					}
					if (material_ptr)
					{
						auto tem = material_ptr->lack_acceptance(at...);
						temporary.insert(tem.begin(), tem.end());
					}
					return temporary;
				}

				template<typename ...PropertyMapping> bool draw(pipeline& p, PropertyMapping&& ...property_mapping)
				{
					if (placemenet_ptr && geometry_ptr && material_ptr)
					{
						if (
							placemenet_ptr->update_implement(p, property_mapping...)
							&& geometry_ptr->update_implement(p, property_mapping...)
							&& material_ptr->update_implement(p, property_mapping...)
							)
						{
							placemenet_ptr->apply(p);
							geometry_ptr->apply(p);
							material_ptr->apply(p);
							geometry_ptr->draw(p);
							return true;
						}
					}
					return false;
				}

			};
		}

		class element
		{
			std::shared_ptr<Implement::element_implement> element_ptr;
			void check_ptr() {
				if (!element_ptr)
					element_ptr = std::make_shared<Implement::element_implement>();
			}
		public:
			template<typename ...PropertyMapping> bool draw(pipeline& p, PropertyMapping&& ...property_mapping) const { return element_ptr && element_ptr->draw(p, property_mapping...); }

			template<typename ...PropertyMapping> bool check_acceptance(const PropertyMapping& ... property_mapping) const { return element_ptr ? element_ptr->check_acceptance(property_mapping) : true; }
			template<typename ...PropertyMapping> Implement::stage_interface::acceptance lack_acceptance(const PropertyMapping& ... property_mapping) const { return element_ptr ? element_ptr->lack_acceptance(property_mapping) : Implement::stage_interface::acceptance{}; }

			template<typename T> element& operator= (std::shared_ptr<T> sp) { check_ptr(); *element_ptr = std::move(sp); return *this; }
			template<typename F> element& operator<< (F&& f) { check_ptr(); *element_ptr << f; return *this; }

			const std::type_index& pipeline_id() const { element_ptr ? element_ptr->pipeline_id() : typeid(void); }
		};

		class element_compute
		{
			std::shared_ptr<Implement::element_compute_implement> element_ptr;
			void check_ptr() {
				if (!element_ptr)
					element_ptr = std::make_shared<Implement::element_compute_implement>();
			}
		public:
			template<typename ...PropertyMapping> bool draw(pipeline& p, PropertyMapping&& ...property_mapping) const { return element_ptr && element_ptr->draw(p, property_mapping...); }

			template<typename ...PropertyMapping> bool check_acceptance(const PropertyMapping& ... property_mapping) const { return element_ptr ? element_ptr->check_acceptance(property_mapping) : true; }
			template<typename ...PropertyMapping> Implement::stage_interface::acceptance lack_acceptance(const PropertyMapping& ... property_mapping) const { return element_ptr ? element_ptr->lack_acceptance(property_mapping) : Implement::stage_interface::acceptance{}; }

			template<typename T> element& operator= (std::shared_ptr<T> sp) { check_ptr(); *element_ptr = std::move(sp); return *this; }
			template<typename F> element& operator<< (F&& f) { check_ptr(); *element_ptr << f; return *this; }
		};

		namespace Implement
		{
			class element_instance_store
			{
				std::unordered_map<std::type_index, std::weak_ptr<placement_interface>> placement_ptr;
				std::unordered_map<std::type_index, std::weak_ptr<geometry_interface>> geometry_ptr;
				std::unordered_map<std::type_index, std::weak_ptr<material_interface>> material_ptr;
				std::unordered_map<std::type_index, std::weak_ptr<compute_interface>> compute_ptr;
			public:

				template<typename T, typename K, typename ...AK> static std::shared_ptr<T> create_if_no_exist_implement(std::unordered_map<std::type_index, std::weak_ptr<K>>& ptr_map, AK&& ...at)
				{
					using type = std::decay_t<T>;
					static_assert(std::is_constructible_v<type, AK...>, "");
					static_assert(std::is_base_of_v<K, type>, "");
					auto ite = ptr_map.find(typeid(type));
					if (ite == ptr_map.end() || ite->second.expired())
					{
						auto ptr = std::make_shared<type>(std::forward<AT>(at)...);
						ptr_map.insert({ ptr->id, std::static_pointer_cast<K>(std::weak_ptr<type>{ ptr }) });
						return ptr;
					}
					else {
						return std::static_pointer_cast<type>(ite->second.lock());
					}
				}

				template<typename T, typename ...AT> std::shared_ptr<T> create_if_no_exist(AT&& ...at)
				{
					using type = std::decay_t<T>;
					static_assert(std::is_constructible_v<type, AT...>, "");
					static constexpr bool is_material = std::is_base_of_v<material_interface, type>;
					if constexpr(is_material)
						return create_if_no_exist_implement<type>(material_ptr, std::forward<AT>(at)...);
					else if constexpr (static constexpr bool is_geometry = std::is_base_of_v<geometry_interface, type>; is_geometry)
						return create_if_no_exist_implement<type>(geometry_ptr, std::forward<AT>(at)...);
					else if constexpr (static constexpr bool is_placement = std::is_base_of_v<placement_interface, type>; is_placement)
						return create_if_no_exist_implement<type>(geometry_ptr, std::forward<AT>(at)...);
					else if constexpr (static constexpr bool is_compute = std::is_base_of_v<compute_interface, type>; is_compute)
						return create_if_no_exist_implement<type>(compute_interface, std::forward<AT>(at)...);
					else
						static_cast(false, "");
				}

			};
		}


	}
}