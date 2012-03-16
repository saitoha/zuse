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
    // es_activation_object
    //
    template <typename stringT>
    struct es_activation_object
    : public base_classes::es_object_impl<IPrimitive, stringT>
    {
        typedef stringT string_t;
        typedef es_activation_object<string_t> self_t;
        typedef base_classes::es_object_impl<IPrimitive, stringT> object_t;
        
        static IPrimitive& __stdcall create_instance(
            IPrimitive& parent_object,
            IPrimitive& property_object
            ) throw()
        {
            return *new self_t(parent_object, property_object);
        }

        explicit es_activation_object(
            IPrimitive& parent_object,
            IPrimitive& property_object
            ) throw()
        : p_parent_object_(&parent_object)
        , property_object_(property_object)
        {
        }

        void mark__(int cookie) throw()
        {
            if (this->get_mark() == cookie)
                return;
            object_t::mark__(cookie);
            p_parent_object_->mark__(cookie);
            property_object_.mark__(cookie);
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
            IPrimitive& value_of_parent_scope = p_parent_object_->get__(key);
            if (VT::Undefined != value_of_parent_scope.type__())
                return value_of_parent_scope;
            return property_object_.get__(key);
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
            IPrimitive& value = property_object_.get__(key);
            return VT::Undefined != value.type__()
                || p_parent_object_->has_property__(key);
        }

        IPrimitive& __stdcall get_prototype__()
        {
            return *p_parent_object_;
        }

        void __stdcall set_prototype__(IPrimitive& prototype)
        {
            p_parent_object_ = &prototype;
        }

        IPrimitive& get_scope__()
        {
            return property_object_;
        }

        operator const_string_t const() const
        {
            return L"activation object";
        }

        operator string_t const() const
        {
            return L"activation object";
        }

//    protected:
        ~es_activation_object() throw()
        {
        }

    private:
        IPrimitive * p_parent_object_;
        IPrimitive& property_object_;
    };

} // namespace ecmascript
