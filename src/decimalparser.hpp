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

    struct es_decimal_parserarser_gen
    : public spirit::parser<es_decimal_parserarser_gen>
    {
        typedef es_decimal_parserarser_gen self_t;

        template <typename ScannerT>
        struct result
        {
            typedef typename spirit::match_result<ScannerT, double>::type
                type;
        };

        template <typename ScannerT>
        typename spirit::parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const
        {
            typedef typename ScannerT::iterator_t iterator_t;
            scan.skip(scan);
            wchar_t const * it_end = 0;
            iterator_t const s(scan.first);
            double result = parse_impl(scan.first, &it_end);
            if (scan.first == it_end)
                return scan.no_match();
            ES_ASSERT(it_end - s > 0);
            scan.first = it_end;
            return scan.create_match(
                std::size_t(it_end - s), result, s, scan.first);
        }

        template <typename charT>
        double parse_impl(charT const* first, charT const ** last) const
        {
            charT const * it = first;
            long long int result = 0;
            charT const * dot = 0;
            bool nagate = false;
            charT c = *it;
            if ('-' == c)
                nagate = true;
            else if  ('+' == c)
                ++it;
            else if ('0' <= c && c <= '9')
                result = c - '0';
            else if ('.' == c)
                dot = it;
            else
                return *last = first, 0;
            while (c = *++it, true)
                if ('0' <= c && c <= '9')
                    result = result * 10 + (c - '0');
                else if ('.' == c)
                    dot = it;
                else if ('E' == c || 'e' == c)
                    return ++it, parse_impl2(
                        it, first, last, nagate ?
                            - result
                            : result, 0 == dot ? 0: dot - it + 1);
                else
                    break;
            return *last = it, it == first ?
                0 :
                (nagate ? - result : result) 
                    * pow(10., 0 == dot ? 0: dot - it + 1);
        }

        template <typename charT>
        double parse_impl2(
            charT const* it,
            charT const* first,
            charT const ** last,
            long long int result,
            int offset_exponent
            ) const
        {
            int exp = 0;
            charT c = *it;
            switch (c)
            {
            case '-':
                c = *++it;
                if ('0' > c && c > '9')
                    return *last = first, 0;
                exp = '0' - c;
                break;
            case '+':
                c = *++it;
            default:
                if ('0' > c && c > '9')
                    return *last = first, 0;
                exp = c - '0';
            }
            c = *++it;
            while ('0' <= c && c <= '9')
                exp = exp * 10 + (c - '0'), c = *++it;
            *last = it;
            return result * pow(10., offset_exponent + exp);
        }
    };

    const es_decimal_parserarser_gen es_decimal_parser
        = es_decimal_parserarser_gen();

} // namespace ecmascript
