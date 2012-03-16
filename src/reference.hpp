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

    template <typename stringT>
    struct es_property_reference;

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_reference
    //  @brief reference object
    //
    template <typename stringT>
    struct es_reference
    : IReference
    {
        typedef stringT string_t;
        typedef es_reference<stringT> self_t;
        typedef es_boolean<string_t> boolean_t;

        es_reference() throw()
        : vt_(VT::Undefined)
        , p_value_(&es_undefined<string_t>::create_instance())
        {
        }

        es_reference(es_property_reference<string_t>& value) throw()
        : vt_(value.type__())
        {
            p_attributes_ = &value.operator es_attributes const&();
        }
        
        es_reference(IPrimitive& value) throw()
        : vt_(value.type__())
        {
            if (VT::Metadata == vt_)
                p_attributes_ = &value.operator es_attributes const&();
            else
                p_value_ = &value.get_value__(),
                p_value_->addref__();
        }

        es_reference(es_attributes const& attributes) throw()
        : vt_(VT::Metadata)
        , p_attributes_(&attributes)
        {
        }

        es_reference(es_reference const& other_value) throw()
        : vt_(other_value.vt_)
        , p_value_(other_value.p_value_)
        {
            if (VT::Metadata != vt_)
                p_value_->addref__();
        }

        virtual ~es_reference() throw()
        {
            if (VT::Metadata != vt_ && *reinterpret_cast<int *>(p_value_))
                p_value_->release__();
        }

        es_reference<string_t> const&
        operator =(es_reference<string_t> const& rhs)
        {
            vt_ = rhs.vt_;
            p_value_ = rhs.p_value_;
            if (VT::Metadata != vt_)
                p_value_->addref__();
            return *this;
        }

    public:
        void mark__(int cookie) throw()
        {
            if (VT::Metadata != vt_)
                p_value_->mark__(cookie);
        }

        void sweep__(int cookie) throw()
        {
            if (VT::Metadata != vt_)
                p_value_->sweep__(cookie);
        }

// IReferenceCounting
        bool collect__() throw()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->collect__();
        }

        ecmascript::uint32_t addref__() throw()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->addref__();
        }

        ecmascript::uint32_t release__() throw()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->release__();
        }

// IEnumProperty
        IPrimitive& __stdcall reset__()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->reset__();
        }

        IPrimitive& __stdcall next__()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->next__();
        }

        void __stdcall push__(IPrimitive& value)
        {
            ES_ASSERT(0 != p_value_);
            p_value_->push__(value);
        }

        IPrimitive& operator [](ecmascript::uint32_t index)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator [](index);
        }

        ecmascript::uint32_t length__()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->length__();
        }

        IPrimitive ** begin__()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->begin__();
        }

        IPrimitive& __stdcall get_by_value__(IPrimitive const& key)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->get_by_value__(key);
        }

        IPrimitive& __stdcall get__(const_string_t const& key)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->get__(key);
        }

        IPrimitive& __stdcall get_prototype__()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->get_prototype__();
        }

        void __stdcall set_prototype__(IPrimitive& given_prototype)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->set_prototype__(given_prototype);
        }

        IPrimitive& __stdcall call__(
            IPrimitive& this_arg, 
            IPrimitive& arguments
            )
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->call__(this_arg, arguments);
        }

        IPrimitive& __stdcall construct__(IPrimitive& arguments)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->construct__(arguments);
        }

        void __stdcall put__(const_string_t const& key, IPrimitive& value)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->put__(key, value);
        }

        void __stdcall put_by_value__(IPrimitive const& key, IPrimitive& value)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->put_by_value__(key, value);
        }

        // scope getter
        IPrimitive& get_scope__()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->get_scope__();
        }

        // scope setter
        void set_scope__(IPrimitive& given_scope_object)
        {
            ES_ASSERT(0 != p_value_);
            p_value_->set_scope__(given_scope_object);
        }

        IPrimitive& __stdcall get_value__()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->get_value__();
        }

        IBoolean& has_instance__(IPrimitive const& value) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->has_instance__(value);
        }

        bool has_property__(const_string_t const& key)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->has_property__(key);
        }

        unsigned char __stdcall type__() const
        {
            return vt_;
        }

        const_string_t const class__() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->class__();
        }

        void const* address__() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->address__();
        }

