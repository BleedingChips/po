#pragma once
#include "../po/tool/script_analyze.h"
#include "../po/tool/lr1.h"
#include <exception>
#include <string>
#include <filesystem>
#include <optional>
namespace PO::Mscf::Syntax
{
	enum class MscfToken
	{
		Empty, //

		//value
		Float,//
		Int, //
		String, //""
		Null, // null
		True, // true
		False, // false
		Id, // id

		CommondStart,
		CommondEnd,

		//operator
		Colon, //:
		
		Comment, // //....
		Comma, //,
		SquareBracketsLeft, // [
		SquareBracketsRight, // ]
		ParenthesesLeft, // (
		ParenthesesRight, // )
		Semicolon, //;
		CurlyBracesLeft, //{
		CurlyBracesRight, // }
		IsEqual, // ==
		Equal, //=
		Point, // .
		SmallEqual, // <=
		Small, // <
		BigEqual, // >=
		Big, // >
		And, //&
		Or, // |
		Not, // !
		NotEqual, // !=
		//Percent, // %
		Question, // ?
		To, // ->
		Add, // +
		Minus, // -
		Mulity, // *
		Divide, // /
		Mod, // %

		//keyword
		Import, // import
		If, // if
		Else, // else
		Var, // var
		Const, // const

		// code
		CodeLine, // between @{ }@
		CodeEnd, //}@
		CodeStart, //@{
	};

	enum class MscfSyntax
	{
		Start,

		CallingParameter,
		Expression,

		DefineIDProperity,
		DefineIDStatement,
		ImportStatement,
		DefineStatement,

		ParametersInstanceElement,
		ParametersInstance,
		
		ElseStatementElement,
		IfStatementElement,
		IfStatement,
		SingleStatement,
		Statements,
	};

	namespace Error
	{
		struct unrecognized_token : std::runtime_error
		{
			std::string m_code;
			unrecognized_token(std::string code) noexcept;
			unrecognized_token(unrecognized_token&&) = default;
			unrecognized_token(const unrecognized_token&) = default;
		};

		struct code_block_unfinish : std::runtime_error
		{
			code_block_unfinish() noexcept;
		};

		struct syntax_error : std::logic_error {
			uint64_t m_line;
			uint64_t m_count;
			syntax_error(const char* L, uint64_t line = 0, uint64_t count = 0);
		};
	}

	struct line_record
	{
		std::tuple<uint64_t, uint64_t> start;
		std::tuple<uint64_t, uint64_t> end;
		line_record merga(const line_record& lr) const;
	};

	struct token_location
	{
		token_location(std::string id, line_record record) :
			m_id(std::move(id)), m_record(record) {}
		std::string id() const { return m_id; }
		line_record record() const { return m_record; }
	private:
		std::string m_id;
		line_record m_record;
	};

	using ast_node = PO::Syntax::ast_node<MscfSyntax, MscfToken>;
	using ast_node_terminal = PO::Syntax::ast_node_terminal<MscfToken>;

	std::optional<ast_node> load_mscf_ast_node(const std::filesystem::path& path);
}