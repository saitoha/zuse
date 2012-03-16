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

//////////////////////////////////////////////////////////////////////////////
//
// global object
//
namespace ecmascript {

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_global
    //  @brief IGlobal implementaion
    //
    template <typename stringT, typename parserT>
    struct es_global
    : public base_classes::es_collectable_object<
        base_classes::es_function_impl<IGlobal, stringT> >
    {
    private:
        typedef stringT string_t;
        typedef es_global<string_t, parserT> self_t;
        typedef base_classes::es_function_impl<IGlobal, stringT> function_t;

    public:
        static IGlobal& create_instance() throw()
        {
            static self_t global;
            return global;
        }
        
        static IGlobal& create_instance(IPrimitive& parent) throw()
        {
            static self_t global(parent);
            return global;
        }
        
    public:
        void mark__(int cookie) throw()
        {
            if (this->get_mark() == cookie)
                return;
            function_t::mark__(cookie);
        }

        const_string_t const class__() const throw()
        {
            return L"Global";
        }

        INumber const& __stdcall NaN() const
        {
            return *new es_number<stringT>(
                (std::numeric_limits<double>::quiet_NaN)());
        }

        INumber const& __stdcall Infinity() const
        {
            return *new es_number<stringT>(
                (std::numeric_limits<double>::infinity)());
        }

        IPrimitive& __stdcall eval(IPrimitive& given_argument)
        {
            const_string_t const source
                = given_argument.operator const_string_t const();
            if (VT::String != given_argument.type__())
                return given_argument;
            vm_.set_eax(es_undefined<string_t>::create_instance());            
            static parserT const parser = parserT();
            IActor& closure = parser.parse(source.begin(), source.end());
            IPrimitive& ecx = vm_.get_ecx();
//            vm_.push(vm_.get_ecx());
            IPrimitive **esp = vm_.get_esp();
#ifdef ES_TRACE_NODE
#endif // ES_TRACE_NODE
#if ES_TEST_JIT            
            compile_and_run(closure, vm_);         
#else
#   if ES_TRACE_RUNNING_TIME
            std::clock_t t = std::clock();
#   endif // ES_TRACE_RUNNING_TIME
            closure(vm_);
#   if ES_TRACE_RUNNING_TIME
            wprintf(L"eval: %.3f\n", double(std::clock() - t) / CLOCKS_PER_SEC);
#   endif // ES_TRACE_RUNNING_TIME
#endif // ES_TEST_JIT
            vm_.set_esp(esp);
//            vm_.set_ecx(vm_.pop());
            vm_.set_ecx(ecx);
            return vm_.get_eax();
        }

        INumber const& __stdcall parseInt(
            IPrimitive const& given_argument) const
        {
            return given_argument.ToInt32();
        }

        INumber const& __stdcall parseFloat(
            IPrimitive const& given_argument) const
        {
            return given_argument.ToNumber();
        }

        IBoolean const& __stdcall isNaN(
            IPrimitive const& given_argument) const
        {
            // boost::math::isnan
            return es_boolean<string_t>::create_instance(
                0 != base_services::es_isnan(given_argument.operator double()));
        }

        IBoolean const& __stdcall isFinite(
            IPrimitive const& given_argument) const
        {
            // boost::math::isinf
            return es_boolean<string_t>::create_instance(
                0 != base_services::es_isinf(given_argument.operator double()));
        }

        IFunction const& __stdcall Object()
        {
            return es_object<string_t>::static_get_constructor();
        }

        IFunction const& __stdcall Boolean()
        {
            return es_boolean<string_t>::static_get_constructor();
        }

        IFunction const& __stdcall Number()
        {
            return es_number<string_t>::static_get_constructor();
        }

        IFunction const& __stdcall String()
        {
            return es_string<string_t>::static_get_constructor();
        }

        IFunction const& __stdcall Array()
        {
            return es_array<string_t>::static_get_constructor();
        }

        IFunction const& __stdcall Function()
        {
            return es_managed_function<string_t>::static_get_constructor();
        }

        IFunction const& __stdcall RegExp()
        {
            return es_regexp<string_t>::static_get_constructor();
        }

        IFunction const& __stdcall Date()
        {
            return es_date<string_t>::static_get_constructor();
        }

        IObject const& __stdcall Math()
        {
            IMath& math = es_math<string_t>::create_instance();
            this->put__(L"Math", math);
            return math;
        }

        IUndefined const& __stdcall verbose(IPrimitive& value)
        {
            es_set_verbose(value);
            return es_undefined<string_t>::create_instance();
        }

// IManagePropaties
        IPrimitive& __stdcall get_prototype__()
        {
            return *this;
        }

    private:
        es_global() throw()
        {
            vm_.set_ecx(*this);
            es_machine<IPrimitive>::get_gc().set_root(*this);
        }

        explicit es_global(IPrimitive& parent) throw()
        {
            static es_activation_object<std::wstring> 
                activation_object(*this, parent);
            vm_.set_ecx(activation_object);
            es_machine<IPrimitive>::get_gc().set_root(*this);
        }


        ~es_global() throw()
        {
        }
        
    private:
        es_machine<IPrimitive> vm_;
    };

} // namespace ecmascript
