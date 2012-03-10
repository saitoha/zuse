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


//////////////////////////////////////////////////////////////////////////////
//
// date object
//
namespace ecmascript {

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class IDate
    //  @brief
    //
    struct IDate
    : IObject
    {
        typedef IObject base_t;

        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 0 };

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }

    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class IDateConstructor
    //  @brief
    //
    struct IDateConstructor
    : IFunction
    {
        typedef IObject base_t;

        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 0 };

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }

    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_date
    //  @brief
    //
    template <typename stringT>
    struct es_date
    : public base_classes::es_object_impl<IDate, stringT>
    {
        typedef stringT string_t;
        typedef typename string_t::value_type char_t;

        static IFunction& static_get_constructor()
        {
            static struct constructor
            : public base_classes::es_function_impl<IDateConstructor, stringT>
            {
                constructor() throw()
                {
                }

                ~constructor() throw()
                {
                }

                IPrimitive& __stdcall prototype()
                {
                    return es_date<string_t>::static_prototype();
                }

                IPrimitive& __stdcall construct__(IPrimitive& arguments)
                {
                    return *new es_date<stringT>(arguments);
                }

                IPrimitive& __stdcall call__(
                    IPrimitive& /*this_arg*/, 
                    IPrimitive& arguments
                    )
                {
                    return construct__(arguments);
                }

            } constructor_;
            return constructor_;
        }

        explicit es_date(IPrimitive& arguments) throw()
        {
        }

        ~es_date() throw()
        {
        }

        const_string_t const class__() const throw()
        {
            return L"Date";
        }

    };

} // namespace ecmascript
