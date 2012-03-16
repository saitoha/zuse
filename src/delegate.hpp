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
    // es_delegate_object
    //
    template <typename stringT>
    struct es_delegate_object
		: public ecmascript::base_classes::es_object_impl<IPrimitive, stringT>
    {
        typedef stringT string_t;
        typedef ecmascript::base_classes::es_object_impl<IPrimitive, string_t>
            object_t;

        void mark__(int cookie) throw()
        {
            if (this->get_mark() == cookie)
                return;
            object_t::mark__(cookie);
            parent_object_.mark__(cookie);
        }

        explicit es_delegate_object(IPrimitive& parent) throw()
        : parent_object_(parent)
        {
        }

        ~es_delegate_object() throw()
        {
        }

        const_string_t const class__() const
        {
            return L"Object";
        }

        IPrimitive& __stdcall get__(const_string_t const& key)
        {
            IPrimitive& value_of_current_scope = object_t::property_map_[key];
            if (VT::Undefined != value_of_current_scope.type__())
                return value_of_current_scope;
            IPrimitive& value_of_parent_scope = parent_object_.get__(key);
            if (VT::Undefined != value_of_parent_scope.type__())
                return value_of_parent_scope;
            return value_of_current_scope;
        }

        void __stdcall put__(const_string_t const& key, IPrimitive& value)
        {
            object_t::property_map_[key] = value;
        }

        void __stdcall put_by_value__(IPrimitive const& key, IPrimitive& value)
        {
            object_t::property_map_[key.operator const_string_t const()] = value;
        }

        bool has_property__(const_string_t const& key)
        {
            return VT::Undefined != object_t::property_map_[key].type__()
                || parent_object_.has_property__(key);
        }

        IPrimitive& __stdcall get_prototype__()
        {
            return parent_object_;
        }

        void __stdcall set_prototype__(IPrimitive& given_prototype)
        {
            throw *new es_native_error<string_t>(
                L"not implement: set_prototype");
        }

        IPrimitive& __stdcall get_value__()
        {
            return parent_object_;
        }

        operator const_string_t const() const
        {
            return L"delegate object";
        }

        operator string_t const() const
        {
            return L"delegate object";
        }

    private:
        IPrimitive& parent_object_;
    };

} // ecmascript
