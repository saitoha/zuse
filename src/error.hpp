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



namespace ecmascript {

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_native_error
    //
    template <typename stringT>
    struct es_native_error
    : public base_classes::es_object_impl<INativeError, stringT>
    {
        typedef stringT string_t;
        typedef es_native_error<string_t> self_t;

        static IFunction& static_get_constructor()
        {
            static struct constructor
            : public base_classes::es_function_impl<
                INativeErrorConstructor, stringT>
            {
                IPrimitive& __stdcall call__(
                    IPrimitive& /*this_arg*/, 
                    IPrimitive& arguments
                    )
                {
                    return construct__(arguments);
                }

                IPrimitive& __stdcall construct__(IPrimitive& arguments)
                {
                    return *new es_native_error<string_t>(
                        arguments.length__() > 0 ?
                            arguments[0].ToString():
                            es_undefined<string_t>::create_instance());
                }

                IPrimitive& __stdcall prototype()
                {
                    return es_native_error<string_t>::static_prototype();
                }

            } constructor_;
            return constructor_;
        }

        es_native_error() throw()
        {
        }

        es_native_error(const_string_t const& message)
        : message_(*new es_string<string_t>(message))
        , name_(*new es_string<string_t>(L"Error"))
        {
        }

        es_native_error(IPrimitive& message)
        : message_(message)
        , name_(*new es_string<string_t>(L"Error"))
        {
        }

        es_native_error(IPrimitive& message, IString& name)
        : message_(message)
        , name_(name)
        {
        }

        ~es_native_error() throw()
        {
        }

        const_string_t const class__() const
        {
            return L"Error";
        }

        IPrimitive& __stdcall message() const
        {
            return message_;
        }

        IPrimitive& __stdcall name() const
        {
            return name_;
        }

        es_native_error(es_native_error const&)
        : message_(*new es_string<string_t>())
        , name_(*new es_string<string_t>(L"Error"))
        {
        }

        operator IUndefined& ()
        {
            union {
                IUndefined *p_dest;
                self_t *p_src;
                ptrdiff_t value;
            } box;
            box.p_src = this;
            ++box.value;
            return *box.p_dest;
        }

        operator IFunction& ()
        {
            union {
                IFunction *p_dest;
                self_t *p_src;
                ptrdiff_t value;
            } box;
            box.p_src = this;
            ++box.value;
            return *box.p_dest;
        }

        operator INumber& ()
        {
            union {
                INumber *p_dest;
                self_t *p_src;
                ptrdiff_t value;
            } box;
            box.p_src = this;
            ++box.value;
            return *box.p_dest;
        }

        operator IBoolean& ()
        {
            union {
                IBoolean *p_dest;
                self_t *p_src;
                ptrdiff_t value;
            } box;
            box.p_src = this;
            ++box.value;
            return *box.p_dest;
        }

        operator IString& ()
        {
            union {
                IString *p_dest;
                self_t *p_src;
                ptrdiff_t value;
            } box;
            box.p_src = this;
            ++box.value;
            return *box.p_dest;
        }

        operator IPrimitive& ()
        {
            union {
                IPrimitive *p_dest;
                self_t *p_src;
                ptrdiff_t value;
            } box;
            box.p_src = this;
            ++box.value;
            return *box.p_dest;
        }

    protected:
        es_native_error& operator=(es_native_error const& rhs)
        {
            message_ = rhs.message_;
            name_ = rhs.name_;
            return *this;
        }

    protected:
        IPrimitive& message_;
        IPrimitive& name_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_type_error
    //
    template <typename stringT>
    struct es_type_error
    : es_native_error<stringT>
    {
        typedef stringT string_t;
        typedef es_native_error<stringT> base_t;
        typedef es_type_error<string_t> self_t;

        explicit es_type_error(IPrimitive& message)
        : base_t(message)
        {
        }

        explicit es_type_error(const_string_t const message)
        : base_t(message)
        {
        }

        const_string_t const class__() const
        {
            return L"TypeError";
        }

        es_type_error(self_t const& rhs)
        : base_t(rhs)
        {
        }

    protected:
        self_t& operator=(self_t const& rhs)
        {
            base_t::message_ = rhs.message_;
            base_t::name_ = rhs.name_;
            return *this;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_range_error
    //
    template <typename stringT>
    struct es_range_error
    : es_native_error<stringT>
    {
        typedef stringT string_t;
        typedef es_native_error<stringT> base_t;
        typedef es_range_error<string_t> self_t;

        explicit es_range_error(IPrimitive& message)
        : base_t(message)
        {
        }

        explicit es_range_error(const_string_t const message)
        : base_t(message)
        {
        }

        const_string_t const class__() const
        {
            return L"RangeError";
        }

        es_range_error(self_t const& rhs)
        : base_t(rhs)
        {
        }

    protected:
        self_t& operator=(self_t const& rhs)
        {
            base_t::message_ = rhs.message_;
            base_t::name_ = rhs.name_;
            return *this;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_reference_error
    //
    template <typename stringT>
    struct es_reference_error
    : es_native_error<stringT>
    {
        typedef stringT string_t;
        typedef es_native_error<stringT> base_t;
        typedef es_reference_error<string_t> self_t;

        explicit es_reference_error(IPrimitive& message)
        : base_t(message)
        {
        }

        explicit es_reference_error(const_string_t const message)
        : base_t(message)
        {
        }

        const_string_t const class__() const
        {
            return L"ReferenceError";
        }

        es_reference_error(self_t const& rhs)
        : base_t(rhs)
        {
        }

    protected:
        self_t& operator=(self_t const& rhs)
        {
            base_t::message_ = rhs.message_;
            base_t::name_ = rhs.name_;
            return *this;
        }
    };


    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_eval_error
    //
    template <typename stringT>
    struct es_eval_error
    : es_native_error<stringT>
    {
        typedef stringT string_t;
        typedef es_native_error<stringT> base_t;
        typedef es_eval_error<string_t> self_t;

        explicit es_eval_error(IPrimitive& message)
        : base_t(message)
        {
        }

        explicit es_eval_error(const_string_t const message)
        : base_t(message)
        {
        }

        const_string_t const class__() const
        {
            return L"EvalError";
        }

        es_eval_error(self_t const& rhs)
        : base_t(rhs)
        {
        }

    protected:
        self_t& operator=(self_t const& rhs)
        {
            base_t::message_ = rhs.message_;
            base_t::name_ = rhs.name_;
            return *this;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_syntax_error
    //
    template <typename stringT>
    struct es_syntax_error
    : es_native_error<stringT>
    {
        typedef stringT string_t;
        typedef es_native_error<stringT> base_t;
        typedef es_syntax_error<string_t> self_t;

        explicit es_syntax_error(IPrimitive& message)
        : base_t(message)
        {
        }

        explicit es_syntax_error(const_string_t const message)
        : base_t(message)
        {
        }

        es_syntax_error(self_t const& rhs)
        : base_t(rhs)
        {
        }

        const_string_t const class__() const
        {
            return L"RangeError";
        }

    protected:
        self_t& operator=(self_t const& rhs)
        {
            base_t::message_ = rhs.message_;
            base_t::name_ = rhs.name_;
            return *this;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_uri_error
    //
    template <typename stringT>
    struct es_uri_error
    : es_native_error<stringT>
    {
        typedef stringT string_t;
        typedef es_native_error<stringT> base_t;
        typedef es_uri_error<string_t> self_t;

        explicit es_uri_error(IPrimitive& message)
        : base_t(message)
        {
        }

        explicit es_uri_error(const_string_t const message)
        : base_t(message)
        {
        }

        const_string_t const class__() const
        {
            return L"URIError";
        }

        es_uri_error(self_t const& rhs)
        : base_t(rhs)
        {
        }

    protected:
        self_t& operator=(self_t const& rhs)
        {
            base_t::message_ = rhs.message_;
            base_t::name_ = rhs.name_;
            return *this;
        }
    };

} // namespace ecmascript
