/// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JMESPATH_JMESPATH_ERROR_HPP
#define JSONCONS_JMESPATH_JMESPATH_ERROR_HPP

#include <jsoncons/json_exception.hpp>
#include <system_error>

namespace jsoncons { namespace jmespath {

class jmespath_error : public std::system_error
{
    std::string buffer_;
    std::size_t line_number_;
    std::size_t column_number_;
public: 
    jmespath_error(std::error_code ec)
        : std::system_error(ec), line_number_(0), column_number_(0)
    {
    }
    jmespath_error(std::error_code ec, std::size_t position)
        : std::system_error(ec), line_number_(0), column_number_(position)
    {
    }
    jmespath_error(std::error_code ec, std::size_t line, std::size_t column)
        : std::system_error(ec), line_number_(line), column_number_(column)
    {
    }
    jmespath_error(const jmespath_error& other) = default;

    jmespath_error(jmespath_error&& other) = default;

    const char* what() const noexcept override
    {
        JSONCONS_TRY
        {
            std::ostringstream os;
            os << std::system_error::what();
            if (line_number_ != 0 && column_number_ != 0)
            {
                os << " at line " << line_number_ << " and column " << column_number_;
            }
            else if (column_number_ != 0)
            {
                os << " at position " << column_number_;
            }
            const_cast<std::string&>(buffer_) = os.str();
            return buffer_.c_str();
        }
        JSONCONS_CATCH(...)
        {
            return std::system_error::what();
        }
    }

    std::size_t line() const noexcept
    {
        return line_number_;
    }

    std::size_t column() const noexcept
    {
        return column_number_;
    }
#if !defined(JSONCONS_NO_DEPRECATED)
    JSONCONS_DEPRECATED_MSG("Instead, use line()")
    std::size_t line_number() const noexcept
    {
        return line();
    }

    JSONCONS_DEPRECATED_MSG("Instead, use column()")
    std::size_t column_number() const noexcept
    {
        return column();
    }
#endif
};

enum class jmespath_errc 
{
    success = 0,
    expected_identifier,
    expected_index,
    expected_A_Za_Z_,
    expected_right_bracket,
    expected_right_brace,
    expected_colon,
    expected_dot,
    expected_or,
    expected_and,
    invalid_number,
    invalid_literal,
    expected_comparator,
    expected_key,
    invalid_argument,
    function_name_not_found,
    invalid_type,
    unexpected_end_of_input,
    step_cannot_be_zero,
    invalid_expression,
    invalid_codepoint,
    illegal_escaped_character,
    unknown_error 
};

class jmespath_error_category_impl
   : public std::error_category
{
public:
    const char* name() const noexcept override
    {
        return "jsoncons/jmespath";
    }
    std::string message(int ev) const override
    {
        switch (static_cast<jmespath_errc>(ev))
        {
            case jmespath_errc::expected_identifier:
                return "Expected identifier";
            case jmespath_errc::expected_index:
                return "Expected index";
            case jmespath_errc::expected_A_Za_Z_:
                return "Expected A-Z, a-z, or _";
            case jmespath_errc::expected_right_bracket:
                return "Expected ]";
            case jmespath_errc::expected_right_brace:
                return "Expected }";
            case jmespath_errc::expected_colon:
                return "Expected :";
            case jmespath_errc::expected_dot:
                return "Expected \".\"";
            case jmespath_errc::expected_or:
                return "Expected \"||\"";
            case jmespath_errc::expected_and:
                return "Expected \"&&\"";
            case jmespath_errc::invalid_number:
                return "Invalid number";
            case jmespath_errc::invalid_literal:
                return "Invalid literal";
            case jmespath_errc::expected_comparator:
                return "Expected <, <=, ==, >=, > or !=";
            case jmespath_errc::expected_key:
                return "Expected key";
            case jmespath_errc::invalid_argument:
                return "Invalid argument type";
            case jmespath_errc::function_name_not_found:
                return "Function name not found";
            case jmespath_errc::invalid_type:
                return "Invalid type";
            case jmespath_errc::unexpected_end_of_input:
                return "Unexpected end of jmespath input";
            case jmespath_errc::step_cannot_be_zero:
                return "Slice step cannot be zero";
            case jmespath_errc::invalid_expression:
                return "Invalid expression";
            case jmespath_errc::invalid_codepoint:
                return "Invalid codepoint";
            case jmespath_errc::illegal_escaped_character:
                return "Illegal escaped character";
            case jmespath_errc::unknown_error:
            default:
                return "Unknown jmespath parser error";
        }
    }
};

inline
const std::error_category& jmespath_error_category()
{
  static jmespath_error_category_impl instance;
  return instance;
}

inline 
std::error_code make_error_code(jmespath_errc result)
{
    return std::error_code(static_cast<int>(result),jmespath_error_category());
}

}}

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::jmespath::jmespath_errc> : public true_type
    {
    };
}

#endif