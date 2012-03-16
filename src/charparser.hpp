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


namespace ecmascript {

    struct es_anychar_parser_gen
    : public spirit::char_parser<es_anychar_parser_gen>
    {
        es_anychar_parser_gen()
        {
        }

        template <typename T>
        bool test(T) const
        {
            return true;
        }
    };

    es_anychar_parser_gen const es_anychar_p = es_anychar_parser_gen();

    struct es_space_parser_gen
    : public spirit::char_parser<es_space_parser_gen>
    {
        typedef es_space_parser_gen self_t;

        es_space_parser_gen()
        {
        }

        template <typename CharT>
        bool test(CharT ch) const throw()
        {
            return unicode::include<WhiteSpace|LineTerminator>(ch);
        }
    };

    es_space_parser_gen const es_space_p = es_space_parser_gen();

    struct line_terminater_ch_gen
    : public spirit::char_parser<line_terminater_ch_gen>
    {
        typedef es_space_parser_gen self_t;
        line_terminater_ch_gen() {}
        template <typename CharT>
        bool test(CharT ch) const throw()
        {
            return unicode::test<LineTerminator>(ch);
        }
    };

    line_terminater_ch_gen const es_line_terminater_ch_p
        = line_terminater_ch_gen();

} // namespace ecmascript

