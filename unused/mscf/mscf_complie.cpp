#include "mscf_complie.h"
#include <sstream>
#include <functional>

/*
using std::get;
using std::holds_alternative;
using std::visit;
using std::tuple;
using std::variant;
using std::array;
using std::optional;
using std::move;

namespace PO::Mscf::Complie
{

	

	

	namespace Implement
	{
		std::string to_string(const line_record& line)
		{
			return to_string(line.m_line) + " - " + to_string(line.m_charactor_count);
		}

		std::array<std::string, 18> value_type_string =
		{
			"null", "int", "bool", "float", "string", 
			"list", "register", "code", "code_template", "inout",
			"inout_template", "shader", "shader_template", "pass", "pass_template",
			"package", "package_template", "mscf_file"
		};

		std::string to_string(const value& index)
		{
			return value_type_string[index.index];
		}
	}

	std::optional<bool> try_cast_bool(const value& in)
	{
		return std::visit(Tool::overloaded{
			[](std::monostate) -> std::optional<bool> { return false; },
			[](int64_t index)-> std::optional<bool> { return index != 0; },
			[](bool value) -> std::optional<bool> { return value; },
			[](const str_ptr& value)-> std::optional<bool> { assert(value); return !value->empty(); },
			[](auto&) -> std::optional<bool> {
			return {};
		}
			}, in);
	}

	std::optional<bool> try_ope_small(const value& input, const value& input2)
	{
		return std::visit(Tool::overloaded{
			[](int64_t input, int64_t input2)-> std::optional<bool> { return input < input2; },
			[](int64_t input, float input2) -> std::optional<bool> { return input < input2; },
			[](float input, float input2) -> std::optional<bool> { return input < input2; },
			[](float input, int64_t input2) -> std::optional<bool> { return input < input2; },
			[](const auto& input, const auto& input2) -> std::optional<bool> {
			return {};
		}
			}, input, input2);
	}

	std::optional<bool> try_ope_equal(const value& input, const value& input2)
	{
		return std::visit(Tool::overloaded{
			[](const auto& input, const auto& input2) -> std::optional<bool> {
				if constexpr (std::is_same_v<Tmp::rm_rc_t<decltype(input)>, Tmp::rm_rc_t<decltype(input2)>>)
				{
					if constexpr (std::is_same_v < Tmp::rm_rc_t<decltype(input)>, std::monostate>)
						return true;
					else if constexpr (Tmp::is_one_of_v<Tmp::rm_rc_t<decltype(input)>, v_register, v_list>)
						return {};
					else
						return input == input2;
				}
				else {
					return {};
				}
			},
			[](const int64_t& input, const float& input2) -> std::optional<bool> {return static_cast<float>(input) == input2; },
			[](const float& input, const int64_t& input2) -> std::optional<bool> {return input == static_cast<float>(input2); },
			}, input, input2);
	}

	bool ope_not(const value& in, const std::string& path, line_record line)
	{
		auto re = try_cast_bool(in);
		if (re)
			return !*re;
		else
			throw Error::complie_error(Error::Type::OperatorUnSupportFormat, path, line, "operator! need a value with type bool, but provide:" + Implement::to_string(in));
	}

	bool ope_short_line(const value& in, const std::string& path, line_record line, bool and_or_or, bool left_or_right)
	{
		auto re = try_cast_bool(in);
		if (re)
			return *re;
		else
		{
			if (and_or_or)
			{
				if(left_or_right)
					throw Error::complie_error(Error::Type::OperatorUnSupportFormat, path, line, "operator && need left value with type bool, but provide:" + Implement::to_string(in));
				else
					throw Error::complie_error(Error::Type::OperatorUnSupportFormat, path, line, "operator && need right value with type bool, but provide:" + Implement::to_string(in));
			}
			else
			{
				if(left_or_right)
					throw Error::complie_error(Error::Type::OperatorUnSupportFormat, path, line, "operator || need left value with type bool, but provide:" + Implement::to_string(in));
				else
					throw Error::complie_error(Error::Type::OperatorUnSupportFormat, path, line, "operator || need right value with type bool, but provide:" + Implement::to_string(in));
			}
		}
	}

	bool ope_small(const value& in, const value& in2, const std::string& path, line_record line)
	{
		auto re = try_ope_small(in, in2);
		if (re)
			return *re;
		else
			throw Error::expression_error(path, line, "operator < unsupport value type", in.index(), in2.index());
	}

	bool ope_small_equal(const value& in, const value& in2, const std::string& path, line_record line)
	{
		auto re = try_ope_small(in, in2);
		auto re2 = try_ope_equal(in, in2);
		if (re && re2)
			return *re || *re2;
		else
			throw Error::expression_error(path, line, "operator <= unsupport value type", in.index(), in2.index());
	}

	bool ope_big(const value& in, const value& in2, const std::string& path, line_record line)
	{
		auto re = try_ope_small(in, in2);
		auto re2 = try_ope_equal(in, in2);
		if (re && re2)
			return !(*re || *re2);
		else
			throw Error::expression_error(path, line, "operator > unsupport value type", in.index(), in2.index());
	}

	bool ope_big_equal(const value& in, const value& in2, const std::string& path, line_record line)
	{
		auto re = try_ope_small(in, in2);
		if (re)
			return !(*re);
		else
			throw Error::expression_error(path, line, "operator >= unsupport value type", in.index(), in2.index());
	}

	bool ope_is_equal(const value& in, const value& in2, const std::string& path, line_record line)
	{
		auto re = try_ope_equal(in, in2);
		if (re)
			return (*re);
		else
			throw Error::expression_error(path, line, "operator == unsupport value type", in.index(), in2.index());
	}

	bool ope_not_equal(const value& in, const value& in2, const std::string& path, line_record line)
	{
		auto re = try_ope_equal(in, in2);
		if (re)
			return (*re);
		else
			throw Error::expression_error(path, line, "operator == unsupport value type", in.index(), in2.index());
	}

	bool ope_true_shift(const value& in, const std::string& path, line_record line, std::string ope)
	{
		auto re = try_cast_bool(in);
		if (re)
			return *re;
		else
			throw Error::expression_error(path, line, "operator " + ope + "need value type with bool", in.index());
	}

	command_array& command_array::append(const command_array& input)
	{
		m_command.insert(m_command.end(), input.m_command.begin(), input.m_command.end());
		return *this;
	}

	command_array& command_array::append(command command, line_record line)
	{
		m_command.push_back({ std::move(command), line });
		return *this;
	}

	bool command_array::is_dependence() const
	{
		for (auto& ite : m_command)
		{
			auto& ref = std::get<0>(ite);
			if (holds_alternative<cm_read_dependence>(ref) || holds_alternative<cm_instance_dependence>(ref))
				return true;
		}
		return false;
	}

	

	enum class TemplateType
	{
		Instanceed,
		Template,
		DependenceTemplate
	};

	TemplateType operator+(TemplateType t1, TemplateType t2)
	{
		switch (t1)
		{
		case TemplateType::DependenceTemplate:
			return TemplateType::DependenceTemplate;
		case TemplateType::Template:
			return t2 == TemplateType::DependenceTemplate ? TemplateType::DependenceTemplate : TemplateType::Template;
		case TemplateType::Instanceed:
			return t2;
		default:
			assert(false);
			return TemplateType::DependenceTemplate;
		}
	}

	bool code_template::check_dependence() const
	{
	}

	code_ptr code_template::try_instance(line_record record)
	{
		m_dependenced = false;
		if (m_command.size() == 0)
		{
			assert(m_register_used == 0);
			code_ptr p = std::make_shared<code>();
			for (size_t i = 0; i < m_include.size(); ++i)
			{
				if (holds_alternative<code_ptr>(m_include[i]))
					p->m_include.push_back(std::move(get<code_ptr>(m_include[i])));
			}
		}
		if (m_command.is_dependence())
		{
			m_dependenced = true;
			return {};
		}
	}
	*/

	



	/*
	void symbol_table::set_code(std::string name, code value)
	{
		m_code.push_back(std::move(value));
		set_id(name, { dt_code{ m_code.size(), 0 } }, true);
	}

	void symbol_table::insert_inout(std::string name, inout_execution code)
	{
		m_inout.push_back(std::move(code));
		set_id(name, { dt_inout{ m_inout.size(), 0 } }, true);
	}

	void symbol_table::insert_pass(std::string name, pass_execution execute)
	{
		m_pass.push_back(std::move(execute));
		set_id(name, { dt_pass{ m_pass.size(), 0 } }, true);
	}

	void symbol_table::push_space(std::string name)
	{
		assert(!m_space.empty());
		auto str = m_space.rbegin()->m_namespace;
		m_space.push_back({ std::move(str + "::" + name) });
	}

	void symbol_table::set_import(std::string name, std::string path)
	{
		m_import.push_back(std::move(path));
		set_id(std::move(name), { dt_import{m_import.size()} }, true);
	}

	void symbol_table::insert_shader(std::string name, shader_execution execute)
	{
		m_shader.push_back(std::move(execute));
		set_id(std::move(name), { dt_shader{m_shader.size()} }, true);
	}

	void symbol_table::insert_command(command_array command)
	{
		auto& top_com = m_space.rbegin()->m_command;
		append_vector(top_com, std::move(command));
	}

	void symbol_table::set_inout_layout(std::vector<std::tuple<std::string, command_array>> command)
	{
		auto top = m_space.rbegin();
		assert(top->m_instance_table.empty());
		std::map<std::string, uint64_t> m_layout;
		for (auto& ite : command)
		{
			append_vector(top->m_pre_instance_command, std::move(std::get<1>(ite)));
			uint64_t index = ++m_gobal_register_used;
			m_layout.insert({ std::move(std::get<0>(ite)), index });
			top->m_pre_instance_command.push_back(cm_set{ dt_gobal{index} });
		}
		top->m_instance_table = std::move(m_layout);
	}

	void symbol_table::insert_gobal_register(GobalRegisterType type)
	{
		m_gobal_register_type.push_back(type);
	}
	*/

	/*
	execution symbol_table::pop_space()
	{
		assert(!m_space.empty());
		execution temporary = std::move(static_cast<execution&>(*m_space.rbegin()));
		m_space.pop_back();
		return std::move(temporary);
	}

	data_storage symbol_table::read_id(const std::string& name)
	{
		auto top = m_space.rbegin();
		for (auto ite = top; ite != m_space.rend(); ++ite)
		{
			auto find = ite->m_id.find(name);
			if (find != ite->m_id.end())
			{
				auto& ref = find->second;

				if (ref.is_register())
				{
					if (ite != top)
					{
						if (ref.is<dt_local>())
						{
							ite->m_command.push_back({ ref });
							ite->m_unused_local_register.push_back(ref.cast<dt_local>().m_index);
							ref = dt_gobal{ ++m_gobal_register_used };
							ite->m_command.push_back({ cm_set{ref} });
						}
					}
				}
				return ref;
			}
		}
		assert(false);
		return {};
	}


	void symbol_table::set_id(std::string name, command_array command, bool is_const)
	{
		auto top = m_space.rbegin();
		auto name_ite = top->m_id.find(name);
		if (name_ite != top->m_id.end())
		{
			if (name_ite->second.is_define())
				assert(false);
		}
		if (command.size() == 1 && std::holds_alternative<data_storage>(command[0]))
		{
			auto& dt = std::get<data_storage>(command[0]);
			if (dt.is_const_value())
			{
				if (name_ite != top->m_id.end())
				{
					if (std::holds_alternative<dt_local>(name_ite->second))
						top->m_unused_local_register.push_back(name_ite->second.cast<dt_local>().m_index);
					name_ite->second = std::move(dt);
					return;
				}
				else {
					top->m_id.insert({ name, dt });
					return;
				}
			}
		}
		append_vector(top->m_command, std::move(command));
		if (name_ite == top->m_id.end() || !name_ite->second.is<dt_local>())
		{
			uint64_t local_index;
			if (!top->m_unused_local_register.empty())
			{
				local_index = *top->m_unused_local_register.rbegin();
				top->m_unused_local_register.pop_back();
			}
			else {
				local_index = ++top->m_local_register_used;
			}
			name_ite = top->m_id.insert({ name, dt_local{local_index} }).first;
		}
		top->m_command.push_back(cm_set{ name_ite->second });
	}
	*/
	
