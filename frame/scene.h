#pragma once
#include <mutex>
#include <map>
#include <typeindex>
#include "../tool/thread_tool.h"
#include <future>
#include <string>
#include <fstream>
namespace PO
{
	class io_method;

	struct io_block
	{
		io_method& metho;
		const std::u16string& path;
		const std::u16string& name;
		std::fstream& stream;
		const Tool::any& parameter;
	};

	class io_method
	{
	public:

		struct block
		{

			using analyze_t = std::function<Tool::any(io_block)>;
			using filter_t = std::function<std::fstream(const std::u16string&, const std::u16string&)>;
			using pair_t = std::pair<filter_t, analyze_t>;
			using d_filter_t = std::function<Tool::any(io_method&, const std::u16string&, const std::u16string&, const Tool::any&)>;

			Tool::variant<pair_t, analyze_t, d_filter_t> method;

			static std::fstream default_filter(const std::u16string& path, const std::u16string& name);

			Tool::optional<Tool::any> operator()(io_method& iom, const std::u16string& path, const std::u16string& name, const Tool::any& a);
		};

		struct path_list
		{
			std::vector<std::u16string> top_path;
			std::map<std::type_index, std::vector<std::u16string>> type_path;
		};

	private:

		Tool::scope_lock<std::map<std::type_index, block>, std::recursive_mutex> fun_map;
		Tool::scope_lock<path_list, std::recursive_mutex> paths;

	public:
		Tool::optional<Tool::any> calling_specified_path_execute(std::type_index ti, const std::u16string&, const std::u16string&, const Tool::any& a);
		Tool::optional<Tool::any> calling_execute(std::type_index ti, const std::u16string&, const Tool::any& a);
		void set_function(std::type_index ti, block b);
		void set_function(std::type_index ti, block::analyze_t analyze) { set_function(ti, block{ std::move(analyze) }); }
		void set_function(std::type_index ti, block::analyze_t analyze, block::filter_t filter) { set_function(ti, block{ std::make_pair(std::move(filter), std::move(analyze)) }); }
		void set_function(std::type_index ti, block::d_filter_t d) { set_function(ti, block{ d }); }
		void add_path(std::type_index ti, std::u16string pa);
		void add_path(std::u16string pa);
		void add_path(std::initializer_list<std::u16string> pa);
		void add_path(std::initializer_list<std::pair<std::type_index, std::u16string>> pa);
		void add_path(std::type_index ti, std::initializer_list<std::u16string> pa);
	};


	namespace Implement
	{
		class io_task_implement
		{
			Tool::thread_task_operator ope;
			io_method method;
			Tool::completeness_ref cr;
		public:
			void set_function(std::type_index ti, io_method::block b) { method.set_function(ti, std::move(b)); }
			void set_function(std::type_index ti, io_method::block::analyze_t analyze) { method.set_function(ti, io_method::block{ std::move(analyze) }); }
			void set_function(std::type_index ti, io_method::block::analyze_t analyze, io_method::block::filter_t filter) { method.set_function(std::move(ti), io_method::block{ std::make_pair(std::move(filter), std::move(analyze)) }); }
			void set_function(std::type_index ti, io_method::block::d_filter_t filter) { method.set_function(std::move(ti), io_method::block{ std::move(filter) }); }
			void add_path(std::type_index ti, std::u16string pa) { method.add_path(ti, std::move(pa)); }
			void add_path(std::u16string pa) { method.add_path(std::move(pa)); }
			void add_path(std::initializer_list<std::u16string> pa) { method.add_path(std::move(pa)); }
			void add_path(std::initializer_list<std::pair<std::type_index, std::u16string>> pa) { method.add_path(std::move(pa)); }
			void add_path(std::type_index ti, std::initializer_list<std::u16string> pa) { method.add_path(ti, std::move(pa)); }
			std::future<Tool::optional<Tool::any>> add_request(std::type_index ti, std::u16string pa, Tool::any a);
			decltype(auto) request(std::type_index ti, const std::u16string& pa, const Tool::any& a) { return method.calling_execute(ti, pa, a); }
			io_task_implement(Tool::completeness_ref rf) :cr(rf) {}
		};
	}

	using io_task = Tool::completeness<Implement::io_task_implement>;
	io_task& io_task_instance();

	class raw_scene
	{
		struct request
		{
			std::type_index ti;
			std::u16string path;
			bool save_raw_data;
		};

		// make Tool::any(Tool::any&)
		using store_type = Tool::variant<Tool::any, std::future<Tool::optional<Tool::any>>>;
		Tool::scope_lock<std::map<std::type_index, std::map<std::u16string, store_type>>> store_map;
	public:
		Tool::optional<Tool::any> find(std::type_index, const std::u16string&, const Tool::any&, bool save_data = true);
		Tool::optional<Tool::any> find(std::type_index ti, const std::u16string& name, bool save_data = true) { return find(ti, name, Tool::any{}, save_data); }
		void pre_load(std::type_index, const std::u16string& path, Tool::any a);
		void pre_load(std::type_index ti, const std::u16string& path) { pre_load(ti, path, Tool::any{}); }
		void pre_load(std::type_index, std::initializer_list<std::pair<std::u16string, Tool::any>> path);
		void pre_load(std::type_index, std::initializer_list<std::u16string> path);
	};
}