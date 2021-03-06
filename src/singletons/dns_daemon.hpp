// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014 - 2017, LH_Mouse. All wrongs reserved.

#ifndef POSEIDON_SINGLETONS_DNS_DAEMON_HPP_
#define POSEIDON_SINGLETONS_DNS_DAEMON_HPP_

#include <boost/shared_ptr.hpp>
#include <string>

namespace Poseidon {

class SockAddr;
template<typename> class JobPromiseContainer;

class DnsDaemon {
private:
	DnsDaemon();

public:
	static void start();
	static void stop();

	// 同步接口。
	static SockAddr look_up(const std::string &host, unsigned port);

	// 异步接口。
	static boost::shared_ptr<const JobPromiseContainer<SockAddr> > enqueue_for_looking_up(std::string host, unsigned port);
};

}

#endif
