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
//  internal object definition
//
namespace ecmascript {

    //////////////////////////////////////////////////////////////////////////
    //
    // es_internal_object
    //
    template <typename stringT>
    struct es_internal_object
    : public base_classes::es_object_impl<IInternalObject, stringT>
    {
        typedef stringT string_t;
        typedef base_classes::es_object_impl<IInternalObject, stringT> object_t;

        es_internal_object() throw()
        : pparent_object_(0)
        {
        }

        es_internal_object(IPrimitive& parent_object) throw()
        : pparent_object_(&parent_object)
        {
        }

        /*virtual*/ ~es_internal_object() throw()
        {
        }

        const_string_t const class__() const
        {
            return L"Object";
        }

        void mark__(int cookie) throw()
        {
            if (this->get_mark() == cookie)
                return;
            object_t::mark__(cookie);
            if (pparent_object_)
                pparent_object_->mark__(cookie);
        }

        IPrimitive& __stdcall get__(const_string_t const& key)
        {
            IPrimitive& value_of_current_scope = object_t::property_map_[key];
            if (!pparent_object_
                || VT::Undefined != value_of_current_scope.type__())
                return value_of_current_scope;
            IPrimitive& value_of_parent_scope = pparent_object_->get__(key);
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
            if (VT::Undefined != object_t::property_map_[key].type__())
                return true;
            if (pparent_object_)
                return pparent_object_->has_property__(key);
            return false;
        }

        operator const_string_t const() const
        {
            return L"internal object";
        }

        IPrimitive& __stdcall get_prototype__()
        {
            return *pparent_object_;
        }

        void __stdcall set_prototype__(IPrimitive& given_prototype)
        {
            pparent_object_ = &given_prototype;
        }

    private:
        IPrimitive *pparent_object_;
    }; // es_internal_object

} // namespace ecmascript
