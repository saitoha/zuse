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



#include "action.hpp"
#include "spirit.hpp"

#include "epsilonparser.hpp"
#include "errorparser.hpp"
#include "charparser.hpp"
#include "skipparser.hpp"
#include "identifierparser.hpp"
#include "decimalparser.hpp"
#include "hexparser.hpp"

namespace ecmascript {

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_singlequoted_stringliteral_parser_gen
    //  @brief
    //
    struct es_singlequoted_stringliteral_parser_gen
    : public spirit::parser<es_singlequoted_stringliteral_parser_gen>
    {
        typedef es_singlequoted_stringliteral_parser_gen self_t;

        template <typename ScannerT>
        struct result
        {
            typedef typename 
                spirit::match_result<ScannerT, spirit::nil_t>::type type;
        };

        es_singlequoted_stringliteral_parser_gen()
        : parser_(
            spirit::es_lexeme_d
            [
                spirit::operator >> (
                    '\''
                    , spirit::operator | (
                        spirit::operator >> (
                            es_singlequoted_string_p
                            [
                                es_action_singlequotedstring<const_string_t::const_iterator>()
                            ]
                            , '\''
                        )
                        , spirit::operator >> (
                            '\''
                            , es_epsilon_p[ es_action_nullstring<const_string_t::const_iterator>() ]
                        )
                    )
                )
            ]
        )
        {
        }
        
        template <typename ScannerT>
        typename spirit::parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const
        {
            return parser_.parse(scan);
        }
        
    private:
        spirit::contiguous<
            spirit::sequence<
                spirit::chlit<char>
                , spirit::alternative<
                    spirit::sequence<
                        spirit::parser<
                            es_quoted_string_parser_gen<'\''>
                        >::action<
                            es_quoted_string_parser_gen<'\''>
                            , es_action_singlequotedstring<const_string_t::const_iterator>
                        >
                        , spirit::chlit<char>
                    >
                    , spirit::sequence<
                        spirit::chlit<char>
                        , spirit::parser<
                            es_epsilon_parser
                        >::action<
                            es_epsilon_parser
                            , es_action_nullstring<const_string_t::const_iterator>
                        >
                    >
                >
            >
        > const parser_;
    };

    es_singlequoted_stringliteral_parser_gen const
        es_singlequoted_stringliteral_parser;
        
    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_doublequoted_stringliteral_parser_gen
    //  @brief
    //
    struct es_doublequoted_stringliteral_parser_gen
    : public spirit::parser<es_doublequoted_stringliteral_parser_gen>
    {
        typedef es_doublequoted_stringliteral_parser_gen self_t;

        template <typename ScannerT>
        struct result
        {
            typedef typename spirit::match_result<ScannerT, spirit::nil_t>::type
                type;
        };
        
        es_doublequoted_stringliteral_parser_gen()
        : parser_(
            spirit::es_lexeme_d
            [
                spirit::operator >> (
                    '"',
                    spirit::operator | (
                        spirit::operator >> (
                            es_doublequoted_string_p
                            [
                                es_action_doublequotedstring<const_string_t::const_iterator>()
                            ]
                            , '"'
                        )
                        , spirit::operator >> (
                            '"'
                            , es_epsilon_p[ es_action_nullstring<const_string_t::const_iterator>() ]
                        )
                    )
                )
            ]
        )
        {
        }

        template <typename ScannerT>
        typename spirit::parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const
        {
            return parser_.parse(scan);
        }
        
    private:        
        spirit::contiguous<
            spirit::sequence<
                spirit::chlit<char>,
                spirit::alternative<
                    spirit::sequence<
                        spirit::parser<
                            es_quoted_string_parser_gen<'"'>
                        >::action<
                            es_quoted_string_parser_gen<'"'>,
                            es_action_doublequotedstring<const_string_t::const_iterator>
                        >,
                        spirit::chlit<char>
                    >,
                    spirit::sequence<
                        spirit::chlit<char>,
                        spirit::parser<
                            ecmascript::es_epsilon_parser
                        >::action<
                            es_epsilon_parser,
                            es_action_nullstring<const_string_t::const_iterator>
                        >
                    >
                >
            >
        > const parser_;
    };

    es_doublequoted_stringliteral_parser_gen const
        es_doublequoted_stringliteral_parser;

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_number_parser_gen
    //  @brief
    //
    struct es_number_parser_gen
    : public spirit::parser<es_number_parser_gen>
    {
        typedef es_number_parser_gen self_t;

        template <typename ScannerT>
        struct result
        {
            typedef typename spirit::match_result<
                ScannerT,
                spirit::nil_t
            >::type type;
        };
        
        es_number_parser_gen()
        : parser_(
            '0'
            >> (
                'x' >> es_hex_p[ es_action_number<const_string_t::const_iterator>() ]
                | 'X' >> es_hex_p[ es_action_number<const_string_t::const_iterator>() ]
            )
            | es_decimal_parser[ es_action_number<const_string_t::const_iterator>() ]
        )
        {
        }

        template <typename ScannerT>
        typename spirit::parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const
        {
            return parser_.parse(scan);
        }
        
    private:
        spirit::alternative<
            spirit::sequence<
                spirit::chlit<char>
                , spirit::alternative<
                    spirit::sequence<
                        spirit::chlit<char>
                        , spirit::parser<
                            es_hex_parser_gen
                        >::action<
                            es_hex_parser_gen
                            , es_action_number<const_string_t::const_iterator>
                        >
                    >
                    , spirit::sequence<
                        spirit::chlit<char>
                        , spirit::parser<
                            es_hex_parser_gen
                        >::action<
                            es_hex_parser_gen
                            , es_action_number<const_string_t::const_iterator>
                        >
                    >
                >
            >
            , spirit::parser<es_decimal_parserarser_gen>::action<
                es_decimal_parserarser_gen
                , es_action_number<const_string_t::const_iterator>
            >
        > const parser_;
    };