// postfix operators
        IPrimitive& __stdcall postfix_inc__()
        {
            ES_ASSERT(0 != p_value_);
            IPrimitive& old_value = *p_value_;
            INumber& new_value = *new es_number<string_t>(operator double() + 1);
//            p_value_->release__();
            p_value_ = &new_value;
            p_value_->addref__();
            vt_ = VT::Number;
            return old_value;
        }

        IPrimitive& __stdcall postfix_dec__()
        {
            ES_ASSERT(0 != p_value_);
            IPrimitive& old_value = *p_value_;
            INumber& new_value = *new es_number<string_t>(operator double() - 1);
//            p_value_->release__();
            p_value_ = &new_value;
            p_value_->addref__();
            vt_ = VT::Number;
            return old_value;
        }

// unary operators
    // delete operator
        IBoolean& __stdcall delete__()
        {
            ES_ASSERT(0 != p_value_);
            if (VT::Undefined == vt_)
                return boolean_t::create_instance(true);
            vt_ = VT::Undefined;
            p_value_->release__();
            p_value_ = &es_undefined<string_t>::create_instance();
            return boolean_t::create_instance(true);
        }

    // void operator
        IUndefined& __stdcall void__() const
        {
            throw std::logic_error("void__");
        }

    // typeof operator
        IString& __stdcall typeof__() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->typeof__();
        }

        IPrimitive& __stdcall prefix_inc__()
        {
            ES_ASSERT(0 != p_value_);
            INumber& new_value = *new es_number<string_t>(p_value_->operator double() + 1);
            p_value_->release__();
            p_value_ = &new_value;
            p_value_->addref__();
            vt_ = VT::Number;
            return new_value;
        }

        IPrimitive& __stdcall prefix_dec__()
        {
            ES_ASSERT(0 != p_value_);
            INumber& new_value = *new es_number<string_t>(p_value_->operator double() - 1);
            p_value_->release__();
            p_value_ = &new_value;
            p_value_->addref__();
            vt_ = VT::Number;
            return new_value;
        }

        INumber& __stdcall unary_plus__() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->unary_plus__();
        }

        INumber& __stdcall unary_minus__() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->unary_minus__();
        }

        INumber& __stdcall tilde__() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->tilde__();
        }

        IBoolean& __stdcall not__() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->not__();
        }

// multiplicative operators
        INumber& __stdcall mul__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->mul__(rhs);
        }

        INumber& __stdcall div__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->div__(rhs);
        }

        INumber& __stdcall mod__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->mod__(rhs);
        }

// additive operators
        IPrimitive& __stdcall binary_plus__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->binary_plus__(rhs);
        }

        INumber& __stdcall binary_minus__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->binary_minus__(rhs);
        }

// shift operators
        INumber& __stdcall shl__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->shl__(rhs);
        }

        INumber& __stdcall sar__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->sar__(rhs);
        }

        INumber& __stdcall shr__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->shr__(rhs);
        }

// relational operators
        IBoolean& __stdcall lt__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->lt__(rhs);
        }

        IBoolean& __stdcall gt__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->gt__(rhs);
        }

        IBoolean& __stdcall le__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->le__(rhs);
        }

        IBoolean& __stdcall ge__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ge__(rhs);
        }

        IBoolean& __stdcall instanceof__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->instanceof__(rhs);
        }

        IBoolean& __stdcall in__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->in__(rhs);
        }

// equality operators
        IBoolean& __stdcall eq__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->eq__(rhs);
        }

        IBoolean& __stdcall ne__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ne__(rhs);
        }

    // strict equality operators
        IBoolean& __stdcall strict_eq__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->strict_eq__(rhs);
        }

        IBoolean& __stdcall strict_ne__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->strict_ne__(rhs);
        }

// binary bitwise operators
        INumber& __stdcall bitwise_and__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->bitwise_and__(rhs);
        }

        INumber& __stdcall bitwise_xor__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->bitwise_xor__(rhs);
        }

        INumber& __stdcall bitwise_or__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->bitwise_or__(rhs);
        }

