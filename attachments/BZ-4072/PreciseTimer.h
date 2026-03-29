#ifndef _PRECISETIMER_H_
#define _PRECISETIMER_H_

#include <ace/OS.h>
#include <ace/OS_NS_time.h>

namespace npl {
	class PreciseTimer {
	public:
		PreciseTimer() {restart();}
		void restart() {start_time_ = ACE_OS::gettimeofday();}
		ACE_Time_Value elapsed() const {
			return ACE_OS::gettimeofday() - start_time_;
		}
		double elapsed_double() const {
			ACE_Time_Value diff = elapsed();
			return double(diff.sec()) + double(diff.usec()) / double(1000000);
		}
	private:
		ACE_Time_Value start_time_;
	};
}

#include <iomanip>
#include <boost/io/ios_state.hpp>

inline std::ostream& operator<<(std::ostream& os, const ACE_Time_Value& tv) {
	boost::io::ios_all_saver ifs(os);
	os << std::dec << tv.sec() << '.' << std::setfill('0') << std::setw(6) << tv.usec();
	return os;
}

#endif // _PRECISETIMER_H_
