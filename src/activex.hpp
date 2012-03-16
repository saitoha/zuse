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
j

#ifdef _MSC_VER

#include <windows.h>
#include <comdef.h>
#include <activscp.h>
#include <tchar.h>

namespace ecmascript {

    template <typename stringT>
    struct es_com_callable_wrapper;

    template <typename stringT>
    struct es_runtime_callable_wrapper;

    template <int I>
    struct es_tag
    {
        enum { tag = I };
    };

    template <typename stringT>
    inline variant_t const
    es_variant_from_primitive(
        IPrimitive& /* value */,
        es_tag<VT::Undefined> const& tag
        )
    {
        return variant_t();
    }

    template <typename stringT>
    inline variant_t const
    es_variant_from_primitive(
        IPrimitive& /* value */,
        es_tag<VT::Null> const& tag
        )
    {
        VARIANT v;
        v.vt = VT_NULL;
        return v;
    }

    template <typename stringT>
    inline variant_t const
    es_variant_from_primitive(
        IPrimitive& value,
        es_tag<VT::Boolean> const& tag
        )
    {
        return value.operator bool();
    }

    template <typename stringT>
    inline variant_t const
    es_variant_from_primitive(
        IPrimitive& value,
        es_tag<VT::String> const& tag
        )
    {
        // TODO: performance tuning
        return value.operator stringT const().c_str();
    }

    template <typename stringT>
    inline variant_t const
    es_variant_from_primitive(
        IPrimitive& value,
        es_tag<VT::Number> const& tag
        )
    {
        // TODO: performance tuning
        return value.operator double();
    }

    template <typename stringT>
    inline variant_t const
    es_variant_from_primitive(
        IPrimitive& value,
        es_tag<VT::Primitive> const& tag
        )
    {
        return reinterpret_cast<
            es_runtime_callable_wrapper<stringT> const&
            >(value).get_dispatch();
    }

    template <typename stringT>
    inline variant_t const
    es_variant_from_primitive(
        IPrimitive& value,
        es_tag<VT::Object> const& tag
        )
    {
        // TODO: exception handling
        IDispatch *pdisp = new es_com_callable_wrapper<stringT>(value);
        pdisp->AddRef();
        return _variant_t(pdisp);
    }

    template <typename stringT>
    VARIANT es_variant_from_primitive(IPrimitive& v)
    {
        switch (v.type__())
        {
        case VT::Undefined:
            return es_variant_from_primitive<stringT>(v, es_tag<VT::Undefined>());
        case VT::Null:
            return es_variant_from_primitive<stringT>(v, es_tag<VT::Null>());
        case VT::Boolean:
            return es_variant_from_primitive<stringT>(v, es_tag<VT::Boolean>());
        case VT::String:
            return es_variant_from_primitive<stringT>(v, es_tag<VT::String>());
        case VT::Number:
            return es_variant_from_primitive<stringT>(v, es_tag<VT::Number>());
        case VT::Primitive:
            return es_variant_from_primitive<stringT>(v, es_tag<VT::Primitive>());
        case VT::Object:
            return es_variant_from_primitive<stringT>(v, es_tag<VT::Object>());
        }
        ES_ASSERT(!"es_variant_from_primitive");
        throw std::logic_error("es_variant_from_primitive");
    }

    template <typename stringT>
    inline IPrimitive&
    es_primitive_from_variant(
        VARIANT const& v,
        es_tag<VT_DISPATCH> const& /* tag */
        )
    {
        return *new es_activex_object(v.vt & VT_BYREF ? *v.ppdispVal: v.pdispVal);
    }

    template <typename stringT>
    inline IPrimitive&
    es_primitive_from_variant(
        VARIANT const& /* v */,
        es_tag<VT_EMPTY> const& /* tag */
        )
    {
        return es_undefined<stringT>::create_instance();
    }

    template <typename stringT>
    inline IPrimitive&
    es_primitive_from_variant(
        VARIANT const& /* v */,
        es_tag<VT_NULL> const& /* tag */
        )
    {
        return es_null<stringT>::create_instance();
    }