// assignment operators
        IPrimitive& __stdcall assign__(IPrimitive& rhs)
        {
            ES_ASSERT(0 != p_value_);
            IPrimitive& old_value = *p_value_;
            p_value_ = &rhs.get_value__();
            p_value_->addref__();
            old_value.release__();
            vt_ = p_value_->type__();
            return *this;
        }

        IPrimitive& __stdcall assign_mul__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->mul__(rhs));
        }

        IPrimitive& __stdcall assign_div__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->div__(rhs));
        }

        IPrimitive& __stdcall assign_mod__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->mod__(rhs));
        }

        IPrimitive& __stdcall assign_plus__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->binary_plus__(rhs));
        }

        IPrimitive& __stdcall assign_minus__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->binary_minus__(rhs));
        }

        IPrimitive& __stdcall assign_shl__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->shl__(rhs));
        }

        IPrimitive& __stdcall assign_sar__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->sar__(rhs));
        }

        IPrimitive& __stdcall assign_shr__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->shr__(rhs));
        }

        IPrimitive& __stdcall assign_and__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->bitwise_and__(rhs));
        }

        IPrimitive& __stdcall assign_xor__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->bitwise_xor__(rhs));
        }

        IPrimitive& __stdcall assign_or__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->bitwise_or__(rhs));
        }

// type conversion
        IBoolean& ToBoolean() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ToBoolean();
        }

        INumber& ToNumber() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ToNumber();
        }

        IString& ToString() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ToString();
        }

        INumber const& ToInteger() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ToInteger();
        }

        INumber const& ToInt32() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ToInt32();
        }

        INumber const& ToUint32() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ToUint32();
        }

        IPrimitive const& ToPrimitive(unsigned char hint) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ToPrimitive(hint);
        }

        IPrimitive& ToObject()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ToObject();
        }

// native type conversion
        __stdcall operator bool() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator bool();
        }

        operator double() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator double();
        }

        operator const_string_t const() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator const_string_t const();
        }

        operator string_t const() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator string_t const();
        }

        operator ecmascript::integer_t() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator ecmascript::integer_t();
        }

        operator ecmascript::int32_t() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator ecmascript::int32_t();
        }

        operator ecmascript::uint32_t() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator ecmascript::uint32_t();
        }

        operator ecmascript::uint16_t() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator ecmascript::uint16_t();
        }

        operator es_attributes const&() const
        {
            ES_ASSERT(0 != p_attributes_);
            return *p_attributes_;
        }

    private:
        unsigned char vt_;
        union {
            IPrimitive *p_value_;
            es_attributes const *p_attributes_;
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_property_reference
    //  @brief reference object
    //
    template <typename stringT>
    struct es_property_reference
    : IReference
//    : es_reference<stringT>
    {
        typedef stringT string_t;
        typedef es_reference<stringT> self_t;
        typedef es_boolean<string_t> boolean_t;

        es_property_reference() throw()
        : vt_(VT::Undefined)
        , p_value_(&es_undefined<string_t>::create_instance())
        {
        }

        es_property_reference(es_attributes const& attributes) throw()
        : vt_(VT::Metadata)
        , p_attributes_(&attributes)
        {
        }

        es_property_reference(self_t const& other_value) throw()
        : vt_(other_value.vt_)
        , p_value_(other_value.p_value_)
        {
        }

        virtual ~es_property_reference() throw()
        {
        }

        es_property_reference<string_t> const&
        operator =(es_reference<string_t> const& rhs)
        {
            vt_ = rhs.vt_;
            p_value_ = rhs.p_value_;
            if (VT::Metadata != vt_)
                p_value_->addref__();
            return *this;
        }

    public:
        void mark__(int cookie) throw()
        {
            if (VT::Metadata != vt_)
                p_value_->mark__(cookie);
        }

        void sweep__(int cookie) throw()
        {
            if (VT::Metadata != vt_)
                p_value_->sweep__(cookie);
        }

// IReferenceCounting
        bool collect__() throw()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->collect__();
        }

        ecmascript::uint32_t addref__() throw()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->addref__();
        }

        ecmascript::uint32_t release__() throw()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->release__();
        }

