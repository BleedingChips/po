#include "mscf_syntax.h"
#include "../po/tool/script_analyze.h"
#include "../po/tool/document.h"
#include "mscf_lr1_implement.h"
namespace PO::Mscf::Syntax
{
	namespace Error
	{
		unrecognized_token::unrecognized_token(std::string code) noexcept : std::runtime_error("unrecognized token"), m_code(std::move(code)) {}
		code_block_unfinish::code_block_unfinish() noexcept : std::runtime_error("define a code should start with code$ and end with $end, $end is missing") {}
		syntax_error::syntax_error(const char* L, uint64_t line, uint64_t count) : std::logic_error(L), m_line(line), m_count(count) {}
	}

	line_record line_record::merga(const line_record& lr) const
	{
		using std::get;
		line_record result;
		if (get<0>(start) > get<0>(lr.start))
			result.start = lr.start;
		else if (get<0>(start) == get<0>(lr.start))
		{
			get<0>(result.start) = get<0>(start);
			get<1>(result.start) = std::min(get<1>(start), get<1>(lr.start));
		}
		else
			result.start = start;

		if (get<0>(end) > get<0>(lr.end))
			result.end = end;
		else if (get<0>(end) == get<0>(lr.end))
		{
			get<0>(result.end) = get<0>(end);
			get<1>(result.end) = std::max(get<1>(end), get<1>(lr.end));
		}
		else
			result.start = lr.end;
		return result;
	}

