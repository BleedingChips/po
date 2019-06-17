#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>
#include "../po/tool/tool.h"
#include "../po/tool/intrusive_ptr.h"
#include "mscf_syntax.h"
#include "mscf_interface.h"
//#include "mscf_tool.h"

namespace PO::Mscf::Complie
{
	using std::variant;
	

	struct ref_type { uint64_t index; };

}













namespace PO::Mscf::PreComplie
{
	

	

	enum class RefType
	{
		Code,
		Shader,
		Pass,
		Material,
		UnComplete,
	};

	struct ref_type { RefType type; uint64_t index; };

	struct list;

	struct mscf_interface;

	using value = variant<std::monostate, bool, int64_t, float, std::string, ref_type, list, mscf_interface>;

	struct list { std::vector<value> data; };

	struct mscf_interface
	{
		std::map<std::string, mscf_interface> 
	};
}





namespace PO::Mscf
{


	namespace Complie
	{
		using std::variant;

		

		enum class StorageType
		{
			Null,
			Int,
			Bool,
			Float,
			String,
			Tuple,
			Map,
			Code,
			Shader,
			Pass,
			SubMaterial,
			Mscf,
			Uncomplete,
		};

		struct storage_ref
		{
			StorageType type;
			int64_t index;
		};

		std::optional<bool> try_cast_bool(storage_ref in);
		std::optional<bool> try_ope_small(storage_ref in, storage_ref in2);
		std::optional<bool> try_ope_equal(storage_ref in, storage_ref in2);

		bool ope_not(storage_ref in, line_record l1, line_record l2);
		bool ope_short_line(storage_ref in, line_record l1, line_record l2, bool and_or_or, bool left_or_right);
		bool ope_small(storage_ref in, storage_ref in2, line_record l1, line_record l2);
		bool ope_small_equal(storage_ref in, storage_ref in2, line_record l1, line_record l2);
		bool ope_big(storage_ref in, storage_ref in2, line_record l1, line_record l2);
		bool ope_big_equal(storage_ref in, storage_ref in2, line_record l1, line_record l2);
		bool ope_is_equal(storage_ref in, storage_ref in2, line_record l1, line_record l2);
		bool ope_not_equal(storage_ref in, storage_ref in2, line_record l1, line_record l2);
		bool ope_question(storage_ref in, storage_ref path, line_record l1, line_record l2);

		

		struct cm_read { uint64_t m_uplook; uint64_t m_index; };
		struct cm_instance_dependence { storage_ref data; };
		struct cm_instance { uint64_t m_index; };
		struct cm_set { uint64_t m_index; };
		enum class OpeType { Not, Small, SmallEqual, Big, BigEqual, IsEqual, NotEqual };
		struct cm_operator { OpeType m_type; };
		struct cm_make_list { uint64_t m_list; };
		struct cm_false_shift { uint64_t m_index; };
		struct cm_shift { uint64_t m_index; };

		using command = variant<
			storage_ref, cm_instance, cm_set, cm_operator, cm_make_list, cm_false_shift,
			cm_shift, cm_read, cm_instance_dependence
		>;

		using value_stack = std::vector<std::vector<storage_ref>>;

		

		struct uncomplate_struct
		{
			enum class StructType
			{
				CODE,
				VERTEX,
				VERTEXOUT,
				PIXEL,
				COMPUTE,
				PASS,
				SUB_MATERIAL
			};
			StructType type;
			std::vector<command> dependence_exe;
			std::vector<command> pre_instance_exe;
			std::vector<command> execution;
			uint64_t register_used;
			std::map<std::string, uint64_t> export_mapping;
			std::vector<uint64_t> special_register_used;
			std::vector<storage_ref> total_register;
		};

		struct code
		{
			std::string name_space;
			line_record record;
			std::string code;
			std::vector<storage_ref> include;
		};

		struct shader
		{
			std::string name_space;
			line_record record;
			std::map<std::string, std::string> define_map;
			std::string code;
			std::vector<storage_ref> include;
		};

		struct pass
		{
			std::string name_space;
			line_record record;
			std::map<std::string, std::string> define_map;
			std::map<std::string, storage_ref> export_id;
			std::array<uint64_t, 2> shader_used;
			std::tuple<std::string, std::vector<std::string>> inout_name;
		};
	}

}

