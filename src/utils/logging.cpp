#include "logging.hpp"

#include <chrono>

namespace logging::detail
{

	double generate_timestamp()
	{
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		long long ms_since_epoch = duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
		return ((double)ms_since_epoch) / 1000.0;
	}
}