#include "../po/tool/intrusive_ptr.h"
#include "mscf_syntax.h"
namespace PO::Mscf
{
	using std::variant;
	using std::optional;
	using std::monostate;
	using std::tuple;
	using line_record = Syntax::line_record;

	enum class Error
	{
		IDAlreadyExist,
		IDDoNotExist,
		EquateToConstValue,
		MscfNotExist,
	};

	enum class ShaderType
	{
		Vertex,
		VertexOut,
		Pixel,
		Compute,
	};

	enum class SpecialRegister
	{

	};

	struct compile_error { Error type; line_record record; };

	template<typename Type> using ip = Tool::intrusive_ptr<Type>;

	struct value : Tool::intrusive_object<value>
	{

	};

	struct type_description : Tool::intrusive_object<type_description>
	{
		virtual std::tuple<ip<type_description>, uint64_t> native_member(const std::string&);
		virtual std::tuple<ip<type_description>, uint64_t> ordered_member(uint64_t);
	private:
		ValueType type;
		uint64_t space_count;
	};

	struct value : Tool::intrusive_object<value>
	{
	protected:
		std::map<std::string, ip<value>> native_value;
		std::map<std::string, ip<value>> export_value;
		std::vector<ip<value>> ordered_native_value;
	};






	

	struct code;
	struct shader;
	struct pass;
	struct material;
	struct uncomplete;

	template<typename Type> struct value_imp;
	struct value : Tool::intrusive_object_base
	{
		virtual ip<value> read_property(const std::string& name);
		virtual ~value();
		virtual void release() noexcept override;
		template<typename Type> ip<value_imp<Type>> cast()
		{
			assert(is_type(typeid(Type)));
			return static_cast<value_imp<Type>*>(this);
		}
		virtual bool is_type(std::type_index) noexcept = 0;
	};

	template<typename Type> struct value_imp : value
	{
		Type storage;
		operator Type&() { return storage; }
		Type& ref() { return storage; }
		virtual bool is_type(std::type_index ti) noexcept override { return ti == typeid(Type); }
	};

	struct list
	{
		std::vector<ip<value>> data;
	};

	struct list;
	using list_ptr = Tool::intrusive_ptr<list>;

	struct value : Tool::intrusive_object<value>
	{
		variant<
			std::monostate, bool, int64_t, float, std::string, list,
			ip<code>, ip<shader>, ip<pass>, ip<material>, ip<uncomplete>
		> data;
		template<typename T> value(T&& t) : data(std::forward<T>(t)) {}
		value(const value&) = default;
		value(value&&) = default;
		template<typename T> static ip<value> create(T&& t) { return new value{std::forward<T>(t)}; }
	};

	struct list
	{
		std::vector<ip<value>> data;
	};

	struct export_table
	{
		std::map<std::string, value> module_export_table;
		std::map<std::string, value> file_export_table;
	};

	struct code : export_table, Tool::intrusive_object<code>
	{
		std::string execute_code;
		std::vector<ip<code>> include_code;
	};

	struct shader : export_table, Tool::intrusive_object<shader>
	{
		ShaderType type;
		ip<code> code_ref;
		std::vector<std::string> enter_function;
		std::map<std::string, std::string> define;
	};

	struct pass : export_table, Tool::intrusive_object<pass>
	{
		std::array<ip<shader>, 2> used_shader;
		std::tuple<std::string, std::vector<std::string>> inout_syntax;
	};

	struct material : export_table, Tool::intrusive_object<material> {};

	struct mscf : export_table, Tool::intrusive_object<mscf> {};

	namespace Compile
	{
		enum class OpeType { Not, Small, SmallEqual, Big, BigEqual, IsEqual, NotEqual, Add, Minus, Divide, Mulity, Mod, And, Or, Unknow};
		struct cm_operator { OpeType type; };
		struct cm_read { std::string index; };
		struct cm_instance { ip<uncomplete> uncomplete; };
		struct cm_set { uint64_t index; };
		struct cm_make_list { uint64_t element_count; };
		struct cm_false_shift { uint64_t command_shift; };
		struct cm_shift { uint64_t command_shift; };

		struct command {
			variant<ip<value>, cm_operator, cm_read, cm_instance, cm_set, cm_make_list, cm_false_shift, cm_shift> storage;
			line_record record;
		};

		struct command_array
		{
			command_array& append(const command_array&);
			command_array& append(command);
			command_array(command com) : m_command({ std::move(com) }) {}
			command_array(std::initializer_list<command> com) : m_command(com) {}
			command_array() = default;
			command_array(command_array&&) = default;
			command_array(const command_array&) = default;
			size_t size() const noexcept { return m_command.size(); }
			bool is_dependence() const;
			optional<ip<value>> cast_const() const;
			//command_array dependence_instance(const &);
			//void execute(value_stack&);
			bool empty() const;
		private:
			std::vector<command> m_command;
		};
	}