    es_number_parser_gen const es_number_parser = es_number_parser_gen();

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class program_parser
    //  @brief
    //
    template <typename T>
    struct program_parser
    : public spirit::parser<program_parser<T> >
    {
        typedef program_parser self_t;
        typedef spirit::es_rule<T> rule_t;

        template <typename ScannerT>
        struct result
        {
            typedef typename spirit::match_result<ScannerT, spirit::nil_t>::type
                type;
        };

        program_parser()
        {
            using namespace spirit;

            ElementList
                = (
                    AssignmentExpression
                    >> (
                        ',' >> (
                            ElementList
                            | ']'
                            >> es_epsilon_p[ es_action_undefined<const_string_t::const_iterator>() ]
                        )
                        | ']'
                        >> es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ]
                    )
                    | ','
                    >> es_epsilon_p[ es_action_undefined<const_string_t::const_iterator>() ]
                    >> (
                        ElementList
                        | ']'
                        >> es_epsilon_p[ es_action_undefined<const_string_t::const_iterator>() ]
                    )
                )[ es_action_arrayelement<const_string_t::const_iterator>() ]

            , PropertyNameAndValueList
                = (
                    es_identifier_p[ es_action_string<const_string_t::const_iterator>() ]
                    | es_singlequoted_stringliteral_parser
                    | es_doublequoted_stringliteral_parser
                    | es_number_parser
                )
                >> ':' >> AssignmentExpression
                >> (
                    ',' >> PropertyNameAndValueList
                    | '}'
                    >> es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ]
                )[ es_action_objectelement<const_string_t::const_iterator>() ]

