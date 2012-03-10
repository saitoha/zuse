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

    struct es_empty_t
    {
    };

    template <CompletionType Type, typename stringT = es_empty_t>
    struct es_completion;

    struct es_completion_base
    {
    };

    template <>
    struct es_completion<CT_Normal>
    : es_completion_base
    {
    };

    template <>
    struct es_completion<CT_Return>
    : es_completion_base
    {
    };

    template <>
    struct es_completion<CT_Break>
    : es_completion_base
    {
    };

    template <typename stringT>
    struct es_completion<CT_Break, stringT>
    : es_completion_base
    {
    };

    template <>
    struct es_completion<CT_Continue>
    : es_completion_base
    {
    };

    template <typename stringT>
    struct es_completion<CT_Continue, stringT>
    : es_completion_base
    {
    };

    template <>
    struct es_completion<CT_Throw>
    : es_completion_base
    {
    };

} // namespace ecmascript

