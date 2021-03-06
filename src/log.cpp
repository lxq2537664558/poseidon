// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014 - 2017, LH_Mouse. All wrongs reserved.

#include "precompiled.hpp"
#include "log.hpp"
#include "atomic.hpp"
#include "time.hpp"
#include "flags.hpp"
#include "singletons/main_config.hpp"

namespace Poseidon {

namespace {
	struct LevelElement {
		char text[16];
		char color;
		bool highlighted;
	};

	const LevelElement LEVEL_ELEMENTS[] = {
		{ "FATAL", '5', 1 },    // 粉色
		{ "ERROR", '1', 1 },    // 红色
		{ "WARN ", '3', 1 },    // 黄色
		{ "INFO ", '2', 0 },    // 绿色
		{ "DEBUG", '6', 0 },    // 青色
		{ "TRACE", '4', 1 },    // 亮蓝
	};

	volatile boost::uint64_t g_mask = (boost::uint64_t)-1;

	// 不要使用 Mutex 对象。如果在其他静态对象的构造函数中输出日志，这个对象可能还没构造。
	::pthread_mutex_t g_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

	__attribute__((__used__, __destructor__))
	void mutex_destructor() NOEXCEPT {
		if(::pthread_mutex_destroy(&g_mutex) != 0){
			std::abort();
		}
	}

	__thread char t_tag[5] = "----";
}

boost::uint64_t Logger::get_mask() NOEXCEPT {
	return atomic_load(g_mask, ATOMIC_RELAXED);
}
boost::uint64_t Logger::set_mask(boost::uint64_t to_disable, boost::uint64_t to_enable) NOEXCEPT {
	boost::uint64_t old_mask = atomic_load(g_mask, ATOMIC_RELAXED), new_mask;
	do {
		new_mask = old_mask;
		remove_flags(new_mask, to_disable);
		add_flags(new_mask, to_enable);
	} while(!atomic_compare_exchange(g_mask, old_mask, new_mask, ATOMIC_RELAXED, ATOMIC_RELAXED));
	return old_mask;
}

bool Logger::initialize_mask_from_config(){
	std::bitset<64> new_mask_bits;
	if(!MainConfig::get(new_mask_bits, "log_masked_levels")){
		return false;
	}
	new_mask_bits.flip();
#ifdef POSEIDON_CXX11
	set_mask((boost::uint64_t)-1, new_mask_bits.to_ullong());
#else
	set_mask((boost::uint64_t)-1, new_mask_bits.to_ulong());
#endif
	return true;
}
void Logger::finalize_mask() NOEXCEPT {
	set_mask(0, SP_POSEIDON | SP_MAJOR | LV_INFO | LV_WARNING | LV_ERROR | LV_FATAL);
}

const char *Logger::get_thread_tag() NOEXCEPT {
	return t_tag;
}
void Logger::set_thread_tag(const char *new_tag) NOEXCEPT {
	unsigned i = 0;
	while(i < sizeof(t_tag)){
		const char ch = new_tag[i];
		if(ch == 0){
			break;
		}
		t_tag[i++] = ch;
	}
	while(i < sizeof(t_tag)){
		t_tag[i++] = ' ';
	}
}

Logger::Logger(boost::uint64_t mask, const char *file, std::size_t line) NOEXCEPT
	: m_mask(mask), m_file(file), m_line(line)
{ }
Logger::~Logger() NOEXCEPT
try {
	static const bool stderr_uses_ascii_colors = ::isatty(STDERR_FILENO);
	static const bool stdout_uses_ascii_colors = ::isatty(STDOUT_FILENO);

	bool use_ascii_colors;
	int fd;
	if(m_mask & SP_MAJOR){
		use_ascii_colors = stderr_uses_ascii_colors;
		fd = STDERR_FILENO;
	} else {
		use_ascii_colors = stdout_uses_ascii_colors;
		fd = STDOUT_FILENO;
	}

	AUTO_REF(level_elem, LEVEL_ELEMENTS[__builtin_ctzll(m_mask | LV_TRACE)]);

	char temp[256];
	unsigned len;

	std::string line;
	line.reserve(255);

	if(use_ascii_colors){
		line += "\x1B[0;32m";
	}
	len = format_time(temp, sizeof(temp), get_local_time(), true);
	line.append(temp, len);

	if(use_ascii_colors){
		line += "\x1B[0;33m";
	}
	len = (unsigned)std::sprintf(temp, " %02X ", (unsigned)((m_mask >> 8) & 0xFF));
	line.append(temp, len);

	if(use_ascii_colors){
		line += "\x1B[0;39m";
	}
	line += '[';
	line.append(t_tag, sizeof(t_tag) - 1);
	line += ']';
	line += ' ';

	if(use_ascii_colors){
		line +="\x1B[0;30;4";
		line += level_elem.color;
		line += 'm';
	}
	line += level_elem.text;
	if(use_ascii_colors){
		line +="\x1B[0;3";
		line += level_elem.color;
		if(level_elem.highlighted){
			line += ';';
			line += '1';
		}
		line += 'm';
	}
	line += ' ';

	StreamBuffer buffer = STD_MOVE(m_stream.get_buffer());
	int ch;
	while((ch = buffer.get()) >= 0){
		if((ch + 1 <= 0x20) || (ch == 0x7F)){
			ch = ' ';
		}
		line.push_back((char)ch);
	}
	line += ' ';

	if(use_ascii_colors){
		line += "\x1B[0;34m";
	}
	line += '#';
	line += m_file;
	len = (unsigned)std::sprintf(temp, ":%lu", (unsigned long)m_line);
	line.append(temp, len);

	if(use_ascii_colors){
		line += "\x1B[0m";
	}
	line += '\n';

	int err_code = ::pthread_mutex_lock(&g_mutex);
	(void)err_code;
	assert(err_code == 0);
	std::size_t total = 0;
	::ssize_t written;
	while((total < line.size()) && (written = ::write(fd, line.data() + total, line.size() - total)) > 0){
		total += (std::size_t)written;
	}
	err_code = ::pthread_mutex_unlock(&g_mutex);
	assert(err_code == 0);
} catch(...){
	return;
}

void Logger::put(bool val){
	m_stream <<std::boolalpha <<val;
}
void Logger::put(char val){
	m_stream <<val;
}
void Logger::put(signed char val){
	m_stream <<static_cast<int>(val);
}
void Logger::put(unsigned char val){
	m_stream <<static_cast<unsigned>(val);
}
void Logger::put(short val){
	m_stream <<static_cast<int>(val);
}
void Logger::put(unsigned short val){
	m_stream <<static_cast<unsigned>(val);
}
void Logger::put(int val){
	m_stream <<val;
}
void Logger::put(unsigned val){
	m_stream <<val;
}
void Logger::put(long val){
	m_stream <<val;
}
void Logger::put(unsigned long val){
	m_stream <<val;
}
void Logger::put(long long val){
	m_stream <<val;
}
void Logger::put(unsigned long long val){
	m_stream <<val;
}
void Logger::put(const char *val){
	m_stream <<val;
}
void Logger::put(const signed char *val){
	m_stream <<static_cast<const void *>(val);
}
void Logger::put(const unsigned char *val){
	m_stream <<static_cast<const void *>(val);
}
void Logger::put(const void *val){
	m_stream <<val;
}

}
