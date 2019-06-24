#include "mscf.h"

using std::variant;
using std::holds_alternative;
using std::get;

namespace PO::Mscf
{

	namespace Compile
	{

		symbol_table::space::space(std::string space_name, const std::map<std::string, command_array>& table)
			: m_namespace(std::move(space_name))
		{
			id_table.push_back({});
			uint64_t index = 0;
			for (auto& ite : table)
			{
				instance_table.insert({ ite.first, index });
				pre_instance_exe.append(ite.second);
				pre_instance_exe.append(command{ cm_set{ index } });
				auto re = id_table.begin()->insert({ ite.first, {index, ValueProperty::Const} });
				assert(re.second);
				++index;
			}
			m_register_used = index;
		}

		uint64_t symbol_table::space::allocate_register()
		{
			if (!m_unused_local_register.empty())
			{
				auto re = *m_unused_local_register.rbegin();
				m_unused_local_register.pop_back();
				return re;
			}
			return m_register_used++;
		}

		variant<monostate, ip<value>, uint64_t> symbol_table::space::read_id(const std::string& name) const
		{
			assert(!id_table.empty());
			for (auto ite = id_table.rbegin(); ite != id_table.rend(); ++ite)
			{
				auto find = ite->find(name);
				if (find != ite->end())
				{
					auto& ref = get<0>(find->second);
					if (holds_alternative<ip<value>>(ref))
						return get<ip<value>>(ref);
					else
						return get<uint64_t>(ref);
				}
			}
			return {  };
		}

		void symbol_table::space::add_sub_scope()
		{
			id_table.push_back({});
		}

		void symbol_table::space::pop_sub_scope()
		{
			assert(id_table.size() >= 2);
			auto table = std::move(*id_table.rbegin());
			id_table.pop_back();
			for (auto& ite : table)
			{
				auto& data = get<0>(ite.second);
				if (holds_alternative<uint64_t>(data))
					m_unused_local_register.push_back(get<uint64_t>(data));
			}
		}

		void symbol_table::space::set_id(optional<ValueProperty> property, const std::string& name, const command_array& input, line_record record)
		{
			auto va = input.cast_const();
			if (!va)
				instance_exe.append(input);

			if (property)
			{
				assert(!id_table.empty());
				auto& ref = *id_table.rbegin();
				auto result = ref.find(name);
				if (result == ref.end())
				{
					if (va)
					{
						auto re = ref.insert({ name, {*va, *property} });
						assert(get<1>(re));
					}
					else {
						uint64_t register_used = allocate_register();
						instance_exe.append({ cm_set{ register_used }, record });
						auto re = ref.insert({ name, {register_used, *property} });
						assert(get<1>(re));
					}
					return;
				}
				else
					throw compile_error{ Error::IDAlreadyExist , record };
			}
			else {

				for (auto ite = id_table.rbegin(); ite != id_table.rend(); ++ite)
				{
					auto find_re = ite->find(name);
					if (find_re != ite->end())
					{
						if (get<1>(find_re->second) == ValueProperty::Normal)
						{
							auto& ref = get<0>(find_re->second);
							if (holds_alternative<uint64_t>(ref))
							{
								if (va)
								{
									m_unused_local_register.push_back(get<uint64_t>(ref));
									ref = *va;
								}
								else {
									instance_exe.append({ cm_set{ get<uint64_t>(ref) }, record });
								}
							}
							else {
								if(va)
									ref = *va;
								else
								{
									uint64_t register_used = allocate_register();
									instance_exe.append({ cm_set{ register_used }, record });
									ref = register_used;
								}
							}
							return;
						}
						else
							throw compile_error{ Error::EquateToConstValue , record };
					}
				}
			}
		}

		void symbol_table::space::insert_special_register(const command_array& array, SpecialRegister reg)
		{
			instance_exe.append(array);
			special_register.push_back(reg);
		}

		symbol_table::symbol_table(std::string current_path, mscf_manager& manager) : path(std::move(current_path)), manager_wrapper(manager){}

		variant<monostate, ip<value>, tuple<uint64_t, uint64_t>> symbol_table::read_id(const std::string& name)
		{
			assert(!m_space.empty());
			uint64_t index = 0;
			for (auto ite = m_space.rbegin(); ite != m_space.rend(); ++ite)
			{
				auto re = ite->read_id(name);
				if (holds_alternative<ip<value>>(re))
					return get<ip<value>>(re);
				else if (holds_alternative<uint64_t>(re))
					return tuple<uint64_t, uint64_t>{ get<uint64_t>(re), index };
				else
					++index;
			}
			return {};
		}

		ip<uncomplete> symbol_table::pop_space()
		{
			assert(!m_space.empty());
			auto ptr = new uncomplete{std::move(static_cast<uncomplete&>(*m_space.rbegin()))};
			return ptr;
		}

		ip<mscf> symbol_table::find(const std::string& target_path, line_record record)
		{
			auto re = manager_wrapper.find(path, target_path);
			if (re)
				return re;
			throw compile_error{Error::MscfNotExist, record};
		}
		
		namespace AST
		{

			OpeType translate_ope_ter(const ast_ter& input)
			{
				switch (input.symbol())
				{
				case T::Mulity: return OpeType::Mulity;
				case T::Divide: return OpeType::Divide;
				case T::Mod: return OpeType::Mod;
				case T::Add: return OpeType::Add;
				case T::Minus: return OpeType::Minus;
				case T::Small: return OpeType::Small;
				case T::SmallEqual: return OpeType::SmallEqual;
				case T::Big: return OpeType::Big;
				case T::BigEqual: return OpeType::BigEqual;
				case T::IsEqual: return OpeType::IsEqual;
				case T::NotEqual: return OpeType::NotEqual;
				case T::And: return OpeType::And;
				case T::Or: return OpeType::Or;
				default:
					assert(false);
					break;
				}
			}

			OpeType translate_ope(const ast& input)
			{
				switch (input.symbol())
				{
				case S::OpeLevel2: return OpeType::Not;
				case S::OpeLevel3:
				case S::OpeLevel4:
				case S::OpeLevel5:
				case S::OpeLevel6:
				case S::OpeLevel7:
				case S::OpeLevel8:
					return translate_ope_ter(input[0]);
				default:
					assert(false);
					return OpeType::Unknow;
				}
			}

			void ImportStatement(symbol_table& table, const ast& input)
			{
				assert(input.symbol() == S::ImportStatement);
				assert(input.size() == 4);
				auto record = input[0].cast_terminal()->record();
				auto& str = input[2].cast_terminal();
				auto result = table.find(str->id(), record);
				auto& ref = input[1].cast_terminal();
				command_array ca{ command{ value::create(result), str->record().merga(ref->record()) } };
				table.set_id(ValueProperty::Const, ref->id(), ca, record);
			}

			void Statement(symbol_table& table, const ast& input)
			{
				assert(input.symbol() == S::Statement);
				switch (input[0].cast().symbol())
				{
				case S::DefineStatement: return;
				case S::EquateStatement: return;
				case S::DefineCodeStatement: return;
				case S::DefineShaderStatement: return;
				case S::DefinePassStatement: return;
				case S::DefineMaterialStatement: return;
				default:
					assert(false);
					return;
				}
			}

			void Start(symbol_table& table, const ast& input)
			{
				assert(input.symbol() == S::Start);
				Start(table, input[0]);
				switch (input[0].cast().symbol())
				{
				case S::Statement:
					return Statement(table, input[1]);
				case S::ImportStatement:
					return ImportStatement(table, input[1]);
				default:
					assert(false);
				}
			}
		}

	}
}