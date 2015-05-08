// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014 - 2015, LH_Mouse. All wrongs reserved.

#ifndef POSEIDON_UUID_HPP_
#define POSEIDON_UUID_HPP_

#include "cxx_ver.hpp"
#include <iosfwd>
#include <string>
#include <cstring>
#include <boost/array.hpp>
#include <boost/cstdint.hpp>

namespace Poseidon {

class Uuid {
public:
	static const Uuid UUID_NULL;
	static const Uuid UUID_MIN;
	static const Uuid UUID_MAX;

public:
	static Uuid random();

private:
	union {
		unsigned char bytes[16];
		boost::uint16_t u16[8];
		boost::uint32_t u32[4];
		boost::uint64_t u64[2];
	} m_storage;

public:
	CONSTEXPR Uuid() NOEXCEPT
		: m_storage()
	{
	}
#ifdef POSEIDON_CXX11
	explicit Uuid(std::nullptr_t){
	}
#endif
	explicit Uuid(const unsigned char (&bytes)[16]){
		std::memcpy(m_storage.bytes, bytes, 16);
	}
	explicit Uuid(const boost::array<unsigned char, 16> &bytes){
		std::memcpy(m_storage.bytes, bytes.data(), 16);
	}
	// 字符串不合法则抛出异常。
	explicit Uuid(const char (&str)[36]);
	explicit Uuid(const std::string &str);

public:
	CONSTEXPR const unsigned char *begin() const {
		return m_storage.bytes;
	}
	unsigned char *begin(){
		return m_storage.bytes;
	}
	CONSTEXPR const unsigned char *end() const {
		return m_storage.bytes + 16;
	}
	unsigned char *end(){
		return m_storage.bytes + 16;
	}
	CONSTEXPR const unsigned char *data() const {
		return m_storage.bytes;
	}
	unsigned char *data(){
		return m_storage.bytes;
	}
	CONSTEXPR std::size_t size() const {
		return 16;
	}

	void toString(char (&str)[36], bool upperCase = true) const;
	void toString(std::string &str, bool upperCase = true) const;
	std::string toString(bool upperCase = true) const;
	bool fromString(const char (&str)[36]);
	bool fromString(const std::string &str);

public:
	const unsigned char &operator[](unsigned index) const {
		return m_storage.bytes[index];
	}
	unsigned char &operator[](unsigned index){
		return m_storage.bytes[index];
	}
};

inline bool operator==(const Uuid &lhs, const Uuid &rhs){
	return std::memcmp(lhs.begin(), rhs.begin(), 16) == 0;
}
inline bool operator!=(const Uuid &lhs, const Uuid &rhs){
	return std::memcmp(lhs.begin(), rhs.begin(), 16) != 0;
}
inline bool operator<(const Uuid &lhs, const Uuid &rhs){
	return std::memcmp(lhs.begin(), rhs.begin(), 16) < 0;
}
inline bool operator>(const Uuid &lhs, const Uuid &rhs){
	return std::memcmp(lhs.begin(), rhs.begin(), 16) > 0;
}
inline bool operator<=(const Uuid &lhs, const Uuid &rhs){
	return std::memcmp(lhs.begin(), rhs.begin(), 16) <= 0;
}
inline bool operator>=(const Uuid &lhs, const Uuid &rhs){
	return std::memcmp(lhs.begin(), rhs.begin(), 16) >= 0;
}

extern std::ostream &operator<<(std::ostream &os, const Uuid &rhs);

}

#endif
