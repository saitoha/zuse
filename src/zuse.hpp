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



#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "grammer.hpp"
#include "activescript.hpp"

#define NO_READLINE

#if defined(_MSC_VER)
#   define NO_READLINE
#endif

#if !defined(NO_READLINE)
#   include <readline/readline.h>
#   include <readline/history.h>
#endif // NO_READLINE


namespace ecmascript {

    //////////////////////////////////////////////////////////////////////////
    //
    //  @fn     es_run
    //  @brief  parse and evaluate input string
    //
    template <typename iteratorT>
    IPrimitive& es_run(
        IGlobal& global,
        iteratorT const begin,
        iteratorT const end
        ) throw()
    {
        typedef std::wstring string_t;
        try {
            return global.eval(*new es_string<string_t>(begin, end));
        } catch (std::logic_error const& e) {
            fprintf(stderr, "logic_error: %s\n", e.what());
        } catch (std::runtime_error const& e) {
            fprintf(stderr, "runtime_error: %s\n", e.what());
        } catch (std::exception const& e) {
            fprintf(stderr, "exception: %s\n", e.what());
        } catch (es_type_error<string_t> const& e) {
            fwprintf(stderr, L"type error: %s",
                     e.message().operator const_string_t const().c_str());
        } catch (es_native_error<string_t> const& e) {
            fwprintf(stderr, L"native error: %s",
                     e.message().operator const_string_t const().c_str());
        } catch (IPrimitive const& e) {
            fwprintf(stderr,
                     L"runtime error: an exception is thrown and not caught.\n"
                     L"the type of thrown object: %d\n%s\n",
                     int(e.type__()), e.operator const std::wstring().c_str());
        } catch (...)  {
            fprintf(stderr, "unknown error\n");
        }
        return es_undefined<string_t>::create_instance();
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  @fn     interactive_mode
    //  @brief  main loop of interactive shell mode
    //
    template <typename charT>
    es_result interactive_mode(IGlobal& global);

    template <>
    es_result interactive_mode<char>(IGlobal& global)
    {
        using namespace base_services;
        typedef std::basic_string<wchar_t> string_t;
        string_t input;
#ifdef NO_READLINE
        while (!std::cin.eof())
        {
            wprintf(L">>> ");
            static std::string line;
            getline(std::cin, line);
            if ("quit" == line)
                break;
            wchar_t *line_w = new wchar_t[line.size() + 1];
            mbstowcs(line_w, line.c_str(), line.size());
            line_w[ line.size() ] = L'\0';
            input.append(line_w).append(L"\n");
            delete[] line_w;
            if (!line.empty())
                continue;
            IPrimitive& result_value = es_run(
                global, &*input.begin(), &*input.rbegin() + 1);
            string_t ws = result_value.operator const_string_t const();
            char *mbstring = new char[ws.size() + 1];
            wcstombs(mbstring, ws.c_str(), ws.size());
            mbstring[ws.size()] = '\0';
            base_services::es_puts(mbstring);
            delete[] mbstring;
            input.clear();
        }
#elif !defined(_MSC_VER)
        char *line = NULL;
        while (true)
        {
            line = readline(">>> ");
            if (0 == line || 0 == strcmp(line, "quit"))
                break;
            printf("%s\n", line);
            add_history(line);
            free(line);
            size_t size = strlen(line);
            printf("%d\n", size);
            wchar_t *line_w = new wchar_t[size + 1];
            mbstate_t state = { 0 };
            mbsinit(&state);
            mbstowcs(line_w, line, size);
            line_w[ size ] = L'\0';
            wprintf(L"%s\n", line_w);
            input.append(line_w).append(L"\n");
            delete[] line_w;
            if (0 != size)
                continue;
            printf("---------\n");
            wprintf(L"%s\n", input.c_str());
            printf("---------\n");
            IPrimitive& result_value = es_run(
                global, &*input.begin(), &*input.rbegin() + 1);
            string_t ws = result_value.operator const_string_t const();
            char *mbstring = new char[ws.size() + 1];
            wcstombs(mbstring, ws.c_str(), ws.size());
            mbstring[ws.size()] = '\0';
            base_services::es_puts(mbstring);
            delete[] mbstring;
            input.clear();
        } 
        clear_history();
        free(line);
#else
        for (string_t input, line;
             (wprintf(L">>> "), getline(std::wcin, line), L"quit" != line);)
            if (input.append(line).append(L"\n"), line.empty())
                es_puts(es_run(global, &*input.begin(), &*input.rbegin() + 1)
                    .operator const_string_t const().c_str()), input.clear();
#endif // NO_READLINE
        return es_success;
    } // interactive_mode<char>(IPrimitive& global)

    //////////////////////////////////////////////////////////////////////////
    //
    //  @fn     execfile
    //  @brief  read and evaluate filestream
    //
    es_result execfile(IGlobal& global, char const *filename)
    {
        using namespace base_services;
        typedef std::wstring string_t;
        char fullpath[ES_MAX_PATH] = { 0 };
        if (!es_abspath(filename, fullpath, sizeof(fullpath) / sizeof(char)))
            return fprintf(stderr, "abspath failed: %s\n", filename), es_fail;
        char dirpath[ES_MAX_PATH] = { 0 };
        if (!es_dirname(fullpath, dirpath, sizeof(dirpath) / sizeof(char)))
            return fprintf(stderr, "dirpath failed: %s\n", fullpath), es_fail;
        if (chdir(dirpath) != 0)
            return fprintf(stderr, "chdir failed: %s\n", fullpath), es_fail;
        string_t input;
#if defined(__CYGWIN__) || defined(__MINGW32__)
        std::basic_ifstream<char> ifs(filename);
#else
        std::basic_ifstream<wchar_t> ifs(filename);
#endif // defined(__CYGWIN__) || defined(__MINGW32__)
        if(ifs.fail())
            return fprintf(stderr, "cannot open file: %s", filename), es_fail;
#if defined(__CYGWIN__) || defined(__MINGW32__)
        char line[1024];
        wchar_t line_w[1024];
        while (!ifs.eof())
        {
            ifs.getline(line, sizeof(line));
            mbstowcs(line_w, line, strlen(line));
            input += line_w;
            input += L'\n';
        }
#else
        wchar_t line[1024];
        while (!ifs.eof())
        {
            ifs.getline(line, sizeof(line));
            input += line;
            input += L'\n';
        }
#endif // defined(__CYGWIN__) || defined(__MINGW32__)
        ifs.close();
        wchar_t const* first = &*input.begin();
        wchar_t const* last = &*input.rbegin() + 1;
        if (0xfe == *first && 0xff == *(first + 1))
            first += 2;
        if ('#' == *first && '!' == *++first)
            while ('\n' != *++first)
                if (first >= last)
                    return es_fail;
        try {
            es_run(global, first, last);
        } catch (...) {
            return es_fail;
        }
        return es_success;
    } // execfile

} // namespace ecmascript

namespace {

