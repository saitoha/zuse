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



#include<deque>

namespace ecmascript {

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_array
    //  @brief IArray implementaion
    //
    template <typename stringT>
    struct es_array
    : public base_classes::es_collectable_object<
        base_classes::es_object_impl<IArray, stringT> >
    {
        typedef stringT string_t;
        typedef es_array<string_t> self_t;
        typedef base_classes::es_object_impl<IArray, string_t> object_t;
        typedef std::deque<es_reference<string_t> > dequeue_t;
        static IFunction& static_get_constructor()
        {
            static struct constructor
            : public base_classes::es_function_impl<IArrayConstructor, stringT>
            {
                constructor() throw()
                {
                }

                ~constructor() throw()
                {
                }

                IPrimitive& __stdcall call__(
                    IPrimitive& /*this_arg*/, 
                    IPrimitive& arguments
                    )
                {
                    return construct__(arguments);
                }

                IPrimitive& __stdcall construct__(IPrimitive& arguments)
                {
                    return *new es_array<string_t>(arguments);
                }

                IPrimitive& __stdcall prototype()
                {
                    return es_array<string_t>::static_prototype();
                }

            } constructor_;
            return constructor_;
        }
// constructor
        static IArray& __stdcall create_instance()
        {
            return *new self_t;
        }

        es_array() throw()
		: internal_value_(*new dequeue_t)
        {
        }

        explicit es_array(IPrimitive& arguments)
		: internal_value_(*new dequeue_t)
        {
            uint32_t argc = arguments.length__();
            if (1 == argc
                && VT::Number == arguments[0].type__()
                && construct_from_length(arguments[0].operator double()))
                    return;
            for (uint32_t i = 0; i < argc; ++ i)
                internal_value_.push_back(arguments[i]);
        }
		
		virtual ~es_array() throw()
		{
			delete &internal_value_;
#if ES_TRACE_DESTRUCTION
            puts("array is deleted.\n");
#endif // ES_TRACE_DESTRUCTION
 		}

        void mark__(int cookie) throw()
        {
            if (this->get_mark() == cookie)
                return;
            object_t::mark__(cookie);
            for (typename dequeue_t::iterator it = internal_value_.begin();
                it != internal_value_.end(); ++it)
                it->mark__(cookie);
        }

        const_string_t const class__() const throw()
        {
            return L"Array";
        }

// IArrayBase override
        void __stdcall push__(IPrimitive& value)
        {
            internal_value_.push_back(value);
        }

        IPrimitive& operator [](ecmascript::uint32_t index)
        {
            return internal_value_[index];
        }

        ecmascript::uint32_t length__()
        {
            es_static_assert<sizeof(ecmascript::uint32_t) == sizeof(size_t)>();
            return ecmascript::uint32_t(internal_value_.size());
        }

        IPrimitive ** begin__()
        {
            ES_ABORT("not implemented: es_array::begin__");
        }

// IEnumProperty implemetation
        IPrimitive& __stdcall reset__()
        {
            return dequeue_counter = 0, object_t::reset__();
        }

        IPrimitive& __stdcall next__()
        {
            return (dequeue_counter == internal_value_.size()) ?
                object_t::next__():
                *new es_number<string_t>(ecmascript::uint32_t(dequeue_counter++));
        }

// IObject implementation
        IPrimitive& __stdcall pop()
        {
            for (IPrimitive& result_value = *internal_value_.rend();;)
                return internal_value_.pop_back(), result_value;
        }

        INumber& __stdcall push(IPrimitive& value)
        {
            return push__(value), length();
        }

        IPrimitive& __stdcall shift()
        {
            for (IPrimitive& result_value = *internal_value_.rend();;)
                return internal_value_.pop_front(), result_value;
        }

        IUndefined& __stdcall unshift(IPrimitive& value)
        {
            return internal_value_.push_front(value),
                es_undefined<string_t>::create_instance();
        }

        INumber& __stdcall length() const
        {
            return *new es_number<string_t>(double(internal_value_.size()));
        }

        IPrimitive& __stdcall get_by_value__(IPrimitive const& key)
        {
            if (VT::Number != key.type__())
                return object_t::get__(key.operator const_string_t const());
            size_t index = size_t(key.ToNumber().operator double());
            return internal_value_[
                index >= internal_value_.size() ?
                    internal_value_.resize(index + 1), index: index];
        }

        IPrimitive& __stdcall get__(const_string_t const& key)
        {
            return object_t::get__(key);
        }

// typeof operator
        IString& __stdcall typeof__() const
        {
            return *new es_string<stringT>(L"array");
        }

// native type conversion
        operator double() const
        {
            return 0.; /* dummy */
        }

        operator const_string_t const() const
        {
            const_string_t ws_;
            typename dequeue_t::const_iterator it = internal_value_.begin();
            if (it == internal_value_.end())
                return ws_;
            ws_ = ws_ + (*it).operator const_string_t const();
            while (++ it != internal_value_.end())
                ws_ = ws_ + L"," + (*it).operator const_string_t const();
            return ws_;
        }

    private:
        bool construct_from_length(double d)
        {
            if (d == ecmascript::uint32_t(d))
                return internal_value_.resize(size_t(d)), true;
            return false;
        }

    private:
        typename dequeue_t::size_type dequeue_counter;
        dequeue_t& internal_value_;
    };

} // namespace ecmascript
