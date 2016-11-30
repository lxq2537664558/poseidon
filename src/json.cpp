// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014 - 2016, LH_Mouse. All wrongs reserved.

#include "precompiled.hpp"
#include "json.hpp"
#include "protocol_exception.hpp"
#include "profiler.hpp"
#include <iomanip>

namespace Poseidon {

namespace {
	const JsonElement g_null_element = JsonNull();

	extern JsonElement accept_element(std::istream &is);

	std::string accept_string(std::istream &is){
		PROFILE_ME;

		std::string ret;
		char temp;
		if(!(is >>temp) || (temp != '\"')){
			DEBUG_THROW(ProtocolException, sslit("JSON parser: String open expected"), -1);
		}

		is >>std::noskipws;
		for(;;){
			if(!(is >>temp)){
				DEBUG_THROW(ProtocolException, sslit("JSON parser: String not closed"), -1);
			}
			if(temp == '\"'){
				if(ret.empty() || (*ret.rbegin() != '\\')){
					break;
				}
			}
			ret.push_back(temp);
		}
		is >>std::skipws;

		std::string::iterator write = ret.begin();
		std::string::const_iterator read = write;
		while(read != ret.end()){
			char ch = *read;
			++read;

			if(ch != '\\'){
				*write = ch;
				++write;
				continue;
			}

			if(read == ret.end()){
				DEBUG_THROW(ProtocolException, sslit("JSON parser: Found escape character at the end"), -1);
			}
			ch = *read;
			++read;

			switch(ch){
			case '\"':
				*write = '\"';
				++write;
				break;

			case '\\':
				*write = '\\';
				++write;
				break;

			case '/':
				*write = '/';
				++write;
				break;

			case 'b':
				*write = '\b';
				++write;
				break;

			case 'f':
				*write = '\f';
				++write;
				break;

			case 'n':
				*write = '\n';
				++write;
				break;

			case 'r':
				*write = '\r';
				++write;
				break;

			case 'u':
				if(ret.end() - read < 4){
					DEBUG_THROW(ProtocolException, sslit("JSON parser: Too few hex digits for \\u"), -1);
				} else {
					unsigned code = 0;
					for(unsigned i = 12; i != (unsigned)-4; i -= 4){
						unsigned hexc = (unsigned char)*read;
						++read;
						if(('0' <= hexc) && (hexc <= '9')){
							hexc -= '0';
						} else if(('A' <= hexc) && (hexc <= 'F')){
							hexc -= 'A' - 0x0A;
						} else if(('a' <= hexc) && (hexc <= 'f')){
							hexc -= 'a' - 0x0A;
						} else {
							DEBUG_THROW(ProtocolException, sslit("JSON parser: Invalid hex digits for \\u"), -1);
						}
						code |= hexc << i;
					}
					if(code <= 0x7F){
						*write = code;
						++write;
					} else if(code <= 0x7FF){
						*write = (code >> 6) | 0xC0;
						++write;
						*write = (code & 0x3F) | 0x80;
						++write;
					} else {
						*write = (code >> 12) | 0xE0;
						++write;
						*write = ((code >> 6) & 0x3F) | 0x80;
						++write;
						*write = (code & 0x3F) | 0x80;
						++write;
					}
				}
				break;

			default:
				DEBUG_THROW(ProtocolException, sslit("JSON parser: Unknown escaped character sequence"), -1);
			}
		}
		ret.erase(write, ret.end());
		return ret;
	}
	double accept_number(std::istream &is){
		PROFILE_ME;

		double ret;
		if(!(is >>ret)){
			DEBUG_THROW(ProtocolException, sslit("JSON parser: Number expected"), -1);
		}
		return ret;
	}
	JsonObject accept_object(std::istream &is){
		PROFILE_ME;

		JsonObject ret;
		char temp;
		if(!(is >>temp) || (temp != '{')){
			DEBUG_THROW(ProtocolException, sslit("JSON parser: Object open expected"), -1);
		}
		for(;;){
			if(!(is >>temp)){
				DEBUG_THROW(ProtocolException, sslit("JSON parser: Object not closed"), -1);
			}
			if(temp == '}'){
				break;
			}
			if(temp == ','){
				continue;
			}
			is.unget();
			std::string name = accept_string(is);
			if(!(is >>temp) || (temp != ':')){
				DEBUG_THROW(ProtocolException, sslit("JSON parser: Colon expected"), -1);
			}
			ret.set(SharedNts(name), accept_element(is));
		}
		return ret;
	}
	JsonArray accept_array(std::istream &is){
		PROFILE_ME;

		JsonArray ret;
		char temp;
		if(!(is >>temp) || (temp != '[')){
			DEBUG_THROW(ProtocolException, sslit("JSON parser: Array open expected"), -1);
		}
		for(;;){
			if(!(is >>temp)){
				DEBUG_THROW(ProtocolException, sslit("JSON parser: Array not closed"), -1);
			}
			if(temp == ']'){
				break;
			}
			if(temp == ','){
				continue;
			}
			is.unget();
			ret.push_back(accept_element(is));
		}
		return ret;
	}
	bool accept_boolean(std::istream &is){
		PROFILE_ME;

		bool ret;
		if(!(is >>std::boolalpha >>ret)){
			DEBUG_THROW(ProtocolException, sslit("JSON parser: Boolean expected"), -1);
		}
		return ret;
	}
	JsonNull accept_null(std::istream &is){
		PROFILE_ME;

		char temp[5];
		if(!(is >>std::setw(sizeof(temp)) >>temp) || (std::strcmp(temp, "null") != 0)){
			DEBUG_THROW(ProtocolException, sslit("JSON parser: Null expected"), -1);
		}
		return JsonNull();
	}

