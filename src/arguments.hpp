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
    //  @class es_arguments
    //  @brief IArguments implementaion
    //
    template <typename stringT>
    struct es_arguments
    : public base_classes::es_collectable_object<
        base_classes::es_object_impl<IArguments, stringT> >
    {
        typedef stringT string_t;
        typedef vector<IPrimitive *> value_t;
        typedef base_classes::es_object_impl<IArguments, stringT> object_t;

        static IArguments& __stdcall create_instance(IPrimitive& callee)
        {
            return *new es_arguments<string_t>(callee);
        }

    // constructor
        explicit es_arguments(IPrimitive& callee)
        : callee_(callee)
        {
        }

    // destructor
        virtual ~es_arguments() throw()
        {
#if ES_TRACE_DESTRUCTION
            puts("arguments is deleted.\n");
#endif // ES_TRACE_DESTRUCTION
        }

        void mark__(int cookie) throw()
        {
            if (this->get_mark() == cookie)
                return;
            object_t::mark__(cookie);
            for (value_t::iterator it = internal_value_.begin();
                it != internal_value_.end(); ++it)
                (*it)->mark__(cookie);
            callee_.mark__(cookie);
        }

        const_string_t const class__() const throw()
        {
            return L"Array";
        }

    // IArrayBase override
        void __stdcall push__(IPrimitive& value)
        {
            internal_value_.push_back(&value);
        }

        IPrimitive& operator [](ecmascript::uint32_t index)
        {
            return length__() <= index ?
                es_undefined<string_t>::create_instance():
                *internal_value_[index];
        }

        ecmascript::uint32_t length__()
        {
            ES_STATIC_ASSERT(sizeof(ecmascript::uint32_t) == sizeof(size_t));
            return ecmascript::uint32_t(internal_value_.size());
        }

        IPrimitive ** begin__()
        {
            return &*internal_value_.begin();
        }

    // IArguments override
        IPrimitive& __stdcall callee()
        {
            return callee_;
        }

        INumber& __stdcall length() const
        {
            return *new es_number<string_t>(double(internal_value_.size()));
        }

        operator const_string_t const() const
        {
            return L"arguments object: "
                + object_t::operator const_string_t const();
        }

        operator string_t const() const
        {
            return L"arguments object: "
                + object_t::operator string_t const();
        }

    // IDictionaryObject override
        IPrimitive& __stdcall get_by_value__(IPrimitive const& key)
        {
            if (VT::Number != key.type__())
                return this->get__(key.operator const_string_t const());
            size_t index = size_t(key.ToNumber().operator double());
            if (index >= internal_value_.size())
                internal_value_.resize(index + 1);
            return *internal_value_[index];
        }
    private:
        typename value_t::size_type dequeue_counter_;
        value_t internal_value_;
        IPrimitive& callee_;
    };

} // namespace ecmascript
