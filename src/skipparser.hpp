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

    //////////////////////////////////////////////////////////////////////////
    //
    //  es_counted_scanner
    //
    template <typename parserT, typename ScannerT>
    struct es_counted_scanner
    {
        typedef es_counted_scanner self_t;

        explicit es_counted_scanner(ScannerT const& scan) throw()
        : scan_(scan)
        , first_position_(scan.first)
        , length_(0)
        {
        }

        self_t& operator ++ () throw()
        {
            ++scan_.first;
            ++length_;
            return *this;
        }

        self_t& operator -- () throw()
        {
            --scan_.first;
            --length_;
            return *this;
        }

        std::size_t length() const throw()
        {
            return length_;
        }

        bool at_end() const throw()
        {
            return scan_.at_end();
        }

        template <typename T>
        bool current_is(T rhs) const throw()
        {
            return *scan_ == rhs;
        }

        template <SOURCE_CHARACTER_CATEGORY category>
        bool test() const throw()
        {
            return unicode::test<category>(*scan_);
        }

        typename spirit::parser_result<parserT, ScannerT>::type
        create_result() const throw()
        {
            return 0 < length_ ?
                scan_.create_match(
                    length_,
                    spirit::nil_t(),
                    first_position_,
                    scan_.first):
                scan_.no_match();
        }

    private:
        ScannerT const& scan_;
        typename ScannerT::iterator_t first_position_;
        std::size_t length_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  skip_parser
    //
    struct skip_parser
    : public spirit::parser<skip_parser>
    {
        typedef skip_parser self_t;

        template <typename parserT, typename ScannerT>
        bool scan_to_line_terminator(
            es_counted_scanner<parserT, ScannerT>& counted_scanner
            ) const throw()
        {
            typedef es_counted_scanner<parserT, ScannerT> scanner_t;
            while (!(++counted_scanner).at_end())
                if (counted_scanner.template test<LineTerminator>())
                    return ++counted_scanner, true;
            return false;
        }

        template <typename parserT, typename ScannerT>
        bool scan_to_multiline_comment_terminator(
            es_counted_scanner<parserT, ScannerT>& counted_scanner
            ) const throw()
        {
            while (!(++counted_scanner).at_end())
            {
                if (counted_scanner.current_is('*'))
                {
                    ++counted_scanner;
                    if (counted_scanner.current_is('/'))
                    {
                        ++counted_scanner;
                        return true;
                    }
                    --counted_scanner;
                }
            }
            return false;
        }

        template <typename ScannerT>
        typename spirit::parser_result<self_t, ScannerT>::type
            parse(ScannerT const& scan) const throw()
        {
            es_counted_scanner<self_t, ScannerT> counted_scanner(scan);
            while (!scan.at_end())
            {
                if (unicode::include<WhiteSpace|LineTerminator>(*scan))
                {
                    ++counted_scanner;
                }
                else if (*scan == '/')
                {
                    ++counted_scanner;
                    if (counted_scanner.at_end())
                    {
                        --counted_scanner;
                        break;
                    }
                    if (counted_scanner.current_is('/'))
                        scan_to_line_terminator(counted_scanner);
                    else if (counted_scanner.current_is('*'))
                        scan_to_multiline_comment_terminator(counted_scanner);
                    else
                    {
                        --counted_scanner;
                        break;
                    }
                }
                else break;
            }
            return counted_scanner.create_result();
        }
    };

    skip_parser const es_skip_p = skip_parser();

} // namespace ecmascript

