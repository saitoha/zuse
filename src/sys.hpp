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



namespace ecmascript { namespace {

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  ISys
    //  @brief
    //
    struct ISys : IObject
    {
        typedef IObject base_t;
        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 5 };

        virtual IString& __stdcall abspath(IPrimitive&) const = 0;
        virtual IString& __stdcall getcwd() const = 0;
        virtual IPrimitive& __stdcall argv() = 0;
        virtual IUndefined& __stdcall verbose(IPrimitive const&) = 0;
        virtual IPrimitive& __stdcall load(IPrimitive const&) const = 0;

        static es_attributes const& get_typelib__() throw()
        {
            static es_attributes const funcdata[] =
            {
                { L"abspath", offset + 0, Stdcall| Method,   1 },
                { L"getcwd",  offset + 1, Stdcall| Method,   0 },
                { L"argv",    offset + 2, Stdcall| Property, 0 },
                { L"verbose", offset + 3, Stdcall| Method,   1 },
                { L"load",    offset + 4, Stdcall| Method,   1 },
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_sys
    //  @brief
    //
    struct es_sys
    : public base_classes::es_collectable_object<
        base_classes::es_object_impl<ISys, const_string_t> >
    {
        typedef const_string_t string_t;
        typedef string_t::value_type char_type;

        es_sys() throw()
        {
        }

        ~es_sys() throw()
        {
        }

        const_string_t const class__() const throw()
        {
            return L"Sys";
        }

        IString& __stdcall abspath(IPrimitive& arg) const
        {
            using namespace base_services;
            char_type path[ES_MAX_PATH * 2];
            const_string_t const given_path
                = arg.operator const_string_t const();
            if (!es_abspath(
                given_path.c_str(), path, sizeof(path) / sizeof(path[0])))
                return *new es_native_error<string_t>(L"abspath");
            return *new es_string<string_t>(path);
        }

        IString& __stdcall getcwd() const
        {
            using namespace base_services;
            char_type path[ES_MAX_PATH * 2];
            if (!es_getcwd(path, sizeof(path) / sizeof(path[0])))
                return *new es_native_error<string_t>(L"getcwd");
            return *new es_string<string_t>(path);
        }

        IPrimitive& __stdcall argv()
        {
            return argv_;
        }

        IUndefined& __stdcall verbose(IPrimitive const& given_argument)
        {
            es_set_verbose(given_argument.operator bool());
            return es_undefined<string_t>::create_instance();
        }

        IPrimitive& __stdcall load(IPrimitive const& given_argument) const throw()
        {
            using namespace base_services;
            union {
                IPrimitive * (*function)();
                void *p;
            } box;
            box.p = es_dynamic_load(
                given_argument.operator const_string_t const().c_str());
            if (0 == box.function)
                return *new es_native_error<string_t>(L"cannot open");	
            IPrimitive *p_constructor = box.function();
            ES_ASSERT(0 != p_constructor);
            return *p_constructor;
        }

    private:
        es_reference<string_t> argv_;
    };

} } // ecmascript::anonymous namespace


namespace ecmascript {

    extern "C" es_init_map * es_init_sys()
    {
        static es_init_map table[] =
        {
            { L"__sys", new es_sys },
            { 0, 0 }
        };
        return table;
    }

} // namespace ecmascript
