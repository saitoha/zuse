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

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class IByteArray
    //  @brief
    //
    struct IByteArray : IObject
    {
        typedef IObject base_t;

        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 3 };

        // dispatch interface
        virtual INumber& __stdcall push(IPrimitive& object) = 0;
        virtual IUndefined& __stdcall execute() = 0;
        virtual INumber& __stdcall length() const = 0;

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { L"push",     offset + 0, Stdcall| Method,   1 },
                { L"execute",  offset + 1, Stdcall| Method,   0 },
                { L"length",   offset + 2, Stdcall| Property, 0 },
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_byte_array
    //  @brief
    //
    struct es_byte_array
    : public base_classes::es_collectable_object<
        base_classes::es_object_impl<
            IByteArray, std::basic_string<wchar_t> > >
    {
        typedef std::basic_string<wchar_t> string_t;
        typedef base_classes::es_object_impl<IByteArray, string_t> object_t;
        typedef es_boolean<string_t> boolean_t;
        typedef std::vector<unsigned char> value_t;
// constructor
        es_byte_array()
        {
        }

        const_string_t const class__() const throw()
        {
            return L"ByteArray";
        }

// IObject implementation
    public:
        IPrimitive& __stdcall get_by_value__(IPrimitive const& key)
        {
            return VT::Number == key.type__() ?
                *new es_number<string_t>(double(
                    vec_[size_t(key.ToNumber().operator double())]))
                : object_t::get__(key.operator const_string_t const());
        }
// typeof operator
        IString& __stdcall typeof__() const
        {
            return *new es_string<string_t>(L"bytearray");
        }
// native type conversion
        operator const_string_t const() const
        {
            std::basic_stringstream<wchar_t> stream;
            stream << std::hex << std::setfill(L'0');
            value_t::const_iterator it = vec_.begin();
            if (it == vec_.end())
                return stream.str();
            stream << std::setw(2) << *it;
            while (++ it != vec_.end())
                stream << L' ' << std::setw(2) << *it;
            return stream.str();
        }

        operator string_t const() const
        {
            std::basic_stringstream<wchar_t> stream;
            stream << std::hex << std::setfill(L'0');
            value_t::const_iterator it = vec_.begin();
            if (it == vec_.end())
                return stream.str();
            stream << std::setw(2) << *it;
            while (++ it != vec_.end())
                stream << L' ' << std::setw(2) << *it;
            return stream.str();
        }
// IByteArray
        INumber& __stdcall push(IPrimitive& object)
        {
            const_string_t const type_string
                = object.typeof__().operator const_string_t const();
            if (type_string == L"array" || type_string == L"bytearray")
                for (object.reset__();;)
                {
                    IPrimitive& next_object = object.next__();
                    if (VT::Undefined == next_object.type__())
                        return length();
                    ecmascript::uint16_t uint16_value
                        = object.get_by_value__(next_object)
                            .get_value__().operator ecmascript::uint16_t();
                    if ((2 << 8) - 1 > uint16_value)
                        throw *new es_type_error<string_t>(L"Type Error: bytearray::push");
                    vec_.push_back(ecmascript::uint8_t(uint16_value));
                }
            ecmascript::uint16_t uint16_value = object.operator ecmascript::uint16_t();
            if (uint16_value > 0xff)
                throw *new es_type_error<string_t>(L"Type Error: bytearray::push");
            vec_.push_back(ecmascript::uint8_t(uint16_value));
            return length();
        }

        IUndefined& __stdcall execute()
        {
            ecmascript::uint8_t *data = new ecmascript::uint8_t[1 << 16];
#ifdef _MSC_VER
            ecmascript::uint16_t * data_size;
            ecmascript::uint8_t * p = &*vec_.begin();
            __asm
	        {
                mov ecx, data_size
                mov edx, data
                call p
            }
#endif // _MSC_VER
            delete[] data;
            return es_undefined<string_t>::create_instance();
        }

        INumber& __stdcall length() const
        {
            return *new es_number<string_t>(double(vec_.size()));
        }
// destructor
    protected:
        virtual ~es_byte_array() throw()
        {
#if ES_TRACE_DESTRUCTION
            wprintf(
                L"%s: '%s' is deleted.\n",
                typeof__().operator const_string_t const(),
                operator const_string_t const().substr(0, 20).c_str());
#endif // ES_TRACE_DESTRUCTION
        }
    private:
        value_t vec_;
    };

    } // anonymous namespace

    extern "C" es_init_map * es_init_bytearray()
    {
        static es_init_map table[] =
        {
            { L"__ar", new es_byte_array },
            { 0, 0 }
        };
        return table;
    }


} // namespace ecmascript
