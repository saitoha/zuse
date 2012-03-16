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

    namespace impl {

        template <typename ScannerT>
        inline bool
        cmp(ScannerT const& scan, char const* it, int& len)
        {
            if (*it == '\0')
                return !unicode::test<IdentifierChar>(*scan);
            if (scan.at_end() || *scan != wchar_t(*it))
                return false;
            return cmp(++scan, ++it, ++len);
        }

        template <typename ScannerT>
        inline int
        scan_to_end(ScannerT const& scan, int len)
        {
            while (unicode::test<IdentifierChar>(*scan))
                ++len, ++scan;
            return len;
        }

        template <typename ScannerT>
        std::ptrdiff_t
        identifier_parse(ScannerT const& scan)
        {

            int len = 0;

            /*if ( scan.at_end() )
                return -1;*/
            switch( *scan )
            {
            case 'a':
                if ( cmp( ++scan, "bstruct", ++len ) )
                    return -1;
                return scan_to_end(scan, len);

            case 'b':
                ++len;
                /*if ( (++scan).at_end() )
                    return len || -1;*/
                switch ( *++scan )
                {
                case 'r':
                    if ( cmp( ++scan, "eak", ++len ) )
                        return -1;
                    break;

                case 'y':
                    if ( cmp( ++scan, "te", ++len ) )
                        return -1;
                    break;

                case 'o':
                    if ( cmp( ++scan, "olean", ++len ) )
                        return -1;
                    break;
                }
                return scan_to_end(scan, len);

            case 'c':
                ++len;
                /*if ( (++scan).at_end() )
                    return len || -1;*/
                switch ( *++scan )
                {
                case 'a':
                    ++len;
                    /*if ( (++scan).at_end() )
                        return len || -1;*/
                    switch ( *++scan )
                    {
                    case 's':
                        if ( cmp( ++scan, "e", ++len ) )
                            return -1;
                        break;

                    case 't':
                        if ( cmp( ++scan, "ch", ++len ) )
                            return -1;
                        break;
                    }
                    break;

                case 'o':
                    ++len;
                    /*if ( (++scan).at_end() )
                        return len || -1;*/
                    switch ( *++scan )
                    {
                    case 'n':
                        ++len;
                        /*if ( (++scan).at_end() )
                            return len || -1;*/
                        switch ( *++scan )
                        {
                        case 's':
                            if ( cmp( ++scan, "t", ++len ) )
                                return -1;
                            break;

                        case 't':
                            if ( cmp( ++scan, "inue", ++len ) )
                                return -1;
                            break;
                        }
                        break;
                    }
                    break;

                case 'h':
                    if ( cmp( ++scan, "ar", ++len ) )
                        return -1;
                    break;

                case 'l':
                    if ( cmp( ++scan, "ass", ++len ) )
                        return -1;
                    break;
                }
                return scan_to_end(scan, len);

            case 'd':
                ++len;
                /*if ( (++scan).at_end() )
                    return len || -1;*/
                switch ( *++scan )
                {
                case 'o':
                    if ( cmp( ++scan, "", ++len ) )
                        return -1;
                    break;

                case 'e':
                    ++len;
                    /*if ( (++scan).at_end() )
                        return len || -1;*/
                    switch ( *++scan )
                    {
                    case 'l':
                        if ( cmp( ++scan, "ete", ++len ) )
                            return -1;
                        break;

                    case 'b':
                        if ( cmp( ++scan, "ugger", ++len ) )
                            return -1;
                        break;

                    case 'f':
                        if ( cmp( ++scan, "ault", ++len ) )
                            return -1;
                        break;
                    }
                    break;
                }
                return scan_to_end(scan, len);

            case 'e':
                ++len;
                /*if ( (++scan).at_end() )
                    return len || -1;*/
                switch ( *++scan )
                {
                case 'l':
                    if ( cmp( ++scan, "se", ++len ) )
                        return -1;
                    break;

                case 'x':
                    ++len;
                    /*if ( (++scan).at_end() )
                        return len || -1;*/
                    switch ( *++scan )
                    {
                    case 'p':
                        if ( cmp( ++scan, "ort", ++len ) )
                            return -1;
                        break;

                    case 't':
                        if ( cmp( ++scan, "ends", ++len ) )
                            return -1;
                        break;
                    }
                    break;

                case 'n':
                    if ( cmp( ++scan, "um", ++len ) )
                       return -1;
                    break;
                }
                return scan_to_end(scan, len);

            case 'f':
                ++len;
                /*if ( (++scan).at_end() )
                    return len || -1;*/
                switch ( *++scan )
                {
                case 'a':
                    if ( cmp( ++scan, "lse", ++len ) )
                        return -1;
                    break;

                case 'o':
                    if ( cmp( ++scan, "r", ++len ) )
                        return -1;
                    break;

                case 'u':
                    if ( cmp( ++scan, "nction", ++len ) )
                        return -1;
                    break;

                case 'i':
                    if ( cmp( ++scan, "nal", ++len ) )
                    {
                        if ( cmp( ++scan, "", ++len ) )
                            return -1;
                        if ( cmp( ++scan, "ly", ++len ) )
                            return -1;
                    }
                    break;

                case 'l':
                    if ( cmp( ++scan, "oat", ++len ) )
                       return -1;
                    break;
                }
                return scan_to_end(scan, len);

            case 'g':
                if ( cmp( ++scan, "oto", ++len ) )
                    return -1;
                return scan_to_end(scan, len);

            case 'i':
                ++len;
                /*if ( (++scan).at_end() )
                    return len || -1;*/
                switch ( *++scan )
                {
                case 'f':
                    if ( cmp( ++scan, "", ++len ) )
                        return -1;
                    break;

                case 'n':
                    if ( cmp( ++scan, "", ++len ) )
                        return -1;
                    switch ( *scan )
                    {
                    case 's':
                        if ( cmp( ++scan, "tanceof", ++len ) )
                            return -1;
                        break;

                    case 't':
                        if ( cmp( ++scan, "", ++len ) )
                            return -1;
                        if ( cmp( scan, "erface", len ) )
                            return -1;
                        break;
                    }
                    break;

                case 'm':
                    ++len;
                    /*if ( (++scan).at_end() )
                        return len || -1;*/
                    switch ( *++scan )
                    {
                    case 'p':
                        ++len;
                        /*if ( (++scan).at_end() )
                            return len || -1;*/
                        switch ( *++scan )
                        {
                        case 'o':
                            if ( cmp( ++scan, "rt", ++len ) )
                                return -1;
                            break;

                        case 'l':
                            if ( cmp( ++scan, "ements", ++len ) )
                                return -1;
                            break;
                        }
                        break;
                    }
                    break;
                }
                return scan_to_end(scan, len);

            case 'l':
                if ( cmp( ++scan, "ong", ++len ) )
                    return -1;
                return scan_to_end(scan, len);

            case 'n':
                ++len;
                /*if ( (++scan).at_end() )
                    return len || -1;*/
                switch ( *++scan )
                {
                case 'e':
                    if ( cmp( ++scan, "w", ++len ) )
                        return -1;
                    break;

                case 'u':
                    if ( cmp( ++scan, "ll", ++len ) )
                        return -1;
                    break;

                case 'a':
                    if ( cmp( ++scan, "tive", ++len ) )
                        return -1;
                    break;
                }
                return scan_to_end(scan, len);

            case 'p':
                ++len;
                /*if ( (++scan).at_end() )
                    return len || -1;*/
                switch ( *++scan )
                {
                case 'a':
                    if ( cmp( ++scan, "ckage", ++len ) )
                        return -1;
                    break;

                case 'r':
                    ++len;
                    /*if ( (++scan).at_end() )
                        return len || -1;*/
                    switch ( *++scan )
                    {
                    case 'i':
                        if ( cmp( ++scan, "vate", ++len ) )
                            return -1;
                        break;

                    case 'o':
                        if ( cmp( ++scan, "tected", ++len ) )
                            return -1;
                        break;
                    }
                    break;

                case 'u':
                    if ( cmp( ++scan, "blic", ++len ) )
                        return -1;
                    break;
                }
                return scan_to_end(scan, len);

            case 'r':
                if ( cmp( ++scan, "eturn", ++len ) )
                    return -1;
                return scan_to_end(scan, len);

            case 's':
                ++len;
                /*if ( (++scan).at_end() )
                    return len || -1;*/
                switch ( *++scan )
                {
                case 'w':
                    if ( cmp( ++scan, "itch", ++len ) )
                        return -1;
                    break;

                case 'h':
                    if ( cmp( ++scan, "ort", ++len ) )
                        return -1;
                    break;

                case 't':
                    if ( cmp( ++scan, "atic", ++len ) )
                        return -1;
                    break;

                case 'u':
                    if ( cmp( ++scan, "per", ++len ) )
                        return -1;
                    break;

                case 'y':
                    if ( cmp( ++scan, "nchronized", ++len ) )
                        return -1;
                }
                return scan_to_end(scan, len);

            case 't':
                ++len;
                /*if ( (++scan).at_end() )
                    return len || -1;*/
                switch ( *++scan )
                {
                case 'r':
                    ++len;
                    /*if ( (++scan).at_end() )
                        return len || -1;*/
                    switch ( *++scan )
                    {
                    case 'y':
                        if ( cmp( ++scan, "", ++len ) )
                            return -1;
                        break;

                    case 'u':
                        if ( cmp( ++scan, "e", ++len ) )
                            return -1;
                        break;

                    case 'a':
                        if ( cmp( ++scan, "nsient", ++len ) )
                            return -1;
                        break;
                    }
                    break;

                case 'h':
                    ++len;
                    /*if ( (++scan).at_end() )
                        return len || -1;*/
                    switch ( *++scan )
                    {
                    case 'r':
                        if ( cmp( ++scan, "ow", ++len ) )
                            return -1;
                        if ( cmp( scan, "s", len ) )
                            return -1;
                        break;

                    case 'i':
                        if ( cmp( ++scan, "s", ++len ) )
                            return -1;
                        break;
                    }
                    break;

                case 'y':
                    if ( cmp( ++scan, "peof", ++len ) )
                        return -1;
                    break;
                }
                return scan_to_end(scan, len);

            case 'v':
                ++len;
                /*if ( (++scan).at_end() )
                    return len || -1;*/
                switch ( *++scan )
                {
                case 'a':
                    if ( cmp( ++scan, "r", ++len ) )
                        return -1;
                    break;

                case 'o':
                    ++len;
                    /*if ( (++scan).at_end() )
                        return len || -1;*/
                    switch ( *++scan )
                    {
                    case 'i':
                        if ( cmp( ++scan, "d", ++len ) )
                            return -1;
                        break;

                    case 'l':
                        if ( cmp( ++scan, "atile", ++len ) )
                            return -1;
                        break;
                    }
                    break;
                }

                return scan_to_end(scan, len);

            case 'w':
                ++len;
                /*if ( (++scan).at_end() )
                    return len || -1;*/
                switch ( *++scan )
                {
                case 'h':
                    if ( cmp( ++scan, "ile", ++len ) )
                        return -1;
                    break;

                case 'i':
                    if ( cmp( ++scan, "th", ++len ) )
                        return -1;
                    break;
                }
                return scan_to_end(scan, len);

//                case '_':
//                    return scan_to_end(++scan, ++len);

            default:
                return unicode::test<StartOfIdentifier>(*scan) ?
                    scan_to_end(++scan, ++len) : -1;
            }
        }

    } // namespace impl


    struct identifier_parser_gen
    : public spirit::parser<identifier_parser_gen>
    {

        typedef identifier_parser_gen self_t;

        template <typename ScannerT>
        struct result
        {
            typedef typename spirit::match_result<ScannerT, spirit::nil_t>::type
                type;
        };

        template <typename ScannerT>
        typename spirit::parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const
        {
            typedef typename spirit::parser_result<self_t, ScannerT>::type
                result_t;
            typedef typename ScannerT::iterator_t iterator_t;
            typedef typename ScannerT::iteration_policy_t iteration_policy_t;
            scan.skip(scan);
            iterator_t const s(scan.first);
            std::ptrdiff_t len
                = parse_impl(
                    scan, 
                    static_cast<iteration_policy_t const * const>(0));
            if (len < 0)
                return scan.no_match();
            ES_ASSERT(len > 0);
            return scan.create_match(
                std::size_t(len), result_t(), s, scan.first);
        }

    private:
        template <typename ScannerT, typename IterationPolicyT>
        std::ptrdiff_t parse_impl(
            ScannerT const& scan,
            IterationPolicyT const * const
            ) const
        {
            typedef spirit::scanner_policies<
                spirit::no_skipper_iteration_policy<
                    typename ScannerT::iteration_policy_t>,
                typename ScannerT::match_policy_t,
                typename ScannerT::action_policy_t
            > policies_t;
            return impl::identifier_parse(
                scan.change_policies(policies_t(scan)));
        }

        template <typename ScannerT, typename IterationPolicyT>
        std::ptrdiff_t parse_impl(
            ScannerT const& scan,
            spirit::no_skipper_iteration_policy<IterationPolicyT> const * const
            ) const
        {
            return impl::identifier_parse(scan);
        }
    };

    static const identifier_parser_gen es_identifier_p
        = identifier_parser_gen();

} // namespace ecmascript

