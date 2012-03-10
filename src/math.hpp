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
// math object
//
namespace ecmascript {

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class IMath
    //  @brief
    //
    struct IMath
    : IObject
    {
        typedef IObject base_t;

        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 6 };

        virtual INumber& __stdcall E() const = 0;
        virtual INumber& __stdcall LN10() const = 0;
        virtual INumber& __stdcall LN2() const = 0;
        virtual INumber& __stdcall LOG2E() const = 0;
        virtual INumber& __stdcall LOG10E() const = 0;
        virtual INumber& __stdcall PI() const = 0;
        virtual INumber& __stdcall SQRT1_2() const = 0;
        virtual INumber& __stdcall SQRT2() const = 0;
        virtual INumber& __stdcall abs(IPrimitive&) const = 0;
        virtual INumber& __stdcall acos(IPrimitive&) const = 0;
        virtual INumber& __stdcall asin(IPrimitive&) const = 0;
        virtual INumber& __stdcall atan(IPrimitive&) const = 0;
        virtual INumber& __stdcall atan2(IPrimitive&, IPrimitive&) const = 0;
        virtual INumber& __stdcall ceil(IPrimitive&) const = 0;
        virtual INumber& __stdcall cos(IPrimitive& value) const = 0;
        virtual INumber& __stdcall exp(IPrimitive&) const = 0;
        virtual INumber& __stdcall floor(IPrimitive&) const = 0;
        virtual INumber& __stdcall log(IPrimitive&) const = 0;
        virtual INumber& __stdcall max_(IPrimitive&) const = 0;
        virtual INumber& __stdcall min_(IPrimitive&) const = 0;
        virtual INumber& __stdcall pow(IPrimitive&, IPrimitive&) const = 0;
        virtual INumber& __stdcall random() const = 0;
        virtual INumber& __stdcall round(IPrimitive&) const = 0;
        virtual INumber& __stdcall sin(IPrimitive& value) const = 0;
        virtual INumber& __stdcall sqrt(IPrimitive&) const = 0;
        virtual INumber& __stdcall tan(IPrimitive&) const = 0;

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { L"E",       offset + 0,  Stdcall| Property, 0 },
                { L"LN10",    offset + 1,  Stdcall| Property, 0 },
                { L"LN2",     offset + 2,  Stdcall| Property, 0 },
                { L"LOG2E",   offset + 3,  Stdcall| Property, 0 },
                { L"LOG10E",  offset + 4,  Stdcall| Property, 0 },
                { L"PI",      offset + 5,  Stdcall| Property, 0 },
                { L"SQRT1_2", offset + 6,  Stdcall| Property, 0 },
                { L"SQRT2",   offset + 7,  Stdcall| Property, 0 },
                { L"abs",     offset + 8,  Stdcall| Method,   1 },
                { L"acos",    offset + 9,  Stdcall| Method,   1 },
                { L"asin",    offset + 10, Stdcall| Method,   1 },
                { L"atan",    offset + 11, Stdcall| Method,   1 },
                { L"atan2",   offset + 12, Stdcall| Method,   2 },
                { L"ceil",    offset + 13, Stdcall| Method,   1 },
                { L"cos",     offset + 14, Stdcall| Method,   1 },
                { L"exp",     offset + 15, Stdcall| Method,   1 },
                { L"floor",   offset + 16, Stdcall| Method,   1 },
                { L"log",     offset + 17, Stdcall| Method,   1 },
                { L"max",     offset + 18, Cdecl  | Method,   2 },
                { L"min",     offset + 19, Cdecl  | Method,   2 },
                { L"pow",     offset + 20, Stdcall| Method,   2 },
                { L"random",  offset + 21, Stdcall| Method,   0 },
                { L"round",   offset + 22, Stdcall| Method,   1 },
                { L"sin",     offset + 23, Stdcall| Method,   1 },
                { L"sqrt",    offset + 24, Stdcall| Method,   1 },
                { L"tan",     offset + 25, Stdcall| Method,   1 },
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_math
    //  @brief
    //
    template <typename stringT>
    struct es_math
    : public base_classes::es_object_impl<IMath, stringT>
    {
    private:
        typedef stringT string_t;
        typedef typename string_t::value_type char_t;
        typedef es_math<string_t> self_t;

    public:
        static IMath& create_instance() throw()
        {
            static self_t instance;
            instance.addref__();
            return instance;
        }

        es_math() throw()
        {
        }

        ~es_math() throw()
        {
        }

        const_string_t const class__() const throw()
        {
            return L"Math";
        }

// math
        INumber& __stdcall E() const
        {
            return *new es_number<string_t>(2.7182818284590452354);
        }

        INumber& __stdcall LN10() const
        {
            return *new es_number<string_t>(2.302585092994046);
        }

        INumber& __stdcall LN2() const
        {
            return *new es_number<string_t>(0.6931471805599453);
        }

        INumber& __stdcall LOG2E() const
        {
            return *new es_number<string_t>(1.4426950408889634);
        }

        INumber& __stdcall LOG10E() const
        {
            return *new es_number<string_t>(0.4342944819032518 );
        }

        INumber& __stdcall PI() const
        {
            return *new es_number<string_t>(
                3.1415926535897932384626433832795);
        }

        INumber& __stdcall SQRT1_2() const
        {
            return *new es_number<string_t>(0.7071067811865476);
        }

        INumber& __stdcall SQRT2() const
        {
            return *new es_number<string_t>(1.4142135623730951);
        }

        INumber& __stdcall abs(IPrimitive& value) const
        {
            return *new es_number<string_t>(
                base_services::es_abs(value.operator double()));
        }

        INumber& __stdcall acos(IPrimitive& value) const
        {
            return *new es_number<string_t>(
                base_services::es_acos(value.operator double()));
        }

        INumber& __stdcall asin(IPrimitive& value) const
        {
            return *new es_number<string_t>(
                base_services::es_asin(value.operator double()));
        }

        INumber& __stdcall atan(IPrimitive& value) const
        {
            return *new es_number<string_t>(
                base_services::es_atan(value.operator double()));
        }

        INumber& __stdcall atan2(IPrimitive& x, IPrimitive& y) const
        {
            return *new es_number<string_t>(
                (base_services::es_atan2)(
                    x.operator double(),
                    y.operator double()));
        }

        INumber& __stdcall ceil(IPrimitive& value) const
        {
            return *new es_number<string_t>(
                base_services::es_ceil(value.operator double()));
        }

        INumber& __stdcall cos(IPrimitive& value) const
        {
            return *new es_number<string_t>(
                base_services::es_cos(value.operator double()));
        }

        INumber& __stdcall exp(IPrimitive& value) const
        {
            return *new es_number<string_t>(
                base_services::es_exp(value.operator double()));
        }

        INumber& __stdcall floor(IPrimitive& value) const
        {
            return *new es_number<string_t>(
                base_services::es_floor(value.operator double()));
        }

        INumber& __stdcall log(IPrimitive& value) const
        {
            return *new es_number<string_t>(
                base_services::es_log(value.operator double()));
        }

        INumber& __stdcall max_(IPrimitive& arguments) const
        {
            if (0 == arguments.length__())
                return es_number<string_t>::create_instance(
                    -(std::numeric_limits<double>::infinity)());
            IPrimitive& argument1 = arguments[0];
            IPrimitive& argument2 = arguments[1];
            return *new es_number<string_t>(
                (base_services::es_max)(
                    argument1.operator double(),
                    argument2.operator double()));
        }

        INumber& __stdcall min_(IPrimitive& arguments) const
        {
            if (0 == arguments.length__())
                return es_number<string_t>::create_instance(
                    (std::numeric_limits<double>::infinity)());
            IPrimitive& argument1 = arguments[0];
            IPrimitive& argument2 = arguments[1];
            return *new es_number<string_t>(
                (base_services::es_min)(
                    argument1.operator double(),
                    argument2.operator double()));
        }

        INumber& __stdcall pow(IPrimitive& x, IPrimitive& y) const
        {
            return *new es_number<string_t>(
                base_services::es_pow(x.operator double(), y.operator double()));
        }

        INumber& __stdcall random() const
        {
            return *new es_number<string_t>(
                double(base_services::es_rand()) / RAND_MAX);
        }

        INumber& __stdcall round(IPrimitive& value) const
        {
            return *new es_number<string_t>(
                base_services::es_floor(value.operator double() + .5));
        }

        INumber& __stdcall sin(IPrimitive& value) const
        {
            return *new es_number<string_t>(
                base_services::es_sin(value.operator double()));
        }

        INumber& __stdcall sqrt(IPrimitive& value) const
        {
            return *new es_number<string_t>(
                base_services::es_sqrt(value.operator double()));
        }

        INumber& __stdcall tan(IPrimitive& value) const
        {
            return *new es_number<string_t>(
                base_services::es_tan(value.operator double()));
        }
    };

} // namespace ecmascript
