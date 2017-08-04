#include "translate_ue4_custion_node.h"
#include <set>
#include <map>
#include <functional>
using namespace std;

bool is_character(char c) { return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z'; }
bool is_underline(char c) { return c == '_'; }
bool is_number(char c) { return c >= '0' && c <= '9'; }
const std::set<char> oper{ '!','@','#','$','%','^','&','*','(',')','-','+','=','[','{',']','}','|','\\', ':',';','\'','\"','<',',','>','.','?','.','/' };
bool is_operator(char c) { return oper.find(c) != oper.end(); }
bool is_linebreak(char c) { return c == '\n'; }
bool is_backspace(char c) { return c == ' '; }
bool is_unknow(char c) { return c < 0; }

template<typename State, typename InputValue> struct state_machine
{
	std::map<State, std::function<State(InputValue)>> mapping_function;
};


enum class CodeState
{
	Input,
	Name,
	Name2,
	Number,
	Number2,
	Operator,
	Control,
	Unknow,
	Finish,
};

std::map<CodeState, std::function<CodeState(char)>> mapping_function
{
	{ CodeState::Input, [](char c) -> CodeState {
	if (is_character(c) || is_underline(c)) return CodeState::Name;
	if (is_number(c)) return CodeState::Number;
	if (is_operator(c)) return CodeState::Operator;
	if (is_linebreak(c)) return CodeState::Control;
	if (is_backspace(c)) return CodeState::Control;
	if (is_unknow(c)) return CodeState::Unknow;
	return CodeState::Finish;
} },
{CodeState::Name, [](char c) -> CodeState {
	if (is_character(c) || is_underline(c) || is_number(c)) return CodeState::Name2;
	return CodeState::Finish;
}},
{ CodeState::Name2, [](char c) -> CodeState {
	if (is_character(c) || is_underline(c) || is_number(c)) return CodeState::Name2;
	return CodeState::Finish;
} },
{ CodeState::Number, [](char c) -> CodeState {
	if (is_number(c)) return CodeState::Number;
	if (c == '.') return CodeState::Number2;
	return CodeState::Finish;
} },
{ CodeState::Number2, [](char c) -> CodeState {
	if (is_number(c)) return CodeState::Number2;
	return CodeState::Finish;
} },
{ CodeState::Operator, [](char c) -> CodeState {
	//if (is_operator(c)) return CodeState::Operator;
	return CodeState::Finish;
} },
{ CodeState::Control, [](char c) -> CodeState {
	return CodeState::Finish;
} },
{ CodeState::Unknow, [](char c) -> CodeState {
	if (is_unknow(c)) return CodeState::Unknow;
	return CodeState::Finish;
} },
};

enum class Synatx
{
	None =0 ,
	PreDefine = 1,
	Define = 2,
	AsMarco = 3,
	MarcoParameter = 4,
	MarcoParameterEnd = 5,
	MarcoDescription = 6,
	MarcoDescriptionEnd = 7,
	MainFunction = 8,
	MainFunctionParameter = 9,
	MainFunctionParameterEnd = 10,
	MainFunctionDescription = 11,
	MainFunctionDescriptionEnd = 12,
};

const std::set<std::string> pro = { "in", "out" };

const std::set<std::string> paramter =
{
	"float", "float2", "float3", "float4", "Texture2D", "uint", "uint2", "uint3", "uint4", "int", "int2", "int3", "int4", "void"
};



void handle_string(ostream& o, const std::string& c, const std::string& main_function)
{
	static Synatx state = Synatx::None;
	static int count = 0;
	static bool get_scription = false;
	
	switch (state)
	{
	case Synatx::None:
		if (c == "#") state = Synatx::PreDefine;
		else if (paramter.find(c) != paramter.end()) state = Synatx::Define;
		break;
	case Synatx::PreDefine:
		if (c == "\n") state = Synatx::None;
		break;
	case Synatx::Define:
		if (c == "ASMACRO") {
			o << "#define ";
			state = Synatx::AsMarco;
		}
		else if (c == main_function) state = Synatx::MainFunction;
		else if( c != " " && c!="\n" && c != "\\")state = Synatx::None;
		break;
	case Synatx::AsMarco:
		if (c == "(") {
			o << "(";
			state = Synatx::MarcoParameter;
		}
		else if (c != "\n")
			o << c;
		break;
	case Synatx::MarcoParameter:
		if (c == ")") {
			o << ") ";
			state = Synatx::MarcoParameterEnd;
		}
		else if (c != "\\" && c != "\n" && c != " " && paramter.find(c) == paramter.end() && pro.find(c) == pro.end())
		{
			if (c == ",")
				o << ", ";
			else
				o << c;
		}
		break;
	case Synatx::MarcoParameterEnd:
		if (c == "{")
		{
			o << "{";
			if(count == 0)
				state = Synatx::MarcoDescription;
			count++;
		}
		break;
	case Synatx::MarcoDescription:
		if (c != "\\" && c != "\n")
		{
			if (c == "}")
			{
				o << "}";
				if (count == 1)
				{
					state = Synatx::None;
					o << endl;
				}
				--count;
			}
			else
			{
				if (c == "{")
					++count;
				o << c;
			}
		}
		
		break;
	case Synatx::MainFunction:
		if (c == "(")
		{
			if (!get_scription)
				o << "/*";
			state = Synatx::MainFunctionParameter;
		}
		break;
	case Synatx::MainFunctionParameter:
		if (c == ")")
		{
			if(!get_scription)
				o << "*/" << endl;
			state = Synatx::MainFunctionParameterEnd;
			get_scription = true;
			
		}
		else
			if(!get_scription)
				o << c;
		break;
	case Synatx::MainFunctionParameterEnd:
		if (c == "{")
		{
			if (count == 0)
				state = Synatx::MainFunctionDescription;
			count++;
		}
		else if (c == ";")
			state = Synatx::None;
		break;
	case Synatx::MainFunctionDescription:
		
		if (c == "}")
		{
			--count;
			if (count == 0)
			{
				state = Synatx::MainFunctionDescriptionEnd;
				o << endl;
			}
			else
				o << c;
		}
		else {
			if (c == "{")
				++count;
			o << c;
		}
		break;
	case Synatx::MainFunctionDescriptionEnd:
		state = Synatx::None;
	default:
		break;
	}
	cout << c <<" \t" << static_cast<int>(state) << endl;
}


bool translate_ue4_node(const std::string& file_name, const std::string& main_function, const std::string& output)
{
	ifstream io(file_name);
	ofstream io2(output);
	if (io.is_open() && io2.is_open())
	{
		CodeState State = CodeState::Input;
		char buffer[2048] = "\0";
		int buffer_count = 0;
		while (io.good())
		{
			char character;
			if (State == CodeState::Finish)
			{
				buffer[buffer_count] = '\0';
				std::string out = buffer;
				buffer_count = 0;
				handle_string(io2, out, main_function);
				State = CodeState::Input;
			}
			else
				io.read(&character, 1);
			State = mapping_function[State](character);
			if (State != CodeState::Finish)
			{
				buffer[buffer_count++] = character;
			}
		}
		if (buffer_count != 0)
		{
			buffer[buffer_count] = '\0';
			std::string out = buffer;
			handle_string(io2, out, main_function);
		}
		return true;
	}
	else
		return false;
}