	const PO::Lexical::regex_token<MscfToken, char>& mscf_token()
	{
		static PO::Lexical::regex_token<MscfToken, char> static_token{
		{R"(\s+)", MscfToken::Empty},

		//value
		{R"([\+\-]([0-9][1-9]*\.[0-9]+))", MscfToken::Value_Float},
		{R"([1-9]+[0-9]*)", MscfToken::Value_Int},
		{R"(true(?!\w))", MscfToken::Value_True},
		{R"(false(?!\w))", MscfToken::Value_False},
		{R"(null(?!\w))", MscfToken::Value_Null},

		//code
		{R"(\@\{)", MscfToken::CodeStart},

		//commond
		{R"(//.*)", MscfToken::Comment},
		{R"(/\*)", MscfToken::CommondStart},

		//operator
		{R"(\.)", MscfToken::Point},
		{R"(\,)", MscfToken::Comma},
		{R"(\:)", MscfToken::Colon},
		{R"(\[)", MscfToken::SquareBracketsLeft},
		{R"(\])", MscfToken::SquareBracketsRight},
		{R"(\()", MscfToken::ParenthesesLeft},
		{R"(\))", MscfToken::ParenthesesRight},
		{R"(;)", MscfToken::Semicolon},
		{R"(\{)", MscfToken::CurlyBracesLeft},
		{R"(\})", MscfToken::CurlyBracesRight},
		{R"(==)", MscfToken::IsEqual },
		{R"(=)", MscfToken::Equal},
		{R"(<=)", MscfToken::SmallEqual},
		{R"(<)", MscfToken::Small},
		{R"(>=)", MscfToken::BigEqual},
		{R"(>)", MscfToken::Big},
		{R"(\-\>)", MscfToken::To},
		{R"(\-)", MscfToken::Minus},
		{R"(\+)", MscfToken::Add},
		{R"(\*)", MscfToken::Mulity},
		{R"(\/)", MscfToken::Divide},
		{R"(\%)", MscfToken::Mod},
		{R"(\&\&)", MscfToken::And},
		{R"(\|\|)", MscfToken::Or},
		{R"(\!\=)", MscfToken::NotEqual},
		{R"(\!)", MscfToken::Not},
		
		
		//{R"(%)", MscfToken::Percent},
		{R"(\?)", MscfToken::Question},

		//keyword
		{R"(pass(?!\w))", MscfToken::Pass},
		{R"(import(?!\w))", MscfToken::Import},
		{R"(vertex(?!\w))", MscfToken::Vertex},
		{R"(vertex_out(?!\w))", MscfToken::Vertex},
		{R"(pixel(?!\w))", MscfToken::Pixel},
		{R"(compute(?!\w))", MscfToken::Compute},
		{R"(code(?!\w))", MscfToken::Code},
		{R"("[^"]*")", MscfToken::Value_String},
		{R"(material(?!\w))", MscfToken::Material},
		{R"(if(?!\w))", MscfToken::If},
		{R"(else(?!\w))", MscfToken::Else},
		{R"(const(?!\w))", MscfToken::Const},
		{R"(var(?!\w))", MscfToken::Var},
		// id
		{R"([a-zA-Z_]\w*)", MscfToken::Id}
		};
		return static_token;
	}

	const PO::Lexical::regex_token<MscfToken, char>& mscf_code_end_token()
	{
		static PO::Lexical::regex_token<MscfToken, char> static_token{
		{R"(.*\}\@(?!\w))", MscfToken::CodeEnd},
		};
		return static_token;
	}

	const PO::Lexical::regex_token<MscfToken, char>& mscf_code_command_end()
	{
		static PO::Lexical::regex_token<MscfToken, char> static_token{
		{R"(.*\*/)", MscfToken::CommondEnd },
		};
		return static_token;
	}

	std::optional<std::vector<std::tuple<MscfToken, token_location>>> load_mscf_token_stream(const std::filesystem::path& path)
	{
		Doc::loader<char> mscf_loader(path);

		if (mscf_loader.is_open())
		{
			uint64_t line_count = 0;
			std::vector<std::tuple<MscfToken, token_location>> result;
			std::string temporary;
			uint64_t code_state = 0;
			std::string code;
			line_record code_lr;
			bool need_input = true;
			decltype(temporary)::iterator start_ite;
			while (!mscf_loader.is_end_of_file() || !need_input)
			{
				if (need_input)
				{
					temporary.clear();
					temporary = mscf_loader.read_line(std::move(temporary));
					start_ite = temporary.begin();
					++line_count;
				}
				else
					need_input = true;

				switch (code_state)
				{
				case 0:
					PO::Lexical::regex_token_wrapper::generate(start_ite, temporary.end(), mscf_token(),
						[&](MscfToken token, std::string::iterator start, std::string::iterator end) {
						if (token == MscfToken::CodeStart)
						{
							//result.push_back({ token, std::unique_ptr<syntax_data>(new token_location{{}, line_count, static_cast<uint64_t>(start - temporary.begin())}) });
							//result.insert_keyword_token(token, line_count, static_cast<uint64_t>(start - temporary.begin()));
							code_state =  1;
							code.clear();
							code_lr.start = { line_count, static_cast<uint64_t>(start - temporary.begin()) };
							need_input = false;
							start_ite = end;
							return false;
						}
						else if (token == MscfToken::CommondStart)
						{
							code_state = 2;
							need_input = false;
							start_ite = end;
							return false;
						}
						else if (token != MscfToken::Empty && token != MscfToken::Comment)
						{
							line_record lr{ {line_count, static_cast<uint64_t>(start - temporary.begin())}, {line_count, static_cast<uint64_t>(end - temporary.begin())} };
							if (token == MscfToken::Value_String)
							{
								std::string tem{ start + 1, end - 1 };
								result.push_back({ token, token_location{std::move(tem), lr} });
								//result.insert_token(token, std::move(tem), line_count, static_cast<uint64_t>(start - temporary.begin()));
							}
							else if (token == MscfToken::Value_Float || token == MscfToken::Value_Int || token == MscfToken::Id)
							{
								std::string tem{ start, end };
								result.push_back({ token, token_location{std::move(tem), lr} });
								//result.insert_token(token, std::move(tem), line_count, static_cast<uint64_t>(start - temporary.begin()));
							}
							else {
								result.push_back({ token, token_location{{}, lr} });
								//result.insert_keyword_token(token, line_count, static_cast<uint64_t>(start - temporary.begin()));
							}
						}
						return true;
					}, [&](std::string::iterator start, std::string::iterator end) -> bool {
						std::string code = { start, end };
						throw Error::unrecognized_token{ std::move(code) };
						return false;
					});
					break;
				case 1:
					PO::Lexical::regex_token_wrapper::generate(start_ite, temporary.end(), mscf_code_end_token(),
						[&](MscfToken token, std::string::iterator start, std::string::iterator end) {
						if (token == MscfToken::CodeEnd)
						{
							code += {start, end - 2};
							code_lr.end = { line_count, static_cast<uint64_t>(end - temporary.begin()) };
							result.push_back({ MscfToken::CodeLine, token_location{std::move(code), code_lr} });
							code_state = 0;
							need_input = false;
							start_ite = end;
							return false;
						}
						return true;
					}, [&](std::string::iterator start, std::string::iterator end) -> bool {
						code += {start, end};
						code += "\r\n";
						return false;
					});
					break;
				case 2:
					PO::Lexical::regex_token_wrapper::generate(start_ite, temporary.end(), mscf_code_command_end(),
						[&](MscfToken token, std::string::iterator start, std::string::iterator end) {
						if (token == MscfToken::CommondEnd)
						{
							code_state = 0;
							need_input = false;
							start_ite = end;
							return false;
						}
						return true;
					}, [&](std::string::iterator start, std::string::iterator end) -> bool {
						return false;
					});
					break;
				default:
					break;
				}

			}
			if (code_state != 0)
				throw Error::code_block_unfinish{};
			return result;
		}
		return {};
	}

	const PO::Syntax::LR1<MscfSyntax, MscfToken>& mscf_lr1()
	{
		using S = MscfSyntax;
		using T = MscfToken;

		static PO::Syntax::LR1<S, T> mscf_lr1_implement(mscf_lr1_implement_data + 1, mscf_lr1_implement_data[0]);
		return mscf_lr1_implement;
	}

	std::optional<PO::Syntax::ast_node<MscfSyntax, MscfToken, token_location>> load_mscf_ast_node(const std::filesystem::path& path)
	{
		auto stream = load_mscf_token_stream(path);
		if (stream)
		{
			return PO::Syntax::generate_ast(mscf_lr1(), stream->begin(), stream->end());
		}
		return {};
	}
}