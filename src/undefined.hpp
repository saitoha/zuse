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
    //  @class Null
    //  @brief null Object
    //
    template <typename stringT>
    struct es_undefined
    : public base_classes::es_immutable_object<
        base_classes::es_primitive_impl<IUndefined, stringT> >
    {
        typedef stringT string_t;
        static IUndefined& __stdcall create_instance()
        {
            static IUndefined& undefined = *new es_undefined<stringT>;
            return undefined;
        }

        void sweep__(int cookie) throw()
        {
        }

        const_string_t const class__() const
        {
            return L"Undefined";
        }

// unary operator
        IString& __stdcall typeof__() const
        {
            return *new es_string<string_t>(L"undefined");
        }

// equalty operators
        IBoolean& __stdcall eq__(IPrimitive const& rhs) const
        {
            switch (rhs.type__())
            {
                case VT::Undefined:
                case VT::Null:
                    return es_boolean<string_t>::create_instance(true);
                default:
                    return es_boolean<string_t>::create_instance(false);
            }
        }

    // strict equality operators
        IBoolean& __stdcall strict_eq__(IPrimitive const& rhs) const
        {
            switch (rhs.type__())
            {
                case VT::Undefined:
                    return es_boolean<string_t>::create_instance(true);
                default:
                    return es_boolean<string_t>::create_instance(false);
            }
        }

// native type conversion
        __stdcall operator bool() const
        {
            return false;
        }

        operator double() const
        {
            return 0.;
        }

        operator const_string_t const() const
        {
            return L"undefined";
        }

        operator string_t const() const
        {
            return L"undefined";
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

    private:
        es_undefined() throw()
        {
        }

        ~es_undefined() throw()
        {
        }
    };

} // namespace ecmascript