	struct uncomplete : Tool::intrusive_object<uncomplete>
	{
		enum class Type
		{
			Vertex,
			VertexOut,
			Pixel,
			Compute,
			Pass,
			Code,
			Material,
		};

		Type type;
		Compile::command_array pre_instance_exe;
		Compile::command_array instance_exe;
		std::map<std::string, uint64_t> instance_table;
		std::vector<SpecialRegister> special_register;
		uint64_t used_register;
		std::map<std::vector<ip<value>>, variant<ip<code>, ip<shader>, ip<pass>, ip<material>>> exist_instance;
	};

	namespace Compile
	{

		enum class ValueProperty
		{
			Normal = 0,
			Const,
			ExportToModule,
			ExportToMscf,
		};

		struct mscf_manager
		{
			std::map<std::string, ip<mscf>> storaged_mscf;
			virtual ip<mscf> find(const std::string& current_path, const std::string& target_path );
		};

		struct symbol_table
		{
			struct space : uncomplete {

				space(std::string name, const std::map<std::string, command_array>& table);
				variant<monostate, ip<value>, uint64_t> read_id(const std::string& name) const;
				void add_sub_scope();
				void pop_sub_scope();
				void set_id(optional<ValueProperty> is_define, const std::string& name, const command_array& array, line_record record);
				void insert_special_register(const command_array& array, SpecialRegister reg);
			private:
				std::vector<std::map<std::string, tuple<variant<ip<value>, uint64_t>, ValueProperty>>> id_table;
				std::vector<uint64_t> m_unused_local_register;
				uint64_t m_register_used;
				std::string m_namespace;
				uint64_t allocate_register();
			};
			symbol_table(std::string current_path, mscf_manager& manager);
			variant<monostate, ip<value>, tuple<uint64_t, uint64_t>> read_id(const std::string& name);
			void set_id(optional<ValueProperty> is_define, const std::string& name, const command_array& array, line_record record) {
				assert(!m_space.empty());
				m_space.rbegin()->set_id(is_define, name, array, record);
			}
			ip<uncomplete> pop_space();
			void push_space(std::string name, const std::map<std::string, command_array>& table) { m_space.push_back({ std::move(name), table }); }
			void add_sub_scope() { assert(!m_space.empty()); m_space.rbegin()->add_sub_scope(); }
			void pop_sub_scope() { assert(!m_space.empty()); m_space.rbegin()->pop_sub_scope(); }
			void insert_special_register(const command_array& array, SpecialRegister reg) { assert(!m_space.empty()); m_space.rbegin()->insert_special_register(array, reg); }
			ip<mscf> find(const std::string& target_path, line_record record);
		private:
			mscf_manager& manager_wrapper;
			std::string path;
			std::vector<space> m_space;
		};

		namespace AST
		{
			using S = Mscf::Syntax::MscfSyntax;
			using T = Mscf::Syntax::MscfToken;
			using ast = Mscf::Syntax::ast_node;
			using ast_ter = Mscf::Syntax::ast_node_terminal;



			OpeType translate_ope(const ast& input);
			ShaderType translate_shader_type(const ast& input);
			command_array ValueExpresstion(symbol_table&, const ast& input);
			command_array OperateExpression(symbol_table&, const ast& input);
			std::tuple<command_array, uint64_t> CommaExpressionTuple(symbol_table&, const ast& input);
			command_array CallElement(symbol_table&, const ast& input);
			command_array FunctionCallExpression(symbol_table&, const ast& input);
			command_array ListExpression(symbol_table&, const ast& input);
			command_array EquateElement(symbol_table&, const ast& input);
			std::vector<std::string, command_array> CommaEquateElement(symbol_table&, const ast& input);
			std::map<std::string, command_array> InstanceExpression(symbol_table&, const ast& input);

			command_array Expression(symbol_table& table, const ast& input);

			
			void ImportStatement(symbol_table& table, const ast& input);
			void Statement(symbol_table& table, const ast& input);
			void Start(symbol_table& table, const ast& input);
		}

		
	}


	/*
	struct mscf_manager
	{
		std::map<std::filesystem::path, 
			std::tuple<Complie::mscf_execution, Complie::mscf_file>
		> m_execution;
	};

	struct mscf_manager_wrapper : Complie::mscf_manager_interface
	{
		std::filesystem::path m_source_path;
		mscf_manager& m_ref;
		const Complie::mscf_execution& find_execution(std::string);
		const Complie::mscf_file& find_file(std::string);
	};
	*/

	//Complie::mscf_file load_mscf_file(const std::filesystem::path& P);

}