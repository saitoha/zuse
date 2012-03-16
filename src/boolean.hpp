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
    struct es_boolean
    : public base_classes::es_immutable_object<
        base_classes::es_primitive_impl<IBoolean, stringT> >
    {
        typedef stringT string_t;
        typedef bool value_type;
        static IFunction& static_get_constructor()
        {
            static struct constructor
            : public base_classes::es_function_impl<
                IBooleanConstructor, stringT>
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
                        arguments[0].ToBoolean():
                        es_boolean<string_t>::create_instance(false);
                }

                IPrimitive& __stdcall construct__(IPrimitive& arguments)
                {
                    return *new es_delegate_object<string_t>(
                        es_boolean<string_t>::create_instance(
                            arguments.length__() > 0
                            && arguments[0].operator double()));
                }

                IPrimitive& __stdcall prototype()
                {
                    return es_boolean<string_t>::static_prototype();
                }

            } constructor_;
            return constructor_;
        }

        static IBoolean& __stdcall create_instance(bool value)
        {
            static IBoolean& const_true = *new es_boolean<string_t>(true);
            static IBoolean& const_false = *new es_boolean<string_t>(false);
            return value ? const_true: const_false;
        };

        void sweep__(int cookie) throw()
        {
        }

        const_string_t const class__() const
        {
            return L"Boolean";
        }

// dispatch interface
        IReference const& __stdcall constructor() const
        {
            static es_reference<string_t>
                constructor_(static_get_constructor());
            return constructor_;
        }

        IString const& __stdcall toString() const
        {
            return this->ToString();
        }

        IBoolean const& __stdcall valueOf() const
        {
            return this->ToBoolean();
        }

// typeof operator
        IString& __stdcall typeof__() const
        {
            return *new es_string<stringT>(L"boolean");
        }

// equality operators
        IBoolean& __stdcall eq__(IPrimitive const& rhs) const
        {
            return create_instance(
                VT::Boolean == rhs.type__() ?
                     value_ == rhs.operator bool():
                     this->ToNumber().eq__(rhs));
        }

        IBoolean& __stdcall strict_eq__(IPrimitive const& rhs) const
        {
            return create_instance(
                VT::Boolean == rhs.type__() && value_ == rhs.operator bool());
        }

// native type conversion
        __stdcall operator bool() const
        {
            return value_;
        }

        operator double() const
        {
            return value_ ? 1. : 0.;
        }

        operator const_string_t const() const
        {
            return value_ ? L"true": L"false";
        }

        operator string_t const() const
        {
            return value_ ? L"true": L"false";
        }

        operator ecmascript::int32_t() const
        {
            return value_ ? 1 : 0;
        }

        operator ecmascript::uint32_t() const
        {
            return value_ ? 1 : 0;
        }

        operator ecmascript::uint16_t() const
        {
            return value_ ? 1 : 0;
        }

    private:
        explicit es_boolean(bool initial_value)
        : value_(initial_value)
        {
        }

        ~es_boolean() throw()
        {
        }

        value_type value_;
    };
} // namespace ecmascript
