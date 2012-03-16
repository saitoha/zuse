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

    namespace {

        template <typename stringT>
        stringT& es_replace_helper(
            stringT& /* in */ given_string,
            IPrimitive& /* in */ match_result,
            IPrimitive const& /* in */ last_index,
            stringT const& /* in */ replace_value
            ) throw()
        {
            ES_ASSERT(L"array" == match_result.typeof__().operator const_string_t const());
//            ES_ASSERT(1 <= match_result.length__().operator ecmascript::uint32_t());
            ES_ASSERT(VT::String == match_result.operator[](0).type__());
            size_t size = match_result.operator[](0).operator const_string_t const().length();
            size_t begin_index = last_index.operator ecmascript::uint32_t() - size;
            return given_string.replace(begin_index, size, replace_value);
        }

        template <typename stringT>
        struct es_string_or_binded_function
        {
            explicit es_string_or_binded_function(
                IPrimitive& string_value) throw()
            : value_(string_value)
            , p_callee_(0)
            {
            }

            explicit es_string_or_binded_function(
                IPrimitive& function_value,
                IPrimitive& callee) throw()
            : value_(function_value)
            , p_callee_(&callee)
            {
            }

            operator std::wstring const() const
            {
                return 0 == p_callee_ ?
                    value_.operator std::wstring const() :
                    value_.call__(
                        value_.get__(L"callee"),
                        *new es_arguments<stringT>(*p_callee_));
            }

        private:
            IPrimitive& value_;
            IPrimitive *const p_callee_;

        };

        struct es_match_with_string
        {
            typedef es_match_with_string match_type;
        };

        struct es_match_with_regexp
        {
            typedef es_match_with_regexp match_type;
        };

        template <typename policyT>
        struct es_match_policy : policyT
        {
            typedef es_match_policy<policyT> match_policy_t;
        };

        struct es_replace_with_string
        {
            typedef es_replace_with_string replace_type;
        };

        struct es_replace_with_call_result
        {
            typedef es_replace_with_call_result replace_type;
        };

        template <typename policyT>
        struct es_replace_action_policy : policyT
        {
            typedef es_replace_action_policy<policyT> action_policy_t;
        };

        template <typename matchPolicyT>
        struct es_replace_traits;

        template <>
        struct es_replace_traits<es_match_policy<es_match_with_string> >
        {
            template <typename stringT>
            static IString const& do_replace(
                IString& lhs_string,
                IPrimitive& search_value,
                es_string_or_binded_function<stringT> const& replace_value
                ) throw()
            {
                stringT this_string = lhs_string.operator std::wstring const();
                return *new es_string<stringT>(
                    this_string.replace(
                        this_string.find(
                            search_value.operator std::wstring const(), 0),
                        search_value.operator std::wstring const().length(),
                        replace_value.operator std::wstring const()));
            }
        };

        template <>
        struct es_replace_traits<es_match_policy<es_match_with_regexp> >
        {
            template <typename stringT>
            static IString const& do_replace(
                IString& lhs_string,
                IPrimitive& search_value,
                es_string_or_binded_function<stringT> const& replace_value
                ) throw()
            {
                ES_ASSERT(L"RegExp" == search_value.class__());
                IRegExp& regexp = reinterpret_cast<IRegExp&>(search_value);
                return regexp.global().operator bool() ?
                    es_replace_when_global_flag_is_on<stringT>(
                        regexp, lhs_string, replace_value):
                    es_replace_when_global_flag_is_off<stringT>(
                        regexp, lhs_string, replace_value);
            }

        protected:
            template <typename stringT>
            static IString const& es_replace_when_global_flag_is_on(
                IRegExp& regexp,
                IString& lhs_string,
                es_string_or_binded_function<stringT> const& replace_value
                ) throw()
            {
                stringT result_string(lhs_string.operator std::wstring const());
                {
                    struct es_match_results_stack
                    {
                        explicit es_match_results_stack(
                            stringT& result_string) throw()
                        : p_match_result_(0)
                        , result_string_(result_string)
                        {
                        }

                        ~es_match_results_stack()
                        {
                            if (0 == p_match_result_)
                                return;
                            ES_ASSERT(0 != p_match_result_);
                            es_match_result current = *p_match_result_;
                            delete p_match_result_;
                            p_match_result_ = current.get_prev();
                            current.apply(result_string_);
                            this->~es_match_results_stack();
                        }

                        void store(
                            IPrimitive& matcher,
                            IPrimitive& last_index,
                            es_string_or_binded_function<stringT> const& replace_value
                            ) throw()
                        {
                            p_match_result_ = new es_match_result(
                                matcher,
                                last_index,
                                replace_value.operator stringT const(),
                                p_match_result_);
                        }
                    private:
                        struct es_match_result
                        {
                            typedef es_match_result self_t;

                            explicit es_match_result(
                                IPrimitive& matcher,
                                IPrimitive& last_index,
                                stringT const& replace_value,
                                self_t const* rhs)
                            : matcher_(matcher)
                            , last_index_(last_index)
                            , replace_value_(replace_value)
                            , prev_(rhs)
                            {
                            }

                            void apply(stringT& result_string)
                            {
                                es_replace_helper<stringT>(
                                    result_string, matcher_,
                                    last_index_, replace_value_);
                            }

                            self_t const* get_prev() const
                            {
                                return prev_;
                            }

                        private:
                            IPrimitive& matcher_;
                            IPrimitive& last_index_;
                            stringT const replace_value_;
                            self_t const* prev_;
                        } const* p_match_result_;
                        stringT& result_string_;

                    } stack(result_string);

                    for (IPrimitive* p_matcher = &regexp.exec(lhs_string);
                        VT::Null != p_matcher->type__();
                        p_matcher = &regexp.exec(lhs_string))
                        stack.store(*p_matcher, regexp.lastIndex(), replace_value);
                }
                return *new es_string<stringT>(result_string);
            }

            template <typename stringT>
            static IString const& es_replace_when_global_flag_is_off(
                IRegExp& regexp,
                IString& lhs_string,
                es_string_or_binded_function<stringT> const& replace_value
                ) throw()
            {
                stringT result_string(lhs_string.operator std::wstring const());
                IPrimitive& match_result = regexp.exec(lhs_string);
                return VT::Null == match_result.type__() ?
                    lhs_string :
                    *new es_string<stringT>(
                        es_replace_helper(
                            result_string,
                            match_result,
                            regexp.lastIndex(),
                            replace_value.operator stringT const()));
            }

        };

        template <typename stringT, typename matchPolicyT>
        inline IString const& es_replace(
            IString& lhs_string,
            IPrimitive& searchValue,
            IPrimitive& replaceValue)
        {
            return es_replace_traits<
                es_match_policy<matchPolicyT>
            >::template do_replace<stringT>(
                lhs_string,
                searchValue,
                es_string_or_binded_function<stringT>(replaceValue));
        }

        template <typename stringT, typename matchPolicyT>
        inline IString const& es_replace(
            IString& lhs_string,
            IPrimitive& searchValue,
            IPrimitive& replaceValue,
            IPrimitive& callee)
        {
            return es_replace_traits<
                es_match_policy<matchPolicyT>
            >::template do_replace<stringT>(
                lhs_string,
                searchValue,
                es_string_or_binded_function<stringT>(replaceValue, callee));
        }

    } // anonymous namespace

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_string
    //  @brief IString implementaion
    //
    template <typename stringT>
    struct es_string
    : public base_classes::es_collectable_object<
        base_classes::es_primitive_impl<IString, stringT> >
    {
        typedef stringT string_t;
        typedef es_string<string_t> self_t;
        typedef const_string_t internal_value_t;

        static IFunction& static_get_constructor()
        {
            // factory class
            static struct constructor
            : public base_classes::es_function_impl<IStringConstructor, stringT>
            {
                constructor() throw()
                {
                }

                ~constructor() throw()
                {
                }

                IPrimitive& __stdcall prototype()
                {
                    return es_string<string_t>::static_prototype();
                }

                IString const& __stdcall fromCharCode(IPrimitive& arguments) const
                {
                    ES_ASSERT(0 <= arguments.length__());
                    if (VT::Undefined == arguments[0].type__())
                        return *new es_string<string_t>();
                    string_t value;
                    for (ecmascript::uint32_t i = 0; i < arguments.length__(); ++i)
                        value += string_t::traits_type::to_char_type(
                            arguments[i].operator ecmascript::uint16_t());
                    return *new es_string<string_t>(value);
                }

                IPrimitive& __stdcall construct__(IPrimitive& arguments)
                {
                    return *new es_delegate_object<string_t>(
                        *new es_string<string_t>(arguments));
                }

                IPrimitive& __stdcall call__(
                    IPrimitive& /*this_arg*/, 
                    IPrimitive& arguments
                    )
                {
                    return arguments.length__() > 0 ?
                        arguments[0].ToString(): *new es_string<string_t>;
                }

            } constructor_;
            return constructor_;
        } // static_get_constructor

// constructor
        es_string() throw()
        : internal_value_()
        {
        }

        template <typename charT>
        explicit es_string(
            charT const *const initial_value        /* c style string */
            ) throw()
        : internal_value_(initial_value, initial_value + wcslen(initial_value))
        {
        }

        explicit es_string(
            internal_value_t const& initial_value   /* native string */
            ) throw()
        : internal_value_(initial_value)
        {
        }

        explicit es_string(
            IPrimitive& arguments       /* array of arguments */
            ) throw()
        : internal_value_(
            arguments.length__() > 0 ?
                arguments[0].operator const_string_t const() : L"")
        {
        }

        template <typename charT>
        explicit es_string(
            charT const* const begin,   /* start of character sequence */
            charT const* const end      /* end of character sequence */
            ) throw()
        : internal_value_(begin, end)
        {
        }

        static IString& __stdcall create_instance(
            internal_value_t const& initial_value   /* native string */
            ) throw()
        {
            return *new self_t(initial_value);
        }

        static IString& create_null_string() throw()
        {
            return *new self_t();
        }

// internal methods
        const_string_t const class__() const throw()
        {
            return L"String";
        }

// dispatch methods
        INumber& __stdcall length() const
        {
            return *new es_number<string_t>(
                ecmascript::uint32_t(operator const_string_t const().length()));
        }

        IString const& __stdcall toString() const
        {
            return this->ToString();
        }

        IString const& __stdcall valueOf() const
        {
            return this->ToString();
        }

        IString const& __stdcall charAt(IPrimitive& pos) const throw()
        {
            const_string_t const& str = operator const_string_t const();
            if (str.length() <= pos.operator ecmascript::uint32_t())
                return *new es_string<string_t>();
            return *new es_string<string_t>(
                string_t(1, str.at(
                    pos.operator ecmascript::integer_t())));
        }

        INumber const& __stdcall charCodeAt(IPrimitive& pos) const
        {
            const_string_t const& str = operator const_string_t const();
            size_t const index = pos.operator ecmascript::integer_t();
            if (0 <= index && index < str.length())
                return *new es_number<string_t>(
                    std::char_traits<wchar_t>::to_int_type(
                        operator const_string_t const().at(index)));
            return *new es_number<stringT>(
                (std::numeric_limits<double>::quiet_NaN)());
        }

        IString const& __stdcall concat(IPrimitive& arguments) const
        {
            string_t result = operator string_t const();
            for (size_t i = 0; i < arguments.length__();)
                result += arguments[i++].operator const_string_t const();
            return *new es_string<string_t>(result);
        }


        INumber const& __stdcall indexOf(IPrimitive& arguments) const
        {
            ES_ASSERT(1 <= arguments.length__());
//            ES_ASSERT(-1 == string_t::npos);
            return *new es_number<string_t>(ecmascript::int32_t(
                operator const_string_t const().find(
                    arguments[0].operator const_string_t const(),
                    arguments.length__() > 1 ?
                        arguments[1].operator integer_t(): 0)));
        }

        INumber const& __stdcall lastIndexOf(IPrimitive& arguments) const
        {
            ES_ASSERT(1 <= arguments.length__());
//            ES_ASSERT(-1 == string_t::npos);
            string_t const& str = operator string_t const();
            return *new es_number<string_t>(ecmascript::int32_t(
                str.rfind(
                    arguments[0].operator const_string_t const(),
                    arguments.length__() > 1 ?
                        arguments[1].operator integer_t(): str.length() - 1)));
        }

        INumber const& __stdcall localeCompare(IPrimitive& that) const
        {
#if 1
            const_string_t const& lhs = operator const_string_t const();
            const_string_t const& rhs = that.operator const_string_t const();
            return *new es_number<string_t>(
                base_services::es_locale_compare(
                    lhs.begin(), lhs.end(), rhs.begin(), rhs.end()));
#else // comparation using utf-16
            return *new es_number<string_t>(
                operator std::wstring const().compare(
                    that.operator string_t const()));
#endif // _MSC_VER
        }

        IPrimitive const& __stdcall match(IPrimitive& regexp) const
        {
            IRegExp& regexp_object = get_regexp(regexp);
            IString& lhs_string = this->ToString();
            if (false == regexp_object.global().operator bool())
                return regexp_object.exec(lhs_string);
            IArray& results = *new es_array<string_t>();
            for (IPrimitive * p_matcher = &regexp_object.exec(lhs_string);
                VT::Null != p_matcher->type__();
                p_matcher = &regexp_object.exec(lhs_string))
                results.push__(*p_matcher);
            return results;
        }

        IRegExp& get_regexp(IPrimitive& regexp) const
        {
            if (L"RegExp" == regexp.class__())
                return reinterpret_cast<IRegExp&>(regexp);
            IArray& arguments = *new es_array<string_t>();
            arguments.push__(regexp);
            return *new es_regexp<string_t>(arguments);
        }

        IString const& __stdcall replace(IPrimitive& arguments) const
        {
            ES_ASSERT(1 <= arguments.length__());
            IPrimitive& searchValue = arguments[0];
            IPrimitive& replaceValue = arguments[1];
            if (L"Function" != replaceValue.class__())
                return L"RegExp" == searchValue.class__() ?
                    es_replace<string_t, es_match_with_regexp>(
                        (*this).ToString(), searchValue, replaceValue)
                    : es_replace<string_t, es_match_with_string>(
                        (*this).ToString(), searchValue, replaceValue);
            IPrimitive& callee = arguments.get__(L"callee");
            ES_ASSERT(VT::Undefined != callee.type__());
            return L"RegExp" == searchValue.class__() ?
                es_replace<string_t, es_match_with_regexp>(
                    (*this).ToString(), searchValue, replaceValue, callee)
                : es_replace<string_t, es_match_with_string>(
                    (*this).ToString(), searchValue, replaceValue, callee);
        }

        INumber const& __stdcall search(IPrimitive& regexp) const
        {
            IRegExp& regexp_object = get_regexp(regexp);
            IPrimitive& match = regexp_object.exec(this->ToString());
            if (VT::Null == match.type__())
                return *new es_number<string_t>(-1);
            ES_ASSERT(
                0 < match.length__()
                && VT::Undefined != match[0].get__(L"length").type__());
            return regexp_object
                .lastIndex()
                .binary_minus__(match[0].get__(L"length").get_value__());
        }

        IString const& __stdcall slice(
            IPrimitive& /* [in] */ start,
            IPrimitive& /* [in] */ end
            ) const
        {
            string_t const lhs_value = operator string_t const();
            using ::ecmascript::base_services::es_max;
            using ::ecmascript::base_services::es_min;
            int32_t length = int32_t(lhs_value.length());
            int32_t start_pos = start.operator int32_t();
            int32_t end_pos = VT::Undefined == end.type__() ?
                length : end.operator int32_t();
            int32_t first = start_pos < 0 ?
                es_max<int32_t>(length + start_pos, 0):
                es_min<int32_t>(start_pos, length);
            int32_t last = end_pos < 0 ?
                es_max<int32_t>(length + end_pos, 0):
                es_min<int32_t>(end_pos, length);
            return *new es_string<string_t>(
                lhs_value.substr(first, es_max<int32_t>(last - first, 0)));
        }

        IArray const& __stdcall split(IPrimitive& arguments) const
        {
            IPrimitive& separator = arguments[0];
            IPrimitive& limit = arguments[1];
            uint32_t limit_value = VT::Undefined == limit.type__() ?
                (std::numeric_limits<uint32_t>::max)():
                limit.operator uint32_t();
            return L"RegExp" == separator.class__() ?
                split_match(
                    (*this).ToString(), reinterpret_cast<IRegExp&>(separator),
                    limit_value, *new es_array<string_t>())
                : split_match_with_string(
                    operator string_t const(),
                    separator.operator string_t const(),
                    limit_value, *new es_array<string_t>());
        }

        IArray const& split_match(
            IString& /* [in] */ lhs_value,
            IRegExp& /* [in] */ separator_regexp,
            typename string_t::size_type /* [in] */ limit_value,
            IArray& /* [in] */ result_array
            ) const
        {
            return result_array;
        }

        IArray const& split_match_with_string(
            string_t const& /* [in] */ lhs_value,
            string_t const& /* [in] */ separator_string,
            typename string_t::size_type /* [in] */ limit_value,
            IArray& /* [in] */ result_array,
            typename string_t::size_type position = 0
            ) const
        {
            while (true)
            {
                typename string_t::size_type length
                    = separator_string.length();
                typename string_t::size_type pos = 0 == length ?
                    position + 1: lhs_value.find(separator_string, position);
                if (string_t::npos == pos)
                    return result_array.push__(
                        *new es_string<string_t>(
                            lhs_value.substr(position))), result_array;
                result_array.push__(*new es_string<string_t>(
                    lhs_value.substr(position, pos - position)));
                position = pos + length;
            }
        }

        IString const& __stdcall substring(
            IPrimitive& /* [in] */ start,
            IPrimitive& /* [in] */ end
            ) const
        {
            if (start.gt__(end).operator bool())
                return substring(end, start);
            string_t const lhs_value = operator string_t const();
            using ::ecmascript::base_services::es_max;
            using ::ecmascript::base_services::es_min;
            int32_t length = int32_t(lhs_value.length());
            int32_t start_pos = start.operator int32_t();
            int32_t end_pos = VT::Undefined == end.type__() ?
                length : end.operator int32_t();
            int32_t l1 = es_min<int32_t>(es_max<int32_t>(start_pos, 0), length);
            int32_t l2 = es_min<int32_t>(es_max<int32_t>(end_pos, 0), length);
            int32_t first = es_min<int32_t>(l1, l2);
            int32_t last = es_max<int32_t>(l1, l2);
            return *new es_string<string_t>(
                lhs_value.substr(first, last - first));
        }

        IString const& __stdcall toLowerCase() const
        {
            throw *new es_native_error<string_t>(L"not implemented: toLowerCase");
        }

        IString const& __stdcall toLocaleLowerCase() const
        {
            throw *new es_native_error<string_t>(L"not implemented: toLocaleLowerCase");
        }

        IString const& __stdcall toUpperCase() const
        {
            throw *new es_native_error<string_t>(L"not implemented: toUpperCase");
        }

        IString const& __stdcall toLocaleUpperCase() const
        {
            throw *new es_native_error<string_t>(L"not implemented: toLocaleUpperCase");
        }

// typeof operator
        IString& __stdcall typeof__() const
        {
            return *new es_string<stringT>(L"string");
        }

// additive operators
        IPrimitive& __stdcall binary_plus__(IPrimitive const& rhs) const
        {
            return *new es_string<string_t>(
                internal_value_ + rhs.operator const_string_t const());
        }

// relational operators
        IBoolean& __stdcall lt__(IPrimitive const& rhs) const
        {
            return VT::String == rhs.type__() ?
                es_boolean<string_t>::create_instance(
                    internal_value_ < rhs.operator const_string_t const()):
                this->ToNumber().lt__(rhs);
        }

        IBoolean& __stdcall gt__(IPrimitive const& rhs) const
        {
            return VT::String == rhs.type__() ?
                es_boolean<string_t>::create_instance(
                    internal_value_ > rhs.operator const_string_t const()):
                this->ToNumber().gt__(rhs);
        }

        IBoolean& __stdcall le__(IPrimitive const& rhs) const
        {
            return VT::String == rhs.type__() ?
                es_boolean<string_t>::create_instance(
                    internal_value_ <= rhs.operator const_string_t const()):
                this->ToNumber().le__(rhs);
        }

        IBoolean& __stdcall ge__(IPrimitive const& rhs) const
        {
            return VT::String == rhs.type__() ?
                es_boolean<string_t>::create_instance(
                    internal_value_ >= rhs.operator const_string_t const()):
                this->ToNumber().ge__(rhs);
        }

// equality operators
        IBoolean& __stdcall eq__(IPrimitive const& rhs) const
        {
            switch (rhs.type__())
            {
                case VT::String:
                    return es_boolean<string_t>::create_instance(
                        internal_value_ == rhs.operator const_string_t const());
                case VT::Number:
                    return this->ToNumber().eq__(rhs);
                case VT::Boolean:
                    return eq__(rhs.ToNumber());
                default:
                    throw std::runtime_error("not implementd: NString::eq__");
            }
            return es_boolean<string_t>::create_instance(false);
        }

    // strict equality operators
        IBoolean& __stdcall strict_eq__(IPrimitive const& rhs) const
        {
            return es_boolean<string_t>::create_instance(
                VT::String == rhs.type__()
                && internal_value_ == rhs.operator const_string_t const());
        }

// native type conversion
        operator const_string_t const() const
        {
             return internal_value_;
        }

        operator string_t const() const
        {
             return internal_value_;
        }

        __stdcall operator bool() const
        {
            return 0 != internal_value_.length();
        }

        operator double() const
        {
            return base_services::es_lexical_cast<double>(internal_value_.c_str());
        }

        operator ecmascript::int32_t() const
        {
            return base_services::es_lexical_cast<ecmascript::int32_t>(
                internal_value_.c_str());
        }

        operator ecmascript::uint32_t() const
        {
            return base_services::es_lexical_cast<ecmascript::uint32_t>(
                internal_value_.c_str());
        }

        operator ecmascript::uint16_t() const
        {
            return base_services::es_lexical_cast<ecmascript::uint16_t>(
                internal_value_.c_str());
        }

//    protected:
        virtual ~es_string() throw()
        {
#if ES_TRACE_DESTRUCTION
            wprintf(
                L"string: '%s' is deleted.\n",
                operator const_string_t const().substr(0, 20).c_str());
#endif // ES_TRACE_DESTRUCTION
        }

    private:
        internal_value_t const internal_value_;
    };

} // namespace ecmascript