            , Token
                = es_identifier_p[ es_action_identifier<const_string_t::const_iterator>() ]
                | '(' >> Expression >> ')'
                | '{' >> (
                    '}'
                    >> es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ]
                    | PropertyNameAndValueList
                )[ es_action_object<const_string_t::const_iterator>() ]
                | '[' >> (
                    ']'
                    >> es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ]
                    | ElementList
                )
                [
                    es_action_array<const_string_t::const_iterator>()
                ]
                | es_singlequoted_stringliteral_parser
                | es_doublequoted_stringliteral_parser
                | "true" >> es_epsilon_p[ es_action_true<const_string_t::const_iterator>() ]
                | "this" >> es_epsilon_p[ es_action_this<const_string_t::const_iterator>() ]
                | "false"  >> es_epsilon_p[ es_action_false<const_string_t::const_iterator>() ]
                | FunctionExpression
                | (
                    "null" >> es_epsilon_p[ es_action_null<const_string_t::const_iterator>() ]
                    | "new" >> Token >> !MemberExpression_Notation
                    [
                        es_action_binary<const_string_t::const_iterator>()
                    ]
                    >> (
                        '(' >> (
                            NewArgument
                            | ')'
                            >> es_epsilon_p[ es_action_newargend<const_string_t::const_iterator>() ]
                            >> !(
                                Arguments
                                | CallUnits
                                [
                                    es_action_binary<const_string_t::const_iterator>()
                                ]
                            )
                        )
                        [
                            es_action_args<const_string_t::const_iterator>()
                        ]
                        [
                            es_action_binary<const_string_t::const_iterator>()
                        ]
                        | (
                            es_epsilon_p[ es_action_new<const_string_t::const_iterator>() ]
                            >> AssignmentOperator
                            | ConditionalExpressionAfterLhs
                        ) >> !( ',' >> Expression )
                        [
                            es_action_binary<const_string_t::const_iterator>()
                        ]
                    )
                    >> (';' | es_epsilon_p)
                )
                | '/' >> es_lexeme_d[ 
                    ( '\\' >> es_anychar_p | (es_anychar_p - '*' - '/') )
                    >> *( '\\' >> es_anychar_p | (es_anychar_p - '/') )
                ][ es_action_nativestring<const_string_t::const_iterator>() ]
                >> (
                    '/' >> !(
                        'g' >> !(
                            'i' >> ('m' | es_epsilon_p) 
                            | 'm' >> ('i' | es_epsilon_p)
                        )
                        | 'i' >> !(
                            'm' >> ('g' | es_epsilon_p) 
                            | 'g' >> ('m' | es_epsilon_p)
                        )
                        | 'm' >> !(
                            'g' >> ('i' | es_epsilon_p) 
                            | 'i' >> ('g' | es_epsilon_p)
                        )
                    )
                )[ es_action_regexp<const_string_t::const_iterator>() ]
                | es_number_parser

            , MemberExpression_Notation
                = (
                    '.' >> es_identifier_p[ es_action_member<const_string_t::const_iterator>() ]
                    | ( '[' >> Expression >> ']' )[ es_action_bracket<const_string_t::const_iterator>() ]
                )
                >> !(
                    Arguments
                    | MemberExpression_Notation
                )
                [
                    es_action_binary<const_string_t::const_iterator>()
                ]

            , NewArgument
                = AssignmentExpression
                >> (
                    ',' >> NewArgument
                    | ')'
                    >> es_epsilon_p[ es_action_newargend<const_string_t::const_iterator>() ]
                    >> !(
                        Arguments
                        | CallUnits
                        [
                            es_action_binary<const_string_t::const_iterator>()
                        ]
                    )
                )[ es_action_arg<const_string_t::const_iterator>() ]

            , CallUnits
                = (
                    '.' >> es_identifier_p[ es_action_member<const_string_t::const_iterator>() ]
                    | ( '[' >> Expression >> ']' )[ es_action_bracket<const_string_t::const_iterator>() ]
                ) >> !(
                    Arguments
                    | CallUnits
                    [
                        es_action_binary<const_string_t::const_iterator>()
                    ]
                )
                [
                    es_action_binary<const_string_t::const_iterator>()
                ]

            , Arguments
                = '(' >> (
                    Argument
                    | ')'
                    >> es_epsilon_p[ es_action_argend<const_string_t::const_iterator>() ]
                    >> !(
                        Arguments
                        | CallUnits
                        [
                            es_action_binary<const_string_t::const_iterator>()
                        ]
                    )
                )[ es_action_args<const_string_t::const_iterator>() ]

            , Argument
                = AssignmentExpression
                >> (
                    ',' >> Argument
                    | ')'
                    >> es_epsilon_p[ es_action_argend<const_string_t::const_iterator>() ]
                    >> !(
                        Arguments
                        | CallUnits
                        [
                            es_action_binary<const_string_t::const_iterator>()
                        ]
                    )
                )[ es_action_arg<const_string_t::const_iterator>() ]

            , LeftHandSideExpression
                = (
                    Token >> (
                        (
                            !MemberExpression_Notation
                            [
                                es_action_binary<const_string_t::const_iterator>()
                            ]
                            >> !Arguments
                            [
                                es_action_binary<const_string_t::const_iterator>()
                            ]
                        )
                        [
                            es_action_call<const_string_t::const_iterator>()
                        ]
                        | es_epsilon_p
                    )
                )

            , PostfixExpression
                = LeftHandSideExpression
                >> PostfixExpressionAfterLhs

            , PostfixExpressionAfterLhs
                = *(
                    "++" >> es_epsilon_p[ es_action_postinc<const_string_t::const_iterator>() ]
                    | "--" >> es_epsilon_p[ es_action_postdec<const_string_t::const_iterator>() ]
                )

            , UnaryExpression
                = PostfixExpression
                | "++" >> UnaryExpression[ es_action_inc<const_string_t::const_iterator>() ]
                | "--" >> UnaryExpression[ es_action_dec<const_string_t::const_iterator>() ]
                | '+' >> UnaryExpression[ es_action_unaryplus<const_string_t::const_iterator>() ]
                | '-' >> UnaryExpression[ es_action_unaryminus<const_string_t::const_iterator>() ]
                | '~' >> UnaryExpression[ es_action_tilde<const_string_t::const_iterator>() ]
                | '!' >> UnaryExpression[ es_action_not<const_string_t::const_iterator>() ]
                | "delete" >> UnaryExpression[ es_action_delete<const_string_t::const_iterator>() ]
                | "void" >> UnaryExpression[ es_action_void<const_string_t::const_iterator>() ]
                | "typeof" >> UnaryExpression[ es_action_typeof<const_string_t::const_iterator>() ]

            , MultiplicativeExpression
                = UnaryExpression
                >> *(
                    '*' >> UnaryExpression[ es_action_mul<const_string_t::const_iterator>() ]
                    | '/' >> UnaryExpression[ es_action_div<const_string_t::const_iterator>() ]
                    | '%' >> UnaryExpression[ es_action_mod<const_string_t::const_iterator>() ]
                )

            , MultiplicativeExpressionAfterLhs
                = PostfixExpressionAfterLhs
                >> *(
                    '*' >> UnaryExpression[ es_action_mul<const_string_t::const_iterator>() ]
                    | '/' >> UnaryExpression[ es_action_div<const_string_t::const_iterator>() ]
                    | '%' >> UnaryExpression[ es_action_mod<const_string_t::const_iterator>() ]
                )

            , AdditiveExpression
                = MultiplicativeExpression
                >> *(
                    '+' >> MultiplicativeExpression[ es_action_plus<const_string_t::const_iterator>() ]
                    | '-' >> MultiplicativeExpression[ es_action_minus<const_string_t::const_iterator>() ]
                )

            , AdditiveExpressionAfterLhs
                = MultiplicativeExpressionAfterLhs
                >> *(
                    '+' >> MultiplicativeExpression[ es_action_plus<const_string_t::const_iterator>() ]
                    | '-' >> MultiplicativeExpression[ es_action_minus<const_string_t::const_iterator>() ]
                )

            , ShiftExpression
                = AdditiveExpression
                >> *(
                    "<<" >> AdditiveExpression[ es_action_shl<const_string_t::const_iterator>() ]
                    | ">>>" >> AdditiveExpression[ es_action_sar<const_string_t::const_iterator>() ]
                    | ">>" >> AdditiveExpression[ es_action_shr<const_string_t::const_iterator>() ]
                )

            , ShiftExpressionAfterLhs
                = AdditiveExpressionAfterLhs
                >> *(
                    "<<" >> AdditiveExpression[ es_action_shl<const_string_t::const_iterator>() ]
                    | ">>>" >> AdditiveExpression[ es_action_sar<const_string_t::const_iterator>() ]
                    | ">>" >> AdditiveExpression[ es_action_shr<const_string_t::const_iterator>() ]
                )

            , RelationalExpression
                = ShiftExpression
                >> *(
                    "<=" >> ShiftExpression[ es_action_le<const_string_t::const_iterator>() ]
                    | ">=" >> ShiftExpression[ es_action_ge<const_string_t::const_iterator>() ]
                    | '<' >> ShiftExpression[ es_action_lt<const_string_t::const_iterator>() ]
                    | '>' >> ShiftExpression[ es_action_gt<const_string_t::const_iterator>() ]
                    | "instanceof" >> ShiftExpression[ es_action_instanceof<const_string_t::const_iterator>() ]
                    | "in" >> ShiftExpression[ es_action_in<const_string_t::const_iterator>() ]
                )

            , RelationalExpressionNoIn
                = ShiftExpression
                >>  *(
                    "<=" >> ShiftExpression[ es_action_le<const_string_t::const_iterator>() ]
                    | ">=" >> ShiftExpression[ es_action_ge<const_string_t::const_iterator>() ]
                    | '<' >> ShiftExpression[ es_action_lt<const_string_t::const_iterator>() ]
                    | '>' >> ShiftExpression[ es_action_gt<const_string_t::const_iterator>() ]
                    | "instanceof" >> ShiftExpression[ es_action_instanceof<const_string_t::const_iterator>() ]
                )

            , RelationalExpressionAfterLhs
                = ShiftExpressionAfterLhs
                >> *(
                    "<=" >> ShiftExpression[ es_action_le<const_string_t::const_iterator>() ]
                    | ">=" >> ShiftExpression[ es_action_ge<const_string_t::const_iterator>() ]
                    | '<' >> ShiftExpression[ es_action_lt<const_string_t::const_iterator>() ]
                    | '>' >> ShiftExpression[ es_action_gt<const_string_t::const_iterator>() ]
                    | "instanceof" >> ShiftExpression[ es_action_instanceof<const_string_t::const_iterator>() ]
                    | "in" >> ShiftExpression[ es_action_in<const_string_t::const_iterator>() ]
                )

            , RelationalExpressionNoInAfterLhs
                = ShiftExpressionAfterLhs
                >> *(
                    "<=" >> ShiftExpression[ es_action_le<const_string_t::const_iterator>() ]
                    | ">=" >> ShiftExpression[ es_action_ge<const_string_t::const_iterator>() ]
                    | '<' >> ShiftExpression[ es_action_lt<const_string_t::const_iterator>() ]
                    | '>' >> ShiftExpression[ es_action_gt<const_string_t::const_iterator>() ]
                    | "instanceof" >> ShiftExpression[ es_action_instanceof<const_string_t::const_iterator>() ]
                )

            , EqualityExpression
                = RelationalExpression
                >> *(
                    "===" >> RelationalExpression[ es_action_stricteq<const_string_t::const_iterator>() ]
                    | "!==" >> RelationalExpression[ es_action_strictne<const_string_t::const_iterator>() ]
                    | "==" >> RelationalExpression[ es_action_eq<const_string_t::const_iterator>() ]
                    | "!=" >> RelationalExpression[ es_action_ne<const_string_t::const_iterator>() ]
                )

            , EqualityExpressionNoIn
                = RelationalExpressionNoIn
                >> *(
                    "===" >> RelationalExpressionNoIn[ es_action_stricteq<const_string_t::const_iterator>() ]
                    | "!==" >> RelationalExpressionNoIn[ es_action_strictne<const_string_t::const_iterator>() ]
                    | "==" >> RelationalExpressionNoIn[ es_action_eq<const_string_t::const_iterator>() ]
                    | "!=" >> RelationalExpressionNoIn[ es_action_ne<const_string_t::const_iterator>() ]
                )

            , EqualityExpressionAfterLhs
                = RelationalExpressionAfterLhs
                >> *(
                    "===" >> RelationalExpression[ es_action_stricteq<const_string_t::const_iterator>() ]
                    | "!==" >> RelationalExpression[ es_action_strictne<const_string_t::const_iterator>() ]
                    | "==" >> RelationalExpression[ es_action_eq<const_string_t::const_iterator>() ]
                    | "!=" >> RelationalExpression[ es_action_ne<const_string_t::const_iterator>() ]
                )

            , EqualityExpressionNoInAfterLhs
                = RelationalExpressionNoInAfterLhs
                >> *(
                    "===" >> RelationalExpressionNoIn[ es_action_stricteq<const_string_t::const_iterator>() ]
                    | "!==" >> RelationalExpressionNoIn[ es_action_strictne<const_string_t::const_iterator>() ]
                    | "==" >> RelationalExpressionNoIn[ es_action_eq<const_string_t::const_iterator>() ]
                    | "!=" >> RelationalExpressionNoIn[ es_action_ne<const_string_t::const_iterator>() ]
                )

            , BitwiseANDExpression
                = EqualityExpression
                >> *( '&' >> EqualityExpression[ es_action_bitand<const_string_t::const_iterator>() ] )

            , BitwiseANDExpressionNoIn
                = EqualityExpressionNoIn
                >> *( '&' >> EqualityExpressionNoIn[ es_action_bitand<const_string_t::const_iterator>() ] )

            , BitwiseANDExpressionAfterLhs
                = EqualityExpressionAfterLhs
                >> *( '&' >> EqualityExpression[ es_action_bitand<const_string_t::const_iterator>() ] )

            , BitwiseANDExpressionNoInAfterLhs
                = EqualityExpressionNoInAfterLhs
                >> *( '&' >> EqualityExpressionNoIn[ es_action_bitand<const_string_t::const_iterator>() ] )

            , BitwiseXORExpression
                = BitwiseANDExpression
                >> *( '^' >> BitwiseANDExpression[ es_action_bitxor<const_string_t::const_iterator>() ] )

            , BitwiseXORExpressionNoIn
                = BitwiseANDExpressionNoIn
                >> *( '^' >> BitwiseANDExpressionNoIn[ es_action_bitxor<const_string_t::const_iterator>() ] )

            , BitwiseXORExpressionAfterLhs
                = BitwiseANDExpressionAfterLhs
                >> *( '^' >> BitwiseANDExpression[ es_action_bitxor<const_string_t::const_iterator>() ] )

            , BitwiseXORExpressionNoInAfterLhs
                = BitwiseANDExpressionNoInAfterLhs
                >> *( '^' >> BitwiseANDExpressionNoIn[ es_action_bitxor<const_string_t::const_iterator>() ] )

            , BitwiseORExpression
                = BitwiseXORExpression
                >> *( '|' >> BitwiseXORExpression[ es_action_bitor<const_string_t::const_iterator>() ] )

            , BitwiseORExpressionNoIn
                = BitwiseXORExpressionNoIn
                >> *( '|' >> BitwiseXORExpressionNoIn[ es_action_bitor<const_string_t::const_iterator>() ] )

            , BitwiseORExpressionAfterLhs
                = BitwiseXORExpressionAfterLhs
                >> *( '|' >> BitwiseXORExpression[ es_action_bitor<const_string_t::const_iterator>() ] )

            , BitwiseORExpressionNoInAfterLhs
                = BitwiseXORExpressionNoInAfterLhs
                >> *( '|' >> BitwiseXORExpressionNoIn[ es_action_bitor<const_string_t::const_iterator>() ] )

            , LogicalANDExpression
                = BitwiseORExpression
                >> *( 
                    "&&" 
                    >> BitwiseORExpression
                    [ 
                        es_action_and<const_string_t::const_iterator>() 
                    ]
                    [ 
                        es_action_binary<const_string_t::const_iterator>() 
                    ] 
                )

            , LogicalANDExpressionNoIn
                = BitwiseXORExpressionNoIn
                >> *( 
                    "&&" 
                    >> BitwiseXORExpressionNoIn
                    [ 
                        es_action_and<const_string_t::const_iterator>() 
                    ]
                    [ 
                        es_action_binary<const_string_t::const_iterator>() 
                    ] 
                )

            , LogicalANDExpressionAfterLhs
                = BitwiseORExpressionAfterLhs
                >> *( 
                    "&&" 
                    >> BitwiseORExpression
                    [ 
                        es_action_and<const_string_t::const_iterator>() 
                    ]
                    [ 
                        es_action_binary<const_string_t::const_iterator>() 
                    ] 
                )

            , LogicalANDExpressionNoInAfterLhs
                = BitwiseORExpressionNoInAfterLhs
                >> *( 
                    "&&" 
                    >> BitwiseORExpressionNoIn
                    [ 
                        es_action_and<const_string_t::const_iterator>() 
                    ]
                    [ 
                        es_action_binary<const_string_t::const_iterator>() 
                    ] 
                )

            , LogicalORExpression
                = LogicalANDExpression
                >> *( 
                    "||" 
                    >> LogicalANDExpression
                    [ 
                        es_action_or<const_string_t::const_iterator>() 
                    ]
                    [ 
                        es_action_binary<const_string_t::const_iterator>() 
                    ] 
                )

            , LogicalORExpressionNoIn
                = LogicalANDExpressionNoIn
                >> *( 
                    "||" 
                    >> LogicalANDExpressionNoIn
                    [ 
                        es_action_or<const_string_t::const_iterator>() 
                    ]
                    [ 
                        es_action_binary<const_string_t::const_iterator>() 
                    ] 
                )

            , LogicalORExpressionAfterLhs
                = LogicalANDExpressionAfterLhs
                >> *( 
                    "||" 
                    >> LogicalANDExpression
                    [ 
                        es_action_or<const_string_t::const_iterator>() 
                    ]
                    [ 
                        es_action_binary<const_string_t::const_iterator>() 
                    ] 
                )

            , LogicalORExpressionNoInAfterLhs
                = LogicalANDExpressionNoInAfterLhs
                >> *( 
                    "||" 
                    >> LogicalANDExpressionNoIn
                    [ 
                        es_action_or<const_string_t::const_iterator>() 
                    ]
                    [ 
                        es_action_binary<const_string_t::const_iterator>() 
                    ] 
                )

            , ConditionalExpression
                = LogicalORExpression
                >> !ConditionalExpression_Alternative

            , ConditionalExpressionNoIn
                = LogicalORExpressionNoIn
                >> !ConditionalExpressionNoIn_Alternative

            , ConditionalExpressionAfterLhs
                = LogicalORExpressionAfterLhs
                >> !ConditionalExpression_Alternative

            , ConditionalExpressionNoInAfterLhs
                = LogicalORExpressionNoInAfterLhs
                >> !ConditionalExpressionNoIn_Alternative

            , ConditionalExpression_Alternative
                = '?' 
                >> AssignmentExpression
                >> ':' 
                >> AssignmentExpression
                [ 
                    es_action_alternative<const_string_t::const_iterator>() 
                ]
                [ 
                    es_action_binary<const_string_t::const_iterator>() 
                ]
                >> !ConditionalExpression_Alternative

            , ConditionalExpressionNoIn_Alternative
                = '?' 
                >> AssignmentExpressionNoIn
                >> ':' 
                >> AssignmentExpressionNoIn
                [ 
                    es_action_alternative<const_string_t::const_iterator>() 
                ]
                [ 
                    es_action_binary<const_string_t::const_iterator>() 
                ]
                >> !ConditionalExpressionNoIn_Alternative

            , AssignmentExpression
                = LeftHandSideExpression
                >> (
                    AssignmentOperator
                    | ConditionalExpressionAfterLhs
                )
                | ConditionalExpression

            , AssignmentExpressionNoIn
                = LeftHandSideExpression
                >> (
                    AssignmentOperatorNoIn
                    | ConditionalExpressionNoInAfterLhs
                )
                | ConditionalExpressionNoIn

            , AssignmentOperator
                = 
                (
                    '=' >> AssignmentExpression
                    [ 
                        es_action_assign<const_string_t::const_iterator>() 
                    ]
                    | "*=" >> AssignmentExpression
                    [ 
                        es_action_assignmul<const_string_t::const_iterator>()
                    ]
                    | "/=" >> AssignmentExpression
                    [ 
                        es_action_assigndiv<const_string_t::const_iterator>() 
                    ]
                    | "%=" >> AssignmentExpression
                    [ 
                        es_action_assignmod<const_string_t::const_iterator>() 
                    ]
                    | "+=" >> AssignmentExpression
                    [ 
                        es_action_assignplus<const_string_t::const_iterator>() 
                    ]
                    | "-=" >> AssignmentExpression
                    [ 
                        es_action_assignminus<const_string_t::const_iterator>() 
                    ]
                    | "<<=" >> AssignmentExpression
                    [ 
                        es_action_assignshl<const_string_t::const_iterator>() 
                    ]
                    | ">>>=" >> AssignmentExpression
                    [ 
                        es_action_assignsar<const_string_t::const_iterator>() 
                    ]
                    | ">>=" >> AssignmentExpression
                    [ 
                        es_action_assignshr<const_string_t::const_iterator>() 
                    ]
                    | "&=" >> AssignmentExpression
                    [ 
                        es_action_assignand<const_string_t::const_iterator>() 
                    ]
                    | "^=" >> AssignmentExpression
                    [ 
                        es_action_assignxor<const_string_t::const_iterator>() 
                    ]
                    | "|=" >> AssignmentExpression
                    [ 
                        es_action_assignor<const_string_t::const_iterator>() 
                    ]
                ) >> !AssignmentOperator

            , AssignmentOperatorNoIn
                = (
                    '=' >> AssignmentExpressionNoIn
                    [ 
                        es_action_assign<const_string_t::const_iterator>() 
                    ]
                    | "*=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignmul<const_string_t::const_iterator>() 
                    ]
                    | "/=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assigndiv<const_string_t::const_iterator>() 
                    ]
                    | "%=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignmod<const_string_t::const_iterator>() 
                    ]
                    | "+=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignplus<const_string_t::const_iterator>() 
                    ]
                    | "-=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignminus<const_string_t::const_iterator>() 
                    ]
                    | "<<=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignshl<const_string_t::const_iterator>() 
                    ]
                    | ">>>=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignsar<const_string_t::const_iterator>() 
                    ]
                    | ">>=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignshr<const_string_t::const_iterator>() 
                    ]
                    | "&=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignand<const_string_t::const_iterator>() 
                    ]
                    | "^=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignxor<const_string_t::const_iterator>() 
                    ]
                    | "|=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignor<const_string_t::const_iterator>() 
                    ]
                ) >> !AssignmentOperatorNoIn

            , Expression
                = AssignmentExpression
                >> !( ',' >> Expression )
                [
                    es_action_binary<const_string_t::const_iterator>()
                ]

            , ExpressionNoIn
                = AssignmentExpressionNoIn
                >> !( ',' >> ExpressionNoIn )
                [
                    es_action_binary<const_string_t::const_iterator>()
                ]

            , Statement
                = LabelledStatement
                | (
                    VariableStatement
                    | IfStatement
                    | IterationStatement_DoWhile
                    | (
                        IterationStatement_While
                        | WithStatement
                    )
                    | (
                       IterationStatement_For
                       | "function"
                       >> illegal_expression_statement_start_p
                    )
                    | SwitchStatement
                    | ReturnStatementNoExp
                    | ContinueStatementNoIdentifier
                    | BreakStatementNoIdentifier
                    | Block
                    | ( TryStatement | ThrowStatement )
                )
                | Expression >> (';' | es_epsilon_p)
                | (es_epsilon_p >> ';')[ es_action_unary<const_string_t::const_iterator>() ]

            , VariableStatement
                = "var" >> (
                    VariableDeclaration
                    >> *( ',' >> VariableDeclaration )
                    [
                        es_action_binary<const_string_t::const_iterator>()
                    ]
                )
                >> (';' | es_epsilon_p)

            , VariableDeclaration
                = es_identifier_p
                [
                    es_action_nativestring<const_string_t::const_iterator>()
                ]
                >> (
                    ( '=' >> AssignmentExpression )[ es_action_varinit<const_string_t::const_iterator>() ]
                    | es_epsilon_p[ es_action_var<const_string_t::const_iterator>() ]
                )

            , IfStatement
                = "if" >> (
                    '('
                    >> (
                        Expression
                        | illegal_if_statement_bad_conditional_expression_p
                    )
                    >> ( ')' | illegal_if_statement_end_bracket_not_found_p )
                ) >> (
                    Statement
                    | illegal_if_statement_statement_not_found_p
                )
                >> (
                    "else" >> (
                        Statement
                        | illegal_if_statement_else_statement_not_found_p
                    )[ es_action_ifelse<const_string_t::const_iterator>() ]
                    | es_epsilon_p[ es_action_if<const_string_t::const_iterator>() ]
                )[ es_action_binary<const_string_t::const_iterator>() ]
                >> (';' | es_epsilon_p)

            , IterationStatement_DoWhile
                = "do" >> Statement
                >> "while" >> '(' >> Expression[ es_action_dowhile<const_string_t::const_iterator>() ] >> ')'
                >> (';' | es_epsilon_p)

            , IterationStatement_While
                = "while" >> ( '(' >> Expression >> ')' )
                >> Statement
                [
                    es_action_while<const_string_t::const_iterator>()
                ]

            , IterationStatement_For
                = "for"
                >> (
                    '('
                    >> (
                        LeftHandSideExpression
                        >> (
                            "in" 
                            >> Expression
                            >> ')'
                            >> Statement[ es_action_forin<const_string_t::const_iterator>() ]
                            | (
                                AssignmentOperatorNoIn
                                | ConditionalExpressionNoInAfterLhs
                            )
                            >> !( ',' >> ExpressionNoIn )
                            [
                                es_action_binary<const_string_t::const_iterator>()
                            ]
                            >> ';'
                            >> ( Expression | es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ] ) 
                            >> ';'
                            >> ( Expression | es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ] )
                            >> ')'
                            >> Statement[ es_action_for<const_string_t::const_iterator>() ]
                            [
                                es_action_binary<const_string_t::const_iterator>()
                            ]
                        )
                        | ConditionalExpressionNoIn
                        >> !( ',' >> ExpressionNoIn )
                        [
                            es_action_binary<const_string_t::const_iterator>()
                        ]
                        >> ';'
                        >> ( Expression | es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ] ) 
                        >> ';'
                        >> ( Expression | es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ] )
                        >> ')'
                        >> Statement[ es_action_for<const_string_t::const_iterator>() ]
                        [
                            es_action_binary<const_string_t::const_iterator>()
                        ]
                        | "var" >> es_identifier_p
                        [
                            es_action_nativestring<const_string_t::const_iterator>()
                        ]
                        >> (
                            ( '=' >> AssignmentExpression )
                            >> (
                                "in" 
                                >> es_epsilon_p[ es_action_forinvarinit<const_string_t::const_iterator>() ] 
                                >> Expression
                                >> ')'
                                >> Statement[ es_action_forin<const_string_t::const_iterator>() ]
                                | es_epsilon_p[ es_action_varinit<const_string_t::const_iterator>() ]
                                >> *( ',' >> VariableDeclaration )
                                [
                                    es_action_binary<const_string_t::const_iterator>()
                                ]
                                >> ';'
                                >> ( 
                                    Expression 
                                    | es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ] 
                                ) 
                                >> ';'
                                >> ( 
                                    Expression 
                                    | es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ] 
                                )
                                >> ')'
                                >> Statement[ es_action_for<const_string_t::const_iterator>() ]
                                [
                                    es_action_binary<const_string_t::const_iterator>()
                                ]
                            )
                            | es_epsilon_p
                            >> (
                                "in"
                                >> es_epsilon_p[ es_action_forinvar<const_string_t::const_iterator>() ] 
                                >> Expression
                                >> ')'
                                >> Statement[ es_action_forin<const_string_t::const_iterator>() ]
                                | es_epsilon_p[ es_action_var<const_string_t::const_iterator>() ]
                                >> *( ',' >> VariableDeclaration )
                                [
                                    es_action_binary<const_string_t::const_iterator>()
                                ]
                                >> ';'
                                >> ( 
                                    Expression 
                                    | es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ] 
                                ) 
                                >> ';'
                                >> ( 
                                    Expression 
                                    | es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ] 
                                )
                                >> ')'
                                >> Statement[ es_action_for<const_string_t::const_iterator>() ]
                                [
                                    es_action_binary<const_string_t::const_iterator>()
                                ]
                            )
                        )
                        | es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ]
                        >> ';'
                        >> ( Expression | es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ] ) 
                        >> ';'
                        >> ( Expression | es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ] )
                        >> ')'
                        >> Statement[ es_action_for<const_string_t::const_iterator>() ]
                        [
                            es_action_binary<const_string_t::const_iterator>()
                        ]
                    )
                )

            , SwitchStatement
                = "switch"
                >> ( '(' >> Expression >> ')' )
                >> '{' >> ( CaseClause | DefaultClause )
                [
                    es_action_switch<const_string_t::const_iterator>()
                ]

            , CaseClause
                = '}'
                >> es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ]
                | "case" >> Expression >> ':'
                >> (
                    StatementList
                    | es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ]
                )
                [
                    es_action_case<const_string_t::const_iterator>()
                ]
                >> ( CaseClause | DefaultClause )
                [
                    es_action_binary<const_string_t::const_iterator>()
                ]
                
            , CaseClause_AfterDefault
                = '}'
                >> es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ]
                | "case" >> Expression >> ':'
                >> (
                    StatementList
                    | es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ]
                )
                [
                    es_action_case<const_string_t::const_iterator>()
                ]
                >> CaseClause_AfterDefault
                [
                    es_action_binary<const_string_t::const_iterator>()
                ]

            , DefaultClause
                = "default"
                >> (
                    ':'
                    >> (
                        StatementList
                        | es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ]
                    )
                    >> (
                        CaseClause
                        | es_epsilon_p
                    )
                    [
                        es_action_default<const_string_t::const_iterator>()
                    ]
                )

            , LabelledStatement
                = ( es_identifier_p >> ':' )
                [
                    es_action_nativestring<const_string_t::const_iterator>()
                ]
                >> Statement
                [
                    es_action_label<const_string_t::const_iterator>()
                ]

            , ContinueStatementNoIdentifier
                = "continue" >> (
                    es_lexeme_d
                    [
                        *es_space_p
                        >> ( es_line_terminater_ch_p | ';' )
                    ]
                    [
                        es_action_continuenoarg<const_string_t::const_iterator>()
                    ]
                    | es_identifier_p
                    [
                        es_action_continue<const_string_t::const_iterator>()
                    ]
                )
                >> (';' | es_epsilon_p)

            , BreakStatementNoIdentifier
                = "break" >> (
                    es_lexeme_d
                    [
                        *es_space_p
                        >> ( es_line_terminater_ch_p | ';' )
                    ]
                    [
                        es_action_breaknoarg<const_string_t::const_iterator>()
                    ]
                    | es_identifier_p[ es_action_break<const_string_t::const_iterator>() ]
                    >> (';' | es_epsilon_p)
                )

            , ReturnStatementNoExp
                = "return" >> (
                    es_lexeme_d
                    [
                        *es_space_p
                        >> ( es_line_terminater_ch_p | ';' )
                    ]
                    [
                        es_action_returnnoarg<const_string_t::const_iterator>()
                    ]
                    | Expression[ es_action_return<const_string_t::const_iterator>() ]
                    >> (';' | es_epsilon_p)
                )

            , ThrowStatement
                = es_lexeme_d
                [
                    "throw" >> (
                        es_space_p >> *es_space_p
                        | ( es_line_terminater_ch_p | ';' )
                        >> illegal_throw_statement_p
                    )
                ]
                >> Expression
                [
                    es_action_throw<const_string_t::const_iterator>()
                ]
                >> (';' | es_epsilon_p)

            , WithStatement
                = "with" >> ( '(' >> Expression >> ')' )
                >> Statement[ es_action_with<const_string_t::const_iterator>() ]

            , TryStatement
                = "try"
                >> Block
                >> (
                    "catch"
                    >> (
                        '('
                        >> es_identifier_p[ es_action_identifier<const_string_t::const_iterator>() ]
                        >> ')'
                    )
                    >> Block[ es_action_trycatch<const_string_t::const_iterator>() ]
                    >> !(
                        "finally"
                        >> Block
                        [
                            es_action_binary<const_string_t::const_iterator>()
                        ]
                    )
                    | "finally" >> Block[ es_action_tryfinally<const_string_t::const_iterator>() ]
                )

            , StatementList
                = Statement
                >> !StatementList
                [
                    es_action_binary<const_string_t::const_iterator>()
                ]

            , Block
                = '{' >> (
                    '}'
                    >> es_epsilon_p
                    [
                        es_action_unary<const_string_t::const_iterator>()
                    ]
                    | StatementList >> '}'
                )

            , FunctionDeclaration
                = "function" >> (
                    es_identifier_p[ es_action_identifier<const_string_t::const_iterator>() ]
                    >> (
                        FormalParameterList
                        | illegal_formal_parameter_list_p
                    )[ es_action_assign<const_string_t::const_iterator>() ]
                )

            , FunctionExpression
                = "function" >> (
                    FormalParameterList
                    | (
                        es_identifier_p[ es_action_identifier<const_string_t::const_iterator>() ]
                        >> FormalParameterList
                    )[ es_action_assign<const_string_t::const_iterator>() ]
                    | illegal_function_expression_p
                )

            , FormalParameterList
                = '(' 
                >> ( ')' >> es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ] | Parameter )
                >> (
                    '{' 
                    | illegal_function_body_block_start_bracket_not_found_p
                )
                >> FunctionBody
                [ 
                    es_action_functionbody<const_string_t::const_iterator>() 
                ]
                [ 
                    es_action_function<const_string_t::const_iterator>() 
                ]

            , Parameter
                = 
                es_identifier_p[ es_action_parameter<const_string_t::const_iterator>() ]
                >> (
                    ')'
                    | (
                        ',' >> (
                            Parameter[ es_action_binary<const_string_t::const_iterator>() ]
                            | illegal_formal_parameter_list_unknown_token_p
                        )
                    )
                )

            , FunctionBody
                = '}'
                >> es_epsilon_p
                [
                    es_action_nop<const_string_t::const_iterator>()
                ]
                | (
                    FunctionDeclaration
                    | Statement
                    | illegal_statement_error_p
                )
                >> FunctionBody
                [
                    es_action_binary<const_string_t::const_iterator>()
                ]

            , SourceElements
                = (
                    FunctionDeclaration
                    | Statement
                    | illegal_statement_error_p
                )
                >> !SourceElements
                [
                    es_action_binary<const_string_t::const_iterator>()
                ]

            , InputText
                = SourceElements
                | es_epsilon_p[ es_action_nop<const_string_t::const_iterator>() ]
                >> es_lexeme_d[ *es_line_terminater_ch_p ]
            ;
        }

        template <typename ScannerT>
        typename spirit::parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const
        {
            return InputText.parse(scan);
        }

    private:
        rule_t InputText
        , Token

        // Expressions
        , ElementList
        , PropertyNameAndValueList
        , MemberExpression_Notation
        , NewArgument
        , CallUnits
        , Arguments
        , Argument
        , LeftHandSideExpression

        , PostfixExpression
        , PostfixExpressionAfterLhs
        , UnaryExpression
        , MultiplicativeExpression
        , MultiplicativeExpressionAfterLhs
        , AdditiveExpression
        , AdditiveExpressionAfterLhs
        , ShiftExpression
        , ShiftExpressionAfterLhs
        , RelationalExpression
        , RelationalExpressionNoIn
        , RelationalExpressionAfterLhs
        , RelationalExpressionNoInAfterLhs
        , EqualityExpression
        , EqualityExpressionNoIn
        , EqualityExpressionAfterLhs
        , EqualityExpressionNoInAfterLhs
        , BitwiseANDExpression
        , BitwiseANDExpressionNoIn
        , BitwiseANDExpressionAfterLhs
        , BitwiseANDExpressionNoInAfterLhs
        , BitwiseXORExpression
        , BitwiseXORExpressionNoIn
        , BitwiseXORExpressionAfterLhs
        , BitwiseXORExpressionNoInAfterLhs
        , BitwiseORExpression
        , BitwiseORExpressionNoIn
        , BitwiseORExpressionAfterLhs
        , BitwiseORExpressionNoInAfterLhs
        , LogicalANDExpression
        , LogicalANDExpressionNoIn
        , LogicalANDExpressionAfterLhs
        , LogicalANDExpressionNoInAfterLhs
        , LogicalORExpression
        , LogicalORExpressionNoIn
        , LogicalORExpressionAfterLhs
        , LogicalORExpressionNoInAfterLhs
        , ConditionalExpression
        , ConditionalExpressionNoIn
        , ConditionalExpressionAfterLhs
        , ConditionalExpressionNoInAfterLhs
        , ConditionalExpression_Alternative
        , ConditionalExpressionNoIn_Alternative
        , AssignmentExpression
        , AssignmentExpressionNoIn
        , AssignmentOperator
        , AssignmentOperatorNoIn
        , Expression
        , ExpressionNoIn

        // Statements
        , Statement
        , StatementList
        , Block
        , VariableStatement
        , VariableDeclaration
        , IfStatement
        , IterationStatement_DoWhile
        , IterationStatement_While
        , IterationStatement_For
        , SwitchStatement

        , CaseClause_AfterDefault
        , CaseClause
        , DefaultClause
        , LabelledStatement
        , ContinueStatementNoIdentifier
        , BreakStatementNoIdentifier
        , ReturnStatementNoExp
        , WithStatement
        , TryStatement
        , ThrowStatement

        // Function and Programs
        , FunctionDeclaration
        , FunctionExpression
        , FormalParameterList
        , Parameter
        , FunctionBody
        , SourceElements
        ;
    };

} // namespace ecmascript

