#pragma once

#include <algorithm>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <vector>

namespace psi::tools {

inline std::string to_hex_string(uint8_t *buffer, size_t sz) noexcept
{
    std::ostringstream os;
    os << std::hex << std::setfill('0');
    for (size_t i = 0; i < sz; ++i) {
        os << std::setw(2) << uint16_t(buffer[i]) << " ";
    }
    return os.str();
}

inline std::string to_hex_string(uint64_t val) noexcept
{
    std::ostringstream os;
    os << std::setfill('0') << std::setw(2) << std::hex << val;
    return os.str();
}

template <typename T>
inline std::string ptr_to_address(T *ptr) noexcept
{
    return "0x" + to_hex_string(reinterpret_cast<size_t>(ptr));
}

inline std::vector<std::pair<std::string, std::string>> parse_to_map(const std::string &s, char delimiter) noexcept
{
    std::vector<std::pair<std::string, std::string>> tokens;
    std::string tokenLine;

    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, tokenLine)) {
        const auto pos = tokenLine.find(delimiter);
        if (pos != std::string::npos) {
            const auto tokenName = tokenLine.substr(0, pos);
            const auto tokenValue = tokenLine.substr(pos + 1, tokenLine.size());
            tokens.emplace_back(std::pair {tokenName, tokenValue});
        }
    }
    return tokens;
}

inline std::vector<std::string> parse_to_vector(const std::string &s, char delimiter) noexcept
{
    std::vector<std::string> tokens;
    std::string tokenLine;

    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, tokenLine, delimiter)) {
        tokens.emplace_back(tokenLine);
    }
    return tokens;
}

inline std::string &ltrim(std::string &str, const std::string &chars = "\t\n\v\f\r ") noexcept
{
    const auto pos = str.find_first_not_of(chars);
    if (pos != std::string::npos) {
        str.erase(0, pos);
    } else {
        str.clear();
    }
    return str;
}

inline std::string &rtrim(std::string &str, const std::string &chars = "\t\n\v\f\r ") noexcept
{
    const auto pos = str.find_last_not_of(chars);
    if (pos != std::string::npos) {
        str.erase(pos + 1);
    } else {
        str.clear();
    }
    return str;
}

inline std::string &trim(std::string &str, const std::string &chars = "\t\n\v\f\r ") noexcept
{
    return ltrim(rtrim(str, chars), chars);
}

inline std::string to_upper(const std::string &str) noexcept
{
    std::string data;
    data.resize(str.size());
    for (size_t index = 0; index < str.size(); ++index) {
        data[index] = uint8_t(::toupper(str[index]));
    }
    return data;
}

std::wstring utf8_to_wstring(const std::string &);
std::string wstring_to_utf8(const std::wstring &);
std::string generateTimeStamp() noexcept;

template <typename Arg>
void convertToAsyncCall(std::function<void(std::function<void(Arg)>)> fn, Arg &arg) noexcept
{
    std::promise<Arg> p;
    std::future<Arg> f = p.get_future();
    auto cb = [&p](Arg res) { p.set_value(res); };
    fn(cb);

    std::future_status callStatus = f.wait_for(std::chrono::seconds(10));
    if (std::future_status::timeout == callStatus) {
        std::cout << "Async call timeout. Sending fallback" << std::endl;
        arg = Arg();
    } else {
        arg = f.get();
    }
}

/// Returns object's address and name
template <typename T>
inline std::string objName(const T &obj) noexcept
{
    size_t address = reinterpret_cast<size_t>(&obj);
    return "(0x" + to_hex_string(address) + "):" + typeid(obj).name();
}

} // namespace psi::tools
