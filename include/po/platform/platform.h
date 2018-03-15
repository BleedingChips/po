#pragma once
#ifdef _WIN32
#include "windows.h"
#else
#endif //_Win32

namespace PO::Platform
{
#if _WIN32
	using process_id = HINSTANCE;
	struct process_scription
	{
		static process_scription& instance();
		process_id id = nullptr;
	private:
		process_scription() {}
	};


	struct cpu_info
	{
		SYSTEM_INFO  info;
	};
	
	

	struct platform_info
	{
		SYSTEM_INFO  info;
		friend const platform_info& platform_info_instance();
		platform_info();
		platform_info(const platform_info&) = delete;
	public:
		size_t cpu_count() const noexcept { return static_cast<size_t>(info.dwNumberOfProcessors); }
	};
	const platform_info& platform_info_instance();

#endif // _WIN32
}