namespace ecmascript {

    struct es_parser
    {
        typedef wchar_t const * iteratorT;
        IActor& parse(iteratorT begin, iteratorT end) const
        {
            typedef spirit::skip_parser_iteration_policy<
                skip_parser,
                es_iteration_policy> const iter_policy_t;
            typedef spirit::scanner_policies<
                iter_policy_t,
                match_policy,
                action_policy> const scanner_policies_t;
            typedef spirit::scanner<
                iteratorT, 
                scanner_policies_t> const scanner_t;
#if ES_TRACE_PARSING_TIME
            clock_t t = clock();
#endif // ES_TRACE_PARSING_TIME
            static program_parser<scanner_t> const program_parser_;
            iter_policy_t const iter_policy(es_skip_p);
            scanner_policies_t const scanner_policies(iter_policy);
            scanner_t const scanner(begin, end, scanner_policies);
            switch (setjmp(error_info::get_jmp_buffer()))
            {
            case 0:
                program_parser_.parse(scanner);
#if ES_TRACE_PARSING_TIME
                wprintf(
                    L"parse: %.3f\n", 
                    double(clock() - t) / CLOCKS_PER_SEC);
                return es_semantic_action_base<const_string_t::const_iterator>().pop();
#endif // ES_TRACE_PARSING_TIME
 
            default:            
                error(begin, end, 
                    error_info::get_position(), 
                    error_info::get_message());
                throw;
            }
        }

