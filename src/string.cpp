// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014 - 2015, LH_Mouse. All wrongs reserved.

#include "precompiled.hpp"
#include "string.hpp"
#include "profiler.hpp"
#include "log.hpp"
#include <iomanip>

namespace Poseidon {

const std::string EMPTY_STRING;

std::ostream &operator<<(std::ostream &os, const HexDumper &dumper){
	AUTO(read, static_cast<const unsigned char *>(dumper.read));
	os <<std::hex;
	for(std::size_t i = 0; i < dumper.size; ++i){
		os <<std::setfill('0') <<std::setw(2) <<static_cast<unsigned>(*read) <<' ';
		++read;
	}
	os << std::dec;
	return os;
}

bool isValidUtf8String(const std::string &str){
	PROFILE_ME;

	boost::uint32_t codePoint;
	for(AUTO(it, str.begin()); it != str.end(); ++it){
		codePoint = static_cast<unsigned char>(*it);
		if((codePoint & 0x80) == 0){
			continue;
		}
		const AUTO(bytes, static_cast<unsigned>(
			__builtin_clz((~codePoint | 1) & 0xFF)) - (sizeof(unsigned) - sizeof(unsigned char)) * CHAR_BIT);
		if(bytes - 2 > 2){ // 2, 3, 4
			LOG_POSEIDON_WARNING("Invalid UTF-8 leading byte: bytes = ", bytes);
			return false;
		}
		codePoint &= (0xFFu >> bytes);
		for(unsigned i = 1; i < bytes; ++i){
			++it;
			if(it == str.end()){
				LOG_POSEIDON_WARNING("String is truncated.");
				return false;
			}
			const unsigned trailing = static_cast<unsigned char>(*it);
			if((trailing & 0xC0u) != 0x80u){
				LOG_POSEIDON_WARNING("Invalid UTF-8 trailing byte: trailing = 0x",
					std::hex, std::setw(2), std::setfill('0'), trailing);
				return false;
			}
		}
		if(codePoint > 0x10FFFFu){
			LOG_POSEIDON_WARNING("Invalid UTF-8 code point: codePoint = 0x",
				std::hex, std::setw(6), std::setfill('0'), codePoint);
			return false;
		}
		if(codePoint - 0xD800u < 0x800u){
			LOG_POSEIDON_WARNING("UTF-8 code point is reserved for UTF-16: codePoint = 0x",
				std::hex, std::setw(4), std::setfill('0'), codePoint);
			return false;
		}
	}
	return true;
}

}