/*
namespace PO::Mscf::Complie
{
	

	/*
	

	

	struct file_record
	{
		std::string path;
		uint64_t index;
	};

	struct code
	{
		file_record m_record;
		std::string m_code;
		std::vector<code_ptr> m_include;
	};

	struct complie_environment;

	struct code_template
	{
		file_record m_record;
		std::string m_code;
		std::vector<value> m_include;
		command_array m_command;
		uint64_t m_register_used;
		bool check_dependence() const;
		code_ptr dependence_instance(complie_environment& ce, value_stack& stack);
	};

	struct inout
	{
		file_record m_record;
		std::string m_code;
		std::vector<code_ptr> m_include;
		std::string m_tag;
		std::vector<std::string> m_fix;
	};

	struct inout_template
	{
		file_record m_record;
		std::string m_code;
		std::vector<value> m_include;
		std::string m_tag;
		std::vector<value> m_fix;
		command_array m_command;
		uint64_t m_register_used;
	};

	struct shader
	{
		file_record m_record;
		ShaderType m_type = ShaderType::Unknow;
		std::string m_code;
		std::vector<code_ptr> m_include;
		std::map<std::string, std::string> m_define;
		std::map<std::string, value> m_output;
		std::array<std::string, 5> m_enter;
		std::vector<inout_ptr> m_inout;
	};

	struct shader_template
	{
		file_record m_record;
		ShaderType m_type = ShaderType::Unknow;
		std::string m_code;
		std::vector<value> m_include;
		std::map<std::string, value> m_define;
		std::map<std::string, value> m_output;
		std::array<value, 5> m_enter;
		std::vector<value> m_inout;
		uint64_t m_register_used;
		command_array m_pre_instance_command;
		std::map<std::string, uint64_t> m_instance_table;
		command_array m_command;
		std::vector<code_template_ptr> m_dependence_code;
		std::vector<inout_template_ptr> m_dependence_inout;
	};

	struct storage
	{
		using type = std::variant<
			code_ptr, code_template_ptr, inout_ptr, inout_template_ptr, shader_ptr, shader_template_ptr
		>;
		std::vector<type> m_storage;
		uint64_t registered(type);
	};

	struct complie_environment : storage
	{
		virtual const std::string& current_path() const = 0;
		virtual std::shared_ptr<mscf_file> find_mscf_file(const std::string& path) const = 0;
		construct_table& table() { return m_table; }
		virtual line_record registered(storage::type);
	private:
		construct_table m_table;
	};

	struct mscf_file
	{
		std::vector<code> m_total_code;
	};
	*/

	/*
	struct file_element { const char* m_source; uint64_t m_index; };

	struct template_interface
	{
		command_array m_pre_instance_command;
		std::map<std::string, uint64_t> m_instance_table;
		uint64_t m_local_register_used = 0;
		command_array m_command;
	};

	struct code
	{
		file_element m_element;
		std::string m_code;
		std::vector<code_ptr> m_include;
	};

	struct code_template
	{
		std::string m_code;
		uint64_t m_include_count;
	};

	struct inout
	{
		std::shared_ptr<code> m_code;
		std::string m_symbol;
		std::vector<std::string> m_fix;
	};

	struct inout_template : template_interface
	{
		code_template m_code;
		std::string m_id;
		uint64_t m_fix_count;
	};

	using inout_template_ptr = std::shared_ptr<inout_template>;

	enum class ShaderType
	{
		VS, PS, CS, AllType,
	};

	enum class StackType {
		Output,
		Define,
	};

	struct shader
	{
		ShaderType m_type;
		std::map<std::string, value> m_output;
		std::map<std::string, std::string> m_define;
		code_ptr m_code;
		std::vector<inout_ptr> m_inout;
	};

	struct shader_template : template_interface
	{
		ShaderType m_type;
		std::vector<StackType> m_stack_type;
		code_template m_code;
		uint64_t m_inout_count;
		std::vector<inout_template_ptr> m_inout;
	};

	struct pass
	{
		std::vector<ref> m_shader;
		std::map<std::string, value> m_export;
	};

	struct pass_template : template_interface
	{
		uint64_t m_export_count;
		std::vector<uint64_t> m_used_shader;
	};

	struct define_storage
	{
		std::vector<std::string> m_import;
		std::vector<code> m_code;
		std::vector<code_dependence> m_code_d;
		std::vector<inout> m_inout;
		std::vector<inout_dependence> m_inout_d;
		std::vector<shader> m_shader;
		std::vector<shader_template> m_shader_t;
		std::vector<shader_template> m_shader_d;
		std::vector<pass> m_pass;
		std::vector<pass_template> m_pass_t;
	};

	enum struct RegisterType
	{
		Output,
		Using,
	};

	struct mscf_file : define_storage
	{
		std::map<std::string, value> m_export;
		std::map<std::string, ref> m_using;
	};
	*/
/*
	using Syntax::MscfSyntax;
	using Syntax::MscfToken;
	using Syntax::token_location;

	using ast_node_t = PO::Syntax::ast_node<MscfSyntax, MscfToken, token_location>;
	using ast_node_ter_t = PO::Syntax::ast_node_terminal<MscfToken, token_location>;

	

	struct mscf_manager_interface
	{
		virtual const mscf_file& find_file(std::string) = 0;
	};

	mscf_file handle_mscf_ast(const ast_node_t& node, mscf_manager_interface& inter);
}
*/