// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014 - 2017, LH_Mouse. All wrongs reserved.

#ifndef POSEIDON_FLAGS_HPP_
#define POSEIDON_FLAGS_HPP_

#include "cxx_util.hpp"
#include <boost/type_traits/is_enum.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/common_type.hpp>
#include <boost/static_assert.hpp>

namespace Poseidon {

template<typename T>
inline T &add_flags(T &val, typename boost::common_type<T>::type flags){
	BOOST_STATIC_ASSERT((boost::is_enum<T>::value || boost::is_integral<T>::value));

	val |= flags;
	return val;
}
template<typename T>
inline T &remove_flags(T &val, typename boost::common_type<T>::type flags){
	BOOST_STATIC_ASSERT((boost::is_enum<T>::value || boost::is_integral<T>::value));

	val &= ~flags;
	return val;
}

template<typename T>
inline bool has_all_flags_of(const T &val, typename boost::common_type<T>::type flags){
	BOOST_STATIC_ASSERT((boost::is_enum<T>::value || boost::is_integral<T>::value));

	return (val & flags) == flags;
}
template<typename T>
inline bool has_any_flags_of(const T &val, typename boost::common_type<T>::type flags){
	BOOST_STATIC_ASSERT((boost::is_enum<T>::value || boost::is_integral<T>::value));

	return (val & flags) != 0;
}
template<typename T>
inline bool has_none_flags_of(const T &val, typename boost::common_type<T>::type flags){
	BOOST_STATIC_ASSERT((boost::is_enum<T>::value || boost::is_integral<T>::value));

	return (val & flags) == 0;
}

}

#endif