// IEnumProperty
        IPrimitive& __stdcall reset__()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->reset__();
        }

        IPrimitive& __stdcall next__()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->next__();
        }

        void __stdcall push__(IPrimitive& value)
        {
            ES_ASSERT(0 != p_value_);
            p_value_->push__(value);
        }

        IPrimitive& operator [](ecmascript::uint32_t index)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator [](index);
        }

        ecmascript::uint32_t length__()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->length__();
        }

        IPrimitive ** begin__()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->begin__();
        }

        IPrimitive& __stdcall get_by_value__(IPrimitive const& key)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->get_by_value__(key);
        }

        IPrimitive& __stdcall get__(const_string_t const& key)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->get__(key);
        }

        IPrimitive& __stdcall get_prototype__()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->get_prototype__();
        }

        void __stdcall set_prototype__(IPrimitive& given_prototype)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->set_prototype__(given_prototype);
        }

        IPrimitive& __stdcall call__(
            IPrimitive& this_arg, 
            IPrimitive& arguments
            )
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->call__(this_arg, arguments);
        }

        IPrimitive& __stdcall construct__(IPrimitive& arguments)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->construct__(arguments);
        }

        void __stdcall put__(const_string_t const& key, IPrimitive& value)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->put__(key, value);
        }

        void __stdcall put_by_value__(IPrimitive const& key, IPrimitive& value)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->put_by_value__(key, value);
        }

        // scope getter
        IPrimitive& get_scope__()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->get_scope__();
        }

        // scope setter
        void set_scope__(IPrimitive& given_scope_object)
        {
            ES_ASSERT(0 != p_value_);
            p_value_->set_scope__(given_scope_object);
        }

        IPrimitive& __stdcall get_value__()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->get_value__();
        }

        IBoolean& has_instance__(IPrimitive const& value) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->has_instance__(value);
        }

        bool has_property__(const_string_t const& key)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->has_property__(key);
        }

        unsigned char __stdcall type__() const
        {
            return vt_;
        }

        const_string_t const class__() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->class__();
        }

        void const* address__() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->address__();
        }

// postfix operators
        IPrimitive& __stdcall postfix_inc__()
        {
            ES_ASSERT(0 != p_value_);
            IPrimitive& old_value = *p_value_;
            INumber& new_value = *new es_number<string_t>(operator double() + 1);
            p_value_->release__();
            p_value_ = &new_value;
            p_value_->addref__();
            vt_ = VT::Number;
            return old_value;
        }

        IPrimitive& __stdcall postfix_dec__()
        {
            ES_ASSERT(0 != p_value_);
            IPrimitive& old_value = *p_value_;
            INumber& new_value = *new es_number<string_t>(operator double() - 1);
            p_value_->release__();
            p_value_ = &new_value;
            p_value_->addref__();
            vt_ = VT::Number;
            return old_value;
        }

// unary operators
    // delete operator
        IBoolean& __stdcall delete__()
        {
            ES_ASSERT(0 != p_value_);
            if (VT::Undefined == vt_)
                return boolean_t::create_instance(true);
            vt_ = VT::Undefined;
            p_value_->release__();
            p_value_ = &es_undefined<string_t>::create_instance();
            return boolean_t::create_instance(true);
        }

    // void operator
        IUndefined& __stdcall void__() const
        {
            throw std::logic_error("void__");
        }

    // typeof operator
        IString& __stdcall typeof__() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->typeof__();
        }

        IPrimitive& __stdcall prefix_inc__()
        {
            ES_ASSERT(0 != p_value_);
            INumber& new_value = *new es_number<string_t>(p_value_->operator double() + 1);
            p_value_->release__();
            p_value_ = &new_value;
            p_value_->addref__();
            vt_ = VT::Number;
            return new_value;
        }

        IPrimitive& __stdcall prefix_dec__()
        {
            ES_ASSERT(0 != p_value_);
            INumber& new_value = *new es_number<string_t>(p_value_->operator double() - 1);
            p_value_->release__();
            p_value_ = &new_value;
            p_value_->addref__();
            vt_ = VT::Number;
            return new_value;
        }

        INumber& __stdcall unary_plus__() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->unary_plus__();
        }

        INumber& __stdcall unary_minus__() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->unary_minus__();
        }

        INumber& __stdcall tilde__() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->tilde__();
        }

        IBoolean& __stdcall not__() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->not__();
        }

// multiplicative operators
        INumber& __stdcall mul__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->mul__(rhs);
        }

        INumber& __stdcall div__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->div__(rhs);
        }

        INumber& __stdcall mod__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->mod__(rhs);
        }

// additive operators
        IPrimitive& __stdcall binary_plus__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->binary_plus__(rhs);
        }

        INumber& __stdcall binary_minus__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->binary_minus__(rhs);
        }

