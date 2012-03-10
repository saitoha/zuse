/* ***** BEGIN LICENSE BLOCK Version: GPL 3.0 ***** 
 * Copyright (C) 2008-2011  zuse <user@zuse.jp>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ***** END LICENSE BLOCK ***** */



namespace ecmascript {

    namespace error_info
    {
        jmp_buf & get_jmp_buffer()
        {
            static jmp_buf jmp_buffer;
            return jmp_buffer;
        }

        wchar_t const*& get_position() 
        {
            static wchar_t const*position = 0;
            return position;
        };

        wchar_t const*& get_message() 
        {
            static wchar_t const*message = 0;
            return message;
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_syntax_error_paser_gen
    //  @brief
    //
    struct es_syntax_error_paser_gen
    : public spirit::parser<es_syntax_error_paser_gen>
    {
        wchar_t const * error_msg_;

        typedef spirit::nil_t result_t;
        typedef es_syntax_error_paser_gen self_t;

        template <typename ScannerT>
        struct result
        {
            typedef typename spirit::match_result<
                ScannerT, result_t>::type type;
        };

        explicit es_syntax_error_paser_gen(wchar_t const *msg) throw()
        : error_msg_(msg)
        {
        }

        template <typename ScannerT>
        typename spirit::parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const
        {
            if (scan.at_end())
                return scan.no_match();
            error_info::get_position() = scan.first;
            error_info::get_message() = error_msg_;
            longjmp(error_info::get_jmp_buffer(), 1);
        }

    };

    static const es_syntax_error_paser_gen
        illegal_statement_error_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal statement."
                );

    static const es_syntax_error_paser_gen
        illegal_block_statement_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal block statement"
                L" - block end bracket '}' is not found."
                );

    static const es_syntax_error_paser_gen
        illegal_function_declaration_statement_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal function decralation statement"
                );

    static const es_syntax_error_paser_gen
        illegal_expression_statement_start_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal expression statement"
                L" - Note that an ExpressionStatement cannot start with "
                L"the function keyword because that might make it "
                L"ambiguous with a FunctionDeclaration."
                );

    static const es_syntax_error_paser_gen
        illegal_formal_parameter_list_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal parameter list"
                );

    static const es_syntax_error_paser_gen
        illegal_formal_parameter_list_unknown_token_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal parameter list"
                L" - unknown token (not identifier) followed by a comma."
                );

    static const es_syntax_error_paser_gen
        illegal_formal_parameter_list_start_bracket_not_found_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal parameter list"
                L" - parameter start bracket '(' is not found."
                );

    static const es_syntax_error_paser_gen
        illegal_formal_parameter_list_end_bracket_not_found_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal parameter list"
                L" - parameter end bracket ')' is not found."
                );

    static const es_syntax_error_paser_gen
        illegal_function_expression_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal function expression"
                );

    static const es_syntax_error_paser_gen
        illegal_function_body_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal function body"
                );

    static const es_syntax_error_paser_gen
        illegal_function_body_block_start_bracket_not_found_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal function body"
                L" - opening curly brace '{' is not found."
                );

    static const es_syntax_error_paser_gen
        illegal_function_body_block_end_bracket_not_found_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal function body"
                L" - closing curly brace '}' is not found."
                );

    static const es_syntax_error_paser_gen
        illegal_for_statement_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal for statement"
                );

    static const es_syntax_error_paser_gen
        illegal_for_statement_start_bracket_not_found_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal for statement"
                L" - start bracket '(' is not found."
                );

    static const es_syntax_error_paser_gen
        illegal_for_statement_end_bracket_not_found_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal for statement"
                L" - end bracket ')' is not found."
                );

    static const es_syntax_error_paser_gen
        illegal_for_statement_semicolon_not_found_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal for statement"
                L" - the semicolon break ';' is not found."
                );

    static const es_syntax_error_paser_gen
        illegal_if_statement_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal if statement"
                );

    static const es_syntax_error_paser_gen
        illegal_if_statement_start_bracket_not_found_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal if statement"
                L" - start bracket '(' is not found."
                );

    static const es_syntax_error_paser_gen
        illegal_if_statement_end_bracket_not_found_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal if statement"
                L" - end bracket ')' is not found."
                );

    static const es_syntax_error_paser_gen
        illegal_if_statement_statement_not_found_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal if statement"
                L" - valid statement is not found at "
                L"next of conditional statement."
                );

    static const es_syntax_error_paser_gen
        illegal_if_statement_else_statement_not_found_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal if statement"
                L" - valid statement is not found at "
                L"next of the keyword 'else'."
                );

    static const es_syntax_error_paser_gen
        illegal_if_statement_bad_conditional_expression_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal if statement"
                L" - bad conditional expression."
                );

    static const es_syntax_error_paser_gen
        illegal_throw_statement_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal throw statement"
                L" - an Expression is expected before the LineTerminator. "
                L"it may unintended effects of the 'automatic semicolon "
                L"Insertion'."
                );

    static const es_syntax_error_paser_gen
        illegal_break_statement_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal break statement"
                L" - an Expression is expected before the LineTerminator. "
                L"it may unintended effects of the 'automatic semicolon "
                L"Insertion'."
                );

    static const es_syntax_error_paser_gen
        illegal_continue_statement_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal continue statement"
                L" - an Expression is expected before the LineTerminator. "
                L"it may unintended effects of the 'automatic semicolon "
                L"Insertion'."
                );

    static const es_syntax_error_paser_gen
        illegal_return_statement_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal return statement"
                L" - an Expression is expected before the LineTerminator. "
                L"it may unintended effects of the 'automatic semicolon "
                L"Insertion'."
                );

    static const es_syntax_error_paser_gen
        illegal_while_statement_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal [while] statement"
                );

    static const es_syntax_error_paser_gen
        illegal_do_while_statement_p
            = es_syntax_error_paser_gen(
                L"syntax error: illegal [do-while] statement"
                );

} // namespace ecmascript
