#include "mscf_complie.h"

/*
namespace PO::Mscf::Complie
{
	using std::get;
	using std::holds_alternative;
	using std::visit;
	using std::tuple;
	using std::variant;
	using std::array;
	using std::optional;
	using std::move;

	std::vector<variant<value, command_array>> element_multiply_expression(const ast_node_t& ast, complie_environment& env);

	variant<value, command_array> expression(const ast_node_t& ast, complie_environment& env)
	{
		using rt = variant<value, command_array>;
		assert(ast.symbol() == MscfSyntax::Expression);
		if (ast[0].is_terminal())
		{
			auto& ref = ast[0].cast_terminal();
			switch (ref.symbol())
			{
			case MscfToken::Id:
			{
				auto result = env.table().read_id(ref->id());
				if (result)
				{
					auto& val = std::get<0>(*result);
					if (is_register(val))
					{
						if (std::get<2>(*result) == 0)
							return val;
						else
							return command_array{ cm_read_dependence {std::get<1>(*result), cast<v_register>(val).m_index } };
					}
					else
						return val;
				}
				else
					throw Error::expression_error(env.current_path(), ref->record(), "unknow id : " + ref->id());
			}
			case MscfToken::Value_String:
				return std::make_shared<std::string>(
					std::string("//form : ") + Implement::to_string(ref->record()) + " \r\n " + ref->id()
					);
			case MscfToken::Value_Float:
				return Implement::cast_form_string<float>(ref->id());
			case MscfToken::Value_Int:
				return Implement::cast_form_string<int64_t>(ref->id());
			case MscfToken::Value_True:
				return value{ true };
			case MscfToken::Value_False:
				return value{ false };
			case MscfToken::Value_Null:
				return value{};
			case MscfToken::ParenthesesLeft:
				return expression(ast[1], env);
			case MscfToken::Not:
			{
				return std::visit(Tool::overloaded{
					[&](value val) -> rt {
					return ope_not(val, env.current_path(), ref->record());
				},
					[&](command_array val)->rt {
					val.append(cm_operator{ OpeType::Not }, ref->record());
					return std::move(val);
				}
					}, expression(ast[1], env));
			}
			case MscfToken::CurlyBracesLeft:
				if (ast.size() == 2)
					return value{ v_list{} };
				else if (ast.size() == 3)
				{
					auto re = element_multiply_expression(ast[1], env);
					bool is_const = true;
					for (auto& ite : re)
					{
						if (is<command_array>(ite))
						{
							is_const = false;
							break;
						}
					}
					if (is_const)
					{
						std::vector<value> temporary;
						for (auto& ite : re)
							temporary.push_back(std::move(cast<value>(ite)));
						return value{ v_list{std::move(temporary)} };
					}
					else {
						command_array temporary;
						for (auto& ite : re)
						{
							if (holds_alternative<value>(ite)) temporary.append(std::move(cast<value>(ite)));
							else temporary.append(std::move(cast<command_array>(ite)));
						}
						temporary.append(cm_make_list{ re.size() }, ast[0].cast_terminal()->record());
						return std::move(temporary);
					}
				}
				else {
					assert(false);
					return value{};
				}
			default:
				assert(false);
				return value{};
			}
		}
		else {
			if (ast.size() == 3)
			{
				auto& ref = ast[1].cast_terminal();
				auto symbol = ref.symbol();
				if (symbol == MscfToken::And || symbol == MscfToken::Or)
				{
					return std::visit(Tool::overloaded{
						[&](value i1, auto i2) -> rt {
						bool re = ope_short_line(il, env.current_path(), ref->record(), symbol == MscfToken::And, true);
						if (re && symbol == MscfToken::Or)
							return true;
						else if (!re && symbol == MscfToken::And)
							return false;
						else {
							if constexpr (std::is_same_v<Tmp::rm_cr_t<decltype(i2)>, value>)
							{
								return ope_short_line(i2, env.current_path(), ref->record(), symbol == MscfToken::And, false);
							}
							else {
								i2.append(cm_cast{ CastType::Bool }, ref->record());
								return std::move(i2);
							}
						}
					}, [&](command_array i1, auto i2) -> rt {
						uint64_t shift = 0;
						if constexpr (std::is_same_v<Tmp::rm_cr_t<decltype(i2)>, value>)
							shift = 1;
						else
							shift = i2.size();
						if (symbol == MscfToken::Or)
						{
							i1.append(cm_false_shift{ 2 }, ref->record())
								.append(value{ true })
								.append(cm_shift{ shift + 1 })
								.append(i2).append(cm_cast{CastType::Bool});
						}
						else {
							i1.append(cm_false_shift{ shift + 2 }, ref->record())
								.append(i2).append(cm_cast{ CastType::Bool })
								.append(cm_shift{ 1 })
								.append(value{ false });
						}
						if (symbol == MscfToken::Or)
							i1.append(cm_operator{ OpeType::Not }, ref->record());
						i1.append(cm_false_shift{ shift + 2 }, ref->record())
							.append(std::move(i2))
							.append(cm_cast{ CastType::Bool }, ref->record())
							.append(cm_shift{ 1 }, ref->record())
							.append(value{symbol == MscfToken::Or});
						return std::move(i1);
					}
						}, expression(ast[0], env), expression(ast[2], env));

				}
				else if (symbol != MscfToken::Point)
				{
					return std::visit(Tool::overloaded{
					[&](value i1, value i2) -> rt {
						switch (symbol)
						{
						case MscfToken::Small:
							return ope_small(i1, i2, env.current_path(), ref->record());
						case MscfToken::SmallEqual:
							return ope_small_equal(i1, i2, env.current_path(), ref->record());
						case MscfToken::Big:
							return ope_big(i1, i2, env.current_path(), ref->record());
						case MscfToken::BigEqual:
							return ope_big_equal(i1, i2, env.current_path(), ref->record());
						case MscfToken::IsEqual:
							return ope_is_equal(i1, i2, env.current_path(), ref->record());
						case MscfToken::NotEqual:
							return ope_not_equal(i1, i2, env.current_path(), ref->record());
						default:
							assert(false);
							return value{};
						}
					},
					[&](auto i1, auto i2) -> rt {
						command_array temporary;
						temporary.append(i1);
						temporary.append(i2);
						switch (symbol)
						{
						case MscfToken::Small:
							temporary.append(cm_operator{ OpeType::Small }, ref->record());
							break;
						case MscfToken::SmallEqual:
							temporary.append(cm_operator{ OpeType::SmallEqual }, ref->record());
							break;
						case MscfToken::Big:
							temporary.append(cm_operator{ OpeType::Big }, ref->record());
							break;
						case MscfToken::BigEqual:
							temporary.append(cm_operator{ OpeType::BigEqual }, ref->record());
							break;
						case MscfToken::IsEqual :
							temporary.append(cm_operator{ OpeType::IsEqual }, ref->record());
							break;
						case MscfToken::NotEqual:
							temporary.append(cm_operator{ OpeType::NotEqual }, ref->record());
							break;
						default:
							assert(false);
						}
						return std::move(temporary);
					} }, expression(ast[0], env), expression(ast[2], env));
				}
				else {
					// to do point
					assert(false);
					return value{};
				}
			}
			else if (ast.size() == 2)
			{
				// to do instance
				assert(false);
				return value{};
			}
			else if (ast.size() == 5)
			{
				auto re = expression(ast[0], env);
				auto re2 = expression(ast[2], env);
				auto re3 = expression(ast[3], env);
				return std::visit(Tool::overloaded{
					[&](value v) ->rt {
					if(ope_true_shift(v, env.current_path(), ast[1].cast_terminal()->record(), "?"))
						return std::move(re2);
					else
						return std::move(re3);
				},	[&](command_array ca) -> rt {
					auto record = ast[1].cast_terminal()->record();
					if (holds_alternative<value>(re2))
						ca.append(cm_false_shift{ 2 }, record)
							.append(std::move(cast<value>(re2)));
					else
						ca.append(cm_false_shift{ cast<command_array>(re2).size() + 1 }, record)
							.append(std::move(cast<command_array>(re2)));
					record = ast[3].cast_terminal()->record();
					if (holds_alternative<value>(re3))
						ca.append(cm_shift{ 1 }, record).append(std::move(cast<value>(re3)));
					else
						ca.append(cm_shift{ cast<command_array>(re3).size() }, record).append(std::move(cast<command_array>(re3)));
					return std::move(ca);
				}
					}, std::move(re));
			}
			else {
				assert(false);
				return value{};
			}
		}
	}

	std::vector<variant<value, command_array>>
		element_multiply_expression(const ast_node_t& ast, complie_environment& env)
	{
		using pre_rt = std::tuple<command_array, uint64_t>;
		using rt = variant<std::vector<value>, pre_rt>;
		assert(ast.symbol() == MscfSyntax::ElementMultiplyExpression);
		if (ast.size() == 1)
		{
			return { expression(ast[0], env) };
		}
		else if (ast.size() == 3)
		{
			auto re = element_multiply_expression(ast[0], env);
			re.push_back(expression(ast[2], env));
			return std::move(re);
		}
		else {
			assert(false);
			return {};
		}
	}

	std::tuple<std::string, variant<value, command_array>, line_record>
		expression_equation(const ast_node_t& ast, complie_environment& env)
	{
		assert(ast.symbol() == MscfSyntax::ExpressionEquation);
		auto id = ast[0].cast_terminal()->id();
		auto re = expression(ast[2], env);
		return { std::move(id),  std::move(re), ast[1].cast_terminal()->record() };
	}

	std::optional<command_array> operate_equate(const ast_node_t& ast, complie_environment& env)
	{
		assert(ast.symbol() == MscfSyntax::OperateEquate);
		bool is_const = ast[0].is_terminal();
		auto& ref = is_const ? ast[1].cast() : ast[0].cast();
		auto re = expression_equation(ref, env);
		auto& id = std::get<0>(re);
		auto& val = std::get<1>(re);
		auto& record = std::get<2>(re);

		if (holds_alternative<value>(val))
		{
			if (!env.table().set_id(std::get<0>(re), std::move(cast<value>(val)), is_const))
				throw Error::expression_error(std::get<2>(re), "id of operator = is const");
			return {};
		}
		else {
			auto read = env.table().read_top_id(std::get<0>(re));
			auto& ref = cast<command_array>(val);
			if (read)
			{
				if (!std::get<1>(*read))
				{
					if (holds_alternative<v_register>(std::get<0>(*read)))
					{
						uint64_t index = cast<v_register>(std::get<0>(*read)).m_index;
						ref.append(cm_set{ index }, record);
						return ref;
					}
				}
				else
					throw Error::complie_error(std::get<2>(re), "id of operator = is const");
			}
			uint64_t index = env.table().allocate_register();
			ref.append(cm_set{ index });
			env.table().set_id(std::move(std::get<0>(re)), v_register{ index }, is_const);
			return ref;
		}
		return {};
	}

	std::vector<variant<value, command_array>>
		element_call_operator(const ast_node_t& ast, complie_environment& env)
	{
		assert(ast.symbol() == MscfSyntax::ElementCallOperator);
		if (ast.size() == 3)
		{
			return element_multiply_expression(ast, env);
		}
		else if (ast.size() == 2)
		{
			return {};
		}
		else {
			assert(false);
			return {};
		}
	}

	std::vector<variant<value, command_array>>
		element_include_expression_s(const ast_node_t& ast, complie_environment& env)
	{
		assert(ast.symbol() == MscfSyntax::ElementIncludeExpressionS);
		if (ast.size() == 2)
			return { expression(ast[0], env) };
		else if (ast.size() == 3)
		{
			auto re = element_include_expression_s(ast[0], env);
			re.push_back(std::move(expression(ast[1], env)));
			return std::move(re);
		}
		else
		{
			assert(false);
			return {};
		}
	}

	std::vector<variant<value, command_array>>
		element_include(const ast_node_t& ast, complie_environment& env)
	{
		assert(ast.symbol() == MscfSyntax::ElementInclude);
		if (ast.size() == 3)
			return { expression(ast[1], env) };
		else if (ast.size() == 4)
		{
			auto re = element_include_expression_s(ast[1], env);
			re.push_back(expression(ast[2], env));
			return std::move(re);
		}
		else if (ast.size() == 2 || ast.size() == 0)
		{
			return {};
		}
		else
		{
			assert(false);
			return {};
		}
	}

	std::vector<std::tuple<std::string, variant<value, command_array>>>
		element_multiply_instance_expression(const ast_node_t& ast, complie_environment& env)
	{
		assert(ast.symbol() == MscfSyntax::ElementMultiplyInstanceExpression);
		if (ast.size() == 1)
		{
			auto e = expression_equation(ast[0], env);
			return { {std::move(std::get<0>(e)), std::move(std::get<1>(e))} };
		}
		else if (ast.size() == 3)
		{
			auto e = element_multiply_instance_expression(ast[0], env);
			auto e2 = expression_equation(ast[2], env);
			e.push_back({ std::move(std::get<0>(e2)), std::move(std::get<1>(e2)) });
			return std::move(e);
		}
		else {
			assert(false);
			return {};
		}
	}

	std::vector<std::tuple<std::string, variant<value, command_array>>>
		element_instance(const ast_node_t& ast, complie_environment& env)
	{
		assert(ast.symbol() == MscfSyntax::ElementInstance);
		if (ast.size() == 3)
			return element_multiply_instance_expression(ast[1], env);
		else if (ast.size() == 2)
			return {};
		else
		{
			assert(false);
			return {};
		}
	}

	std::vector<std::tuple<std::string, variant<value, command_array>>>
		element_nullable_instance(const ast_node_t& ast, complie_environment& env)
	{
		assert(ast.symbol() == MscfSyntax::ElementNullableInstance);
		if (ast.size() == 1)
			return element_instance(ast[0], env);
		else if (ast.size() == 0)
			return {};
		else {
			assert(false);
			return {};
		}
	}

	std::tuple<std::string, std::vector<variant<value, command_array>>>
		element_code_expression(const ast_node_t& ast, complie_environment& env)
	{
		assert(ast.symbol() == MscfSyntax::ElementCodeExpression);
		return { ast[0].cast_terminal()->id(), element_include(ast, env) };
	}

	std::tuple<std::vector<variant<value, command_array>>, line_record>
		operate_enter(const ast_node_t& ast, complie_environment& env)
	{
		assert(ast.symbol() == MscfSyntax::OperateEnter);
		return { element_call_operator(ast[1], env), ast[0].cast_terminal()->record() };
	}

	std::tuple<std::string, variant<value, command_array>, line_record>
		operate_define(const ast_node_t& ast, complie_environment& env)
	{
		assert(ast.symbol() == MscfSyntax::OperateDefine);
		auto line = ast[0].cast_terminal()->record();
		if (ast.size() == 4)
			return { ast[1].cast_terminal()->id(), expression(ast[3], env), line };
		else if (ast.size() == 2)
			return { std::string{}, expression(ast[3], env), line };
		else {
			assert(false);
			return { {}, {}, line };
		}
	}

	std::tuple<std::vector<variant<value, command_array>>, line_record>
		operate_inout(const ast_node_t& ast, complie_environment& env)
	{
		assert(ast.symbol() == MscfSyntax::OperateInout);
		return { element_call_operator(ast[1], env), ast[0].cast_terminal()->record() };
	}

	std::tuple<std::string, variant<value, command_array>, line_record>
		operate_output(const ast_node_t& ast, complie_environment& env)
	{
		assert(ast.symbol() == MscfSyntax::OperateOutput);
		return { ast[1].cast_terminal()->id(), expression(ast[3], env), ast[0].cast_terminal()->record() };
	}

	void allocate_register_for_command_array(std::vector<value>& output, command_array& output2, variant<value, command_array>&& v, construct_table& table)
	{
		visit(Tool::overloaded{
			[&](value&& input) {output.push_back(std::move(input)); },
			[&](command_array&& input) {
			uint64_t index = table.allocate_register();
			output2.append(std::move(input)).append(cm_set{index});
			output.push_back(v_register{ index });
		}
			}, std::move(v));
	}
	void allocate_register_for_command_array(std::vector<value>& output, command_array& output2, std::vector<variant<value, command_array>>&& v, construct_table& table)
	{
		for (auto& ite : v)
			allocate_register_for_command_array(output, output2, move(ite), table);
	}



	std::tuple<std::string, variant<code_ptr, code_template_ptr>, line_record>
		statement_define_code(const ast_node_t& ast, complie_environment& env)
	{
		assert(ast.symbol() == MscfSyntax::StatementDefineCode);
		auto id = ast[1].cast_terminal()->id();
		env.table().push_space(id, 0);
		auto re = element_code_expression(ast[2], env);
		code_template_ptr ptr = std::make_shared<code_template>();
		ptr->m_code = std::move(std::get<0>(re));
		allocate_register_for_command_array(ptr->m_include, ptr->m_command, move(get<1>(re)), env.table());
		auto space = env.table().pop_space();
		ptr->m_register_used = space.m_register_used;
		return { move(id), move(ptr), ast[0].cast_terminal()->record() };
	}

	std::tuple<std::string, inout_template_ptr, line_record>
		statement_define_inout(const ast_node_t& ast, complie_environment& env)
	{
		assert(ast.symbol() == MscfSyntax::StatementDefineInout);
		auto id = ast[1].cast_terminal()->id();
		env.table().push_space(id, 0);
		inout_template_ptr ptr = std::make_shared<inout_template>();
		auto re = element_code_expression(ast[2], env);
		ptr->m_code = std::move(get<0>(re));
		allocate_register_for_command_array(ptr->m_include, ptr->m_command, move(get<1>(re)), env.table());
		ptr->m_tag = ast[4].cast_terminal()->id();
		auto re2 = element_call_operator(ast[5], env);
		allocate_register_for_command_array(ptr->m_fix, ptr->m_command, move(re2), env.table());
		auto space = env.table().pop_space();
		ptr->m_register_used = space.m_register_used;
		return { move(id), move(ptr), ast[0].cast_terminal()->record() };
	}

	ShaderType type_shader(const ast_node_t& ast)
	{
		assert(ast.symbol() == MscfSyntax::TypeShader);
		switch (ast[0].cast_terminal().symbol())
		{
		case MscfToken::Vertex: return ShaderType::Vertex;
		case MscfToken::Pixel: return ShaderType::Pixel;
		case MscfToken::VertexOut: return ShaderType::VertexOut;
		default: assert(false); return ShaderType::Unknow;
		}
	}

	void statement_level1(const ast_node_t& ast, std::function<void(const ast_node_t&)> func)
	{
		assert(ast.symbol() == MscfSyntax::StatementLevel1);
		func(std::move(ast[0]));
	}

	void element_define_shader(const ast_node_t& ast, complie_environment& ev, shader_template& st)
	{
		assert(ast.symbol() == MscfSyntax::ElementDefineShader);
		assert(ast.size() == 2);
		switch (ast[0].cast().symbol())
		{
		case MscfSyntax::StatementLevel1:
			return statement_level1(ast[0], [&](const ast_node_t& input_ast) {
				switch (input_ast.symbol())
				{
				case MscfSyntax::OperateEquate:
					return operate_equate(input_ast, ev);
				case MscfSyntax::StatementDefineCode:

				}
			});
		case MscfSyntax::OperateDefine:
			return std::forward<decltype(func) && >(func)(ast[1]);
		case MscfSyntax::ElementDefineShader:
			element_define_shader(ast[0], ev, st);
			element_define_shader(ast[1], ev, st);
			return;
		default:
			assert(false);
		}
	}

	void nullable_element_define_shader()

		std::tuple<std::string, shader_template_ptr, line_record>
		statement_define_shader(const ast_node_t& ast, complie_environment& ev)
	{

	}
}
*/