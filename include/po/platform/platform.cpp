#include "platform.h"
namespace PO::Platform
{
	process_scription& process_scription::instance()
	{
		static process_scription ins;
		return ins;
	}

	platform_info::platform_info()
	{
		GetSystemInfo(&info);
	}

	const platform_info& platform_info::instance()
	{
		static platform_info info;
		return info;
	}
}