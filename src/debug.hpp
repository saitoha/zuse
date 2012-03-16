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
    //  @class  IDebugT
    //  @brief  Debug object interface
    //
    struct IDebug : IObject
    {
        typedef IObject base_t;
        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 10 };

        virtual IUndefined&  __stdcall alert(IPrimitive const&) const = 0;
        virtual IUndefined& __stdcall beep() const = 0;
        virtual IUndefined& __stdcall print(IPrimitive const&) const = 0;
        virtual IString& __stdcall readtextfile(IPrimitive const&) const = 0;
        virtual IUndefined& __stdcall test(IPrimitive const&, IPrimitive const&) const = 0;
        virtual IString& __stdcall inspect(IPrimitive&) const = 0;
        virtual IUndefined& __stdcall red() const = 0;
        virtual IUndefined& __stdcall green() const = 0;
        virtual IUndefined& __stdcall blue() const = 0;
        virtual IUndefined& __stdcall white() const = 0;

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { L"alert",         offset + 0, Stdcall| Method,   1 },
                { L"beep",          offset + 1, Stdcall| Method,   0 },
                { L"print",         offset + 2, Stdcall| Method,   1 },
                { L"readtextfile",  offset + 3, Stdcall| Method,   1 },
                { L"test",          offset + 4, Stdcall| Method,   2 },
                { L"inspect",       offset + 5, Stdcall| Method,   1 },
                { L"red",           offset + 6, Stdcall| Method,   0 },
                { L"green",         offset + 7, Stdcall| Method,   0 },
                { L"blue",          offset + 8, Stdcall| Method,   0 },
                { L"white",         offset + 9, Stdcall| Method,   0 },
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_debug
    //  @brief IDebug implementaion
    //
    struct es_debug
		: public base_classes::es_collectable_object<
            base_classes::es_object_impl<IDebug, std::wstring> >
    {
        typedef std::wstring string_t;

        es_debug() throw()
        {
        }

        ~es_debug() throw()
        {
        }

        const_string_t const class__() const throw()
        {
            return L"Debug";
        }

        IUndefined& __stdcall beep() const
        {
#ifdef _MSC_VER
            MessageBeep(0);
#else
            puts("debug.beep() called.");
#endif
            return es_undefined<string_t>::create_instance();
        }

        IUndefined& __stdcall alert(IPrimitive const& v) const
        {
            const_string_t const& str = v.operator const_string_t const();
#ifdef _MSC_VER
            MessageBoxW(NULL, str.c_str(), NULL, MB_OK);
#elif defined(__CYGWIN__) || defined(__MINGW32__)
            char *output_str = new char[str.length() + 1];
            wcstombs(output_str, str.c_str(), str.length());
            output_str[ str.length() ] = '\0';
            base_services::es_puts(output_str);
            delete[] output_str;
#else
            wprintf(L"debug.alert() called: %s\n", str.c_str());
#endif
            return es_undefined<string_t>::create_instance();
        }

        IUndefined& __stdcall print(IPrimitive const& v) const
        {
            const_string_t const& str = v.operator const_string_t const();
#if defined(__CYGWIN__) || defined(__MINGW32__)
            char *output_str = new char[str.length() + 1];
            wcstombs(output_str, str.c_str(), str.length());
            output_str[ str.length() ] = '\0';
            puts(output_str);
            delete[] output_str;
#else
            char *output_str = new char[str.length() + 1];
            wcstombs(output_str, str.c_str(), str.length());
            output_str[ str.length() ] = '\0';
            puts(output_str);
            delete[] output_str;
//            base_services::es_puts(str.c_str());
#endif
            return es_undefined<string_t>::create_instance();
        }

        IString& __stdcall readtextfile(IPrimitive const& path) const
        {
            std::basic_ifstream<wchar_t> ifs;
            const_string_t const& wide_path
                = path.operator const_string_t const();
#if _MSC_VER > 1310
            ifs.open( wide_path.c_str() );
#else

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif // _MSC_VER

            static char narrow_path[FILENAME_MAX + 1] = {0};
            wcstombs(
                narrow_path,
                wide_path.c_str(),
                sizeof(narrow_path) / sizeof(char));
            ifs.open( narrow_path );

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // _MSC_VER
            std::basic_string<wchar_t> input;
            std::basic_string<wchar_t> line;
            while ( ifs && !ifs.eof() )
            {
                std::getline( ifs, line );
                input += line + L'\n';
            }
            return *new es_string<string_t>(&*input.begin(), &*input.end());
        }

        IUndefined& __stdcall test(IPrimitive const& v1, IPrimitive const& v2) const
        {
#ifdef _MSC_VER
            MessageBeep(0);
            this->print(v1);
            this->print(v2);
#else
            base_services::es_puts("debug.test() called.\n");
#endif
            return es_undefined<string_t>::create_instance();
        }

        IString& __stdcall inspect(IPrimitive& given_object) const
        {
            given_object.addref__();
            char buffer[256];
            wchar_t wbuffer[256];
            sprintf(buffer, "ref count: %u\n", (unsigned int)(given_object.release__()));
            mbstowcs(wbuffer, buffer, sizeof(buffer));
            return *new es_string<string_t>(wbuffer);
        }

        IUndefined& __stdcall red() const
        {
            es_con_red();
            return es_undefined<string_t>::create_instance();
        }

        IUndefined& __stdcall green() const
        {
            es_con_green();
            return es_undefined<string_t>::create_instance();
        }

        IUndefined& __stdcall blue() const
        {
            es_con_blue();
            return es_undefined<string_t>::create_instance();
        }

        IUndefined& __stdcall white() const
        {
            es_con_white();
            return es_undefined<string_t>::create_instance();
        }

        operator const_string_t const() const
        {
            return L"debug object";
        }

        operator string_t const() const
        {
            return L"debug object";
        }

    private:
        const_string_t runtimepath_;
    };

    } // anonymous namespace

    extern "C" es_init_map * es_init_debug()
    {
        static es_init_map table[] =
        {
            { L"__debug", new es_debug },
            { 0, 0 }
        };
        return table;
    }

} // namespace ecmascript