// shift operators
        INumber& __stdcall shl__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->shl__(rhs);
        }

        INumber& __stdcall sar__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->sar__(rhs);
        }

        INumber& __stdcall shr__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->shr__(rhs);
        }

// relational operators
        IBoolean& __stdcall lt__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->lt__(rhs);
        }

        IBoolean& __stdcall gt__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->gt__(rhs);
        }

        IBoolean& __stdcall le__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->le__(rhs);
        }

        IBoolean& __stdcall ge__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ge__(rhs);
        }

        IBoolean& __stdcall instanceof__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->instanceof__(rhs);
        }

        IBoolean& __stdcall in__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->in__(rhs);
        }

// equality operators
        IBoolean& __stdcall eq__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->eq__(rhs);
        }

        IBoolean& __stdcall ne__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ne__(rhs);
        }

    // strict equality operators
        IBoolean& __stdcall strict_eq__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->strict_eq__(rhs);
        }

        IBoolean& __stdcall strict_ne__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->strict_ne__(rhs);
        }

// binary bitwise operators
        INumber& __stdcall bitwise_and__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->bitwise_and__(rhs);
        }

        INumber& __stdcall bitwise_xor__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->bitwise_xor__(rhs);
        }

        INumber& __stdcall bitwise_or__(IPrimitive const& rhs) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->bitwise_or__(rhs);
        }

// assignment operators
        IPrimitive& __stdcall assign__(IPrimitive& rhs)
        {
            ES_ASSERT(0 != p_value_);
            IPrimitive& old_value = *p_value_;
            p_value_ = &rhs.get_value__();
            p_value_->addref__();
            old_value.release__();
            vt_ = p_value_->type__();
            return *this;
        }

        IPrimitive& __stdcall assign_mul__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->mul__(rhs));
        }

        IPrimitive& __stdcall assign_div__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->div__(rhs));
        }

        IPrimitive& __stdcall assign_mod__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->mod__(rhs));
        }

        IPrimitive& __stdcall assign_plus__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->binary_plus__(rhs));
        }

        IPrimitive& __stdcall assign_minus__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->binary_minus__(rhs));
        }

        IPrimitive& __stdcall assign_shl__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->shl__(rhs));
        }

        IPrimitive& __stdcall assign_sar__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->sar__(rhs));
        }

        IPrimitive& __stdcall assign_shr__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->shr__(rhs));
        }

        IPrimitive& __stdcall assign_and__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->bitwise_and__(rhs));
        }

        IPrimitive& __stdcall assign_xor__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->bitwise_xor__(rhs));
        }

        IPrimitive& __stdcall assign_or__(IPrimitive const& rhs)
        {
            ES_ASSERT(0 != p_value_);
            return this->assign__(p_value_->bitwise_or__(rhs));
        }

// type conversion
        IBoolean& ToBoolean() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ToBoolean();
        }

        INumber& ToNumber() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ToNumber();
        }

        IString& ToString() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ToString();
        }

        INumber const& ToInteger() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ToInteger();
        }

        INumber const& ToInt32() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ToInt32();
        }

        INumber const& ToUint32() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ToUint32();
        }

        IPrimitive const& ToPrimitive(unsigned char hint) const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ToPrimitive(hint);
        }

        IPrimitive& ToObject()
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->ToObject();
        }

// native type conversion
        __stdcall operator bool() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator bool();
        }

        operator double() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator double();
        }

        operator const_string_t const() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator const_string_t const();
        }

        operator string_t const() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator string_t const();
        }

        operator ecmascript::integer_t() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator ecmascript::integer_t();
        }

        operator ecmascript::int32_t() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator ecmascript::int32_t();
        }

        operator ecmascript::uint32_t() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator ecmascript::uint32_t();
        }

        operator ecmascript::uint16_t() const
        {
            ES_ASSERT(0 != p_value_);
            return p_value_->operator ecmascript::uint16_t();
        }

        operator es_attributes const&() const
        {
            ES_ASSERT(0 != p_attributes_);
            return *p_attributes_;
        }
    private:
        unsigned char vt_;
        union {
            IPrimitive *p_value_;
            es_attributes const *p_attributes_;
        };
    };

} // namespace ecmascript