    template <typename stringT>
    inline IPrimitive&
    es_primitive_from_variant(
        VARIANT const& v,
        es_tag<VT_BOOL> const& /* tag */
        )
    {
        return es_boolean<stringT>::create_instance(
            (v.vt & VT_BYREF ? *v.pboolVal: v.boolVal) == VARIANT_TRUE);
    }

    template <typename stringT>
    inline IPrimitive&
    es_primitive_from_variant(
        VARIANT const& v,
        es_tag<VT_BSTR> const& /* tag */
        )
    {
        BSTR bstr = v.vt & VT_BYREF ? *v.pbstrVal: v.bstrVal;
        if (!bstr)
            return *new es_string<stringT>(L"");
        UINT length = ::SysStringByteLen(bstr);
        IPrimitive& result
            = *new es_string<stringT>(const_string_t(bstr, bstr + length));
        ::SysFreeString(bstr);
        return result;
    }

    template <typename stringT>
    inline IPrimitive&
    es_primitive_from_variant(
        VARIANT const& v,
        es_tag<VT_I4> const& /* tag */
        )
    {
        return *new es_number<stringT>(v.vt & VT_BYREF ? *v.plVal: v.lVal);
    }

    template <typename stringT>
    inline IPrimitive&
    es_primitive_from_variant(
        VARIANT const& v,
        es_tag<VT_UI4> const& /* tag */
        )
    {
        return *new es_number<stringT>(v.vt & VT_BYREF ? *v.pulVal: v.ulVal);
    }

    template <typename stringT>
    inline IPrimitive&
    es_primitive_from_variant(
        VARIANT const& v,
        es_tag<VT_R8> const& /* tag */
        )
    {
        return *new es_number<stringT>(v.vt & VT_BYREF ? *v.pdblVal: v.dblVal);
    }