    ///////////////////////////////////////////////////////////////////////
    //
    //  es_init_module
    // 
    inline void 
    es_init_module(
        ecmascript::IGlobal& global, 
        ecmascript::es_init_map& map
        ) throw()
    {
        for (ecmascript::es_init_map *it = &map; it->name; ++it)
            global.put__(it->name, *it->object);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  es_set_locale
    //
    inline bool es_set_locale() throw()
    {
#if defined(__CYGWIN__) || defined(__MINGW32__)

#elif defined(_MSC_VER)
        std::locale::global(std::locale("japanese"));
#elif defined(__APPLLE__)
        //    std::locale::global(std::locale("C");
#else
//    std::locale::global(std::locale(""));
//    std::locale::global(std::locale("C"));
//    std::locale::global(std::locale("C"));
//    std::locale::global(std::locale("ja_JP.UTF-8"));
#endif // defined(__CYGWIN__) || defined(__MINGW32__)
        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  es_initialize_modules
    //
    inline bool 
    es_initialize_modules(ecmascript::IGlobal& global) throw()
    {
        using namespace ecmascript;
        es_init_module(global, *es_init_debug());
        es_init_module(global, *es_init_sys());
//        es_init_module(global, *es_init_bytearray());
#ifdef _MSC_VER
        es_init_module(global, *es_init_activex());
#endif // _MSC_VER
        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  es_create_global
    //
    template <typename stringT>
    inline ecmascript::IGlobal * 
    es_create_global(ecmascript::IArray& array_of_argv) throw()
    {
        using namespace ecmascript;
        typedef es_global<stringT, es_parser> global_t;
        IGlobal& global = global_t::create_instance();
        bool bret = es_initialize_modules(global);
        ES_ASSERT(true == bret);
		if (!bret)
			return 0;  
        global.get__(L"__sys").put__(L"argv", array_of_argv);    
        return &global;
    }

} // anonymous namespace

//////////////////////////////////////////////////////////////////////////////
//
//  es_main
//
extern "C" int es_main(int argc, char **argv)
{
    using namespace ecmascript;
    if (true != es_set_locale())
        return -1;
    typedef std::wstring string_t;
    IArray& array_of_argv = *new es_array<string_t>;
    array_of_argv.addref__();
    array_of_argv.addref__();
    for (int i = 1; i < argc; ++i)
    {
        size_t size = mbstowcs(NULL, argv[i], 0) + 1;
        struct buffer
        {
            explicit buffer(size_t size): value_(new wchar_t[size]) {}
            ~buffer() { delete [] value_; }
            operator wchar_t * () const { return value_; }
            wchar_t *value_;
        } arg(size);
        if (size_t(-1) == mbstowcs(arg, argv[i], size))
            return -1;
        IString& path_string 
            = es_string<string_t>::create_instance(arg.operator wchar_t *());
        path_string.addref__();
        path_string.addref__();
        array_of_argv.push__(path_string);
    }
    IGlobal * p_global = es_create_global<string_t>(array_of_argv);
    if (0 == p_global)
        return -1;
    IGlobal& global = *p_global;
    if (argc > 1)
        return execfile(global, argv[1]), 0;
    struct stat fs;
    if (-1 == fstat(fileno(stdin), &fs))
        return -1;
#if 0

#ifndef S_IFIFO
  #define S_IFIFO _S_IFIFO
#endif // S_IFIFO

    if(fs.st_mode & S_IFREG || fs.st_mode & S_IFIFO)
    {
        ES_ASSERT(stdin != 0);
        size_t const buffer_size = 2 << 4;
        wchar_t buffer[buffer_size + 1];
        buffer[buffer_size] = 0;
        std::wstring source;
        while (std::wcin.read(buffer, buffer_size).gcount())
            source += buffer;
        wchar_t *first = &*source.begin();
        wchar_t *last = &*source.rbegin() + 1;
        if (0xfe == *first && 0xff == *(first + 1))
            first += 2;
        if ('#' == *first && '!' == *++first)
            while (*++first != '\n')
                if (first >= last)
                    return -1;
    }
    else
#endif // 0
    {
        base_services::es_print(
            "*" PACKAGE_STRING "\n"
            "Type \"copyright\", \"credits\" or \"license\" for more "
            "information.\n\n");
        global.put__(L"copyright",
            *new es_string<string_t>(
                L"Copyright (c) 2009 System Design Associates Inc.\n"));
        global.put__(L"credits",
            *new es_string<string_t>(
                L"credits\n"
                PACKAGE_NAME L" is an implementation of ECMA-262 (ECMAScript "
                L"language).\n"
                L"Thanks for Brendan Eich, creator of the JavaScript "
                L"programming language."));
        global.put__(L"license",
            *new es_string<string_t>(
                L"The MIT License\n\n"
                L"Copyright (c) 2009 System Design Associates Inc.\n\n"
                L"Permission is hereby granted, free of charge, to any "
                L"person obtaining a copy\n"
                L"of this software and associated documentation files (the"
                L" \"" PACKAGE_NAME L"\"), to deal\n"
                L"in the Software without restriction, including without "
                L"limitation the rights\n"
                L"to use, copy, modify, merge, publish, distribute, "
                L"sublicense, and/or sell\n"
                L"copies of the Software, and to permit persons to whom the "
                L"Software is\n"
                L"furnished to do so, subject to the following "
                L"conditions:\n\n"
                L"The above copyright notice and this permission notice "
                L"shall be included in\n"
                L"all copies or substantial portions of the Software.\n\n"
                L"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF "
                L"ANY KIND, EXPRESS OR\n"
                L"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF "
                L"MERCHANTABILITY,\n"
                L"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN "
                L"NO EVENT SHALL THE\n"
                L"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, "
                L"DAMAGES OR OTHER\n"
                L"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR "
                L"OTHERWISE, ARISING FROM,\n"
                L"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR "
                L"OTHER DEALINGS IN\n"
                L"THE SOFTWARE.\n"));
        interactive_mode<char>(global);
    }
    return 0;
}

#ifdef _MSC_VER

#include <comcat.h>
#include <activscp.h>

namespace {

    HINSTANCE hInst;

    /* ---- object implementation ---- */

    ///////////////////////////////////////////////////////////////////////////
    //
    // @struct INTERFACE_ENTRY
    //
    struct INTERFACE_ENTRY
    {
        IID const * p_iid;
        IUnknown *interface_pointer;
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // @class IUnknownImpl
    //
    template <class T>
    class IUnknownImpl 
    : T
    {
    public:
        IUnknownImpl()
        : m_Count(0)
        {
        }

        STDMETHODIMP_(ULONG) AddRef(VOID) throw()
	    {
	        return ++m_Count;
	    }

        STDMETHODIMP_(ULONG) Release(VOID) throw()
	    {
	        return --m_Count;
	    };

	    STDMETHOD(QueryInterface)(REFIID iid, void**ppv) throw()
	    {
	        for (INTERFACE_ENTRY const* p_entry = T::GetInterfaceMap();
	            NULL != p_entry->p_iid; ++p_entry)
	            if (IsEqualGUID(iid, *p_entry->p_iid))
	                return *ppv = p_entry->interface_pointer, S_OK;
	        return E_NOINTERFACE;
	    }

    private:
        ULONG m_Count;

    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // @class IActiveScriptImpl
    //
    template <class T>
    class __declspec(novtable) IActiveScriptImpl
    : public IActiveScript
    {
        public:
            IActiveScriptImpl() throw()
            : m_p_global_scope(NULL)
            , m_p_scriptsite_object(NULL)
            {
            }

            STDMETHOD(SetScriptSite)(IActiveScriptSite *pass)
            {
                using namespace ecmascript;
                ES_TRACE ("IActiveScript::SetScriptSite");
                m_ActiveScriptSite = pass;
                m_p_scriptsite_object
                    = new es_scriptsite_object(m_ActiveScriptSite);
                if (NULL == m_p_scriptsite_object)
                    return E_OUTOFMEMORY;
                m_script_state = SCRIPTSTATE_INITIALIZED;
                IPrimitive& window 
                    = m_p_scriptsite_object->get__(L"window").get_value__();
                m_p_global_scope
                    = &es_global<std::wstring, es_parser>::create_instance(window);
                es_initialize_modules(*m_p_global_scope);
                return S_OK;
            }

            STDMETHOD(GetScriptSite)(REFIID riid, void **ppvObject)
            {
                ES_TRACE ("IActiveScript::GetScriptSite");
                return m_ActiveScriptSite->QueryInterface(riid, ppvObject);
            }

            STDMETHOD(SetScriptState)(SCRIPTSTATE ss)
            {
                ES_TRACE ("IActiveScript::SetScriptState");
                if (m_script_state = ss)
                    return S_FALSE;
		        if (SCRIPTSTATE_UNINITIALIZED != ss)
			        m_ActiveScriptSite->OnStateChange(ss);
                m_script_state = ss;
                return S_OK;
            }

            STDMETHOD(GetScriptState)(SCRIPTSTATE *pssState)
            {
                ES_TRACE ("IActiveScript::GetScriptState");
                return *pssState = m_script_state, S_OK;
            }

            STDMETHOD(Close)(void)
            {
                ES_TRACE ("IActiveScript::Close");
                return S_OK;
            };

            STDMETHOD(AddNamedItem)(LPCOLESTR pstrName, DWORD dwFlags)
            {
                ES_TRACE ("IActiveScript::AddNamedItem");
                if (!m_p_global_scope)
                    return E_POINTER;
                if (!m_p_scriptsite_object)
                    return E_POINTER;
                using namespace ecmascript;
                IPrimitive& scriptsite
                    = m_p_scriptsite_object->get__(pstrName).get_value__();
                m_p_global_scope->put__(pstrName, scriptsite);
                return S_OK;
            }

            STDMETHOD(AddTypeLib)(REFGUID, DWORD, DWORD, DWORD)
            {
                ES_TRACE ("IActiveScript::AddTypeLib");
                ES_ASSERT (!"IActiveScriptbase_services::AddTypeLib");
                return E_NOTIMPL;
            }

            STDMETHOD(GetScriptDispatch)(LPCOLESTR pstrItemName, IDispatch **ppdisp)
            {
                ES_TRACE ("IActiveScript::GetCurrentScriptThreadID");
                if (!m_p_scriptsite_object)
                    return E_POINTER;
                return m_p_scriptsite_object->get_member(pstrItemName, ppdisp);
            }

            STDMETHOD(GetCurrentScriptThreadID)(SCRIPTTHREADID *)
            {
                ES_TRACE ("IActiveScript::GetCurrentScriptThreadID");
                ES_ASSERT (!"IActiveScriptbase_services::GetCurrentScriptThreadID");
                return E_NOTIMPL;
            }

            STDMETHOD(GetScriptThreadID)(DWORD, SCRIPTTHREADID *)
            {
                ES_TRACE ("IActiveScript::GetScriptThreadID");
                ES_ASSERT (!"IActiveScriptbase_services::GetScriptThreadID");
                return E_NOTIMPL;
            }

            STDMETHOD(GetScriptThreadState)(SCRIPTTHREADID, SCRIPTTHREADSTATE *)
            {
                ES_TRACE ("IActiveScript::GetScriptThreadState");
                ES_ASSERT (!"IActiveScriptbase_services::GetScriptThreadState");
                return E_NOTIMPL;
            }

            STDMETHOD(InterruptScriptThread)(SCRIPTTHREADID, const EXCEPINFO *, DWORD)
            {
                ES_TRACE ("IActiveScript::InterruptScriptThread");
                ES_ASSERT (!"IActiveScriptbase_services::InterruptScriptThread");
                return E_NOTIMPL;
            }

            STDMETHOD(Clone)(IActiveScript **)
            {
                ES_TRACE ("IActiveScript::Clone");
                ES_ASSERT (!"IActiveScriptbase_services::Clone");
                return E_NOTIMPL;
            }

        public:
            ecmascript::es_scriptsite_object *m_p_scriptsite_object;
            SCRIPTSTATE m_script_state;
            _com_ptr_t<_com_IIID<IActiveScriptSite, &IID_IActiveScriptSite> >
                m_ActiveScriptSite;
            ecmascript::IGlobal *m_p_global_scope;

    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // @class IActiveScriptParseImpl
    //
    template <class T>
    class __declspec(novtable) IActiveScriptParseImpl
    : public IActiveScriptParse
    {
    public:
        IActiveScriptParseImpl()
        : m_p_activation_object(NULL)
        {
        }

        STDMETHOD(AddScriptlet)(
            LPCOLESTR pstrDefaultName,
            LPCOLESTR pstrCode,
            LPCOLESTR pstrItemName,
            LPCOLESTR pstrSubItemName,
            LPCOLESTR pstrEventName,
            LPCOLESTR pstrDelimiter,
            DWORD dwSourceContextCookie,
            ULONG ulStartingLineNumber,
            DWORD dwFlags,
            BSTR *pbstrName,
            EXCEPINFO *pexcepinfo)
        {
            HRESULT hr = S_OK;
            T* pthis = static_cast<T*>(this);
            if (0 == pthis->m_p_scriptsite_object)
                return E_POINTER;
            if (0 == pstrItemName || 0 == pstrEventName)
                return E_INVALIDARG;
            std::wstring code = pstrItemName;
            try {
                if (pstrSubItemName)
                    code = code + L"." + pstrSubItemName;
                code = code + L"." + pstrEventName + L"=function(){" + pstrCode + L"}";
            } catch (...) {
                ES_ASSERT(0);
                return E_UNEXPECTED;
            }
            pthis->m_p_global_scope->eval(
                *new ecmascript::es_string<std::wstring>(
                    &*code.begin(), &*code.rbegin() + 1));
            return S_OK;
        }

        STDMETHOD(ParseScriptText)(
            LPCOLESTR pstrCode,
            LPCOLESTR pstrItemName,
            IUnknown *punkContext,
            LPCOLESTR pstrDelimiter,
            DWORD dwSourceContextCookie,
            ULONG ulStartingLineNumber,
            DWORD dwFlags,
            VARIANT *pvarResult,
            EXCEPINFO *pexcepinfo)
        {
            HRESULT hr = S_OK;
            T* pthis = static_cast<T*>(this);
            pthis->m_ActiveScriptSite->OnStateChange(
                pthis->m_script_state = SCRIPTSTATE_STARTED);
            OLECHAR *it_start = const_cast<OLECHAR *>(pstrCode);
            OLECHAR *it_end = it_start + SysStringByteLen(it_start) / sizeof(OLECHAR);
            if (FAILED(hr = TrimComment(&it_start, &it_end)))
                return hr;
            pthis->m_p_global_scope->eval(
                *new ecmascript::es_string<std::wstring>(it_start, it_end));
            pthis->m_ActiveScriptSite->OnLeaveScript();
		    pthis->m_ActiveScriptSite->OnStateChange(
		        pthis->m_script_state = SCRIPTSTATE_INITIALIZED);
            return S_OK;
        }

        STDMETHOD(InitNew)(void)
        {
            T *const pthis = static_cast<T*>(this);
            if (pthis->m_ActiveScriptSite)
                pthis->m_ActiveScriptSite->OnStateChange(
                    pthis->m_script_state = SCRIPTSTATE_INITIALIZED);
            return S_OK;
        }

    private:
        HRESULT TrimComment(
            /* [in, out] */ OLECHAR ** pp_start,
            /* [in, out] */ OLECHAR ** pp_end)
        {
            ES_ASSERT (NULL != pp_start && NULL != pp_end && pp_start < pp_end);
            OLECHAR *it = NULL;
            for (it = *pp_start; iswspace(*it) && it != *pp_end; ++it);
            if (*it == L'<' && *++it == L'!' && *++it == L'-' && *++it == L'-')
                *pp_start = ++it;
            for (it = *pp_end - 1; iswspace(*it) && it != *pp_start; --it);
            if (*it == L'>' && *--it == L'-' && *--it == L'-')
                *pp_end = it;
            return S_OK;
        }

    private:
        ecmascript::IPrimitive *m_p_activation_object;
    };

    ////////////////////////////////////////////////////////////////////////////
    //
    // @class IActiveScriptGarbageCollectorImpl
    //
    template <class T>
    class __declspec(novtable) IActiveScriptGarbageCollectorImpl
    : public IActiveScriptGarbageCollector
    {
    // IActiveScriptGarbageCollector
            HRESULT STDMETHODCALLTYPE CollectGarbage(
                SCRIPTGCTYPE scriptgctype)
            {
                ecmascript::es_machine<ecmascript::IPrimitive>::get_gc().collect();
                ::MessageBoxW(0,L"CollectGarbage",0,0);
                return S_OK;
            };
    };

    ////////////////////////////////////////////////////////////////////////////
    //
    // @class InterfaceMapImpl
    //
    template <class T>
    class __declspec(novtable) InterfaceMapImpl
    {
    public:
        INTERFACE_ENTRY const * GetInterfaceMap()
        {
            T * pthis = static_cast<T *>(this);
            static INTERFACE_ENTRY interface_map[] = {
                { &__uuidof(IUnknown)
                    , static_cast<IActiveScript *>(pthis) }
                , { &__uuidof(IActiveScript)
                    , static_cast<IActiveScript *>(pthis) }
                , { &__uuidof(IActiveScriptParse)
                    , static_cast<IActiveScriptParse *>(pthis) }
                , { &__uuidof(IActiveScriptGarbageCollector)
                    , static_cast<IActiveScriptGarbageCollector *>(pthis) }
                , { NULL, NULL }
            };
            return interface_map;
        }

    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // @class CScriptObject
    //
    class __declspec(uuid("{B7BCEFC5-FD47-4986-B418-A6686F9760CC}")) 
    CScriptObject
        : public IActiveScriptImpl<CScriptObject>
        , public IActiveScriptParseImpl<CScriptObject>
        , public IActiveScriptGarbageCollectorImpl<CScriptObject>
        , public InterfaceMapImpl<CScriptObject>
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // @class CComFactory
    //
    template <typename T>
    class CComFactory : public IClassFactory
    {
    public:
        CComFactory()
        : m_count(1)
        {
        }

        ~CComFactory() {}

    //IUnknown
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv)
        {
            if(IID_IUnknown != riid && IID_IClassFactory != riid)
                return E_NOINTERFACE;
            AddRef();
            *ppv = this;
            return S_OK;
        }

        STDMETHODIMP_(ULONG) AddRef(VOID)
        {
            return ++m_count;
        }

        STDMETHODIMP_(ULONG) Release(VOID)
        {
            if (--m_count == 0)
                delete this;
            return m_count;
        }

    //IClassFactory
        STDMETHODIMP CreateInstance(
            IUnknown* pUnkOuter, REFIID riid, LPVOID *ppv)
        {
            if (pUnkOuter)
                return CLASS_E_NOAGGREGATION;
            T* p = new T;
            if(NULL == p)
                return E_OUTOFMEMORY;
            HRESULT hr = p->QueryInterface(riid, ppv);
            if (FAILED(hr))
                return hr;
            p->Release();
            return hr;
        }

        STDMETHODIMP LockServer(BOOL bLock)
        {
            return S_OK;
        }

    private:
        LONG m_count;
    };

    const char *g_RegTable[][3] = {
      {"CLSID\\{B7BCEFC5-FD47-4986-B418-A6686F9760CC}", 0, PACKAGE_NAME " script language"},
      {"CLSID\\{B7BCEFC5-FD47-4986-B418-A6686F9760CC}\\InProcServer32", 0, (const char*)-1},
      {"CLSID\\{B7BCEFC5-FD47-4986-B418-A6686F9760CC}\\InProcServer32", "ThreadingModel", "Both"},
      {"CLSID\\{B7BCEFC5-FD47-4986-B418-A6686F9760CC}\\OLEScript", 0, ""},
      {"CLSID\\{B7BCEFC5-FD47-4986-B418-A6686F9760CC}\\ProgID", 0, PACKAGE_NAME},
      {PACKAGE_NAME, 0, PACKAGE_NAME " script language"},
      {PACKAGE_NAME "\\CLSID", 0, "{B7BCEFC5-FD47-4986-B418-A6686F9760CC}"},
      {PACKAGE_NAME "\\OLEScript", 0, ""},
    };

    static const GUID CATID_ActiveScript =
    { 0xf0b7a1a1, 0x9847, 0x11cf, 
        { 0x8f, 0x20, 0x00, 0x80, 0x5f, 0x2c, 0xd0, 0x64 } };
    static const GUID CATID_ActiveScriptParse =
    { 0xf0b7a1a2, 0x9847, 0x11cf, 
        { 0x8f, 0x20, 0x00, 0x80, 0x5f, 0x2c, 0xd0, 0x64 } };
    static const CATID ** GetCategoryMap() throw() {
        static const CATID * pMap[] = {
            &CATID_ActiveScript
            , &CATID_ActiveScriptParse
            , NULL
        };
        return pMap;
    }

} // anonymous namespace

/* ---- standard COM dll entry points ---- */

//////////////////////////////////////////////////////////////////////////////
//
//  @struct     es_com_module
//
struct es_com_module
{
    es_com_module() throw()
    : count_(0)
    {
    }

    int increment() throw()
    {
        ES_ASSERT((std::numeric_limits<long>::max)() > count_);
        if ((std::numeric_limits<long>::max)() <= count_)
            return -1;
        InterlockedIncrement(&count_);
        return 0;
    }

    int decrement() throw()
    {
        ES_ASSERT(count_ > 0);
        if (count_ <= 0)
            return -1;
        InterlockedDecrement(&count_);
        return 0;
    }

    int can_unload_now() const throw()
    {
        ES_ASSERT(count_ >= 0);
        return count_ == 0 ? 0 : -1;
    }

private:
    long count_;

} g_module;

//////////////////////////////////////////////////////////////////////////////
//
//  @fn     DllCanUnloadNow
//
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID )
{
    hInst = hInstance;
    if (dwReason == DLL_PROCESS_ATTACH)
        return 0 == g_module.increment();
    else if (dwReason == DLL_PROCESS_DETACH)
        return 0 == g_module.decrement();
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
//  @fn     DllCanUnloadNow
//
STDAPI DllCanUnloadNow(void)
{
    return g_module.can_unload_now() == 0 ? S_OK : S_FALSE;
}

//////////////////////////////////////////////////////////////////////////////
//
//  @fn     DllGetClassObject
//
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    CComFactory<IUnknownImpl<CScriptObject> > *pCF
        = new CComFactory<IUnknownImpl<CScriptObject> >();
    return pCF != NULL ? pCF->QueryInterface(riid, ppv): S_OK;
}

//////////////////////////////////////////////////////////////////////////////
//
//  @fn     DllRegisterServer
//
STDAPI DllRegisterServer(void)
{
    _com_ptr_t<_com_IIID<ICatRegister, &IID_ICatRegister> > pCatRegister;
    HRESULT hr = CoCreateInstance(
        CLSID_StdComponentCategoriesMgr, NULL,
	    CLSCTX_INPROC_SERVER,
	    __uuidof(ICatRegister),
	    reinterpret_cast<void**>(&pCatRegister) );
    if(FAILED(hr))
	    return E_FAIL;
    for (CATID const** pEntry = GetCategoryMap(); *pEntry; ++pEntry)
        if (FAILED(hr = pCatRegister->RegisterClassImplCategories(
            __uuidof(CScriptObject), 1, const_cast<CATID *>(*pEntry))))
            return hr;
    char szFileName[MAX_PATH] = { 0 };
    GetModuleFileNameA(hInst, szFileName, MAX_PATH);
    for (int i = 0;
        SUCCEEDED(hr) && i < sizeof(g_RegTable) / sizeof(*g_RegTable); ++i)
    {
        const char *pszKeyName   = g_RegTable[i][0];
        const char *pszValueName = g_RegTable[i][1];
        const char *pszValue     = g_RegTable[i][2];
        if(pszValue == (const char*)-1)
            pszValue = szFileName;
        HKEY hkey;
        if (RegCreateKeyA(HKEY_CLASSES_ROOT, pszKeyName, &hkey)
            != ERROR_SUCCESS)
            return DllUnregisterServer(), SELFREG_E_CLASS;
        RegSetValueExA(hkey, pszValueName, 0, REG_SZ, (const BYTE*)pszValue,
            (strlen(pszValue) + 1));
        RegCloseKey(hkey);
    }
    return hr;
}

//////////////////////////////////////////////////////////////////////////////
//
//  @fn     DllUnregisterServer
//
STDAPI DllUnregisterServer(void)
{
    _com_ptr_t<_com_IIID<ICatRegister, &IID_ICatRegister> > pCatRegister;
    HRESULT hr = CoCreateInstance(
        CLSID_StdComponentCategoriesMgr, NULL,
	    CLSCTX_INPROC_SERVER, __uuidof(ICatRegister),
	    reinterpret_cast<void**>(&pCatRegister) );
    if (FAILED(hr) || !pCatRegister)
	    return E_FAIL;
    for (CATID const** pEntry = GetCategoryMap(); *pEntry; ++pEntry)
        if (FAILED(hr = pCatRegister->UnRegisterClassImplCategories(
            __uuidof(CScriptObject), 1, const_cast<CATID *>(*pEntry))))
            return hr;
    for (int i = sizeof(g_RegTable)/sizeof(*g_RegTable) - 1; i >= 0; --i)
        if(RegDeleteKeyA(HKEY_CLASSES_ROOT, g_RegTable[i][0]) != ERROR_SUCCESS)
            hr = S_FALSE;
    return hr;
}

#endif // _MSC_VER

