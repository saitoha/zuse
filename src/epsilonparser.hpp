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
    //  @class es_epsilon_parser
    //  @brief
    //
    struct es_epsilon_parser
    : public spirit::parser<es_epsilon_parser>
    {
        typedef es_epsilon_parser self_t;

        template <typename ScannerT>
        typename spirit::parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const
        {
            return scan.empty_match();
        }
    };

    es_epsilon_parser const es_epsilon_p = es_epsilon_parser();

} // namespace ecmascript
