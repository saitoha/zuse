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

#ifdef _MSC_VER

#include <comdef.h>
#include <activscp.h>
#include <tchar.h>
#include <shlguid.h>
#include <dispex.h>

namespace ecmascript {

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_scriptsite_object
    //
    struct es_scriptsite_object
    : public base_classes::es_collectable_object<
        base_classes::es_object_impl<IPrimitive, std::wstring> >
    {
        typedef std::basic_string<wchar_t> string_t;
        typedef base_classes::es_object_impl<IPrimitive, string_t> object_t;

        explicit es_scriptsite_object(IActiveScriptSite *pActiveScriptSite)
        : m_pActiveScriptSite(pActiveScriptSite)
        {
            ES_ASSERT(0 != m_pActiveScriptSite);
            if (NULL == pActiveScriptSite)
                throw std::runtime_error(
                    "es_scriptsite_object::es_scriptsite_object");
            m_pActiveScriptSite->AddRef();
        }

        virtual ~es_scriptsite_object() throw()
        {
            ES_ASSERT(0 != m_pActiveScriptSite);
            m_pActiveScriptSite->Release();
        }

        const_string_t const class__() const throw()
        {
            return L"Script";
        }

        IPrimitive& __stdcall get__(const_string_t const& key)
        {
            IDispatch *pdisp = NULL;
            HRESULT hr = get_member(key, &pdisp);
            if (FAILED(hr) || NULL == pdisp)
                return ecmascript::es_undefined<string_t>::create_instance();
            return *new es_activex_object(pdisp);
        }

        HRESULT get_member(const_string_t const& key, IDispatch **ppdisp)
        {
            LPOLESTR rgszNames = const_cast<LPOLESTR>(key.c_str());
            DISPID rgDispid = 0;
            HRESULT hr = S_OK;
            IUnknown *pUnkown = NULL;
            ES_ASSERT(0 != m_pActiveScriptSite);
            hr = m_pActiveScriptSite->GetItemInfo(
                rgszNames, SCRIPTINFO_IUNKNOWN, &pUnkown, NULL);
            if (FAILED(S_OK))
                return hr;
            return pUnkown->QueryInterface(
                IID_IDispatch, reinterpret_cast<void **>(ppdisp));
        }

    private:
        IActiveScriptSite *m_pActiveScriptSite;
    };

    extern "C" es_init_map * es_init_activex()
    {
        static es_init_map table[] =
        {
            { L"ActiveXObject", &es_activex_object::static_get_constructor() },
            { 0, 0 }
        };
        return table;
    }

} // namespace ecmascript

#endif // _MSC_VER