/*
	command_array expression(symbol_table& table, const ast_node_t& ast);

	std::tuple<std::string, command_array> equation_expression(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::EquationExpression);
		return {
			ast[0].cast_terminal()->id(),
			expression(table, ast[2])
		};
	}

	auto mulity_erquation_expression(symbol_table& table, const ast_node_t& ast)
		-> std::vector<decltype(equation_expression(table, ast))>
	{
		assert(ast.symbol() == MscfSyntax::MulityEquationExpression);
		auto& ref = ast[0].cast();
		if (ref.symbol() == MscfSyntax::EquationExpression)
			return { equation_expression(table, ref) };
		else {
			auto res = mulity_erquation_expression(table, ref);
			res.push_back(equation_expression(table, ast[2]));
			return std::move(res);
		}
	}

	auto instance_expression(symbol_table& table, const ast_node_t& ast)
		-> decltype(mulity_erquation_expression(table, ast))
	{
		assert(ast.symbol() == MscfSyntax::InstanceExpreesion);
		if (ast.size() == 2)
			return { };
		else {
			assert(ast.size() == 3);
			return mulity_erquation_expression(table, ast[1]);
		}
	}

	command_array ref_expression(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::RefExpression);
		return std::visit(Tool::overloaded{ [&](const ast_node_ter_t& node) -> command_array {
			auto id = node->id();
			auto data = table.read_id(id);
			return { command{std::move(data)} };
		}, [&](const ast_node_t& node) -> command_array {
			auto result = ref_expression(table, node);
			auto& ref = ast[1];
			if (ref.is_terminal())
				result.push_back(cm_ref{ ast[2].cast_terminal()->id() });
			else {
				auto result2 = instance_expression(table, ref);
				for (auto& ite : result2)
				{
					result.push_back(std::move(std::get<0>(ite)));
					auto& com_list = std::get<1>(ite);
					append_vector(result, std::move(com_list));
				}
				result.push_back(cm_instance{ result2.size() });
			}
			return std::move(result);
		} }, ast[0].var());
	}

	command_array expression(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::Expression);
		if (ast.size() == 1)
		{
			return std::visit(Tool::overloaded{ [&](const ast_node_ter_t& node) -> command_array {
				switch (node.symbol())
				{
				case MscfToken::String:
					return { node->id()};
				case MscfToken::Float:
				{
					std::stringstream ss;
					ss << node->id();
					float data;
					ss >> data;
					return { data };
				}
				case MscfToken::IntegerDecimal:
				{
					std::stringstream ss;
					ss << node->id();
					int64_t data;
					ss >> data;
					return { data };
				}
				case MscfToken::True:
					return { true };
				case MscfToken::False:
					return { false };
				case MscfToken::Null:
					return { std::monostate{} };
				default:
					assert(false);
					return {};
				}
			}, [&](const ast_node_t& node) -> command_array {
				return ref_expression(table, node);
			} }, ast[0].var());
		}
		else if (ast.size() == 2)
		{
			auto result = expression(table, ast[1]);
			if (result.size() == 1 && result[0].is<data_storage>())
			{
				auto& ref = result[0].cast<data_storage>();
				if (ref.is_const_value())
				{
					return { cm_not::execute(ref) };
				}
			}
			result.push_back(cm_not{});
			return result;
		}
		else if (ast.size() == 3)
		{
			if (ast[0].is_terminal())
			{
				MscfToken token = ast[1].cast_terminal().symbol();
				auto re = expression(table, ast[0]);
				auto re2 = expression(table, ast[2]);
				if (is_const(re) && is_const(re2))
				{
					auto& v1 = get_const(re);
					auto& v2 = get_const(re2);
					switch (token)
					{
					case MscfToken::Small: return { cm_small::execute(v1, v2) };
					case MscfToken::SmallEqual: return { cm_small_equal::execute(v1, v2) };
					case MscfToken::Big: return { cm_big::execute(v1, v2) };
					case MscfToken::BigEqual: return { cm_big_equal::execute(v1, v2) };
					case MscfToken::IsEquate: return { cm_is_equal::execute(v1, v2) };
					case MscfToken::NotEquate: return { cm_not_equal::execute(v1, v2) };
					case MscfToken::And: return { cm_and::execute(v1, v2) };
					case MscfToken::Or: return { cm_or::execute(v1, v2) };
					default: assert(false);
					}
					return {};
				}
				else {
					append_vector(re, std::move(re2));
					switch (token)
					{
					case MscfToken::Small: re.push_back(cm_small{}); break;
					case MscfToken::SmallEqual: re.push_back(cm_small_equal{}); break;
					case MscfToken::Big: re.push_back(cm_big{}); break;
					case MscfToken::BigEqual: re.push_back(cm_big_equal{}); break;
					case MscfToken::IsEquate: re.push_back(cm_is_equal{}); break;
					case MscfToken::NotEquate: re.push_back(cm_not_equal{}); break;
					case MscfToken::And: re.push_back(cm_and{}); break;
					case MscfToken::Or: re.push_back(cm_or{}); break;
					default: assert(false);
					}
					return std::move(re);
				}
			}
		}
		else if (ast.size() == 5)
		{
			auto re = expression(table, ast[0]);
			auto re2 = expression(table, ast[2]);
			auto re3 = expression(table, ast[4]);
			if (is_const(re))
			{
				return std::visit([&](const auto& dt) -> command_array {
					return cast_bool(dt) ? std::move(re2) : std::move(re3);
				}, get_const(re).var());
			}
			else {
				re.push_back(cm_question{ re2.size() + 1 });
				append_vector(re, std::move(re2));
				re.push_back(cm_shift{ re3.size() });
				append_vector(re, std::move(re3));
				return std::move(re);
			}
		}
		else
			assert(false);
		return {};
	}

	std::tuple<command_array, uint64_t> include_expression_start(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::IncludeExpressionStart);
		auto& ref = ast[0].cast();
		if (ref.symbol() == MscfSyntax::IncludeExpressionStart)
		{
			auto result = include_expression_start(table, ref);
			auto ex_result = expression(table, ast[1]);
			append_vector(std::get<0>(result), std::move(ex_result));
			std::get<1>(result)++;
			return std::move(result);
		}
		else {
			auto ex_result = expression(table, ref);
			return { std::move(ex_result), 1 };
		}
	}

	std::tuple<command_array, uint64_t> include_expression(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::IncludeExpression);
		if (ast.size() == 0)
		{
			return { {}, 0 };
		}
		else {
			return std::visit(Tool::overloaded{ [&](const ast_node_t& node) -> std::tuple<command_array, uint64_t> {
				if (node.symbol() == MscfSyntax::IncludeExpressionStart)
				{
					auto result = include_expression_start(table, node);
					auto ex_result = expression(table, ast[2]);
					append_vector(std::get<0>(result), std::move(ex_result));
					std::get<1>(result)++;
					return std::move(result);
				}
				else {
					auto ex_result = expression(table, node);
					return { std::move(ex_result), 1 };
				}
			}, [&](const ast_node_ter_t& node) -> std::tuple<command_array, uint64_t> {
				return { {}, 0 };
			} }, ast[1].var());
		}
	}

	std::tuple<command_array, uint64_t, std::string> code_statement_element(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::CodeStatementElement);
		auto define = include_expression(table, ast[0]);
		auto code = ast[1].cast_terminal()->id();
		return { std::move(std::get<0>(define)), std::get<1>(define), std::move(code) };
	}

	void code_define_statement(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::CodeDefineStatement);
		auto name = ast[1].cast_terminal()->id();
		table.push_space(name);
		auto re = code_statement_element(table, ast[2]);
		table.insert_command(std::move(std::get<0>(re)));
		auto tem = table.pop_space();
		code_execution code;
		code.m_execution = std::move(tem);
		code.m_code = std::move(std::get<2>(re));
		code.m_include = std::get<1>(re);
		table.insert_code(std::move(name), std::move(code));
	}

	std::tuple<command_array, uint64_t> mulity_expression(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::MulityExpression);
		if (ast.size() == 1)
			return { expression(table, ast[0]), 1 };
		else {
			auto result = mulity_expression(table, ast[0]);
			auto result2 = expression(table, ast[2]);
			auto& ref = std::get<0>(result);
			append_vector(ref, std::move(result2));
			++std::get<1>(result);
			return std::move(result);
		}
	}

	std::tuple<command_array, uint64_t> dependence_expression(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::DependenceExpression);
		switch (ast.size())
		{
		case 3:
			return mulity_expression(table, ast[1]);
		case 0:
		case 2:
			break;
		default:
			assert(false);
			break;
		}
		return { {}, 0 };
	}

	void inout_define_statement(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::InoutStatement);
		auto name = ast[1].cast_terminal()->id();
		table.push_space(name);
		auto re = code_statement_element(table, ast[2]);
		table.insert_command(std::move(std::get<0>(re)));
		auto re2 = dependence_expression(table, ast[5]);
		table.insert_command(std::move(std::get<0>(re2)));
		execution des = table.pop_space();
		inout_execution tem;
		tem.m_execution = std::move(des);
		tem.m_code = std::move(std::get<std::string>(re));
		tem.m_include = std::get<uint64_t>(re);
		tem.m_export = ast[4].cast_terminal()->id();
		tem.m_fix_count = std::get<uint64_t>(re2);
		table.insert_inout(std::move(name), std::move(tem));
	}

	void statement(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::Statement);
		std::visit(Tool::overloaded{ [&](const ast_node_t& node) {
			switch (node.symbol())
			{
			case MscfSyntax::EquationExpression:
			{
				std::string id;
				command_array m_command;
				std::tie(id, m_command) = equation_expression(table, ast[0]);
				table.set_id(id, m_command);
				break;
			}
			case MscfSyntax::CodeDefineStatement:
				code_define_statement(table, node);
				break;
			case MscfSyntax::InoutStatement:
				inout_define_statement(table, node);
				break;
			default:
				assert(false);
				break;
			}
		}, [&](const ast_node_ter_t& node) {
			switch (node.symbol())
			{
			case MscfToken::Semicolon:
				break;
			default:
				assert(false);
				break;
			}
		} }, ast[0].var());
	}

	void import(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::ImportStatement);
		table.set_import(ast[2].cast_terminal()->id(), ast[1].cast_terminal()->id());
	}

	ShaderType shader_type_world(const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::ShaderTypeWord);
		switch (ast[0].cast_terminal().symbol())
		{
		case MscfToken::Vertex:
			return ShaderType::VS;
		case MscfToken::Pixel:
			return ShaderType::PS;
		default:
			assert(false);
			return ShaderType::AllType;
		}
	}

	void define_statement(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::DefineStatement);
		auto result = expression(table, ast[2]);
		table.insert_command(std::move(result));
	}

	uint64_t shader_define_statemenet_element(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::ShaderDefineStatementElement);
		if (ast.size() != 0)
		{
			auto result = shader_define_statemenet_element(table, ast[0]);
			auto& ref = ast[1].cast();
			switch (ref.symbol())
			{
			case MscfSyntax::Statement:
				statement(table, ref);
				return result;
			case MscfSyntax::DefineStatement:
				define_statement(table, ref);
				return result + 1;
				break;
			default:
				assert(false);
				break;
			}
		}
		return 0;
	}

	void nullable_instance_expression(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::NullableInstanceExpression);
		if (ast.size() == 0)
			return;
		else if (ast.size() == 1)
		{
			auto result = instance_expression(table, ast[0]);
			table.set_inout_layout(result);
		}
	}

	void define_shader_statement(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::DefineShaderStatement);
		auto name = ast[1].cast_terminal()->id();
		if (ast.size() == 4)
		{
			auto result = ref_expression(table, ast[2]);
			table.set_id(std::move(name), std::move(result));
		}
		else if (ast.size() == 13)
		{
			table.push_space(name);
			shader_execution result;
			result.m_type = shader_type_world(ast[0]);
			nullable_instance_expression(table, ast[2]);
			result.m_define = shader_define_statemenet_element(table, ast[4]);
			auto re = code_statement_element(table, ast[6]);
			result.m_code = std::move(std::get<std::string>(re));
			result.m_include = std::get<uint64_t>(re);
			table.insert_command(std::move(std::get<0>(re)));
			auto re2 = dependence_expression(table, ast[9]);
			table.insert_command(std::move(std::get<0>(re2)));
			result.m_enter = std::get<1>(re2);
			auto re3 = dependence_expression(table, ast[11]);
			table.insert_command(std::move(std::get<0>(re3)));
			result.m_enter = std::get<1>(re3);
			result.m_execution = std::move(table.pop_space());
			table.insert_shader(std::move(name), std::move(result));
		}
		else
			assert(false);
	}

	void output_statement(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::OutputStatement);
		auto result = expression(table, ast[3]);
		auto id = ast[1].cast_terminal()->id();
		table.set_id(id, std::move(result));
		auto id_re = table.read_id(id);
		table.insert_command({ id, id_re });
	}

	uint64_t define_pass_statement_element(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::DefinePassStatementElement);
		if (ast.size() == 0)
		{
			return 0;
		}
		else if (ast.size() == 2)
		{
			auto index = define_pass_statement_element(table, ast[0]);
			auto& ref = ast[1].cast();
			switch (ref.symbol())
			{
			case MscfSyntax::DefineShaderStatement:
				define_shader_statement(table, ref);
				break;
			case MscfSyntax::Statement:
				define_statement(table, ref);
				break;
			case MscfSyntax::OutputStatement:
				output_statement(table, ref);
				index += 1;
			}
			return index;
		}
		else
			assert(false);
		return 0;
	}

	std::vector<ShaderType> define_pass_statement_element_pos(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::DefinePassStatementElementPos);
		if (ast.size() == 2)
		{
			auto result1 = define_pass_statement_element_pos(table, ast[0]);
			auto result2 = define_pass_statement_element_pos(table, ast[1]);
			append_vector(result1, std::move(result2));
			return std::move(result1);
		}
		else {
			auto result = ref_expression(table, ast[2]);
			ShaderType type = shader_type_world(ast[0]);
			return { type };
		}
	}

	void define_pass_statement(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::DefinePassStatement);
		auto name = ast[1].cast_terminal()->id();
		if (ast.size() == 4)
		{
			auto re = ref_expression(table, ast[2]);
			table.set_id(name, std::move(re));
		}
		else {
			pass_execution pass;
			table.push_space(name);
			nullable_instance_expression(table, ast[2]);
			pass.m_output = define_pass_statement_element(table, ast[4]);
			pass.m_shader = define_pass_statement_element_pos(table, ast[7]);
			pass.m_execution = table.pop_space();
			table.insert_pass(std::move(name), std::move(pass));
		}
	}

	void using_pass_statement(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::UsingPassStatement);
		auto commands = ref_expression(table, ast[1]);
		table.insert_command(std::move(commands));
		auto commands2 = expression(table, ast[2]);
		table.insert_command(std::move(commands2));
	}

	void start(symbol_table& table, const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::Start);
		if (ast.size() != 0)
		{
			start(table, ast[0]);
			auto& node = ast[1].cast();
			switch (node.symbol())
			{
			case MscfSyntax::Statement:
				statement(table, node);
				break;
			case MscfSyntax::DefineShaderStatement:
				define_shader_statement(table, node);
				break;
			case MscfSyntax::ImportStatement:
				import(table, node);
				break;
			case MscfSyntax::DefinePassStatement:
				define_pass_statement(table, node);
				break;
			case MscfSyntax::OutputStatement:
				output_statement(table, node);
				table.insert_gobal_register(GobalRegisterType::Output);
				break;
			case MscfSyntax::UsingPassStatement:
				using_pass_statement(table, node);
				table.insert_gobal_register(GobalRegisterType::Using);
				break;
			default:
				assert(false);
				break;
			}
		}
	}

	mscf_execution::mscf_execution(symbol_table&& symbol)
	{
		assert(symbol.m_space.size() == 1);
		static_cast<execution_storage&>(*this) = std::move(symbol);
		auto& space = *symbol.m_space.begin();
		m_command = std::move(space.m_command);
		m_local_register_used = std::move(space.m_local_register_used);
		m_gobal_register_used = symbol.m_gobal_register_used;
		m_gobal_register_type = std::move(symbol.m_gobal_register_type);
		m_export = std::move(space.m_id);
		for (auto& ite : m_export)
		{

		}
	}

	mscf_execution handle_mscf_ast(const ast_node_t& node)
	{
		symbol_table table;
		start(table, node);
		//return std::move(id_map);
		return std::move(table);
	}

	storage_array complie(const command_array& command, storage_array& gobal, uint64_t local, mscf_manager_interface& interface)
	{
		storage_array stack;
		storage_array local_register;
		local_register.resize(local);
		for (auto ite = command.begin(); ite != command.end(); ++ite)
		{
			std::visit(Tool::overloaded{
				[&](auto& input) {
				if constexpr (Tmp::is_one_of<Tmp::rm_rc_t<decltype(input)>,
					cm_small, cm_small_equal, cm_big, cm_big_equal, cm_not_equal,
					cm_and, cm_or, cm_is_equal
				>::value)
				{
					assert(stack.size() >= 2);
					auto v1 = std::move(*stack.rbegin()); stack.pop_back();
					auto v2 = std::move(*stack.rbegin()); stack.pop_back();
					stack.push_back(Tmp::rm_rc_t<decltype(input)>::execute(v1, v2));
				}
				else
					assert(false);
			},
				[&](const data_storage& input) {
				if (input.is<dt_local>())
					stack.push_back(local_register[input.cast<dt_local>().m_index]);
				else if (input.is<dt_gobal>())
					stack.push_back(gobal[input.cast<dt_gobal>().m_index]);
				else stack.push_back(input);
			},
				[&](const cm_ref& input) {assert(false); },
				[&](const cm_instance& input) {assert(false); },
				[&](const cm_set& input) {
				assert(!stack.empty());
				auto target = std::move(*stack.rbegin());
				stack.pop_back();
				auto& ref = input.m_data;
				if (ref.is<dt_local>())
					local_register[ref.cast<dt_local>().m_index] = std::move(target);
				else if (ref.is<dt_gobal>())
					gobal[ref.cast<dt_gobal>().m_index] = std::move(target);
				else assert(false);
			},
				[&](const cm_not& input) {
				assert(!stack.empty());
				*stack.rbegin() = cm_not::execute(std::move(*stack.rbegin()));
			},
				[&](const cm_question& input) {
				assert(!stack.empty());
				auto re = std::move(*stack.rbegin());
				stack.pop_back();
				assert(re.is<bool>());
				if (!re.cast<bool>())
					ite += input.m_data;
			},
				[&](const cm_shift& input) {
				ite += input.m_data;
			}
				}, ite->var());
		}
		return std::move(stack);
	}

	mscf_file complie(const mscf_execution& self, mscf_manager_interface& interface)
	{
		mscf_file m_total_file;
		for(auto se = self.)
		return std::move(m_total_file);
	}
	*/
//}