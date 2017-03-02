#include "simul_debug.h"

void simul_debug::init(ticker& t)
{
	sl.load(t.form(), u"simul_gpu_sky_generator_loss.cso");
}