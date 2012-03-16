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

    namespace { // file scope declaration

        static char const *const char_map
            = "0123456789abcdefghijklmnopqrstuvwxyz";

        //////////////////////////////////////////////////////////////////////
        //
        // @fn make_integral_part
        //
        template <typename charT>
        inline charT const *const make_integral_part(
            ecmascript::uint32_t part,
            ecmascript::uint32_t radix,
            register charT * __restrict& it,
            bool negate
            ) throw()
        {
            do
                *--it = char_map[part % radix];
            while (0 != (part /= radix));
            return negate ? &(*--it = '-'): it;
        }

        //////////////////////////////////////////////////////////////////////
        //
        // @fn make_fractional_part
        //
        template <typename charT>
        inline charT const *const make_fractional_part(
            double part,
            double radix,
            register charT * __restrict& it,
            charT const* const end
            ) throw()
        {
            if (0 == part)
                return it;
            *it = '.';
            do
                *++it = char_map[size_t(part = fmod(part * radix, radix))];
            while (0 != part && end != it);
            return ++it;
        }

        //////////////////////////////////////////////////////////////////////
        //
        // @fn to_string_impl
        //
        template <typename stringT>
        inline IString const& to_string_impl(
            double value,
            ecmascript::uint32_t radix,
            bool negate
            ) throw()
        {
            typedef typename stringT::value_type char_type;
            static char_type buffer[2 << 5];
            char_type * __restrict first = buffer + 21;
            char_type * __restrict last = first;
            char_type const * const end = buffer + sizeof(buffer);
            ecmascript::uint32_t integral_part = ecmascript::uint32_t(value);
            return *new es_string<stringT>(
                make_integral_part(integral_part, radix, first, negate),
                make_fractional_part(value - integral_part, radix, last, end)
            );
        }

        //////////////////////////////////////////////////////////////////////
        //
        // @fn es_double_to_float_string
        //
        template <typename stringT>
        inline IString const& es_double_to_float_string(
            double value,
            ecmascript::uint32_t radix
            ) throw()
        {
            ES_ASSERT((2 <= radix && radix <= 9)
                || (11 <= radix && radix <= 36));
            if (__builtin_expect(base_services::es_isnan(value), false))
                return *new es_string<stringT>(L"NaN");
            if (__builtin_expect(base_services::es_isinf(value), false))
                return *new es_string<stringT>(
                    0 > value ? L"Infinity": L"-Infinity");
            bool negate = false;
            if (0 > value)
                negate = true, value = - value;
            return to_string_impl<stringT>(value, radix, negate);
        }

        //////////////////////////////////////////////////////////////////////
        //
        // @fn write_to_buffer
        //
        template <typename stringT>
        inline IString const& write_to_buffer(
            ecmascript::int64_t n,
            ecmascript::integer_t digits,
            typename stringT::value_type * buffer,
            bool negate
            ) throw()
        {
            typedef typename stringT::value_type char_type;
            char_type * __restrict last = buffer + sizeof(buffer);
            char_type * __restrict first = last;
            do {
                *--first = char_map[n % 10];
                if (last - digits == first)
                    *--first = '.';
            } while (0 != (n /= 10));
            return *new es_string<stringT>(
                negate ? &(*--first = '-'): first, buffer + sizeof(buffer));
        }

        //////////////////////////////////////////////////////////////////////
        //
        // @fn to_fixed_impl
        //
        template <typename stringT>
        inline IString const& to_fixed_impl(
            double value,
            ecmascript::integer_t digits,
            bool negate
            )
        {
            typedef typename stringT::value_type char_type;
            static char_type buffer[2 << 5];
            if (__builtin_expect(1.0e+21 > value, true))
                return write_to_buffer<stringT>(
                    int64_t(base_services::es_floor(
                        value * pow(10., digits) + .5)),
                    digits,
                    buffer,
                    negate);
            if (__builtin_expect(
                base_services::es_lexical_cast(value, buffer), true))
                return *new es_string<stringT>(buffer);
            throw *new es_native_error<stringT>(L"to_fixed_impl");
        }

        //////////////////////////////////////////////////////////////////////
        //
        // @fn es_double_to_fixed_string
        //
        template <typename stringT>
        inline IString const& es_double_to_fixed_string(
            double value,
            ecmascript::integer_t digits
            )
        {
            if (__builtin_expect(base_services::es_isnan(value), false))
                return *new es_string<stringT>(L"NaN");
            if (__builtin_expect(base_services::es_isinf(value), false))
                return *new es_string<stringT>(
                    0 > value ? L"Infinity": L"-Infinity");
            if (0 > digits || 20 < digits)
                throw *new es_range_error<stringT>(L"es_number::toFixed");
            bool negate = false;
            if (0 > value)
                negate = true, value = - value;
            return to_fixed_impl<stringT>(value, digits, negate);
        }

        //////////////////////////////////////////////////////////////////////
        //
        // @fn to_fixed_impl
        //
        template <typename stringT>
        inline IString const& to_exponential_impl(
            double value,
            ecmascript::integer_t digits,
            bool negate
            )
        {
            typedef typename stringT::value_type char_type;
            static char_type buffer[2 << 5];
            if (__builtin_expect(1.0e+21 > value, true))
                return write_to_buffer<stringT>(
                    int64_t(base_services::es_floor(
                        value * pow(10., digits) + .5)),
                    digits,
                    buffer,
                    negate);
            if (__builtin_expect(
                base_services::es_lexical_cast(value, buffer), true))
                return *new es_string<stringT>(buffer);
            throw *new es_native_error<stringT>(L"to_fixed_impl");
        }

        //////////////////////////////////////////////////////////////////////
        //
        // @fn es_double_to_exponential_string
        //
        template <typename stringT>
        inline IString const& es_double_to_exponential_string(
            double value,
            ecmascript::integer_t digits
            ) throw()
        {
            if (__builtin_expect(base_services::es_isnan(value), false))
                return *new es_string<stringT>(L"NaN");
            if (__builtin_expect(base_services::es_isinf(value), false))
                return *new es_string<stringT>(
                    0 > value ? L"Infinity": L"-Infinity");
            bool negate = false;
            if (0 > value)
                negate = true, value = - value;
            return to_exponential_impl<stringT>(value, digits, negate);
        }

    } // anonymous namespace

    //////////////////////////////////////////////////////////////////////////
    //
    // Number Object
    //
    template <typename stringT>
    struct es_number
    : public base_classes::es_collectable_object<
        base_classes::es_primitive_impl<INumber, stringT> >
    {
        typedef stringT string_t;
        typedef es_boolean<string_t> boolean_t;
        typedef double value_type;
        
        static IFunction& static_get_constructor() throw()
        {
            static struct constructor
            : public base_classes::es_function_impl<INumberConstructor, stringT>
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
                    return arguments.length__() > 0 ?
                        arguments[0].ToNumber():
                        *new es_number<string_t>(+0.);
                }

                IPrimitive& __stdcall construct__(IPrimitive& arguments)
                {
                    return *new es_delegate_object<string_t>(
                        *new es_number<string_t>(arguments));
                }

                IPrimitive& __stdcall prototype()
                {
                    return es_number<string_t>::static_prototype();
                }

                INumber const& __stdcall MAX_VALUE() const
                {
                    return *new es_number<string_t>(
                        (std::numeric_limits<double>::max)());
                }

                INumber const& __stdcall MIN_VALUE() const
                {
                    return *new es_number<string_t>(
                        (std::numeric_limits<double>::min)());
                }

                INumber const& __stdcall NaN() const
                {
                    return *new es_number<string_t>(
                        (std::numeric_limits<double>::quiet_NaN)());
                }

                INumber const& __stdcall NEGATIVE_INFINITY() const
                {
                    return *new es_number<string_t>(
                        - (std::numeric_limits<double>::infinity)());
                }

                INumber const& __stdcall POSITIVE_INFINITY() const
                {
                    return *new es_number<string_t>(
                        (std::numeric_limits<double>::infinity)());
                }

            } constructor_;
            return constructor_;
        }

        explicit es_number(double given_value) throw()
        : internal_value_(given_value)
        {
        }

        explicit es_number(ecmascript::integer_t given_value) throw()
        : internal_value_(given_value)
        {
        }

