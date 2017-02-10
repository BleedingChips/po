#include "test_plugin.h"

void test_plugin::tick(ticker& op)
{

}

void test_plugin::init(PO::form_self& fs, ticker& op)
{
	float2 data[] =
	{
		float2(-1.0, 1.0),
		float2(-1.0, -1.0),
		float2(1.0, -1.0),

		float2(1.0, -1.0),
		float2(1.0, 1.0),
		float2(-1.0, 1.0),
	};
	ver.create(op, data, 6);
}