	JsonElement accept_element(std::istream &is){
		PROFILE_ME;

		JsonElement ret;
		char temp;
		if(!(is >>temp)){
			DEBUG_THROW(ProtocolException, sslit("JSON parser: No input character"), -1);
		}
		is.unget();
		switch(temp){
		case '\"':
			ret = accept_string(is);
			break;

		case '-':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			ret = accept_number(is);
			break;

		case '{':
			ret = accept_object(is);
			break;

		case '[':
			ret = accept_array(is);
			break;

		case 't':
		case 'f':
			ret = accept_boolean(is);
			break;

		case 'n':
			ret = accept_null(is);
			break;

		default:
			DEBUG_THROW(ProtocolException, sslit("JSON parser: Unknown element type"), -1);
		}
		return ret;
	}
}

const JsonElement &null_json_element() NOEXCEPT {
	return g_null_element;
}

std::string JsonObject::dump() const {
	PROFILE_ME;

	std::ostringstream oss;
	dump(oss);
	return oss.str();
}
void JsonObject::dump(std::ostream &os) const {
	PROFILE_ME;

	os <<'{';
	AUTO(it, begin());
	if(it != end()){
		os <<'\"';
		os <<it->first.get();
		os <<'\"';
		os <<':';
		it->second.dump(os);

		while(++it != end()){
			os <<',';

			os <<'\"';
			os <<it->first.get();
			os <<'\"';
			os <<':';
			it->second.dump(os);
		}
	}
	os <<'}';
}
void JsonObject::parse(std::istream &is){
	accept_object(is).swap(*this);
}

std::string JsonArray::dump() const {
	PROFILE_ME;

	std::ostringstream oss;
	dump(oss);
	return oss.str();
}
void JsonArray::dump(std::ostream &os) const {
	PROFILE_ME;

	os <<'[';
	AUTO(it, begin());
	if(it != end()){
		it->dump(os);

		while(++it != end()){
			os <<',';

			it->dump(os);
		}
	}
	os <<']';
}
void JsonArray::parse(std::istream &is){
	PROFILE_ME;

	accept_array(is).swap(*this);
}

std::string JsonElement::dump() const {
	PROFILE_ME;

	std::ostringstream oss;
	dump(oss);
	return oss.str();
}
void JsonElement::dump(std::ostream &os) const {
	PROFILE_ME;

	switch(type()){
	case T_BOOL:
		os <<std::boolalpha <<get<bool>();
		break;

	case T_NUMBER:
		os <<std::setprecision(20) <<get<double>();
		break;

	case T_STRING:
		{
			const AUTO_REF(str, get<std::string>());
			os <<'\"';
			for(AUTO(it, str.begin()); it != str.end(); ++it){
				const unsigned ch = (unsigned char)*it;
				switch(ch){
				case '\"':
				case '\\':
				case '/':
					os <<'\\' <<*it;
					break;

				case '\b':
					os <<'\\' <<'b';
					break;

				case '\f':
					os <<'\\' <<'f';
					break;

				case '\n':
					os <<'\\' <<'n';
					break;

				case '\r':
					os <<'\\' <<'r';
					break;

				case '\t':
					os <<'\\' <<'t';
					break;

				default:
					if((ch < 0x20) || (ch == 0x7F) || (ch == 0xFF)){
						os <<'\\' <<'u' <<std::setfill('0') <<std::setw(4) <<ch;
					} else {
						os <<(char)ch;
					}
					break;
				}
			}
			os <<'\"';
		}
		break;

	case T_OBJECT:
		os <<get<JsonObject>();
		break;

	case T_ARRAY:
		os <<get<JsonArray>();
		break;

	case T_NULL:
		os <<"null";
		break;

	default:
		assert(false);
	}
}
void JsonElement::parse(std::istream &is){
	accept_element(is).swap(*this);
}

}