#if !defined(__APPLE__)
        explicit es_number(std::wint_t given_value) throw()
        : internal_value_(given_value)
        {
        }
#endif

        explicit es_number(ecmascript::int32_t given_value) throw()
        : internal_value_(given_value)
        {
        }

        explicit es_number(ecmascript::uint32_t given_value) throw()
        : internal_value_(given_value)
        {
        }

        explicit es_number(IPrimitive& arguments) throw()
        : internal_value_(
            arguments.length__() > 0 ? arguments[0].operator double() : 0.)
        {
        }

        static INumber& __stdcall create_instance(double d)
        {
            return *new es_number<string_t>(d);
        }

        ~es_number() throw()
        {
#if ES_TRACE_DESTRUCTION
            wprintf(
                L"number: '%s' is deleted.\n",
                operator const_string_t const().substr(0, 20).c_str());
#endif // ES_TRACE_DESTRUCTION
        }

        const_string_t const class__() const throw()
        {
            return L"Number";
        }

// INumber override
        IReference const& __stdcall constructor() const
        {
            static es_reference<string_t>
                constructor_(static_get_constructor());
            return constructor_;
        }

        IString const& __stdcall toString(IPrimitive& radix) const
        {
            if (__builtin_expect(VT::Undefined == radix.type__(), true))
                return *new es_string<string_t>(
                    operator const_string_t const());
            ecmascript::uint32_t radix_value = radix.operator uint32_t();
            if (__builtin_expect(10 == radix_value, false))
                return *new es_string<string_t>(
                    operator const_string_t const());
            if (36 < radix_value || 2 > radix_value)
                throw *new es_native_error<string_t>(L"es_umber::toString");
            return es_double_to_float_string<string_t>(
                operator double(), radix_value);
        }

        IString const& __stdcall toLocaleString() const
        {
            // TODO:
            return *new es_string<string_t>(
                operator const_string_t const());
        }

        INumber const& __stdcall valueOf() const
        {
            if (VT::Number != (*this).type__())
                throw *new es_type_error<string_t>(L"es_number::valueOf");
            return *new es_number<string_t>(operator double());
        }

        IString const& __stdcall toFixed(IPrimitive& fractionDigits) const
        {
            return es_double_to_fixed_string<string_t>(
                operator double(),
                fractionDigits.operator ecmascript::integer_t());
        }

        IString const& __stdcall toExponential(IPrimitive& fractionDigits) const
        {
            double value = operator double();
            ecmascript::integer_t digits
                = fractionDigits.operator ecmascript::integer_t();
            if (__builtin_expect(base_services::es_isnan(value), false))
                return *new es_string<stringT>(L"NaN");
            if (__builtin_expect(base_services::es_isinf(value), false))
                return *new es_string<stringT>(
                    0 > value ? L"Infinity": L"-Infinity");
            if (0 > digits || 20 < digits)
                throw *new es_range_error<string_t>(L"es_number::toFixed");
            wchar_t buffer[2 << 8];
            ecmascript::int32_t e_f = ecmascript::int32_t(log10(value));
            double n = base_services::es_floor(value * pow(10., e_f) + .5);
            ecmascript::int32_t f = ecmascript::int32_t(log10(n));
            ecmascript::int32_t e = f + e_f;
            wchar_t *it = buffer;

            int64_t b = int64_t(pow(10.0, int(log10(n))));
            *it = char_map[int64_t(n) / b];
            *++it = '.';
            n = double(int64_t(n) % b);
            while (0 != n)
            {
                int64_t b = int64_t(pow(10.0, int(log10(n))));
                *++it = char_map[int64_t(n) / b];
                n = double(int64_t(n) % b);
            }

            *++it = 'e';
            if (0 == e)
                *++it = '+', *++it = '0';
            else if (0 > e)
                *++it = '-', e = - e;
            else
                *++it = '+';

            while (0 != e)
            {
                int32_t b = int32_t(pow(10.0, int(log10(double(e)))));
                *++it = char_map[e / b];
                e = e % b;
            }

            return *new es_string<string_t>(buffer, ++it);
        }

        IString const& __stdcall toPrecision(IPrimitive& precision) const
        {
            if (VT::Undefined == precision.type__())
                return (*this).ToString();
            return (*this).ToString();
        }

