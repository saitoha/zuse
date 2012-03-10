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
	
    struct es_hex_parser_gen
    : public spirit::parser<es_hex_parser_gen>
    {
		
        typedef es_hex_parser_gen self_t;
		
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
            wchar_t * it_end = 0;
            iterator_t const s(scan.first);
            double result = wcstol(scan.first, &it_end, 16);
            if (scan.first == it_end)
                return scan.no_match();
            ES_ASSERT(it_end - s > 0);
            scan.first = it_end;
            return scan.create_match(size_t(it_end - s), result, s, scan.first);
        }
		
    };
	
    static const es_hex_parser_gen es_hex_p
	= es_hex_parser_gen();
	
    template <char quote_char>
    struct es_quoted_string_parser_gen
    : public spirit::parser<es_quoted_string_parser_gen<quote_char> >
    {
        typedef es_quoted_string_parser_gen<quote_char> self_t;
		
        template <typename ScannerT>
        struct result
        {
            typedef typename spirit::match_result<ScannerT, const_string_t>::type
			type;
        };
		
        template <typename ScannerT>
        typename spirit::parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const
        {
            typedef typename spirit::parser_result<self_t, ScannerT>::type result_t;
            typedef typename ScannerT::iterator_t iterator_t;
            typedef spirit::scanner_policies<
				spirit::no_skipper_iteration_policy<
					typename ScannerT::iteration_policy_t>,
				typename ScannerT::match_policy_t,
				typename ScannerT::action_policy_t
            > policies_t;
            scan.skip(scan);
            iterator_t const s(scan.first);
            const_string_t buffer;
            ptrdiff_t len = escapechar_parse(scan.change_policies(policies_t(scan)), buffer);
            if (len < 0)
                return scan.no_match();
            ES_ASSERT(len >= 0);
            return scan.create_match(size_t(len), buffer, s, scan.first);
        }
		
    private:
        template <typename ScannerT>
        ptrdiff_t
        escapechar_parse(ScannerT const& scan, const_string_t& buffer) const
        {
            typedef typename spirit::parser_result<self_t, ScannerT>::type
			result_t;
            ES_ASSERT(buffer.empty());
			
            int len = 0;
            if ( scan.at_end() )
                return -1;
		step:
            if ( *scan == quote_char )
				return len;			
			if ( scan.at_end() )
				return -1;
			++len;
			if ( *scan != '\\' )
			{
				buffer += *scan;
				++scan;
				goto step;
			}
			++len, ++scan;
			if ( scan.at_end() )
				return -1;
			switch ( *scan )
			{
				case 'r':
					buffer += '\r';
					break;
				case 'n':
					buffer += '\n';
					break;
				case 't':
					buffer += '\t';
					break;
				case 'v':
					buffer += '\v';
					break;
				case '"':
					buffer += quote_char;
					break;
				case '\\':
					buffer += '\\';
					break;
				default:
					buffer += *scan;
					break;
			}
			++scan;
			goto step;
		}
    };
	
    static const es_quoted_string_parser_gen<'\''> es_singlequoted_string_p
	= es_quoted_string_parser_gen<'\''>();
    static const es_quoted_string_parser_gen<'"'> es_doublequoted_string_p
	= es_quoted_string_parser_gen<'"'>();
	
} // namespace ecmascript

