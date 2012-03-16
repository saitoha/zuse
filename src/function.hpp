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
// function object
//
namespace ecmascript {

    //////////////////////////////////////////////////////////////////////////
    //
    // @fn get_argument_length
    // 
    inline int32_t const
    get_argument_length(
        IActor const * p_closure, 
        int32_t const length = 0
        ) throw()
    {
        return 0 == p_closure ? 
            length: 
            get_argument_length(p_closure->operator [] (1), length + 1);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_managed_function
    //  @brief IFunction implementaion of calling ast-based object's members.
    //
    template <typename stringT>
    struct es_managed_function
    : public base_classes::es_collectable_object<
        base_classes::es_function_impl<IFunction, stringT> >
    {
    private:
        typedef stringT string_t;
        typedef es_managed_function<string_t> self_t;
        typedef base_classes::es_object_impl<IFunction, string_t> object_t;
        typedef base_classes::es_function_impl<IFunction, string_t> function_t;
        
    public:
        static IFunction& static_get_constructor()
        {
            static struct constructor
            : public base_classes::es_function_impl<IFunctionConstructor, stringT>
            {
                constructor() throw()
                {
                }

                ~constructor() throw()
                {
                }

                IPrimitive& __stdcall prototype()
                {
                    return es_managed_function<stringT>::static_prototype();
                }

                IPrimitive& __stdcall construct__(IPrimitive& arguments)
                {
                    throw std::logic_error(
                        "not implemented: es_constructor::construct__");
                }

                IPrimitive& __stdcall call__(
                    IPrimitive& /*this_arg*/, 
                    IPrimitive& arguments
                    )
                {
                    return construct__(arguments);
                }

            } constructor_;
            return constructor_;
        }

        static IFunction& __stdcall create_instance(
            es_machine<IPrimitive>& vm,
            IPrimitive& this_object,
            IActor const& closure
            )
        {            
            return *new self_t(
                vm,
                this_object, 
#if ES_TEST_JIT
                (new es_codestream<IPrimitive>(vm, closure))->as_actor<IActorBase>(), 
#else
                closure, 
#endif // ES_TEST_JIT 1
                get_argument_length(closure.operator[] (0)));
        }
        
// constructors
        explicit es_managed_function(
            es_machine<IPrimitive>& vm,
            IPrimitive& this_object,
            IActorBase const& closure,
            uint32_t length
            )
        : length_(length)
        , closure_(closure)
        , vm_(vm)
        {
        }
        
// INativeCastOperators
        const_string_t const class__() const throw()
        {
            return L"Function";
        }

        operator const_string_t const() const
        {
            return L"managed function";
        }

        operator string_t const() const
        {
            return L"managed function";
        }

// IDynamicInoking
        IPrimitive& __stdcall call__(IPrimitive& this_arg, IPrimitive& arguments)
        {
            vm_.push(vm_.get_ecx());
            vm_.push(*this);
            vm_.set_ecx(es_activation_object<string_t>::create_instance(
                this_arg, vm_.get_ecx()));
            vm_.get_ecx().put__(L"arguments", arguments);
            for (uint32_t i = 0; i < arguments.length__(); ++i)
                vm_.push(arguments[arguments.length__() - 1 - i]);
            closure_(vm_);
            vm_.pop();
            vm_.set_ecx(vm_.pop());
            return vm_.get_eax().get_value__();
        }

        IPrimitive& __stdcall construct__(IPrimitive& arguments)
        {
            IObject& object = *new es_object<string_t>();
            object.set_prototype__(this->prototype());
            call__(object, arguments);
            return object;
        }

// IFunction
        IPrimitive& __stdcall call(IPrimitive& arguments)
        {
            return call__(arguments.get__(L"callee").get_value__(), arguments);
        }

        IPrimitive& __stdcall apply(IPrimitive& this_arg, IPrimitive& arguments)
        {
            return call__(this_arg, arguments);
        }

        INumber& __stdcall length() const
        {
            return *new es_number<string_t>(length_);
        }

    protected:
        /*virtual*/ ~es_managed_function() throw()
        {
        }

    private:
        uint32_t length_;
        IActorBase const& closure_;
        es_machine<IPrimitive>& vm_;
    };

#if defined _MSC_VER

    __declspec(naked) IPrimitive * __stdcall es_invoke(
        IPrimitive&, void*, bool, size_t, IPrimitive **const)
    {
        __asm {
            mov   esi, esp          ; ESI = ESP
            mov   ecx, [esi+10h]    ; ECX = arg4
            mov   eax, [esi+14h]    ; EAX = arg5
            jecxz switch_and_call
        enumarg:                    ; push arguments into stack
            push  dword ptr[eax+ecx*4-4]
            loop  enumarg
        switch_and_call:
            test  dword ptr[esi+0Ch], 1h
            jz    invoke_cdecl
            push  [esi+04h]         ; push 'this' (stdcall)
            call  [esi+08h]         ; call arg2
            ret   14h
        invoke_cdecl:
            push  [esi+10h]
            push  [esi+04h]         ; push 'this' (stdcall)
            call  [esi+08h]         ; call arg2
            mov   esp, esi
            ret   14h
        }
    }

#elif defined(__GNUC__)

#    if defined(__x86__)
    __asm__ volatile (
        "setargs_:                     \n\t"
        "    popl  %ebx                \n\t"
        "    popl  %ecx                \n\t"
        "    popl  %edx                \n\t"
        "    jecxz return_             \n\t"
        "enumargs_:                    \n\t"
        "    push  -4(%edx, %ecx, 4)   \n\t"
        "    loop  enumargs_           \n\t"
        "return_:                      \n\t"
        "    jmp   *%ebx               \n\t"
        "invoke_stdcall_:              \n\t"
        "    popl  %ecx                \n\t"
        "    popl  %eax                \n\t"
        "    push  %ecx                \n\t"
        "    jmp   *%eax               \n\t"
    );

    extern void 
    __attribute__((noinline)) 
    __attribute__((stdcall)) 
    setargs(int argc, IPrimitive **const argv) 
    __asm__ ("setargs_");
    
    extern __attribute__((stdcall)) IPrimitive * 
    __attribute__((noinline))
    invoke_stdcall(void *, IPrimitive&) 
    __asm__ ("invoke_stdcall_");

    __attribute__((cdecl)) 
    IPrimitive * 
    __attribute__((noinline))
    es_invoke(
        IPrimitive& this_arg, 
        void *addr, 
        bool calltype, 
        size_t argc, 
        IPrimitive **const argv
        )
    {
        setargs(argc, argv);
        return invoke_stdcall(addr, this_arg);
    }

#    else
    
    IPrimitive * 
    es_invoke(
        IPrimitive& this_arg,
        void *addr,
        bool calltype,
        size_t argc,
        IPrimitive **const argv
        )
    {
        typedef IPrimitive * (* invoke0)(IPrimitive *);
        typedef IPrimitive * (* invoke1)(IPrimitive *, IPrimitive *);
        typedef IPrimitive * (* invoke2)(
            IPrimitive *, IPrimitive *, IPrimitive *);
        typedef IPrimitive * (* invoke3)(
            IPrimitive *, IPrimitive *, IPrimitive *, IPrimitive*);
        typedef IPrimitive * (* invoke4)(
            IPrimitive *, IPrimitive *, IPrimitive *, IPrimitive*, IPrimitive*);
        
        union {
            void *addr_;
            invoke0 pfn0_;
            invoke1 pfn1_;
            invoke2 pfn2_;
            invoke3 pfn3_;
            invoke4 pfn4_;
        } box;

        box.addr_ = addr;

        switch (argc)
        {
        case 0:
            return box.pfn0_(&this_arg);
        case 1:
            return box.pfn1_(&this_arg, argv[0]);
        case 2:
            return box.pfn2_(&this_arg, argv[0], argv[1]);
        case 3:
            return box.pfn3_(&this_arg, argv[0], argv[1], argv[2]);
        case 4:
            return box.pfn4_(&this_arg, argv[0], argv[1], argv[2], argv[3]);
        }
        throw es_native_error<IPrimitive::string_t>(L"invalid arguments size");
    }
#    endif

#endif

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct es_native_function
    //  @brief IFunction implementaion of calling IPrimitive-based object's
    //         members.
    //
    template <typename stringT>
    struct es_native_function
    : public base_classes::es_collectable_object<
        base_classes::es_function_impl<IFunction, stringT> >
    {
    private:
        typedef stringT string_t;
        
    protected:
        typedef base_classes::es_function_impl<IFunction, stringT> function_t;

    public:
        explicit es_native_function(
            IPrimitive& prototype,
            es_attributes const& attributes)
        : p_vtable_(*reinterpret_cast<void ***>(&prototype))
        , attributes_(attributes)
        {
        }

        ~es_native_function() throw()
        {
        }

        void mark__(int cookie) throw()
        {
            if (this->get_mark() == cookie)
                return;
            function_t::mark__(cookie);
        }

        const_string_t const class__() const throw()
        {
            return L"Function";
        }

// INativeCastOperators
        operator const_string_t const() const
        {
            return L"function() { [native code] }";
        }

        operator string_t const() const
        {
            return L"function() { [native code] }";
        }

// IFunction Override
        IPrimitive& __stdcall call(IPrimitive& arguments)
        {
            return call__(arguments.get__(L"callee").get_value__(), arguments);
        }

        IPrimitive& __stdcall apply(IPrimitive& this_arg, IPrimitive& arguments)
        {
            es_arguments<string_t>& new_arguments
                = *new es_arguments<string_t>(this_arg);
            for (size_t i = 0; i < arguments.length__(); ++i)
                new_arguments.push__(arguments[i]);
            
            return call__(this_arg, new_arguments);
        }

        INumber& __stdcall length() const
        {
            return *new es_number<string_t>(
                uint32_t(attributes_.argc));
        }

        IPrimitive& __stdcall prototype()
        {
            throw std::logic_error("not implemented");
        }

        IPrimitive& __stdcall call__(IPrimitive& this_arg, IPrimitive& arguments)
        {
            return dispatch(this_arg, arguments);
        }

        IPrimitive& __stdcall get_value__()
        {
            return *this;
        }

        IPrimitive& __stdcall assign__(IPrimitive& rhs)
        {
            return get_value__().assign__(rhs);
        }

    private:
        es_native_function() throw();

        IPrimitive& dispatch(IPrimitive& this_arg, IPrimitive& arguments) const
        {
            uint32_t argc = arguments.length__();
            while (argc < attributes_.argc && ++argc)
                arguments.push__(es_undefined<string_t>::create_instance());
            void *p_method = *(p_vtable_ + attributes_.offset);
            if (Cdecl != (attributes_.flag & 0x0001))
                return *es_invoke(
                    this_arg, p_method, true, attributes_.argc,
                    attributes_.argc == 0 ? 0: arguments.begin__());
            union {
                void * src;
                IPrimitive& (__stdcall *dest)(IPrimitive&, IPrimitive&);
            } box;
            box.src = p_method;
            return box.dest(this_arg, arguments);
        }

    private:
        void ** p_vtable_;
        es_attributes const& attributes_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct es_native_property
    //  @brief IFunction implementaion of calling IPrimitive-based object's
    //         members.
    //
    template <typename stringT>
    struct es_native_property
    : public base_classes::es_collectable_object<
        base_classes::es_object_impl<IInternalObject, stringT> >
    {
        typedef stringT string_t;
        typedef base_classes::es_object_impl<IInternalObject, stringT> primitive_t;

        explicit es_native_property(
            IPrimitive& prototype,
            es_attributes const& attributes
            ) throw()
        : p_method_(*(*reinterpret_cast<void ***>(&prototype) + attributes.offset))
        , prototype_(prototype)
        {
        }

        ~es_native_property() throw()
        {
        }

        void mark__(int cookie) throw()
        {
            if (this->get_mark() != cookie)
            	primitive_t::mark__(cookie);
        }

        const_string_t const class__() const throw()
        {
            return L"Function";
        }

        IPrimitive& __stdcall call__(IPrimitive& this_arg, IPrimitive& arguments)
        {
            return get_value__().call__(this_arg, arguments);
        }

        IPrimitive& __stdcall get_value__()
        {
            return dispatch_(prototype_);
        }

        IPrimitive& __stdcall assign__(IPrimitive& rhs)
        {
            return get_value__().assign__(rhs);
        }

    private:
        es_native_property() throw();

    private:
        union {
            void * p_method_;
            IPrimitive& (__stdcall *dispatch_)(IPrimitive&);
        };
        IPrimitive& prototype_;
    };

} // namespace ecmascript