// postfix operators
        IPrimitive& __stdcall postfix_inc__()
        {
            throw *new es_native_error<string_t>(
                L"not implemented: es_number::postfix_inc__");
        }

        IPrimitive& __stdcall postfix_dec__()
        {
            throw *new es_native_error<string_t>(
                L"not implemented: es_number::postfix_dec__");
        }
// unary operations
    // typeof operator
        IString& __stdcall typeof__() const
        {
            return *new es_string<stringT>(L"number");
        }

        IPrimitive& __stdcall prefix_inc__()
        {
            throw *new es_native_error<string_t>(
                L"not implemented: es_number::prefix_inc__");
        }

        IPrimitive& __stdcall prefix_dec__()
        {
            throw *new es_native_error<string_t>(
                L"not implemented: es_number::prefix_dec__");
        }

        INumber& __stdcall unary_plus__() const
        {
            return *new es_number<string_t>(+ internal_value_);
        }

        INumber& __stdcall unary_minus__() const
        {
            return *new es_number<string_t>(- internal_value_);
        }

// multiplicative operators
        INumber& __stdcall mul__(IPrimitive const& rhs) const
        {
            return *new es_number<string_t>(
                internal_value_ * rhs.operator double());
        }

        INumber& __stdcall div__(IPrimitive const& rhs) const
        {
            double d_lhs = internal_value_;
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
            double d_lhs = internal_value_;
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
                d_lhs - std::floor(d_lhs / d_rhs) * d_rhs);
        }