    private:
        es_result
        output_error_message(
            unsigned int line_number,
            unsigned int colmn_number,
            iteratorT line,
            iteratorT message
            ) const
        {
            es_result result = es_con_green();
            ES_ASSERT(es_success == result);
            wprintf(L"line: %d\ncolmn: %d\n\n", line_number, colmn_number);
            result = es_con_intensity();
            ES_ASSERT(es_success == result);
            base_services::es_puts(line);
            wprintf(L"\n%s^\n", colmn_number ?
                const_string_t(colmn_number - 1, L' ').c_str(): L"");
            result = es_con_green();
            ES_ASSERT(es_success == result);
            base_services::es_puts(message);
            result = es_con_white();
            ES_ASSERT(es_success == result);
            return es_success;
        }

        es_result
        error(
            wchar_t const * begin,
            wchar_t const * end,
            wchar_t const * position,
            wchar_t const * message
            ) const
        {
            wprintf(L"parse error\n");
            typedef const_string_t string_t;
            unsigned int line_number = 0, colmn_number = 0;
            wchar_t const *it = begin, *start_of_line = begin;
            for (;it != position && it != end; ++it, ++colmn_number)
                if ((*it == '\r' || *it == '\n') 
                        && (it == begin || *(it - 1) != '\r'))
                    ++line_number, start_of_line = it + 1, colmn_number = 0;
            if (it == end)
                return es_fail;
            while (it != end 
                    && *it != 0 && *it != '\r' && *it != '\n' 
                    && ++it);
            return output_error_message(
                line_number, colmn_number,
                string_t(start_of_line, it).c_str(), message);
        }

    };
} // namespace ecmascript


