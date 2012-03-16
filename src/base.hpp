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


//////////////////////////////////////////////////////////////////////////////
//
// primitive type
//
#include "hash.hpp"

namespace ecmascript { namespace base_classes {

    //////////////////////////////////////////////////////////////////////////
    //
    // IReferenceCounting implementation
    //

    //////////////////////////////////////////////////////////////////////////
    //
    // es_immutable_object
    //
    template<typename baseT>
    struct es_immutable_object
    : public baseT
    {
        ecmascript::uint32_t addref__() throw()
        {
            return 0;
        }

        virtual ecmascript::uint32_t release__() throw()
        {
            return 0;
        }

        bool collect__() throw()
        {
            return false;
        }

    protected:
        es_immutable_object() throw()
        {
        }

        virtual ~es_immutable_object() throw()
        {
        }

    };

    //////////////////////////////////////////////////////////////////////////
    //
    // es_collectable_object
    //
    template<typename baseT>
    struct es_collectable_object
    : public baseT
    {
        es_collectable_object() throw()
        : reference_counter_(0)
        {
        }

        ecmascript::uint32_t addref__() throw()
        {
            return ++ reference_counter_;
        }

        ecmascript::uint32_t release__() throw()
        {
            ES_ASSERT(reference_counter_ > 0);
            if (0 != --reference_counter_)
                return reference_counter_;
            delete this;
            return 0;
        }

        bool collect__() throw()
        {
            return reference_counter_ == 0 ? delete this, true: false;
        }

    protected:
        virtual ~es_collectable_object() throw()
        {
        }

    private:
        ecmascript::uint32_t reference_counter_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // es_object_initialization_impl
    //
    template<typename baseT, typename stringT>
    struct es_object_initialization_impl
    {
        typedef baseT derived_t;
        typedef stringT string_t;
        typedef es_object_initialization_impl<derived_t, string_t> self_t;

        static IPrimitive& static_prototype()
        {
            static es_internal_object<string_t> internal_object;
            static IPrimitive& prototype = make_map(
                internal_object,
                derived_t::get_typelib__());
            return prototype;
        }

    private:
        static IPrimitive& make_map(
            IPrimitive& prototype,
            es_attributes const& attributes)
        {
            es_property_reference<string_t>& attr 
                = *new es_property_reference<string_t>(attributes);
            if (attributes.parent)
                make_map(prototype, *attributes.parent);
			return !attributes.name ?
                prototype: (prototype.put__(attributes.name, attr),
                    make_map(prototype, *(&attributes + 1)));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // es_primitive_impl
    //
    template<typename baseT, typename stringT>
    struct es_primitive_impl_base
    : public es_immutable_object<baseT>
    , public es_object_initialization_impl<baseT, stringT>
    {
        typedef baseT derived_t;
        typedef stringT string_t;
        typedef es_boolean<string_t> boolean_t;
        typedef es_object_initialization_impl<derived_t, string_t> 
            initialization_t;

    public:
        void * operator new (size_t size) throw()
        {
            return es_machine<IPrimitive>::get_gc().allocate(size);
        }

        void operator delete (void* p) throw()
        {
            return es_machine<IPrimitive>::get_gc().deallocate(p);
        }

        void mark__(int cookie) throw()
        {
            if (get_mark() == cookie)
                return;
            mark_ = cookie;
            if (p_parent_)
                p_parent_->mark__(cookie);
        }

        void sweep__(int cookie) throw()
        {
            if (mark_ != cookie)
                delete this;
        }

        IPrimitive& __stdcall get_by_value__(IPrimitive const& key)
        {
            return get__(key);
        }

        IPrimitive& __stdcall get__(const_string_t const& key)
        {
            IPrimitive& value_of_current_scope = p_parent_->get__(key);
            if (VT::Metadata != value_of_current_scope.type__())
                return value_of_current_scope;
            es_attributes const& attributes
                = value_of_current_scope.operator es_attributes const&();
            if (Property == (attributes.flag & 0x0002))
                return *new es_native_property<string_t>(*this, attributes);
            IPrimitive& new_function
                = *new es_native_function<string_t>(*this, attributes);
            p_parent_->put__(key, new_function);
            return new_function;
        }

        void __stdcall put__(const_string_t const& key, IPrimitive& value)
        {
            p_parent_->put__(key, value);
        }

        void __stdcall put_by_value__(IPrimitive const& key, IPrimitive& value)
        {
            p_parent_->put__(key.operator const_string_t const(), value);
        }

        IPrimitive& __stdcall get_prototype__()
        {
            return *p_parent_;
        }

        void __stdcall set_prototype__(IPrimitive& p_parent)
        {
            p_parent_ = &p_parent;
        }

        IPrimitive& get_scope__()
        {
            return *this;
        }

        void set_scope__(IPrimitive&)
        {
            ES_ASSERT(
                !"'es_primitive_base_services::set_scope__' is not implemented.");
            throw std::logic_error(
                "'es_primitive_base_services::set_scope__' is not implemented.");
        }

        IPrimitive& __stdcall call__(
            IPrimitive& /*this_arg*/, 
            IPrimitive& /*arguments*/)
        {
            throw *new es_native_error<string_t>(
                L"'es_primitive_base_services::call__' is not implemented.");
        }

        IPrimitive& __stdcall construct__(IPrimitive&)
        {
            ES_ASSERT(
                !"'es_primitive_base_services::construct__' is not implemented.");
            throw std::logic_error(
                "'es_primitive_base_services::construct__' is not implemented.");
        }

        IPrimitive& __stdcall get_value__()
        {
            return *this;
        }

        IBoolean& has_instance__(IPrimitive const& value) const
        {
            ES_ASSERT(
                !"'es_primitive_base_services::has_instance__' is not implemented.");
            throw std::logic_error(
                "'es_primitive_base_services::has_instance__' is not implemented.");
        }

        bool has_property__(const_string_t const& key)
        {
            return p_parent_->has_property__(key);
        }

        unsigned char __stdcall type__() const
        {
            return baseT::id;
        }

// instance id
        void const* address__() const
        {
            return this;
        }

// IEnumProperty
        IPrimitive& __stdcall reset__()
        {
            ES_ASSERT(
                !"'es_primitive_base_services::reset__' is not implemented.");
            throw std::logic_error(
                "'es_primitive_base_services::reset__' is not implemented.");
        }

        IPrimitive& __stdcall next__()
        {
            ES_ASSERT(
                !"'es_primitive_base_services::next__' is not implemented.");
            throw std::logic_error(
                "'es_primitive_base_services::next__' is not implemented.");
        }

        void __stdcall push__(IPrimitive& value)
        {
            ES_ASSERT(
                !"'es_primitive_base_services::push__' is not implemented.");
            throw std::logic_error(
                "'es_primitive_base_services::push__' is not implemented.");
        }

        IPrimitive& operator [](ecmascript::uint32_t index)
        {
            ES_ASSERT(
                !"'es_primitive_base_services::operator []' is not implemented.");
            throw std::logic_error(
                "'es_primitive_base_services::operator []' is not implemented.");
        }

        ecmascript::uint32_t length__()
        {
            throw std::runtime_error("not implemented: length__");
        }

        IPrimitive ** begin__()
        {
            throw std::runtime_error("not implemented: begin__");
        }

// postfix operators
        IPrimitive& __stdcall postfix_inc__()
        {
            throw std::runtime_error("bad postfix increment");
        }

        IPrimitive& __stdcall postfix_dec__()
        {
            throw std::runtime_error("bad postfix decrement");
        }

// unary operators
    // delete operator
        IBoolean& __stdcall delete__()
        {
            return boolean_t::create_instance(false);
        }

    // void operator
        IUndefined& __stdcall void__() const
        {
            return es_undefined<string_t>::create_instance();
        }

        IString& __stdcall typeof__() const
        {
            throw std::runtime_error("not implemented: typeof");
        }

    // unary operators
        IPrimitive& __stdcall prefix_inc__()
        {
            return *new es_number<string_t>(operator double() + 1);
        }

        IPrimitive& __stdcall prefix_dec__()
        {
            return *new es_number<string_t>(operator double() - 1);
        }

        INumber& __stdcall unary_plus__() const
        {
            return *new es_number<string_t>(+ operator double());
        }

        INumber& __stdcall unary_minus__() const
        {
            return *new es_number<string_t>(- operator double());
        }

        INumber& __stdcall tilde__() const
        {
            return *new es_number<string_t>(~ operator ecmascript::int32_t());
        }

        IBoolean& __stdcall not__() const
        {
            return boolean_t::create_instance(!operator bool());
        }

// multiplicative operators
        INumber& __stdcall mul__(IPrimitive const& rhs) const
        {
            return *new es_number<string_t>(
                operator double() * rhs.operator double());
        }

        INumber& __stdcall div__(IPrimitive const& rhs) const
        {
            double d_lhs = operator double();
            double d_rhs = rhs.operator double();
            if (base_services::es_isnan(d_lhs)
                || base_services::es_isnan(d_rhs))
                return *new es_number<stringT>(
                    (std::numeric_limits<double>::quiet_NaN)());
            if (d_rhs == 0.)
                return *new es_number<stringT>(
                    (d_lhs < 0 ? -1: 1)
                        * std::numeric_limits<double>::infinity());
            return *new es_number<string_t>(d_lhs / d_rhs);
        }

        INumber& __stdcall mod__(IPrimitive const& rhs) const
        {
            double d_lhs = operator double();
            double d_rhs = rhs.operator double();
            if (d_rhs == 0
                || base_services::es_isnan(d_lhs)
                || base_services::es_isnan(d_rhs)
                || base_services::es_isinf(d_lhs))
                return *new es_number<stringT>(
                    (std::numeric_limits<double>::quiet_NaN)());
            if (d_lhs == 0 || base_services::es_isinf(d_rhs))
                return *new es_number<stringT>(d_lhs);
            return *new es_number<string_t>(
                d_lhs - base_services::es_floor(d_lhs / d_rhs) * d_rhs);
        }

// additive operators
        IPrimitive& __stdcall binary_plus__(IPrimitive const& rhs) const
        {
            return VT::String == rhs.type__() ?
                static_cast<IPrimitive&>(
                    *new es_string<string_t>(
                        operator const_string_t const()
                            + rhs.operator const_string_t const())):
                *new es_number<string_t>(
                    operator double() + rhs.operator double());
        }

        INumber& __stdcall binary_minus__(IPrimitive const& rhs) const
        {
            return *new es_number<string_t>(
                operator double() - rhs.operator double());
        }

        INumber& __stdcall shl__(IPrimitive const& rhs) const
        {
            return *new es_number<string_t>(
                operator ecmascript::int32_t() << rhs.operator ecmascript::int32_t());
        }

        INumber& __stdcall sar__(IPrimitive const& rhs) const
        {
            return *new es_number<string_t>(
                operator ecmascript::uint32_t() >> rhs.operator ecmascript::int32_t());
        }

        INumber& __stdcall shr__(IPrimitive const& rhs) const
        {
            return *new es_number<string_t>(
                operator ecmascript::int32_t() >> rhs.operator ecmascript::int32_t());
        }

// relational operators
        IBoolean& __stdcall lt__(IPrimitive const& rhs) const
        {
            return ToNumber().lt__(rhs);
        }

        IBoolean& __stdcall gt__(IPrimitive const& rhs) const
        {
            return ToNumber().gt__(rhs);
        }

        IBoolean& __stdcall le__(IPrimitive const& rhs) const
        {
            return ToNumber().le__(rhs);
        }

        IBoolean& __stdcall ge__(IPrimitive const& rhs) const
        {
            return ToNumber().ge__(rhs);
        }

        IBoolean& __stdcall instanceof__(IPrimitive const& lhs) const
        {
            return has_instance__(lhs);
        }

        IBoolean& __stdcall in__(IPrimitive const& lhs)
        {
            return es_boolean<string_t>::create_instance(has_property__(lhs));
        }

// equality operations
        IBoolean& __stdcall eq__(IPrimitive const& rhs) const
        {
            throw std::logic_error("not implemented: eq__");
        }

        IBoolean& __stdcall ne__(IPrimitive const& rhs) const
        {
            return eq__(rhs).not__();
        }

        IBoolean& __stdcall strict_eq__(IPrimitive const& rhs) const
        {
            throw std::logic_error("not implemented: strict_eq__");
        }

        IBoolean& __stdcall strict_ne__(IPrimitive const& rhs) const
        {
            return strict_eq__(rhs).not__();
        }

// binary bitwise operators
        INumber& __stdcall bitwise_and__(IPrimitive const& rhs) const
        {
            return *new es_number<string_t>(
                operator ecmascript::int32_t() & rhs.operator ecmascript::int32_t());
        }

        INumber& __stdcall bitwise_xor__(IPrimitive const& rhs) const
        {
            return *new es_number<string_t>(
                operator ecmascript::int32_t() ^ rhs.operator ecmascript::int32_t());
        }

        INumber& __stdcall bitwise_or__(IPrimitive const& rhs) const
        {
            return *new es_number<string_t>(
                operator ecmascript::int32_t() | rhs.operator ecmascript::int32_t());
        }

// assignment operators
        IPrimitive& __stdcall assign__(IPrimitive& rhs)
        {
            throw std::runtime_error("Cannot assign to ''");
        }

        IPrimitive& __stdcall assign_mul__(IPrimitive const&)
        {
            throw std::runtime_error("Cannot assign to ''");
        }

        IPrimitive& __stdcall assign_div__(IPrimitive const&)
        {
            throw std::runtime_error("Cannot assign to ''");
        }

        IPrimitive& __stdcall assign_mod__(IPrimitive const&)
        {
            throw std::runtime_error("Cannot assign to ''");
        }

        IPrimitive& __stdcall assign_plus__(IPrimitive const&)
        {
            throw std::runtime_error("Cannot assign to ''");
        }

        IPrimitive& __stdcall assign_minus__(IPrimitive const&)
        {
            throw std::runtime_error("Cannot assign to ''");
        }

        IPrimitive& __stdcall assign_shl__(IPrimitive const&)
        {
            throw std::runtime_error("Cannot assign to ''");
        }

        IPrimitive& __stdcall assign_sar__(IPrimitive const&)
        {
            throw std::runtime_error("Cannot assign to ''");
        }

        IPrimitive& __stdcall assign_shr__(IPrimitive const&)
        {
            throw std::runtime_error("Cannot assign to ''");
        }

        IPrimitive& __stdcall assign_and__(IPrimitive const&)
        {
            throw std::runtime_error("Cannot assign to ''");
        }

        IPrimitive& __stdcall assign_xor__(IPrimitive const&)
        {
            throw std::runtime_error("Cannot assign to ''");
        }

        IPrimitive& __stdcall assign_or__(IPrimitive const&)
        {
            throw std::runtime_error("Cannot assign to ''");
        }

// type conversion
        IBoolean& ToBoolean() const
        {
            return boolean_t::create_instance(operator bool());
        }

        INumber& ToNumber() const
        {
            return *new es_number<string_t>(operator double());
        }

        IString& ToString() const
        {
            return *new es_string<string_t>(operator const_string_t const());
        }

        INumber const& ToInteger() const
        {
            return *new es_number<string_t>(double(operator ecmascript::integer_t()));
        }

        INumber const& ToInt32() const
        {
            return *new es_number<string_t>(operator ecmascript::int32_t());
        }

        INumber const& ToUint32() const
        {
            return *new es_number<string_t>(operator ecmascript::uint32_t());
        }

        IPrimitive const& ToPrimitive(unsigned char type) const
        {
            return *this;
        }

        IPrimitive& ToObject()
        {
            IObject& object = *new es_object<string_t>;
            switch (type__())
            {
            case VT::Undefined:
            case VT::Null:
                throw std::runtime_error("TypeError: ToObject");
            default:
                object.put__(L"value", *this);
            }
            return object;
        }

// native type conversion
        __stdcall operator bool() const
        {
            throw std::runtime_error("cannnot cast to bool.");
        }

        operator double() const
        {
            throw std::runtime_error("cannnot cast to double.");
        }

        operator const_string_t const() const
        {
            throw std::runtime_error("cannnot cast to es_const_string.");
        }

        operator string_t const() const
        {
            throw std::runtime_error("cannnot cast to string_t.");
        }

        operator ecmascript::integer_t() const
        {
            double d = operator double();
            return ecmascript::integer_t(
                base_services::es_isnan(d) ?
                    0:
                    base_services::es_sign(d) *
                        base_services::es_floor(
                            base_services::es_abs(d)));
        }

        operator ecmascript::int32_t() const
        {
            throw std::runtime_error("cannnot cast to ecmascript::int32_t.");
        }

        operator ecmascript::uint32_t() const
        {
            throw std::runtime_error("cannnot cast to ecmascript::uint16_t.");
        }

        operator ecmascript::uint16_t() const
        {
            throw std::runtime_error("cannnot cast to ecmascript::uint16_t.");
        }

        operator es_attributes const&() const
        {
            throw std::runtime_error("cannnot cast to es_attributes");
        }

    protected:
        es_primitive_impl_base() throw()
        : mark_(-1)
        , p_parent_(0)
        {
        }

        virtual ~es_primitive_impl_base() throw()
        {
        }

        void initialize_prototype() throw()
        {
            p_parent_ = &initialization_t::static_prototype();
        }

        int get_mark() const throw()
        {
            return mark_;
        }

    private:
        int mark_;
        IPrimitive * p_parent_;
    }; // es_primitive_impl_base

    //////////////////////////////////////////////////////////////////////////
    //
    // @class es_primitive_impl
    //
    template<typename baseT, typename stringT>
    struct es_primitive_impl
    : public es_primitive_impl_base<baseT, stringT>
    {
    protected:
        es_primitive_impl() throw()
        {
            this->initialize_prototype();
        }

        virtual ~es_primitive_impl() throw()
        {
        }

    }; // es_primitive_impl

    template<typename stringT>
    struct es_primitive_impl<IInternalObject, stringT>
    : public es_primitive_impl_base<IInternalObject, stringT>
    {
    protected:
        es_primitive_impl() throw()
        {
        }

        virtual ~es_primitive_impl() throw()
        {
        }

    }; // es_primitive_impl<IInternalObject, stringT>

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_object_impl
    //  @brief IObject implementaion
    //
    template <typename baseT, typename stringT>
    struct es_object_impl
    : public base_classes::es_primitive_impl<baseT, stringT>
    {
        typedef stringT string_t;
        typedef baseT base_t;

        typedef map<const_string_t const, es_reference<string_t> > map_t;

        typedef base_classes::es_primitive_impl<baseT, string_t> prototype_t;

        es_object_impl() throw()
        {
            p_scope_object_ = this;
        }

        virtual ~es_object_impl() throw()
        {
        }

// IEnumProperty implemetation
        IPrimitive& __stdcall reset__()
        {
            return it_enum_property_ = property_map_.begin(), *this;
        }

        IPrimitive& __stdcall next__()
        {
            return it_enum_property_ == property_map_.end() ?
                static_cast<IPrimitive&>(
                    es_undefined<string_t>::create_instance()):
                    *new es_string<string_t>((it_enum_property_++)->first);
        }

// IObfect implementation
        IFunction const& __stdcall constructor() const
        {
            throw std::logic_error("not imiplemented: constructor");
        }

        IString const& __stdcall toString() const
        {
            return this->ToString();
        }

        IString const& __stdcall toLocaleString() const
        {
            return this->ToString();
        }

        IObject& __stdcall valueOf()
        {
            return *this;
        }

        IBoolean& __stdcall hasOwnProperty(IPrimitive const& property_name)
        {
            return es_boolean<string_t>::create_instance(
                has_property__(property_name.operator const_string_t const()));
        }

        IBoolean& __stdcall isPrototypeOf(IPrimitive& value)
        {
            return es_boolean<string_t>::create_instance(
                value.get_prototype__().address__() == this->address__());
        }

        IBoolean& __stdcall propertyIsEnumerable(
            IPrimitive const& property_name)
        {
            throw std::runtime_error("not implemented");
        }

// IReferenceCounting

        void mark__(int cookie) throw()
        {
            if (this->get_mark() == cookie)
                return;
            prototype_t::mark__(cookie);
            for (typename map_t::iterator it = property_map_.begin();
                it != property_map_.end(); ++it)
                it->second.mark__(cookie);
            p_scope_object_->mark__(cookie);
        }

// IDictionaryObject
        IPrimitive& __stdcall get__(const_string_t const& key)
        {
            IPrimitive& value_of_current_scope = property_map_[key];
            if (VT::Undefined != value_of_current_scope.type__())
                return value_of_current_scope;
            IPrimitive& value_of_parent_scope = prototype_t::get__(key);
            return VT::Undefined != value_of_parent_scope.type__() ?
                value_of_parent_scope: value_of_current_scope;
        }

        void __stdcall put__(const_string_t const& key, IPrimitive& value)
        {
            property_map_[key] = value;
        }

        void __stdcall put_by_value__(IPrimitive const& key, IPrimitive& value)
        {
            property_map_[key.operator const_string_t const()] = value;
        }

        bool has_property__(const_string_t const& key)
        {
            return VT::Undefined != property_map_.find(key)->second.type__()
                || prototype_t::has_property__(key);
        }

        IPrimitive& get_scope__()
        {
            return *p_scope_object_;
        }

        void set_scope__(IPrimitive& given_scope_object)
        {
            p_scope_object_ = &given_scope_object;
        }

        IString& __stdcall typeof__() const
        {
            return *new es_string<stringT>(L"object");
        }

// equality operators
        IBoolean& __stdcall eq__(IPrimitive const& rhs) const
        {
            switch (rhs.type__())
            {
            case VT::Object:
                return es_boolean<string_t>::create_instance(
                    static_cast<void const*>(this) == rhs.address__());
            case VT::String:
                return ToPrimitive(VT::String).eq__(rhs);
            case VT::Number:
                return ToPrimitive(VT::Number).eq__(rhs);
            default:
                return es_boolean<string_t>::create_instance(false);
            }
        }

    // strict equality operators
        IBoolean& __stdcall strict_eq__(IPrimitive const& rhs) const
        {
            return es_boolean<string_t>::create_instance(
                VT::Object == rhs.type__()
                    && static_cast<void const*>(this) == rhs.address__());
        }

// type conversion
        IPrimitive const& ToPrimitive(unsigned char hint) const
        {
            ES_ASSERT(hint >= VT::Undefined && hint <= VT::String);
            switch (hint)
            {
            case VT::Undefined:
                return es_undefined<string_t>::create_instance();
            case VT::Null:
                return es_null<string_t>::create_instance();
            case VT::Boolean:
                return this->ToBoolean();
            case VT::Number:
                return this->ToNumber();
            case VT::String:
                return this->ToString();
            }
            throw std::runtime_error("es_object_base_services::ToPrimitive: bad hint");
        }

        IPrimitive& ToObject()
        {
            return *this;
        }

// native type conversion
        operator double() const
        {
            return 0.; /* dummy */
        }

        __stdcall operator bool() const
        {
            return true;
        }

        operator const_string_t const() const
        {
            const_string_t ws = L"{\n";
            typename map_t::const_iterator it = property_map_.begin();
            if (it != property_map_.end())
                for (ws = ws + it->first + L": \n"; ++it != property_map_.end();)
                    if (VT::Undefined != it->second.type__())
                        ws = ws + L"," + it->first + L": \n";
            return ws + L"}";
        }

        operator string_t const() const
        {
            string_t ws = L"{\n";
            typename map_t::const_iterator it = property_map_.begin();
            if (it != property_map_.end())
                for (ws += it->first + L": \n"; ++it != property_map_.end();)
                    if (VT::Undefined != it->second.type__())
                        ws += L"," + it->first + L": \n";
            return ws + L"}";
        }

        operator ecmascript::int32_t() const
        {
            return 0;  /* dummy */
        }

        operator ecmascript::uint32_t() const
        {
            return 0;  /* dummy */
        }

        operator ecmascript::uint16_t() const
        {
            return 0;  /* dummy */
        }

    protected:
        map_t property_map_;

    private:
        IPrimitive *p_scope_object_;
        typename map_t::iterator it_enum_property_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // es_function_impl
    //
    template <typename baseT, typename stringT>
    struct es_function_impl
    : public es_object_impl<baseT, stringT>
    {
        typedef stringT string_t;
        typedef es_object_impl<baseT, string_t> object_t;

    // constructor
        es_function_impl() throw()
        : original_prototype_object_(
            *new es_reference<stringT>(*new es_internal_object<string_t>))
        {
        }

        ~es_function_impl() throw()
        {
        }

    // IReferenceCounting
        void mark__(int cookie) throw()
        {
            if (this->get_mark() == cookie)
                return;
            object_t::mark__(cookie);
            original_prototype_object_.mark__(cookie);
        }

        const_string_t const class__() const throw()
        {
            return L"Function";
        }

    // IDictionaryObject
        IPrimitive& __stdcall get_prototype__()
        {
            return original_prototype_object_;
        }

        void __stdcall set_prototype__(IPrimitive& given_prototype)
        {
            throw std::logic_error("not implemented: set_prototype");
        }

    // IUnaryOperations
        IString& __stdcall typeof__() const
        {
            return *new es_string<stringT>(L"function");
        }

    // IFunction
        IPrimitive& __stdcall apply(IPrimitive&, IPrimitive&)
        {
            throw std::logic_error("not implemented: apply");
        }

        IPrimitive& __stdcall call(IPrimitive&)
        {
            throw std::logic_error("not implemented: call");
        }

        INumber& __stdcall length() const
        {
            throw std::logic_error("not implemented: length 1");
        }

        IPrimitive& __stdcall prototype()
        {
            return get_prototype__();
        }

    private:
        es_function_impl(es_function_impl const&);
        es_function_impl& operator=(es_function_impl const&);

    private:
        IPrimitive& original_prototype_object_;
    };

} } // namespace ecmascript::impl