// additive operators
        IPrimitive& __stdcall binary_plus__(IPrimitive const& rhs) const
        {
            return VT::String == rhs.type__() ?
                static_cast<IPrimitive&>(*new es_string<string_t>(
                    operator const_string_t const()
                        + rhs.operator const_string_t const())):
                *new es_number<string_t>(
                    internal_value_ + rhs.operator double());
        }

        INumber& __stdcall binary_minus__(IPrimitive const& rhs) const
        {
            return *new es_number<string_t>(
                internal_value_ - rhs.operator double());
        }

// relational operators
        IBoolean& __stdcall lt__(IPrimitive const& rhs) const
        {
            return boolean_t::create_instance(
                internal_value_ < rhs.operator double());
        }

        IBoolean& __stdcall gt__(IPrimitive const& rhs) const
        {
            return boolean_t::create_instance(
                internal_value_ > rhs.operator double());
        }

        IBoolean& __stdcall le__(IPrimitive const& rhs) const
        {
            return boolean_t::create_instance(
                internal_value_ <= rhs.operator double());
        }

        IBoolean& __stdcall ge__(IPrimitive const& rhs) const
        {
            return boolean_t::create_instance(
                internal_value_ >= rhs.operator double());
        }

// equality operators
    private:
        IBoolean& __stdcall eq__(IPrimitive const& rhs) const
        {
            switch (rhs.type__())
            {
                case VT::Number:
                case VT::String:
                case VT::Boolean:
                    return es_boolean<string_t>::create_instance(
                        internal_value_ == rhs.operator double());
                case VT::Object:
                    throw std::runtime_error(
                        "not implemented: es_number::eq__");
                default:
                    return es_boolean<string_t>::create_instance(false);
            }
        }

        IBoolean& __stdcall strict_eq__(IPrimitive const& rhs) const
        {
            return es_boolean<string_t>::create_instance(
                VT::Number == rhs.type__()
                    && internal_value_ == rhs.operator double());
        }

// native type conversion
        __stdcall operator bool() const
        {
            return internal_value_ != 0
                && !base_services::es_isnan(internal_value_);
        }

        operator const_string_t const() const
        {
            wchar_t buffer[1 << 8];
            if (base_services::es_lexical_cast(internal_value_, buffer))
                return buffer;
            throw std::runtime_error("const_string_t const() const");
        }

        operator string_t const() const
        {
            wchar_t buffer[1 << 8];
            if (base_services::es_lexical_cast(internal_value_, buffer))
                return buffer;
            throw std::runtime_error("operator string_t const() const");
        }

        operator double() const
        {
            return internal_value_;
        }

        operator ecmascript::int32_t() const
        {
            /* overflow check */
            if (internal_value_ > INT_MAX
                || internal_value_ < INT_MIN)
                throw std::runtime_error("overflow/underflow:");
            return ecmascript::int32_t(internal_value_);
        }

        operator ecmascript::uint32_t() const
        {
            /* overflow check */
            // boost::numeric_cast
            if (internal_value_ > UINT_MAX)
                throw std::runtime_error("overflow:");
            return ecmascript::uint32_t(internal_value_);
        }

        operator ecmascript::uint16_t() const
        {
            // boost::numeric_cast
            if (internal_value_ > USHRT_MAX)
                throw std::runtime_error("overflow:");
            return ecmascript::uint16_t(internal_value_);
        }

    private:
        value_type const internal_value_;
    };
} // namespace ecmascript

