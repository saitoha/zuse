/* ***** BEGIN LICENSE BLOCK Version: GPL 3.0 ***** 
 * Copyright (C) 2008-2011  Hayaki Saito <user@zuse.jp>
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
            typedef typename spirit::match_result<ScannerT, spirit::nil_t>::type
                type;
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
                                es_action_singlequotedstring()
                            ]
                            , '\''
                        )
                        , spirit::operator >> (
                            '\''
                            , es_epsilon_p[ es_action_nullstring() ]
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
                            , es_action_singlequotedstring
                        >
                        , spirit::chlit<char>
                    >
                    , spirit::sequence<
                        spirit::chlit<char>
                        , spirit::parser<
                            es_epsilon_parser
                        >::action<
                            es_epsilon_parser
                            , es_action_nullstring
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
                                es_action_doublequotedstring()
                            ]
                            , '"'
                        )
                        , spirit::operator >> (
                            '"'
                            , es_epsilon_p[ es_action_nullstring() ]
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
                            es_action_doublequotedstring
                        >,
                        spirit::chlit<char>
                    >,
                    spirit::sequence<
                        spirit::chlit<char>,
                        spirit::parser<
                            ecmascript::es_epsilon_parser
                        >::action<
                            es_epsilon_parser,
                            es_action_nullstring
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
        
        es_number_parser_gen() throw()
        : parser_(
            '0'
            >> (
                'x' >> es_hex_p[ es_action_number() ]
                | 'X' >> es_hex_p[ es_action_number() ]
            )
            | es_decimal_parser[ es_action_number() ]
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
                            , es_action_number
                        >
                    >
                    , spirit::sequence<
                        spirit::chlit<char>
                        , spirit::parser<
                            es_hex_parser_gen
                        >::action<
                            es_hex_parser_gen
                            , es_action_number
                        >
                    >
                >
            >
            , spirit::parser<es_decimal_parserarser_gen>::action<
                es_decimal_parserarser_gen
                , es_action_number
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

        program_parser() throw()
        {
            using namespace spirit;

            ElementList
                = (
                    AssignmentExpression
                    >> (
                        ',' >> (
                            ElementList
                            | ']'
                            >> es_epsilon_p[ es_action_undefined() ]
                        )
                        | ']'
                        >> es_epsilon_p[ es_action_nop() ]
                    )
                    | ','
                    >> es_epsilon_p[ es_action_undefined() ]
                    >> (
                        ElementList
                        | ']'
                        >> es_epsilon_p[ es_action_undefined() ]
                    )
                )[ es_action_arrayelement() ]

            , PropertyNameAndValueList
                = (
                    es_identifier_p[ es_action_string() ]
                    | es_singlequoted_stringliteral_parser
                    | es_doublequoted_stringliteral_parser
                    | es_number_parser
                )
                >> ':' >> AssignmentExpression
                >> (
                    ',' >> PropertyNameAndValueList
                    | '}'
                    >> es_epsilon_p[ es_action_nop() ]
                )[ es_action_objectelement() ]

            , Token
                = es_identifier_p[ es_action_identifier() ]
                | '(' >> Expression >> ')'
                | '{' >> (
                    '}'
                    >> es_epsilon_p[ es_action_nop() ]
                    | PropertyNameAndValueList
                )[ es_action_object() ]
                | '[' >> (
                    ']'
                    >> es_epsilon_p[ es_action_nop() ]
                    | ElementList
                )
                [
                    es_action_array()
                ]
                | es_singlequoted_stringliteral_parser
                | es_doublequoted_stringliteral_parser
                | "true" >> es_epsilon_p[ es_action_true() ]
                | "this" >> es_epsilon_p[ es_action_this() ]
                | "false"  >> es_epsilon_p[ es_action_false() ]
                | FunctionExpression
                | (
                    "null" >> es_epsilon_p[ es_action_null() ]
                    | "new" >> Token >> !MemberExpression_Notation
                    [
                        es_action_binary()
                    ]
                    >> (
                        '(' >> (
                            NewArgument
                            | ')'
                            >> es_epsilon_p[ es_action_newargend() ]
                            >> !CallUnits
                            [
                                es_action_binary()
                            ]
                        )
                        [
                            es_action_args()
                        ]
                        [
                            es_action_binary()
                        ]
                        | (
                            es_epsilon_p[ es_action_new() ]
                            >> AssignmentOperator
                            | ConditionalExpressionAfterLhs
                        ) >> !( ',' >> Expression )
                        [
                            es_action_binary()
                        ]
                    )
                    >> (';' | es_epsilon_p)
                )
                | '/' >> (
                    ( '\\' >> es_anychar_p | (es_anychar_p - '*' - '/') )
                    >> *( '\\' >> es_anychar_p | (es_anychar_p - '/') )
                )[ es_action_nativestring() ]
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
                )[ es_action_regexp() ]
                | es_number_parser

            , MemberExpression_Notation
                = (
                    '.' >> es_identifier_p[ es_action_member() ]
                    | ( '[' >> Expression >> ']' )[ es_action_bracket() ]
                )
                >> !(
                    Arguments
                    | MemberExpression_Notation
                )
                [
                    es_action_binary()
                ]

            , NewArgument
                = AssignmentExpression
                >> (
                    ',' >> NewArgument
                    | ')'
                    >> es_epsilon_p[ es_action_newargend() ]
                    >> !CallUnits
                    [
                        es_action_binary()
                    ]
                )[ es_action_arg() ]

            , CallUnits
                = (
                    '.' >> es_identifier_p[ es_action_member() ]
                    | ( '[' >> Expression >> ']' )[ es_action_bracket() ]
                ) >> !(
                    CallUnits
                    | Arguments
                )
                [
                    es_action_binary()
                ]

            , Arguments
                = '(' >> (
                    Argument
                    | ')'
                    >> es_epsilon_p[ es_action_argend() ]
                    >> !CallUnits
                    [
                        es_action_binary()
                    ]
                )[ es_action_args() ]

            , Argument
                = AssignmentExpression
                >> (
                    ',' >> Argument
                    | ')'
                    >> es_epsilon_p[ es_action_argend() ]
                    >> !CallUnits
                    [
                        es_action_binary()
                    ]
                )[ es_action_arg() ]

            , LeftHandSideExpression
                = (
                    Token >> (
                        (
                            !MemberExpression_Notation
                            [
                                es_action_binary()
                            ]
                            >> !Arguments
                            [
                                es_action_binary()
                            ]
                        )
                        [
                            es_action_call()
                        ]
                        | es_epsilon_p
                    )
                )

            , PostfixExpression
                = LeftHandSideExpression
                >> PostfixExpressionAfterLhs

            , PostfixExpressionAfterLhs
                = *(
                    "++" >> es_epsilon_p[ es_action_postinc() ]
                    | "--" >> es_epsilon_p[ es_action_postdec() ]
                )

            , UnaryExpression
                = PostfixExpression
                | "++" >> UnaryExpression[ es_action_inc() ]
                | "--" >> UnaryExpression[ es_action_dec() ]
                | '+' >> UnaryExpression[ es_action_unaryplus() ]
                | '-' >> UnaryExpression[ es_action_unaryminus() ]
                | '~' >> UnaryExpression[ es_action_tilde() ]
                | '!' >> UnaryExpression[ es_action_not() ]
                | "delete" >> UnaryExpression[ es_action_delete() ]
                | "void" >> UnaryExpression[ es_action_void() ]
                | "typeof" >> UnaryExpression[ es_action_typeof() ]

            , MultiplicativeExpression
                = UnaryExpression
                >> *(
                    '*' >> UnaryExpression[ es_action_mul() ]
                    | '/' >> UnaryExpression[ es_action_div() ]
                    | '%' >> UnaryExpression[ es_action_mod() ]
                )

            , MultiplicativeExpressionAfterLhs
                = PostfixExpressionAfterLhs
                >> *(
                    '*' >> UnaryExpression[ es_action_mul() ]
                    | '/' >> UnaryExpression[ es_action_div() ]
                    | '%' >> UnaryExpression[ es_action_mod() ]
                )

            , AdditiveExpression
                = MultiplicativeExpression
                >> *(
                    '+' >> MultiplicativeExpression[ es_action_plus() ]
                    | '-' >> MultiplicativeExpression[ es_action_minus() ]
                )

            , AdditiveExpressionAfterLhs
                = MultiplicativeExpressionAfterLhs
                >> *(
                    '+' >> MultiplicativeExpression[ es_action_plus() ]
                    | '-' >> MultiplicativeExpression[ es_action_minus() ]
                )

            , ShiftExpression
                = AdditiveExpression
                >> *(
                    "<<" >> AdditiveExpression[ es_action_shl() ]
                    | ">>>" >> AdditiveExpression[ es_action_sar() ]
                    | ">>" >> AdditiveExpression[ es_action_shr() ]
                )

            , ShiftExpressionAfterLhs
                = AdditiveExpressionAfterLhs
                >> *(
                    "<<" >> AdditiveExpression[ es_action_shl() ]
                    | ">>>" >> AdditiveExpression[ es_action_sar() ]
                    | ">>" >> AdditiveExpression[ es_action_shr() ]
                )

            , RelationalExpression
                = ShiftExpression
                >> *(
                    "<=" >> ShiftExpression[ es_action_le() ]
                    | ">=" >> ShiftExpression[ es_action_ge() ]
                    | '<' >> ShiftExpression[ es_action_lt() ]
                    | '>' >> ShiftExpression[ es_action_gt() ]
                    | "instanceof" >> ShiftExpression[ es_action_instanceof() ]
                    | "in" >> ShiftExpression[ es_action_in() ]
                )

            , RelationalExpressionNoIn
                = ShiftExpression
                >>  *(
                    "<=" >> ShiftExpression[ es_action_le() ]
                    | ">=" >> ShiftExpression[ es_action_ge() ]
                    | '<' >> ShiftExpression[ es_action_lt() ]
                    | '>' >> ShiftExpression[ es_action_gt() ]
                    | "instanceof" >> ShiftExpression[ es_action_instanceof() ]
                )

            , RelationalExpressionAfterLhs
                = ShiftExpressionAfterLhs
                >> *(
                    "<=" >> ShiftExpression[ es_action_le() ]
                    | ">=" >> ShiftExpression[ es_action_ge() ]
                    | '<' >> ShiftExpression[ es_action_lt() ]
                    | '>' >> ShiftExpression[ es_action_gt() ]
                    | "instanceof" >> ShiftExpression[ es_action_instanceof() ]
                    | "in" >> ShiftExpression[ es_action_in() ]
                )

            , RelationalExpressionNoInAfterLhs
                = ShiftExpressionAfterLhs
                >> *(
                    "<=" >> ShiftExpression[ es_action_le() ]
                    | ">=" >> ShiftExpression[ es_action_ge() ]
                    | '<' >> ShiftExpression[ es_action_lt() ]
                    | '>' >> ShiftExpression[ es_action_gt() ]
                    | "instanceof" >> ShiftExpression[ es_action_instanceof() ]
                )

            , EqualityExpression
                = RelationalExpression
                >> *(
                    "===" >> RelationalExpression[ es_action_stricteq() ]
                    | "!==" >> RelationalExpression[ es_action_strictne() ]
                    | "==" >> RelationalExpression[ es_action_eq() ]
                    | "!=" >> RelationalExpression[ es_action_ne() ]
                )

            , EqualityExpressionNoIn
                = RelationalExpressionNoIn
                >> *(
                    "===" >> RelationalExpressionNoIn[ es_action_stricteq() ]
                    | "!==" >> RelationalExpressionNoIn[ es_action_strictne() ]
                    | "==" >> RelationalExpressionNoIn[ es_action_eq() ]
                    | "!=" >> RelationalExpressionNoIn[ es_action_ne() ]
                )

            , EqualityExpressionAfterLhs
                = RelationalExpressionAfterLhs
                >> *(
                    "===" >> RelationalExpression[ es_action_stricteq() ]
                    | "!==" >> RelationalExpression[ es_action_strictne() ]
                    | "==" >> RelationalExpression[ es_action_eq() ]
                    | "!=" >> RelationalExpression[ es_action_ne() ]
                )

            , EqualityExpressionNoInAfterLhs
                = RelationalExpressionNoInAfterLhs
                >> *(
                    "===" >> RelationalExpressionNoIn[ es_action_stricteq() ]
                    | "!==" >> RelationalExpressionNoIn[ es_action_strictne() ]
                    | "==" >> RelationalExpressionNoIn[ es_action_eq() ]
                    | "!=" >> RelationalExpressionNoIn[ es_action_ne() ]
                )

            , BitwiseANDExpression
                = EqualityExpression
                >> *( '&' >> EqualityExpression[ es_action_bitand() ] )

            , BitwiseANDExpressionNoIn
                = EqualityExpressionNoIn
                >> *( '&' >> EqualityExpressionNoIn[ es_action_bitand() ] )

            , BitwiseANDExpressionAfterLhs
                = EqualityExpressionAfterLhs
                >> *( '&' >> EqualityExpression[ es_action_bitand() ] )

            , BitwiseANDExpressionNoInAfterLhs
                = EqualityExpressionNoInAfterLhs
                >> *( '&' >> EqualityExpressionNoIn[ es_action_bitand() ] )

            , BitwiseXORExpression
                = BitwiseANDExpression
                >> *( '^' >> BitwiseANDExpression[ es_action_bitxor() ] )

            , BitwiseXORExpressionNoIn
                = BitwiseANDExpressionNoIn
                >> *( '^' >> BitwiseANDExpressionNoIn[ es_action_bitxor() ] )

            , BitwiseXORExpressionAfterLhs
                = BitwiseANDExpressionAfterLhs
                >> *( '^' >> BitwiseANDExpression[ es_action_bitxor() ] )

            , BitwiseXORExpressionNoInAfterLhs
                = BitwiseANDExpressionNoInAfterLhs
                >> *( '^' >> BitwiseANDExpressionNoIn[ es_action_bitxor() ] )

            , BitwiseORExpression
                = BitwiseXORExpression
                >> *( '|' >> BitwiseXORExpression[ es_action_bitor() ] )

            , BitwiseORExpressionNoIn
                = BitwiseXORExpressionNoIn
                >> *( '|' >> BitwiseXORExpressionNoIn[ es_action_bitor() ] )

            , BitwiseORExpressionAfterLhs
                = BitwiseXORExpressionAfterLhs
                >> *( '|' >> BitwiseXORExpression[ es_action_bitor() ] )

            , BitwiseORExpressionNoInAfterLhs
                = BitwiseXORExpressionNoInAfterLhs
                >> *( '|' >> BitwiseXORExpressionNoIn[ es_action_bitor() ] )

            , LogicalANDExpression
                = BitwiseORExpression
                >> *( 
                    "&&" 
                    >> BitwiseORExpression
                    [ 
                        es_action_and() 
                    ]
                    [ 
                        es_action_binary() 
                    ] 
                )

            , LogicalANDExpressionNoIn
                = BitwiseXORExpressionNoIn
                >> *( 
                    "&&" 
                    >> BitwiseXORExpressionNoIn
                    [ 
                        es_action_and() 
                    ]
                    [ 
                        es_action_binary() 
                    ] 
                )

            , LogicalANDExpressionAfterLhs
                = BitwiseORExpressionAfterLhs
                >> *( 
                    "&&" 
                    >> BitwiseORExpression
                    [ 
                        es_action_and() 
                    ]
                    [ 
                        es_action_binary() 
                    ] 
                )

            , LogicalANDExpressionNoInAfterLhs
                = BitwiseORExpressionNoInAfterLhs
                >> *( 
                    "&&" 
                    >> BitwiseORExpressionNoIn
                    [ 
                        es_action_and() 
                    ]
                    [ 
                        es_action_binary() 
                    ] 
                )

            , LogicalORExpression
                = LogicalANDExpression
                >> *( 
                    "||" 
                    >> LogicalANDExpression
                    [ 
                        es_action_or() 
                    ]
                    [ 
                        es_action_binary() 
                    ] 
                )

            , LogicalORExpressionNoIn
                = LogicalANDExpressionNoIn
                >> *( 
                    "||" 
                    >> LogicalANDExpressionNoIn
                    [ 
                        es_action_or() 
                    ]
                    [ 
                        es_action_binary() 
                    ] 
                )

            , LogicalORExpressionAfterLhs
                = LogicalANDExpressionAfterLhs
                >> *( 
                    "||" 
                    >> LogicalANDExpression
                    [ 
                        es_action_or() 
                    ]
                    [ 
                        es_action_binary() 
                    ] 
                )

            , LogicalORExpressionNoInAfterLhs
                = LogicalANDExpressionNoInAfterLhs
                >> *( 
                    "||" 
                    >> LogicalANDExpressionNoIn
                    [ 
                        es_action_or() 
                    ]
                    [ 
                        es_action_binary() 
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
                    es_action_alternative() 
                ]
                [ 
                    es_action_binary() 
                ]
                >> !ConditionalExpression_Alternative

            , ConditionalExpressionNoIn_Alternative
                = '?' 
                >> AssignmentExpressionNoIn
                >> ':' 
                >> AssignmentExpressionNoIn
                [ 
                    es_action_alternative() 
                ]
                [ 
                    es_action_binary() 
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
                = es_lexeme_d
                [
                    '=' >> AssignmentExpression
                    [ 
                        es_action_assign() 
                    ]
                    | "*=" >> AssignmentExpression
                    [ 
                        es_action_assignmul()
                    ]
                    | "/=" >> AssignmentExpression
                    [ 
                        es_action_assigndiv() 
                    ]
                    | "%=" >> AssignmentExpression
                    [ 
                        es_action_assignmod() 
                    ]
                    | "+=" >> AssignmentExpression
                    [ 
                        es_action_assignplus() 
                    ]
                    | "-=" >> AssignmentExpression
                    [ 
                        es_action_assignminus() 
                    ]
                    | "<<=" >> AssignmentExpression
                    [ 
                        es_action_assignshl() 
                    ]
                    | ">>>=" >> AssignmentExpression
                    [ 
                        es_action_assignsar() 
                    ]
                    | ">>=" >> AssignmentExpression
                    [ 
                        es_action_assignshr() 
                    ]
                    | "&=" >> AssignmentExpression
                    [ 
                        es_action_assignand() 
                    ]
                    | "^=" >> AssignmentExpression
                    [ 
                        es_action_assignxor() 
                    ]
                    | "|=" >> AssignmentExpression
                    [ 
                        es_action_assignor() 
                    ]
                ] >> !AssignmentOperator

            , AssignmentOperatorNoIn
                = (
                    '=' >> AssignmentExpressionNoIn
                    [ 
                        es_action_assign() 
                    ]
                    | "*=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignmul() 
                    ]
                    | "/=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assigndiv() 
                    ]
                    | "%=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignmod() 
                    ]
                    | "+=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignplus() 
                    ]
                    | "-=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignminus() 
                    ]
                    | "<<=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignshl() 
                    ]
                    | ">>>=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignsar() 
                    ]
                    | ">>=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignshr() 
                    ]
                    | "&=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignand() 
                    ]
                    | "^=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignxor() 
                    ]
                    | "|=" >> AssignmentExpressionNoIn
                    [ 
                        es_action_assignor() 
                    ]
                ) >> !AssignmentOperatorNoIn

            , Expression
                = AssignmentExpression
                >> !( ',' >> Expression )
                [
                    es_action_binary()
                ]

            , ExpressionNoIn
                = AssignmentExpressionNoIn
                >> !( ',' >> ExpressionNoIn )
                [
                    es_action_binary()
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
                | (es_epsilon_p >> ';')[ es_action_unary() ]

            , VariableStatement
                = "var" >> (
                    VariableDeclaration
                    >> *( ',' >> VariableDeclaration )
                    [
                        es_action_binary()
                    ]
                )
                >> (';' | es_epsilon_p)

            , VariableDeclaration
                = es_identifier_p
                [
                    es_action_nativestring()
                ]
                >> (
                    ( '=' >> AssignmentExpression )[ es_action_varinit() ]
                    | es_epsilon_p[ es_action_var() ]
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
                    )[ es_action_ifelse() ]
                    | es_epsilon_p[ es_action_if() ]
                )[ es_action_binary() ]
                >> (';' | es_epsilon_p)

            , IterationStatement_DoWhile
                = "do" >> Statement
                >> "while" >> '(' >> Expression[ es_action_dowhile() ] >> ')'
                >> (';' | es_epsilon_p)

            , IterationStatement_While
                = "while" >> ( '(' >> Expression >> ')' )
                >> Statement
                [
                    es_action_while()
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
                            >> Statement[ es_action_forin() ]
                            | (
                                AssignmentOperatorNoIn
                                | ConditionalExpressionNoInAfterLhs
                            )
                            >> !( ',' >> ExpressionNoIn )
                            [
                                es_action_binary()
                            ]
                            >> ';'
                            >> ( Expression | es_epsilon_p[ es_action_nop() ] ) 
                            >> ';'
                            >> ( Expression | es_epsilon_p[ es_action_nop() ] )
                            >> ')'
                            >> Statement[ es_action_for() ]
                            [
                                es_action_binary()
                            ]
                        )
                        | ConditionalExpressionNoIn
                        >> !( ',' >> ExpressionNoIn )
                        [
                            es_action_binary()
                        ]
                        >> ';'
                        >> ( Expression | es_epsilon_p[ es_action_nop() ] ) 
                        >> ';'
                        >> ( Expression | es_epsilon_p[ es_action_nop() ] )
                        >> ')'
                        >> Statement[ es_action_for() ]
                        [
                            es_action_binary()
                        ]
                        | "var" >> es_identifier_p
                        [
                            es_action_nativestring()
                        ]
                        >> (
                            ( '=' >> AssignmentExpression )
                            >> (
                                "in" 
                                >> es_epsilon_p[ es_action_forinvarinit() ] 
                                >> Expression
                                >> ')'
                                >> Statement[ es_action_forin() ]
                                | es_epsilon_p[ es_action_varinit() ]
                                >> *( ',' >> VariableDeclaration )
                                [
                                    es_action_binary()
                                ]
                                >> ';'
                                >> ( 
                                    Expression 
                                    | es_epsilon_p[ es_action_nop() ] 
                                ) 
                                >> ';'
                                >> ( 
                                    Expression 
                                    | es_epsilon_p[ es_action_nop() ] 
                                )
                                >> ')'
                                >> Statement[ es_action_for() ]
                                [
                                    es_action_binary()
                                ]
                            )
                            | es_epsilon_p
                            >> (
                                "in"
                                >> es_epsilon_p[ es_action_forinvar() ] 
                                >> Expression
                                >> ')'
                                >> Statement[ es_action_forin() ]
                                | es_epsilon_p[ es_action_var() ]
                                >> *( ',' >> VariableDeclaration )
                                [
                                    es_action_binary()
                                ]
                                >> ';'
                                >> ( 
                                    Expression 
                                    | es_epsilon_p[ es_action_nop() ] 
                                ) 
                                >> ';'
                                >> ( 
                                    Expression 
                                    | es_epsilon_p[ es_action_nop() ] 
                                )
                                >> ')'
                                >> Statement[ es_action_for() ]
                                [
                                    es_action_binary()
                                ]
                            )
                        )
                        | es_epsilon_p[ es_action_nop() ]
                        >> ';'
                        >> ( Expression | es_epsilon_p[ es_action_nop() ] ) 
                        >> ';'
                        >> ( Expression | es_epsilon_p[ es_action_nop() ] )
                        >> ')'
                        >> Statement[ es_action_for() ]
                        [
                            es_action_binary()
                        ]
                    )
                )

            , SwitchStatement
                = "switch"
                >> ( '(' >> Expression >> ')' )
                >> '{' >> ( CaseClause | DefaultClause )
                [
                    es_action_switch()
                ]

            , CaseClause
                = '}'
                >> es_epsilon_p[ es_action_nop() ]
                | "case" >> Expression >> ':'
                >> (
                    StatementList
                    | es_epsilon_p[ es_action_nop() ]
                )
                [
                    es_action_case()
                ]
                >> ( CaseClause | DefaultClause )
                [
                    es_action_binary()
                ]
                
            , CaseClause_AfterDefault
                = '}'
                >> es_epsilon_p[ es_action_nop() ]
                | "case" >> Expression >> ':'
                >> (
                    StatementList
                    | es_epsilon_p[ es_action_nop() ]
                )
                [
                    es_action_case()
                ]
                >> CaseClause_AfterDefault
                [
                    es_action_binary()
                ]

            , DefaultClause
                = "default"
                >> (
                    ':'
                    >> (
                        StatementList
                        | es_epsilon_p[ es_action_nop() ]
                    )
                    >> (
                        CaseClause
                        | es_epsilon_p
                    )
                    [
                        es_action_default()
                    ]
                )

            , LabelledStatement
                = ( es_identifier_p >> ':' )
                [
                    es_action_nativestring()
                ]
                >> Statement
                [
                    es_action_label()
                ]

            , ContinueStatementNoIdentifier
                = "continue" >> (
                    es_lexeme_d
                    [
                        *es_space_p
                        >> ( es_line_terminater_ch_p | ';' )
                    ]
                    [
                        es_action_continuenoarg()
                    ]
                    | es_identifier_p
                    [
                        es_action_continue()
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
                        es_action_breaknoarg()
                    ]
                    | es_identifier_p[ es_action_break() ]
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
                        es_action_returnnoarg()
                    ]
                    | Expression[ es_action_return() ]
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
                    es_action_throw()
                ]
                >> (';' | es_epsilon_p)

            , WithStatement
                = "with" >> ( '(' >> Expression >> ')' )
                >> Statement[ es_action_with() ]

            , TryStatement
                = "try"
                >> Block
                >> (
                    "catch"
                    >> (
                        '('
                        >> es_identifier_p[ es_action_identifier() ]
                        >> ')'
                    )
                    >> Block[ es_action_trycatch() ]
                    >> !(
                        "finally"
                        >> Block
                        [
                            es_action_binary()
                        ]
                    )
                    | "finally" >> Block[ es_action_tryfinally() ]
                )

            , StatementList
                = Statement
                >> !StatementList
                [
                    es_action_binary()
                ]

            , Block
                = '{' >> (
                    '}'
                    >> es_epsilon_p
                    [
                        es_action_unary()
                    ]
                    | StatementList >> '}'
                )

            , FunctionDeclaration
                = "function" >> (
                    es_identifier_p[ es_action_identifier() ]
                    >> (
                        FormalParameterList
                        | illegal_formal_parameter_list_p
                    )[ es_action_assign() ]
                )

            , FunctionExpression
                = "function" >> (
                    FormalParameterList
                    | (
                        es_identifier_p[ es_action_identifier() ]
                        >> FormalParameterList
                    )[ es_action_assign() ]
                    | illegal_function_expression_p
                )

            , FormalParameterList
                = (
                    '(' 
                    | illegal_formal_parameter_list_start_bracket_not_found_p
                )
                >> ( ')' >> es_epsilon_p[ es_action_nop() ] | Parameter )
                >> (
                    '{' 
                    | illegal_function_body_block_start_bracket_not_found_p
                )
                >> FunctionBody
                [ 
                    es_action_functionbody() 
                ]
                [ 
                    es_action_function() 
                ]

            , Parameter
                = 
                es_identifier_p[ es_action_parameter() ]
                >> (
                    ')'
                    | (
                        ',' >> (
                            Parameter[ es_action_binary() ]
                            | illegal_formal_parameter_list_unknown_token_p
                        )
                    )
                )

            , FunctionBody
                = '}'
                >> es_epsilon_p
                [
                    es_action_nop()
                ]
                | (
                    FunctionDeclaration
                    | Statement
                    | illegal_statement_error_p
                )
                >> FunctionBody
                [
                    es_action_binary()
                ]

            , SourceElements
                = (
                    FunctionDeclaration
                    | Statement
                    | illegal_statement_error_p
                )
                >> !SourceElements
                [
                    es_action_binary()
                ]

            , InputText
                = SourceElements
                | es_epsilon_p[ es_action_nop() ]
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
        , UnaryExpressionAfterLhs
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
            std::clock_t t = std::clock();
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
                    double(std::clock() - t) / CLOCKS_PER_SEC);
                return es_semantic_action_base().pop();
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
                std::wstring(colmn_number - 1, L' ').c_str(): L"");
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
            typedef std::wstring string_t;
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


