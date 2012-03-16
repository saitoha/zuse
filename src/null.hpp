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



//////////////////////////////////////////////////////////////////////////
//
// definition and implementation of Null type object, es_null.
//
namespace ecmascript {

    //////////////////////////////////////////////////////////////////////////
    //
    // es_null
    //
    template <typename stringT>
    struct es_null
    : public base_classes::es_immutable_object<
        base_classes::es_primitive_impl<INull, stringT> >
    {
        es_null() throw()
        {
        }

        ~es_null() throw()
        {
        }

        typedef stringT string_t;
// factory
        static INull& create_instance()
        {
            static INull& null = *new es_null<stringT>;
            return null;
        }

        void sweep__(int cookie) throw()
        {
        }

        const_string_t const class__() const
        {
            return L"Null";
        }

// typeof operator
        IString& __stdcall typeof__() const
        {
            return *new es_string<stringT>(L"null");
        }

// equality operators
        IBoolean& __stdcall eq__(IPrimitive const& rhs) const
        {
            unsigned char type = rhs.type__();
            return es_boolean<string_t>::create_instance(
                VT::Null == type || VT::Undefined == type);
        }

        IBoolean& __stdcall strict_eq__(IPrimitive const& rhs) const
        {
            return es_boolean<string_t>::create_instance(
                VT::Null == rhs.type__());
        }

// native type conversion
        __stdcall operator bool() const
        {
            return false;
        }

        operator double() const
        {
            return 0;
        }

        operator const_string_t const() const
        {
            return L"null";
        }

        operator string_t const() const
        {
            return L"null";
        }

        operator ecmascript::int32_t() const
        {
            return 0;
        }

        operator ecmascript::uint32_t() const
        {
            return 0;
        }

        operator ecmascript::uint16_t() const
        {
            return 0;
        }
    };
} //ecmascript