    template <typename stringT>
    IPrimitive& es_primitive_from_variant(VARIANT const& v)
    {
        switch (v.vt & VT_TYPEMASK)
        {
        case VT_DISPATCH:
            return es_primitive_from_variant<stringT>(v, es_tag<VT_DISPATCH>());
        case VT_EMPTY:
            return es_primitive_from_variant<stringT>(v, es_tag<VT_EMPTY>());
        case VT_NULL:
            return es_primitive_from_variant<stringT>(v, es_tag<VT_NULL>());
        case VT_BOOL:
            return es_primitive_from_variant<stringT>(v, es_tag<VT_BOOL>());
        case VT_BSTR:
            return es_primitive_from_variant<stringT>(v, es_tag<VT_BSTR>());
        case VT_I4:
            return es_primitive_from_variant<stringT>(v, es_tag<VT_I4>());
        case VT_UI4:
            return es_primitive_from_variant<stringT>(v, es_tag<VT_UI4>());
        case VT_R8:
            return es_primitive_from_variant<stringT>(v, es_tag<VT_R8>());
        }
        ES_ASSERT(!"es_primitive_from_variant");
        throw std::logic_error("es_primitive_from_variant");
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  es_lock
    //  @brief
    //
    struct es_lock
    {
        explicit es_lock(CRITICAL_SECTION &cs) throw()
        : pcs_(&cs)
        {
            EnterCriticalSection(pcs_);
        }

        ~es_lock() throw()
        {
            LeaveCriticalSection(pcs_);
        }

        CRITICAL_SECTION *pcs_;

    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_critical_section
    //
    struct es_critical_section
    : public CRITICAL_SECTION
    {
        es_critical_section() throw()
        {
            InitializeCriticalSection(this);
        }

        ~es_critical_section() throw()
        {
            DeleteCriticalSection(this);
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  IActiveXObject
    //  @brief  ActiveX object interface
    //
    struct IActiveXObject : IObject
    {
        typedef IObject base_t;
        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 0 };

        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_com_callable_wrapper
    //
    template <typename stringT>
    struct es_com_callable_wrapper
    : IDispatch
    {
        explicit es_com_callable_wrapper(IPrimitive& primitive) throw()
        : m_count(0)
        , primitive_(primitive)
        {
            es_machine<IPrimitive>::get_gc().push_object(primitive);
        }

        ~es_com_callable_wrapper() throw()
        {
        }

    // IUnknown implementation
        HRESULT STDMETHODCALLTYPE QueryInterface(
                /* [in] */ REFIID riid,
                /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject) throw()
        {
	        if (!ppvObject)
	            return E_POINTER;
	        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDispatch))
	            return *ppvObject = this, S_OK;
	        return E_NOINTERFACE;
        }

        ULONG STDMETHODCALLTYPE AddRef() throw()
        {
            es_lock lock(critical_section_);
            return ++ m_count;
        }

        ULONG STDMETHODCALLTYPE Release() throw()
        {
            es_lock lock(critical_section_);
            if (m_count < 1)
                delete this;
            return -- m_count;
        }

    // IDispatch implementation
        STDMETHOD(GetTypeInfoCount)(UINT*) throw()
        {
            return E_NOTIMPL;
        }

        STDMETHOD(GetTypeInfo)(UINT, LCID, ITypeInfo**) throw()
        {
            return E_NOTIMPL;
        }

        STDMETHOD(GetIDsOfNames)(
            REFIID,
            LPOLESTR* rgszNames,
            UINT cNames,
            LCID,
            DISPID* rgdispid) throw()
        {
            es_lock lock(critical_section_);
            ES_ASSERT(cNames == 1);
            key_.push_back(*rgszNames);
            es_static_assert<sizeof(DISPID) == sizeof(size_t)>();
            *rgdispid = DISPID(key_.size());
            return S_OK;
        }

        STDMETHOD(Invoke)(
            DISPID dispidMember,
            REFIID,
            LCID,
            WORD wFlags,
            DISPPARAMS* pdispparams,
            VARIANT* pvarResult,
            EXCEPINFO* pexcepinfo,
            UINT* puArgErr)
        {
            es_lock lock(critical_section_);
            switch (wFlags)
            {
            case DISPATCH_METHOD:
                return invoke_method(pdispparams, pvarResult);
            case DISPATCH_PROPERTYGET:
                return invoke_propertyget(dispidMember, pvarResult);
            default:
                ES_ASSERT(!"es_com_callable_wrapper::Invoke");
			    return E_UNEXPECTED;
            }
            __assume(0);
        }
    private:
        HRESULT invoke_method(
            DISPPARAMS* pdispparams, 
            VARIANT* pvarResult
            ) const throw()
        {
            try {
                IArray& arg = *new es_array<stringT>;
                for (UINT i = 0; i < pdispparams->cArgs; ++i)
                   arg.push(es_primitive_from_variant<stringT>(
                       pdispparams->rgvarg[pdispparams->cArgs - i - 1]));
                *pvarResult = es_variant_from_primitive<stringT>(
                    primitive_.call__(primitive_, arg));
            }
            catch (std::exception& /*e*/) {
                ES_ASSERT(0);            
            }
            catch (es_native_error<std::wstring>& e) {
                ::MessageBox( 
                    0,
                    e.name().operator const_string_t const().c_str(), 
                    e.message().operator const_string_t const().c_str(), 
                    0);
                ES_ASSERT(0);            
            }
            catch (...) {
                ES_ASSERT(0);
                *pvarResult = _variant_t();
                return E_UNEXPECTED;
            }
            return S_OK;
        }

        HRESULT invoke_propertyget(
            DISPID dispidMember, VARIANT* pvarResult) const throw()
        {
            try {
                return *pvarResult = dispidMember == 0 ?
                    _variant_t(
                        SysAllocString(
                            primitive_.operator stringT const().c_str())):
                    es_variant_from_primitive<stringT>(
                        primitive_.get_value__()
                        .get__(key_[dispidMember - 1]).get_value__()
                    ), S_OK;
            }
            catch (...) {
                *pvarResult = _variant_t();
                return E_UNEXPECTED;
            }
        }

    private:
        ULONG m_count;
        IPrimitive& primitive_;
        es_critical_section critical_section_;
        std::vector<std::basic_string<wchar_t> > key_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @fn es_raise_com_error
    //
    template <typename stringT>
    void es_raise_com_error(_com_error const& e)
    {
        typedef stringT string_t;
        std::wstringstream ss;
        try {
            ss << e.Error() << L": " << e.ErrorMessage() << std::endl;
        }
        catch (...) {
//            throw *new es_native_error<stringT>(L"com error");
        }
        throw *new es_native_error<string_t>(ss.str().c_str());
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_runtime_callable_wrapper
    //
    template <typename string_t>
    struct es_runtime_callable_wrapper
    : public base_classes::es_collectable_object<
        base_classes::es_object_impl<IActiveXObject, string_t> >
    {
        typedef typename base_classes::es_object_impl<
            IActiveXObject, string_t> object_t;

        explicit es_runtime_callable_wrapper(
            IDispatch *pDispatch,
            DISPID dispid
            ) throw()
            
        : m_pDispatch(pDispatch)
        , m_dispid(dispid)
        {
            m_pDispatch->AddRef();
        }

        ~es_runtime_callable_wrapper() throw()
        {
            m_pDispatch->Release();
        }

        void sweep__() throw()
        {
        }

        const_string_t const class__() const throw()
        {
            return L"Actor";
        }

        IPrimitive& __stdcall call__(IPrimitive& /*this_arg*/, IPrimitive& arguments)
        {
            return call_impl(arguments);
        }

        IPrimitive& __stdcall get_value__()
        {
            return get_value_impl();
        }

        IBoolean& __stdcall delete__()
        {
            return delete_impl();
        }

        IPrimitive& __stdcall postfix_inc__()
        {
            IPrimitive& old_value = get_value_impl();
            assign_impl(*new es_number<string_t>(old_value.operator double() + 1));
            return old_value;
        }

        IPrimitive& __stdcall postfix_dec__()
        {
            IPrimitive& old_value = get_value_impl();
            assign_impl(*new es_number<string_t>(old_value.operator double() - 1));
            return old_value;
        }

        IPrimitive& __stdcall prefix_inc__()
        {
            return assign_impl(
                *new es_number<string_t>(
                    get_value_impl().operator double() + 1));
        }

        IPrimitive& __stdcall prefix_dec__()
        {
            return assign_impl(
                *new es_number<string_t>(
                    get_value_impl().operator double() - 1));
        }

        IPrimitive& __stdcall assign__(IPrimitive& rhs)
        {
            return assign_impl(rhs);
        }

        operator const_string_t const() const
        {
             return L"actor object";
        }

        operator string_t const() const
        {
             return L"actor object";
        }

        IPrimitive& __stdcall assign_mul__(IPrimitive const& rhs)
        {
            return assign_impl(get_value_impl().mul__(rhs));
        }

        IPrimitive& __stdcall assign_div__(IPrimitive const& rhs)
        {
            return assign_impl(get_value_impl().div__(rhs));
        }

        IPrimitive& __stdcall assign_mod__(IPrimitive const& rhs)
        {
            return assign_impl(get_value_impl().mod__(rhs));
        }

        IPrimitive& __stdcall assign_plus__(IPrimitive const& rhs)
        {
            return assign_impl(get_value_impl().binary_plus__(rhs));
        }

        IPrimitive& __stdcall assign_minus__(IPrimitive const& rhs)
        {
            return assign_impl(get_value_impl().binary_minus__(rhs));
        }

        IPrimitive& __stdcall assign_shl__(IPrimitive const& rhs)
        {
            return assign_impl(get_value_impl().shl__(rhs));
        }

        IPrimitive& __stdcall assign_sar__(IPrimitive const& rhs)
        {
            return assign_impl(get_value_impl().sar__(rhs));
        }

        IPrimitive& __stdcall assign_shr__(IPrimitive const& rhs)
        {
            return assign_impl(get_value_impl().shr__(rhs));
        }

        IPrimitive& __stdcall assign_and__(IPrimitive const& rhs)
        {
            return assign_impl(get_value_impl().bitwise_and__(rhs));
        }

        IPrimitive& __stdcall assign_xor__(IPrimitive const& rhs)
        {
            return assign_impl(get_value_impl().bitwise_xor__(rhs));
        }

        IPrimitive& __stdcall assign_or__(IPrimitive const& rhs)
        {
            return assign_impl(get_value_impl().bitwise_or__(rhs));
        }

        IDispatch *get_dispatch() const
        {
            return m_pDispatch;
        }

    private:
        IPrimitive& __stdcall call_impl(IPrimitive& arguments)
        {
            VARIANT result = {VT_EMPTY};
            EXCEPINFO excepinfo = {0};
            UINT argerr = 0;
            std::vector<VARIANTARG> variant_arg;
            ecmascript::uint32_t length = arguments.length__();
            for (ecmascript::uint32_t i = 0; i < length; ++i)
                variant_arg.push_back(es_variant_from_primitive<string_t>(
                    arguments[length - i - 1].get_value__()));
            DISPPARAMS params
                = {length > 0 ? &*variant_arg.begin(): NULL, NULL, length, 0};
            HRESULT hr = m_pDispatch->Invoke(
                m_dispid, IID_NULL, LOCALE_USER_DEFAULT,
                DISPATCH_METHOD, &params, &result, &excepinfo, &argerr);
            if (DISP_E_EXCEPTION == hr)
                es_raise_com_error<string_t>(_com_error(hr));
            if (FAILED(hr))
                es_raise_com_error<string_t>(_com_error(hr));
            return es_primitive_from_variant<string_t>(result);
        }

        IPrimitive& get_value_impl()
        {
            VARIANT result = {VT_EMPTY};
            EXCEPINFO excepinfo = {0};
            UINT argerr = 0;
            DISPPARAMS params = {NULL, NULL, 0, 0};
            HRESULT hr = m_pDispatch->Invoke(
                m_dispid, IID_NULL, LOCALE_USER_DEFAULT,
                DISPATCH_PROPERTYGET, &params, &result, &excepinfo, &argerr);
            if (FAILED(hr))
                es_raise_com_error<string_t>(_com_error(hr));
            return es_primitive_from_variant<string_t>(result);
        }

        IPrimitive& assign_impl(IPrimitive& rhs)
        {
            VARIANT result = {VT_EMPTY};
            EXCEPINFO excepinfo = {0};
            UINT argerr = 0;
            VARIANT value = es_variant_from_primitive<string_t>(rhs);
            DISPID dispid = DISPID_PROPERTYPUT;
            DISPPARAMS params = {&value, &dispid, 1, 1};
            HRESULT hr = m_pDispatch->Invoke(
                m_dispid, IID_NULL, LOCALE_USER_DEFAULT,
                DISPATCH_PROPERTYPUT, &params, &result, &excepinfo, &argerr);
            if (FAILED(hr))
                es_raise_com_error<string_t>(_com_error(hr));
            return rhs;
        }

        IBoolean& __stdcall delete_impl()
        {
            throw std::logic_error(
                "not implemented: es_com_callable_wrapper::delete_impl()");
        }

        IDispatch *m_pDispatch;
        DISPID m_dispid;
    };


    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_activex_object
    //
    struct es_activex_object
    : public base_classes::es_collectable_object<
        base_classes::es_object_impl<IPrimitive, std::wstring> >
    {
        typedef std::wstring string_t;
        typedef base_classes::es_object_impl<IPrimitive, string_t> object_t;

    public:
        static IFunction& static_get_constructor()
        {
            static struct constructor
            : public base_classes::es_function_impl<IFunction, string_t>
            {
                constructor() throw()
                {
                }

                ~constructor() throw()
                {
                }

                IPrimitive& __stdcall construct__(IPrimitive& arguments)
                {
                    if (arguments.length__() == 0)
                        throw std::runtime_error("invalid arguments.");
                    IDispatch *pdisp;
                    HRESULT hr = ::CoInitialize(NULL);
                    if (FAILED(hr))
                        throw _com_error(hr);
                    CLSID clsid;
                    hr = ::CLSIDFromProgID(
                        arguments[0].operator const_string_t const().c_str(), &clsid);
                    if (FAILED(hr))
                        throw _com_error(hr);
                    hr = ::CoCreateInstance(
                        clsid, NULL, CLSCTX_ALL, IID_IDispatch,
                        reinterpret_cast<void **>(&pdisp));
                    if (FAILED(hr))
                        throw _com_error(hr);
                    return *new es_activex_object(pdisp);
                }

                IPrimitive& __stdcall prototype()
                {
                    return es_activex_object::static_prototype();
                }

            } constructor_;
            return constructor_;
        }

    public:
        explicit es_activex_object(IDispatch *pdisp) throw()
        : m_pDispatch(pdisp)
        {
            ES_ASSERT (NULL != m_pDispatch);
            m_pDispatch->AddRef();
        }

        virtual ~es_activex_object() throw()
        {
            m_pDispatch->Release();
        }

        const_string_t const class__() const throw()
        {
            return L"ActiveX";
        }

        IPrimitive& __stdcall get__(const_string_t const& key)
        {
            return get_impl(key);
        }

        void __stdcall put__(const_string_t const& key, IPrimitive& rhs)
        {
            put_impl(key, rhs);
        }

        void __stdcall put_by_value__(IPrimitive const& key, IPrimitive& value)
        {
            put_impl(key.operator const_string_t const(), value);
        }

        operator const_string_t const() const
        {
             return L"activex object";
        }

        operator string_t const() const
        {
             return L"activex object";
        }

    private:
        IPrimitive& get_impl(const_string_t const& key)
        {
            LPOLESTR rgszNames = const_cast<LPOLESTR>(key.c_str());
            DISPID rgDispid = 0;
            HRESULT hr = S_OK;
            hr = m_pDispatch->GetIDsOfNames(
                IID_NULL, &rgszNames, 1, LOCALE_USER_DEFAULT, &rgDispid);
            if (hr == DISP_E_UNKNOWNNAME)
                return object_t::property_map_[key];
            if (SUCCEEDED(hr))
                return *new es_runtime_callable_wrapper<string_t>(m_pDispatch, rgDispid);
            _com_error const e(hr);
            fwprintf(stderr, L"com error: code=%d message=%s\n", e.Error(), e.ErrorMessage());
            throw std::runtime_error("com error: ");
        }

     private:
        void put_impl(const_string_t const& key, IPrimitive& rhs)
        {
            LPOLESTR rgszNames = const_cast<LPOLESTR>(key.c_str());
            DISPID rgDispid = 0;
            HRESULT hr = m_pDispatch->GetIDsOfNames(
                IID_NULL, &rgszNames, 1, LOCALE_USER_DEFAULT, &rgDispid);
            if (hr != S_OK)
                throw _com_error(hr);
            VARIANT result = {VT_EMPTY};
            EXCEPINFO excepinfo = {0};
            UINT argerr = 0;
            VARIANT value = es_variant_from_primitive<string_t>(rhs);
            DISPPARAMS params = {&value, &rgDispid, 1, 1};
            hr = m_pDispatch->Invoke(
                rgDispid, IID_NULL, LOCALE_USER_DEFAULT,
                DISPATCH_PROPERTYPUT, &params, &result, &excepinfo, &argerr);
            if (FAILED(hr))
                throw _com_error(hr);
            hr = m_pDispatch->GetIDsOfNames(
                IID_NULL, &rgszNames, 1, LOCALE_USER_DEFAULT, &rgDispid);
            if (hr != S_OK)
                throw _com_error(hr);
        }

     private:
        IDispatch *m_pDispatch;
    };


} // namespace ecmascript

#endif // _MSC_VER
