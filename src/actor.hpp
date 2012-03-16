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


#include "types.hpp"
#include "x86.hpp"


void * junction_break;

#ifdef _DEBUG
namespace {
    struct es_stack_checker {
        explicit es_stack_checker(
            ecmascript::es_machine<ecmascript::IPrimitive> const& vm) throw()
        : vm_(vm)
        , esp_(vm.get_esp())
        {
        }

        ~es_stack_checker() throw()
        {
            if (esp_ != vm_.get_esp())
                ES_ASSERT(esp_ == vm_.get_esp());
        }
    private:
        ecmascript::es_machine<ecmascript::IPrimitive> const& vm_;
        ecmascript::IPrimitive const* const* const esp_;
    };
} // anonymous namespace
#    define ES_STACK_CHECK() es_stack_checker stack_checker(vm);
#else
#    define ES_STACK_CHECK() ;
#endif // _DEBUG

// core engine
namespace ecmascript {

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  es_callstack
    //
    struct es_callstack
    {
        static const size_t ES_CALLSTACK_SIZE = 1000;
        std::ptrdiff_t buffer_[ES_CALLSTACK_SIZE];
        typedef std::ptrdiff_t * iterator_t;

        es_callstack() throw()
        : it_(buffer_)
        {
        }

        iterator_t get_iterator () const throw()
        {
            return it_;
        }
         
    private:
        iterator_t it_;
        
    };
    static es_callstack callstack;

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  es_codestream
    //
    template <typename primitiveT>
    struct es_codestream
    {
        typedef es_codestream self_t;
        typedef char value_type;
        typedef size_t size_type;
        typedef es_machine<primitiveT> machine_t;
        
        template <typename actorT>
        es_codestream(machine_t& vm, actorT const& actor) throw()
        : buffer_(mmap_alloc<value_type>(50319))
        , it_(buffer_)
        , p_vm_(&vm)
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            *this 
                << push(ebx) 
                << mov(ebx, ptr[esp + 8]) 
                << push(edi)  // -4
                << push(ebx)
                << call(&es_machine<IPrimitive>::get_ecx)
                << mov(edi, eax)
                << push(esi)  // -4
                << mov(eax, &callstack.get_iterator())
                << mov(esi, ptr[eax])
                << actor 
                << mov(eax, &callstack.get_iterator())
                << mov(ptr[eax], esi)
                << pop(esi)  // +4
                << push(edi)
                << push(ebx)
                << call(&es_machine<IPrimitive>::set_ecx) // +4
                << pop(edi)  // +4                        
                << pop(ebx) 
                << ret
            ;
#endif // ES_TEST_JIT
        }

        template <typename generatorT>
        self_t& operator << (generatorT const& generator) throw()
        {
            generator.assemble(*this);
            return *this;
        }

        void write(void const* src, size_type length) throw()
        {
            memcpy(it_, src, length);
            it_ += length;
        }

        value_type const *begin() const throw()
        {
            return buffer_;
        }

        value_type *begin() throw()
        {
            return buffer_;
        }

        value_type const *end() const throw()
        {
            return it_;
        }

        value_type *end() throw()
        {
            return it_;
        }

        size_type empty() const throw()
        {
            return it_ == buffer_;
        }

        void relative_patch(unsigned int address) const throw()
        {
            do 
            {
                int *p = reinterpret_cast<int *>(address) - 1; 
                unsigned int next = *p;                       
                *p = unsigned(it_) - unsigned(address);
                address = next;
            } 
            while (0 != address);
        }
        
        void absolute_patch(unsigned int address) const throw()
        {
            do 
            {
                unsigned int *p = reinterpret_cast<unsigned int *>(address) - 1; 
                unsigned int next = *p;                       
                *p = unsigned(it_);
                address = next;
            } 
            while (0 != address);
        }
        
        template <typename machineT>
        primitiveT& __stdcall operator ()(
            machineT& vm
            ) const throw()
        {
            union {
                primitiveT& (__cdecl *pfn)(machineT&);
                value_type const* p;
            } box;
            box.p = begin();
            return box.pfn(vm);
        }
        
        template <typename actorT>
        actorT const& as_actor() const 
        {
            struct actor 
            : actorT
            {
                explicit actor(self_t const& code) throw()
                : code_(code)
                {
                }
                
                void ES_CLOSURECALLTYPE 
                operator () (es_machine<IPrimitive>& vm) const 
                {
                    code_(vm);
                }
                
            private:    
                self_t const& code_;
            };
            
            return *new actor(*this);
        }
                
        typedef primitiveT& (__cdecl *function_type)(
            void const*, 
            es_machine<primitiveT>&
            );
        
        template <typename actorT>
        function_type __stdcall 
        make_function() const throw()
        {
            union {
                function_type pfn;
                value_type const* p;
            } box;
            return box.pfn;
        }
        
        es_machine<primitiveT>& get_vm() const
        {
            return *p_vm_;
        }
        
        void get_vm(es_machine<primitiveT>& vm) const
        {
            p_vm_ = &vm;
        }
        
    private:
        value_type *buffer_;
        value_type *it_;
        machine_t * p_vm_;
    };    
    
    struct IClosure
    : IActor
    {
        virtual void assemble(es_codestream<IPrimitive>&) const = 0;    
    };
    
    template <typename closureT, typename machineT>
    inline void 
    compile_and_run(closureT const& actor, machineT& vm)
    {
#if ES_TRACE_COMPILE_TIME
        std::clock_t t = std::clock();
#endif // ES_TRACE_COMPILE_TIME
        es_codestream<IPrimitive> code(vm, (IClosure const&)actor);
#if ES_TRACE_COMPILE_TIME
        wprintf(L"compile: %.3f\n", double(std::clock() - t) / CLOCKS_PER_SEC);
#endif // ES_TRACE_COMPILE_TIME
        
        {
#if ES_TRACE_RUNNING_TIME
            std::clock_t t = std::clock();
#endif // ES_TRACE_RUNNING_TIME
            code(vm);
#if ES_TRACE_RUNNING_TIME
            wprintf(L"eval: %.3f\n", double(std::clock() - t) / CLOCKS_PER_SEC);
#endif // ES_TRACE_RUNNING_TIME
        }
    };
        
    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_binded_vfunc
    //
    struct es_binded_vfunc
    : IClosure
    {
        typedef void (ES_CLOSURECALLTYPE *Fp1)(
            IClosure const&, es_machine<IPrimitive>&);
        union {
            IClosure const *const p_closure_;
            Fp1 const* const* fp1_;
        };

        explicit es_binded_vfunc(IClosure const& closure) throw()
        : p_closure_(&closure)
        {
        }

        void /*override*/ ES_CLOSURECALLTYPE 
        operator () (es_machine<IPrimitive>& vm) const
        {
            (**fp1_)(*p_closure_, vm);
        }

        void /*override*/ 
        assemble(es_codestream<IPrimitive>& code) const
        {
            code << *p_closure_;
        }

        es_value_or_hint<IPrimitive> /*override*/ 
        const hint__() const 
        {
            return p_closure_->hint__();
        }

        IActor const * /*override*/ 
        operator [] (ecmascript::uint32_t index) const
        {
            return p_closure_->operator [] (index);
        }
    };

#ifdef _MSC_VER
    typedef es_binded_vfunc const functor_t;
#else
    typedef IClosure const& functor_t;
#endif // _MSC_VER

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_memento_stack
    //
    struct es_memento_stack
    {
        es_memento_stack(es_machine<IPrimitive>& vm)
        : vm_(vm)
        , esp_(vm.get_esp())
        {
        }

        ~es_memento_stack()
        {
            ES_ASSERT(0 <= esp_ - vm_.get_esp());
            while (esp_ != vm_.get_esp())
                vm_.pop();
        }

    private:
        es_machine<IPrimitive>& vm_;
        IPrimitive **esp_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_leaf_operator
    //
    struct es_leaf_operator
    : IClosure
    {
    private:
        typedef es_leaf_operator self_t;
        
    public:
        es_leaf_operator() throw()
        {
        }

// IActor
        void /*override*/ ES_CLOSURECALLTYPE
        operator () (es_machine<IPrimitive>& vm) const
        {
        }

        void /*override*/
        assemble(es_codestream<IPrimitive>& code) const
        {
        }

        es_value_or_hint<IPrimitive> const /*override*/
        hint__() const 
        {
            return TH_Empty;
        }
        
        IActor const * /*override*/
        operator [] (ecmascript::uint32_t /* index */) const 
        { 
            return 0; 
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_unary_operator
    //
    struct es_unary_operator
    : es_leaf_operator
    {
    private:
        typedef es_unary_operator self_t;
        typedef es_leaf_operator base_t;
        
    public:
        explicit es_unary_operator(IClosure const& arg1) throw()
        : functor_(arg1)
        {
        }

// IActor
        void /*override*/ ES_CLOSURECALLTYPE
        operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
        }

        void /*override*/
        assemble(es_codestream<IPrimitive>& code) const
        {
            code
                << this->actor1()
                ;
        }

        void ES_CLOSURECALLTYPE op1(es_machine<IPrimitive>& vm) const
        {
            functor_(vm);
        }

        IClosure const& ES_CLOSURECALLTYPE actor1() const
        {
            return functor_;
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return functor_.hint__();
        }
        
        IActor const * /*override*/
        operator [] (ecmascript::uint32_t index) const 
        { 
            return 0 == index ? &functor_ : base_t::operator [] (index); 
        }
        
    private:
        functor_t functor_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_binary_operator
    //
    struct es_binary_operator
    : public es_unary_operator
    {
    private:
        typedef es_binary_operator self_t;
        typedef es_unary_operator base_t;
        
    public:
        explicit es_binary_operator(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_unary_operator(arg1)
        , functor_(arg2)
        {
        }

// IActor
        void /*override*/ ES_CLOSURECALLTYPE
        operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            this->op2(vm);
        }

        void /*override*/
        assemble(es_codestream<IPrimitive>& code) const
        {
            code
                << this->actor1()
                << this->actor2()
                ;
        }

        void ES_CLOSURECALLTYPE op2(es_machine<IPrimitive>& vm) const
        {
            functor_(vm);
        }

        IClosure const& ES_CLOSURECALLTYPE actor2() const
        {
            return functor_;
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return functor_.hint__();
        }

        IActor const * /*override*/
        operator [] (ecmascript::uint32_t index) const 
        { 
            return 1 == index ? &functor_ : base_t::operator [] (index); 
        }
        
    private:
        functor_t functor_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_trinary_operator
    //
    struct es_trinary_operator
    : public es_binary_operator
    {
    private:
        typedef es_trinary_operator self_t;
        typedef es_binary_operator base_t;
        
    public:
        explicit es_trinary_operator(
            IClosure const& arg1,
            IClosure const& arg2,
            IClosure const& arg3
            ) throw()
        : es_binary_operator(arg1, arg2)
        , functor_(arg3)
        {
        }

        void /*override*/ ES_CLOSURECALLTYPE
        operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            this->op2(vm);
            this->op3(vm);
        }

        void /*override*/
        assemble(es_codestream<IPrimitive>& code) const
        {
            code
                << this->actor1()
                << this->actor2()
                << this->actor3()
                ;
        }

        void ES_CLOSURECALLTYPE op3(es_machine<IPrimitive>& vm) const
        {
            functor_(vm);
        }

        IClosure const& ES_CLOSURECALLTYPE actor3() const
        {
            return functor_;
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return functor_.hint__();
        }

        IActor const * /*override*/
        operator [] (ecmascript::uint32_t index) const 
        { 
            return 2 == index ? &functor_ : base_t::operator [] (index); 
        }
        
    private:
        functor_t functor_;
    };

    template <Rule id>
    struct es_lazy_closure;

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Nop>
    //
    template <>
    struct es_lazy_closure<Nop>
    : es_leaf_operator
    {
        es_lazy_closure() throw()
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
        }

        void assemble(es_codestream<IPrimitive>& /* code */) const
        {
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<And>
    //
    //                                                    true
    //                                                     -->  op2
    //                    +-----------+
    //                op1 | value1    |     bool(value1)?
    //  +-----------+ --> +-----------+ --> +-----------+       +-----------+
    //  |           |     |           |     |           |       | false     |
    //                                                     -->  +-----------+
    //                                                   false  |           |
    //
    template <>
    struct es_lazy_closure<And>
    : public es_unary_operator
    {
    private:
        typedef es_lazy_closure<And> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            if (vm.get_eax().get_value__().operator bool())
                this->op1(vm); // push value2
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            long_relative_label__ end;
            code
                // if (vm.get_eax().get_value__().operator bool()) this->op2(vm);
                << push(eax)
                << push(eax)                
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)                
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_operator_bool * 4])
                << test(al, al)
                << pop(eax)
                << je(end)
                << actor1()
            
            << end
                ;
#endif // ES_TEST_JIT
        }
        
        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Primitive;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<If>
    //
    //                                                    true
    //                                                     -->  op2
    //                    +-----------+
    //                op1 | value1    |     bool(value1)?
    //  +-----------+ --> +-----------+ --> +-----------+       +-----------+
    //  |           |     |           |     |           |       | false     |
    //                                                     -->  +-----------+
    //                                                   false  |           |
    //
    template <>
    struct es_lazy_closure<If>
    : public es_unary_operator
    {
    private:
        typedef es_lazy_closure<If> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            if (vm.get_eax().get_value__().operator bool())
                this->op1(vm); // push value2
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            long_relative_label__ end;
            code
                // if (vm.get_eax().get_value__().operator bool()) this->op2(vm);
                << push(eax)                
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_operator_bool * 4])
                << test(al, al)
                << je(end)
                << actor1()
                
            << end                
                ;
#endif // ES_TEST_JIT
        }
        
        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };
    
    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Or>
    //                                                          +-----------+
    //                                                    true  | true      |
    //                                                     -->  +-----------+
    //                    +-----------+                         |           |
    //                op1 | value1    |     bool(value1)?
    //  +-----------+ --> +-----------+ --> +-----------+
    //  |           |     |           |     |           |
    //                                                     -->  op2
    //                                                   false
    //
    template <>
    struct es_lazy_closure<Or>
    : es_unary_operator
    {
        typedef es_lazy_closure<And> self_t;

        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            if (!vm.get_eax().get_value__().operator bool())
                this->op1(vm); // push value2
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            long_relative_label__ end;
            code
                // if (!vm.get_eax().get_value__().operator bool()) this->op2(vm)
                << push(eax)
                << push(eax)                
                << mov(ecx, ptr[eax])
                << call(ptr[ecx + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(ecx, ptr[eax])
                << call(ptr[ecx + IPrimitive::verb_operator_bool * 4])
                << test(al, al)
                << pop(eax)
                << je(end)
                << actor1()
            
            << end
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Primitive;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Alternative>
    //
    //
    //                                                    true
    //                                                     -->  op2
    //                    +-----------+
    //                op1 | value1    |     bool(value1)?
    //  +-----------+ --> +-----------+ --> +-----------+
    //  |           |     |           |     |           |
    //                                                     -->  op3
    //                                                   false
    template <>
    struct es_lazy_closure<Alternative>
    : es_binary_operator
    {
        typedef es_lazy_closure<Alternative> self_t;

        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.get_eax().get_value__().operator bool() ? this->op1(vm): this->op2(vm);
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            long_relative_label__ else_clause, end;
            code
                // vm.get_eax().get_value__().operator bool() ? this->op2(vm): this->op3(vm);
                << push(eax)                
                << mov(ecx, ptr[eax])
                << call(ptr[ecx + IPrimitive::verb_get_value__ * 4])
                << push(eax)                
                << mov(ecx, ptr[eax])
                << call(ptr[ecx + IPrimitive::verb_operator_bool * 4])                
                << test(al, al)                
                << je(else_clause)
                << actor1()
                << jmp(end)
            
            << else_clause
                << actor2()
            
            << end
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Primitive;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<IfElse>
    //
    //
    //                                                    true
    //                                                     -->  op2
    //                    +-----------+
    //                op1 | value1    |     bool(value1)?
    //  +-----------+ --> +-----------+ --> +-----------+
    //  |           |     |           |     |           |
    //                                                     -->  op3
    //                                                   false
    template <>
    struct es_lazy_closure<IfElse>
    : es_binary_operator
    {
        typedef es_lazy_closure<IfElse> self_t;

        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.get_eax().get_value__().operator bool() ? this->op1(vm): this->op2(vm);
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            long_relative_label__ else_clause, end;
            code
                // vm.get_eax().get_value__().operator bool() ? this->op2(vm): this->op3(vm);
                << push(eax)                
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)                
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_operator_bool * 4])
                << test(al, al)                
                << je(else_clause)
                << actor1()
                << jmp(end)
            
            << else_clause
                << actor2()
            
            << end
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<DoWhile>
    //
    template <>
    struct es_lazy_closure<DoWhile>
    : es_binary_operator
    {
        typedef es_lazy_closure<DoWhile> self_t;

        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.collect();

            IPrimitive **p_primitive = vm.get_esp();
            IPrimitive& context = vm.get_ecx();
            
            jmp_buf jmp_buf_;
        loop:
            vm.push_context__(jmp_buf_);
            switch (setjmp(jmp_buf_))
            {
            case CT_Normal:
                this->op1(vm);
                break;
            case CT_Return:
                vm.return__();
            case CT_Continue:
                if (VT::Undefined != vm.get_signal().type__())
                    vm.continue__();
                vm.set_esp(p_primitive);
                vm.set_ecx(context);
                break;
            case CT_Break:
                if (VT::Undefined != vm.get_signal().type__())
                    vm.break__();
                vm.set_esp(p_primitive);
                vm.set_ecx(context);
                return;
            case CT_Throw:
                vm.throw__();
            }
            this->op2(vm);
            vm.pop_context__();
            if (vm.get_eax().get_value__().operator bool())
                goto loop;
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            long_absolute_label__ l1;
            long_relative_label__ l2, l3, l4, l5;
            code
                << push(ptr[ebx]) // -4                               : 
                << push(edi) // -4                
                << push(ebx) // -4
                << push(l1)  // -4                
                << mov(ptr[esi], esp)                
                << add(esi, 0x4)                
                << jmp(l4)

            << l1
                << cmp(al, CT_Break)
                << je(l2)
                
                << push(eax)                
                << push(edx)
                
                << push(edx)
                << mov(eax, ptr[edx])
                << call(ptr[eax + IPrimitive::verb_type__ * 4])
                << cmp(al, VT::Undefined)
                
                << pop(edx)                
                << pop(eax)
                
                << jne(l3)
                
                << pop(ebx)  // +4
                << pop(edi)
                << pop(ecx)
                << mov(ptr[ebx], ecx)
                << jmp(l5)
                
            << l2
                << cmp(al, CT_Continue)
                << je(l3)
                                              
                << push(eax)                
                << push(edx)
                
                << push(edx)
                << mov(ecx, ptr[edx])
                << call(ptr[ecx + IPrimitive::verb_type__ * 4])
                << cmp(al, VT::Undefined)
                
                << pop(edx)
                << pop(eax)
                
                << jne(l3)
                
                << pop(ebx)  // +4                
                << pop(edi)                
                << pop(ecx)
                << mov(ptr[ebx], ecx)                             
                << add(esi, 0x4)                           
                << sub(esp, 0x10)
                << jmp(l4)                
                                
            << l3
                << sub(esi, 0x4)  
                << mov(esp, ptr[esi])
                << ret
                
            << l4                
                << actor1()                
                << sub(esi, 0x4)               
                << add(esp, 0x10)    
                << actor2()                
                << push(eax)                
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)                
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_operator_bool * 4])                
                << test(al, al)
                << je(l5)                
                << add(esi, 0x4)                
                << sub(esp, 0x10)
                << jmp(l4)                
                
            << l5
                ;
#endif // ES_TEST_JIT
        }
        
        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<While>
    //
    template <>
    struct es_lazy_closure<While>
    : es_binary_operator
    {
        typedef es_lazy_closure<While> self_t;

        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.collect();
            
            IPrimitive **p_primitive = vm.get_esp();
            IPrimitive& context = vm.get_ecx();
            
            jmp_buf jmp_buf_;
            vm.push_context__(jmp_buf_);
        loop:
            this->op1(vm);
            if (!vm.get_eax().get_value__().operator bool())
                goto end;
            switch (setjmp(jmp_buf_))
            {
            case CT_Normal:
                this->op2(vm);
                goto loop;
            case CT_Return:
                vm.return__();
            case CT_Continue:
                if (VT::Undefined != vm.get_signal().type__())
                    vm.continue__();
                vm.set_esp(p_primitive);
                vm.set_ecx(context);
                vm.push_context__(jmp_buf_);
                goto loop;
            case CT_Break:
                if (VT::Undefined != vm.get_signal().type__())
                    vm.break__();
                vm.set_esp(p_primitive);
                vm.set_ecx(context);
                return;
            case CT_Throw:
                vm.throw__();
            }
        end:
            vm.pop_context__();
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            long_absolute_label__ l2;
            long_relative_label__ l1, l3, l4, l5;
            code
                
            << l1
                << actor1()  
                
                << push(eax)                
                << mov(ecx, ptr[eax])
                << call(ptr[ecx + IPrimitive::verb_get_value__ * 4])
                << push(eax)                
                << mov(ecx, ptr[eax])
                << call(ptr[ecx + IPrimitive::verb_operator_bool * 4])                

                << test(al, al)
                << je(l5)                
                << push(ptr[ebx]) // -4                                
                << push(edi) // -4                
                << push(ebx) // -4
                << push(l2)  // -4
                
                << mov(ptr[esi], esp)                
                << add(esi, 0x4)            
                << actor2()
                << sub(esi, 0x4)               
                << add(esp, 0x10)
                << jmp(l1)
                
            << l2
                << cmp(al, CT_Break)
                << jne(l3)            
                << push(eax)
                
                << push(edx)
                << push(edx)
                << mov(ecx, ptr[edx])
                << call(ptr[ecx + IPrimitive::verb_type__ * 4])
                << cmp(al, VT::Undefined)
                << pop(edx)
                << pop(eax)
                << jne(l4)

                << pop(ebx)  // +4                
                << pop(edi)

                << pop(ecx)
                << mov(ptr[ebx], ecx)
                
                << jmp(l5)
                
            << l3
                << cmp(al, CT_Continue)
                << jne(l4)                                
                << push(eax)

                << push(edx)
                << push(edx)
                << mov(eax, ptr[edx])
                << call(ptr[eax + IPrimitive::verb_type__ * 4])
                << cmp(al, VT::Undefined)
                << pop(edx)
                << pop(eax)
                << jne(l4)

                << pop(ebx)  // +4                
                << pop(edi)
                
                << pop(ecx)
                << mov(ptr[ebx], ecx)                                
                << add(esi, 0x4)

                << sub(esp, 0x10)
                << jmp(l1)            
                                
            << l4
                << sub(esi, 0x4)                  
                << mov(esp, ptr[esi])                
                << ret
                
            << l5
                ;
#endif // ES_TEST_JIT
        }
        
        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<For>
    //
    template <>
    struct es_lazy_closure<For>
    : es_trinary_operator
    {
        typedef es_lazy_closure<For> self_t;

        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2,
            IClosure const& arg3
            ) throw()
        : es_trinary_operator(arg1, arg2, arg3)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.collect();
            
            IPrimitive **p_primitive = vm.get_esp();
            IPrimitive& context = vm.get_ecx();
            
            jmp_buf jmp_buf_;
            vm.push_context__(jmp_buf_);
        loop:
            this->op1(vm);
            if (!vm.get_eax().get_value__().operator bool())
                goto end;
            switch (setjmp(jmp_buf_))
            {
            case CT_Normal:
                this->op3(vm);
                break;
            case CT_Return:
                vm.return__();
            case CT_Continue:
                if (VT::Undefined != vm.get_signal().type__())
                    vm.continue__();
                vm.set_esp(p_primitive);
                vm.set_ecx(context);
                vm.push_context__(jmp_buf_);
                break;
            case CT_Break:
                if (VT::Undefined != vm.get_signal().type__())
                    vm.break__();
                vm.set_esp(p_primitive);
                vm.set_ecx(context);
                return;
            case CT_Throw:
                vm.throw__();
            }
            this->op2(vm);
            goto loop;
        end:
            vm.pop_context__();
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            long_absolute_label__ l2;
            long_relative_label__ l1, l3, l4, l5, l6;
            code                
                << push(ebx)
                << call(&es_machine<IPrimitive>::collect)
                
            << l1
                << actor1()             
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)                
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_operator_bool * 4])
                << test(al, al)
                << je(l5)                
                << push(ptr[ebx]) // -4                                
                << push(edi) // -4                
                << push(ebx) // -4
                << push(l2)  // -4       
                << mov(ptr[esi], esp)
                << add(esi, 0x4)            
                << actor3()
                << sub(esi, 0x4)               
                << add(esp, 0x10)
                << actor2()
                << jmp(l1)
                
            << l2
                << cmp(al, CT_Break)
                << jne(l3)            
                << push(eax)
                << push(edx)
                << push(edx)
                << mov(ecx, ptr[edx])
                << call(ptr[ecx + IPrimitive::verb_type__ * 4])
                << cmp(al, VT::Undefined)
                << pop(edx)
                << pop(eax)
                << jne(l4)
                << pop(ebx)  // +4                
                << pop(edi)
                << pop(ecx)
                << mov(ptr[ebx], ecx)
                << jmp(l5)
                
            << l3
                << cmp(al, CT_Continue)
                << jne(l4)                                
                << push(eax)
                << push(edx)
                << push(edx)
                << mov(ecx, ptr[edx])
                << call(ptr[ecx + IPrimitive::verb_type__ * 4])
                << cmp(al, VT::Undefined)
                << pop(edx)
                << pop(eax)
                << jne(l4)
                << pop(ebx)  // +4                
                << pop(edi)
                << pop(ecx)
                << mov(ptr[ebx], ecx)
                << actor2()  
                << jmp(l1)                
                                
            << l4
                << sub(esi, 0x4)                
                << mov(esp, ptr[esi])                
                << ret
                
            << l5
                ;
#endif // ES_TEST_JIT
        }
        
        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<ForIn>
    //
    template <>
    struct es_lazy_closure<ForIn>
    : es_trinary_operator
    {
        typedef es_lazy_closure<ForIn> self_t;

        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2,
            IClosure const& arg3
            ) throw()
        : es_trinary_operator(arg1, arg2, arg3)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.collect();
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            IPrimitive &expression = vm.get_eax().get_value__().reset__();
            
            IPrimitive **p_primitive = vm.get_esp();
            IPrimitive& context = vm.get_ecx();
            
            jmp_buf jmp_buf_;
            vm.push_context__(jmp_buf_);
        loop:
            IPrimitive &key = expression.next__();
            if (VT::Undefined == key.type__())
                goto end;
            vm.top().assign__(key);
            switch (setjmp(jmp_buf_))
            {
            case CT_Normal:
                this->op3(vm);
                goto loop;
            case CT_Return:
                vm.return__();
            case CT_Continue:
                if (VT::Undefined != vm.get_signal().type__())
                    vm.continue__();
                vm.set_esp(p_primitive);
                vm.set_ecx(context);
                vm.push_context__(jmp_buf_);
                goto loop;
            case CT_Break:
                if (VT::Undefined != vm.get_signal().type__())
                    vm.break__();
                vm.set_esp(p_primitive);
                vm.set_ecx(context);
                return;
            case CT_Throw:
                vm.throw__();
            }
        end:
            vm.pop();
            vm.pop_context__();
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            long_absolute_label__ l2;
            long_relative_label__ l1, l3, l4, l5, l6;
            code
                
                // vm.collect();
                << push(ebx)
                << call(&es_machine<IPrimitive>::collect)
                
                // this->op1(vm);
                << actor1()  
                
                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)
                
                // this->op2(vm);
                << actor2() 
                 
                // IPrimitive &expression = vm.get_eax().get_value__().reset__();
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_reset__ * 4])
            << l1
                
                // IPrimitive &key = expression.next__();
                << push(eax)
                << breakpoint
                << pop(eax)
                
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_next__ * 4])
                
                // if (VT::Undefined == key.type__())
                //     goto end;
                << push(eax)
                << push(eax)
                << mov(ecx, ptr[eax])
                << call(ptr[ecx + IPrimitive::verb_type__ * 4])
                << cmp(al, VT::Undefined)
                << je(l5)
                
                // vm.top().assign__(key);
                << mov(ecx, ptr[ebx])
                << mov(eax, ptr[ecx - 4])
                << push(eax)
                << mov(ecx, ptr[eax])
                << call(ptr[ecx + IPrimitive::verb_assign__ * 4])
                << push(ptr[ebx]) // -4                                
                << push(edi) // -4                
                << push(ebx) // -4
                << push(l2)  // -4
                << mov(ptr[esi], esp)
                << add(esi, 0x4)            
                << actor3()
                << sub(esi, 0x4)               
                << add(esp, 0x10)
                << jmp(l1)
                
            << l2
                << cmp(al, CT_Break)
                << jne(l3)            
                << push(eax)
                << push(edx)
                << push(edx)
                << mov(ecx, ptr[edx])
                << call(ptr[ecx + IPrimitive::verb_type__ * 4])
                << cmp(al, VT::Undefined)
                << pop(edx)
                << pop(eax)
                << jne(l4)
                << pop(ebx)  // +4                
                << pop(edi)
                << pop(ecx)
                << mov(ptr[ebx], ecx)
                << jmp(l6)
                
            << l3
                << cmp(al, CT_Continue)
                << jne(l4)                                
                << push(eax)
                << push(edx)
                << push(edx)
                << mov(ecx, ptr[edx])
                << call(ptr[ecx + IPrimitive::verb_type__ * 4])
                << cmp(al, VT::Undefined)
                << pop(edx)
                << pop(eax)
                << jne(l4)
                << pop(ebx)  // +4                
                << pop(edi)
                << pop(ecx)
                << mov(ptr[ebx], ecx)
                << jmp(l1)                
                                
            << l4
                << sub(esi, 0x4)                
                << mov(esp, ptr[esi])                
                << ret
                
            << l5
                << add(esp, 4)

            << l6
                << add(esp, 4)
                ;
#endif // ES_TEST_JIT
        }
        
        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Switch>
    //
    // @brief binded functor with 2 arguments.
    //
    // grammer:
    //
    //   switch, (, [Expression], ), {, ( [Case] | [Default] )
    //
    // arguments:
    //   arg1: an action for [Expression] action.
    //   arg2: an action for [Case] or [Default].
    //
    // action:
    //   precondition: none.
    //   postcondition: none.
    //
    //                                          +-----------+
    //                                          | false     |
    //                        +-----------+     +-----------+
    //                    op1 | lhs value |     | lhs value |
    //      +-----------+ --> +-----------+ --> +-----------+
    //      |           |     |           |     |           |
    //
    //      +-----------+
    //      | signal    |
    //      +-----------+
    //  op2 | lhs value |
    //  --> +-----------+ --> +-----------+
    //      |           |     |           |
    //
    template <>
    struct es_lazy_closure<Switch>
    : es_binary_operator
    {
        typedef es_lazy_closure<Switch> self_t;

        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.collect();
            typedef IPrimitive::string_t string_t;
            this->op1(vm);
            
            IPrimitive **p_primitive = vm.get_esp();
            IPrimitive& context = vm.get_ecx();
            
            jmp_buf jmp_buf_;
            vm.push_context__(jmp_buf_);
            switch (setjmp(jmp_buf_))
            {
            case CT_Normal:
                vm.push(vm.get_eax().get_value__());
                vm.set_eax(es_boolean<string_t>::create_instance(false));
                this->op2(vm);
                vm.set_esp(p_primitive);
                vm.set_ecx(context);
                goto end;
            case CT_Return:
                vm.return__();
            case CT_Continue:
                vm.set_signal(*new es_syntax_error<string_t>(L"bad continue statement."));
                vm.throw__();
            case CT_Break:
                if (VT::Undefined != vm.get_signal().type__())
                    vm.break__();
                vm.set_esp(p_primitive);
                vm.set_ecx(context);
                return;
            case CT_Throw:
                vm.throw__();
            }
        end:
            vm.pop_context__();
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            long_absolute_label__ l1;
            long_relative_label__ l2, l3, l4;
            code
                // this->op1(vm);
                << actor1()                                  
                // vm.push(vm.get_eax().get_value__());
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])                
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)
                // vm.set_eax(es_boolean<string_t>::create_instance(false));
                << mov(eax, false)
                << push(ptr[ebx]) // -4                    
                << push(edi) // -4
                << push(ebx) // -4
                << push(l1)  // -4       
                << mov(ptr[esi], esp)
                << add(esi, 0x4)
                << actor2()                  
                << sub(esi, 0x4)           
                << mov(esp, ptr[esi])                
                << add(esp, 0x4)
                << jmp(l3)
                
            << l1
                << cmp(al, CT_Break)
                << jne(l2)
                << push(eax)
                << push(edx)                
                << push(edx)
                << mov(eax, ptr[edx])
                << call(ptr[eax + IPrimitive::verb_type__ * 4])
                << cmp(al, VT::Undefined)                
                << pop(edx)
                << pop(eax)
                << jne(l4)
                << jmp(l3)
                
            << l2
                << sub(esi, 0x4)                 
                << mov(esp, ptr[esi])                
                << ret
                
            << l3
                << pop(ebx)  // +4                
                << pop(edi)
                << pop(ecx)
                << mov(ptr[ebx], ecx)
            
            << l4
                ;
#endif // ES_TEST_JIT
        }
        
        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Case>
    //
    // @brief binded functor with 3 arguments.
    //
    // grammer:
    //
    //   case, [Expression], :, ![StatementList], ( [Case] | [Default] )
    //
    // arguments:
    //   arg1: an action for [Expression] action.
    //   arg2: an action for [StatementList] or epsilon.
    //   arg3: an action for [Case] or [Default].
    //
    // action:
    //   precondition:
    //     the stack top value is a Boolean object that means whether any
    //     above case-clause match with lhs-value.
    //     the second value from the top is the lhs-value.
    //   postcondition:
    //     the stack top value is a Boolean object that means the lhs-value
    //     strictly equals to actioin for 1st argument.
    //     the second value from the top is lhs-value.
    //
    //                                            +-------------+
    //                                            | signal'     |
    //                                            +-------------+
    //                  false                     | lhs value   |
    //   bool(signal)?  ----> +-------------+ --> +-------------+
    //  +-------------+       |             |     |             |
    //  | signal      |
    //  +-------------+       +-------------+
    //  | lhs value   |       | signal      |
    //  +-------------+       +-------------+     +-------------+
    //  |             | true  | lhs value   |     | lhs value   |
    //                  ----> +-------------+ --> +-------------+
    //                        |             |     |             |
    //
    template <>
    struct es_lazy_closure<Case>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Case> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            if (vm.get_eax().operator bool())
                goto l1;
            this->op1(vm);
            if (vm.top().strict_eq__(vm.get_eax().get_value__()).operator bool())
                goto l1;
            vm.set_eax(es_boolean<IPrimitive::string_t>::create_instance(false));
            goto l2;
        l1:
            this->op2(vm);
            vm.set_eax(es_boolean<IPrimitive::string_t>::create_instance(true));            
        l2:
            ;
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            long_relative_label__ l1, l2;
            code
                // if (vm.get_eax().operator bool())
                //     goto l1;
                << test(al, al)
                << jne(l1)                
                // this->op1(vm);
                << actor1()
                // if (vm.top().strict_eq__(vm.get_eax().get_value__()).operator bool())
                //    goto l1;
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(ecx, ptr[ebx])
                << mov(eax, ptr[ecx - 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_strict_eq__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_operator_bool * 4])
                << test(al, al)
                << jne(l1)                
                // vm.set_eax(es_boolean<IPrimitive::string_t>::create_instance(false));
                << mov(eax, false)
                // goto l2;                            
                << jmp(l2)
                
            << l1
                // this->op2(vm);
                << actor2()                
                // vm.set_eax(es_boolean<IPrimitive::string_t>::create_instance(true));
                << mov(eax, true)
                      
            << l2
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Default>
    //
    template <>
    struct es_lazy_closure<Default>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Default> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            if (vm.get_eax().operator bool())
                goto l1;
            vm.set_eax(es_boolean<IPrimitive::string_t>::create_instance(false));
            this->op2(vm);
            if (vm.get_eax().get_value__().operator bool())
                goto l2;
        l1:
            this->op1(vm);
            vm.set_eax(es_boolean<IPrimitive::string_t>::create_instance(true));
            this->op2(vm);
        l2:
            ;
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            long_relative_label__ l1, l2;
            code
                // if (vm.pop().operator bool())
                //    goto end;
                << test(al, al)                
                << jne(l1)                          
                // vm.set_eax(es_boolean<IPrimitive::string_t>::create_instance(false));
                << mov(eax, false)
                // this->op2(vm);
                << actor2()                
                // if (vm.get_eax().get_value__().operator bool())
                //     goto l2;
                << test(al, al)                
                << jne(l2)
                
            << l1                
                // this->op1(vm);
                << actor1()                
                // vm.set_eax(es_boolean<IPrimitive::string_t>::create_instance(true));
                << mov(eax, true)
                // this->op2(vm);
                << actor2()
                
            << l2
                ;
#endif // ES_TEST_JIT
        }
        
        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<With>
    //
    template <>
    struct es_lazy_closure<With>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<With> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            IPrimitive& context = vm.get_ecx();
            vm.push(context);
            vm.set_ecx(es_activation_object<IPrimitive::string_t>::create_instance(
                vm.get_eax().get_value__(), context));
            this->op2(vm);
            vm.set_ecx(vm.pop());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()
                                
                // IPrimitive& context = vm.get_ecx();                                
                // vm.set_ecx(es_activation_object<IPrimitive::string_t>::create_instance(
                //     vm.get_eax().get_value__(), context));
                << push(edi)
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << call(&es_activation_object<IPrimitive::string_t>::create_instance)
                << mov(edi, eax)                
                
                // vm.push(context);
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], edi)
                << add(ptr[ebx], 4)
                
                // this->op2(vm);
                << actor2()
                
                // vm.set_ecx(vm.pop());
                << sub(ptr[ebx], 4)
                << mov(ecx, ptr[ebx])
                << mov(edi, ptr[ecx])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Identifier>
    //
    //                    +-----------+
    //                    | value     |
    //  +-----------+ --> +-----------+
    //  |           |     |           |
    //
    template <>
    struct es_lazy_closure<Identifier>
    : es_leaf_operator
    {
        template <typename iteratorT>
        explicit es_lazy_closure(
            iteratorT const first,
            iteratorT const last
            ) throw()
        : value_(first, last)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_eax(vm.get_ecx().get__(value_));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.set_eax(vm.get_ecx().get__(value_));
                << push(&value_)
                << push(edi)
                << mov(ecx, ptr[edi])
                << call(ptr[ecx + IPrimitive::verb_get__ * 4])
                ;
#endif // ES_TEST_JIT
        }
        
        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Reference;
        }
        
    private:
        es_const_string<wchar_t> const value_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Bracket>
    //
    // @brief binded functor with 1 arguments.
    //
    // grammer:
    //
    //   [context-1], '.', ( Identifer | '[', Expression, ']' )
    //
    // arguments:
    //   arg1: an action for [Identifer] or [Expression] action.
    //
    // action:
    //   precondition:
    //     the stack top value is [context-1] object.
    //     the second value from the top is [Identifer] or [Expression]
    //     object.
    //   postcondition:
    //     the stack top value is new context object.
    //     the second value from the top is context-2 object.
    //
    // state of the stack:
    //
    //                    +-----------+
    //                    | value     |
    //  +-----------+     +-----------+                       +-----------+
    //  | context-2 |     | context-2 |                       | context-3 |
    //  +-----------+     +-----------+     +-----------+     +-----------+
    //  | context-1 | op1 | context-1 |     | context-2 |     | context-2 |
    //  +-----------+ --> +-----------+ --> +-----------+ --> +-----------+
    //  |           |     |           |     |           |     |           |
    //
    template <>
    struct es_lazy_closure<Bracket>
    : es_unary_operator
    {
        typedef es_lazy_closure<Bracket> self_t;

        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.push(vm.get_eax());
            this->op1(vm);
            IPrimitive& value = vm.get_eax().get_value__();
            IPrimitive& context = vm.pop();
            vm.pop();
            vm.push(context);
            vm.set_eax(context.get_value__().get_by_value__(value));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op1(vm);
                << actor1()

                // IPrimitive& value = vm.get_eax().get_value__();
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)

                // IPrimitive& context = vm.pop();
                << sub(ptr[ebx], 4)
                << mov(ecx, ptr[ebx])
                << mov(eax, ptr[ecx])

                // vm.pop();
                << sub(ptr[ebx], 4)

                // vm.push(context);
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // vm.set_eax(context.get_value__().get_by_value__(value));
                << push(eax)
                << mov(ecx, ptr[eax])
                << call(ptr[ecx + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(ecx, ptr[eax])
                << call(ptr[ecx + IPrimitive::verb_get_by_value__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Primitive;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Member>
    //
    // @brief binded functor with 1 arguments.
    //
    // grammer:
    //
    //   [context-1], '.', Identifer
    //
    // arguments:
    //   arg1: the first position of [Identifer].
    //   arg2: the last position of [Identifer].
    //
    // action:
    //   precondition:
    //     the stack top value is [context-1] object.
    //   postcondition:
    //     the stack top value is [context-2] object.
    //
    // state of the stack:
    //
    //  +-----------+                       +-----------+
    //  | context-1 |                       | context-2 |
    //  +-----------+ --> +-----------+ --> +-----------+
    //  |           |     |           |     |           |
    //
    template <>
    struct es_lazy_closure<Member>
    : es_leaf_operator
    {
    private:
        typedef es_lazy_closure<Member> self_t;
        
    public:
        template <typename iteratorT>
        explicit es_lazy_closure(
            iteratorT const first,
            iteratorT const last
            ) throw()
        : value_(first, last)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.pop();
            IPrimitive& context = vm.get_eax().get_value__();
            vm.push(context);
            vm.set_eax(context.get__(value_));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // IPrimitive& context = vm.get_eax().get_value__();
                << push(eax)
                << mov(ecx, ptr[eax])
                << call(ptr[ecx + IPrimitive::verb_get_value__ * 4])

                // vm.pop();
                << sub(ptr[ebx], 4)

                // vm.push(context);
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // vm.set_eax(context.get__(value_));
                << push(&value_)
                << push(eax)
                << mov(ecx, ptr[eax])
                << call(ptr[ecx + IPrimitive::verb_get__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Primitive;
        }
        
    private:
        es_const_string<wchar_t> const value_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Call>
    //
    // @brief binded functor with 1 arguments.
    //
    // grammer:
    //
    //   [context-1], '.', Identifer
    //
    // arguments:
    //   arg1: the first position of [Identifer].
    //   arg2: the last position of [Identifer].
    //
    // action:
    //   precondition:
    //     none.
    //   postcondition:
    //     stack position should be fixed.
    //
    // state of the stack:
    //
    //                    +-----------+     +-----------+
    //                    | context   | op1 | context   |
    //  +-----------+ --> +-----------+ --> +-----------+ --> +-----------+
    //  |           |     |           |     |           |     |           |
    //
    template <>
    struct es_lazy_closure<Call>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<Call> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.push(vm.get_ecx().get_prototype__());
            this->op1(vm);
            vm.pop();
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.push(vm.get_ecx().get_prototype__())
                << push(edi)
                << mov(eax, ptr[edi])
                << call(ptr[eax + IPrimitive::verb_get_prototype__ * 4])
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op1(vm);
                << actor1()

                // vm.pop();
                << sub(ptr[ebx], 4)
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Primitive;
        }        
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Args>
    //
    // @brief binded functor with 1 arguments.
    //
    // grammer:
    //
    //   '(' , ( Argument | ')' )
    //
    // arguments:
    //   arg1: the first position of [Identifer].
    //   arg2: the last position of [Identifer].
    //
    // action:
    //   precondition:
    //     none.
    //   postcondition:
    //     stack position should be fixed.
    //
    // state of the stack:
    //                                     +-----------+
    //                                     | arguments |
    //                    +-----------+    +-----------+
    //                    | method    |    | method    | op1
    //   +-----------+ -> +-----------+ -> +-----------+ --> +-----------+
    //   | context   |    | context   |    | context   |     | context   |
    //
    template <>
    struct es_lazy_closure<Args>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<Args> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            IPrimitive& method = vm.get_eax();
            vm.push(method);
            vm.push(es_arguments<IPrimitive::string_t>::create_instance(method));
            this->op1(vm);
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // IPrimitive& method = vm.get_eax();
                // vm.push(method);
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // vm.push(es_arguments<IPrimitive::string_t>::create_instance(method));
                << push(eax)
                << call(&es_arguments<IPrimitive::string_t>::create_instance)
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op1(vm);
                << actor1()
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Primitive;
        }        
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Arg>
    //
    // arguments:
    //   arg1: current token(argument).
    //   arg2: next token.
    //
    // action:
    //   precondition:
    //     none.
    //   postcondition:
    //     stack position should decrease by 2.
    //
    // state of the stack:
    //  +-----------+     +-----------+
    //  | arguments |     | arguments'|
    //  +-----------+     +-----------+
    //  | method    | op1 | method    | op2
    //  +-----------+ --> +-----------+ --> +-----------+
    //  | context   |     | context   |     | context   |
    //
    template <>
    struct es_lazy_closure<Arg>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Arg> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.top().push__(vm.get_eax().get_value__());
            this->op2(vm);
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.top().push__(vm.get_eax().get_value__());
                << push(eax)
                << mov(ecx, ptr[eax])
                << call(ptr[ecx + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(ecx, ptr[ebx])
                << mov(eax, ptr[ecx - 4])
                << push(eax)
                << mov(ecx, ptr[eax])
                << call(ptr[ecx + IPrimitive::verb_push__ * 4])

                // this->op2(vm);
                << actor2()
                ;
#endif // ES_TEST_JIT
        }
        
        virtual es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Primitive;
        }        
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<ArgEnd>
    //
    //  +-----------+
    //  | arguments |
    //  +-----------+
    //  | method    |
    //  +-----------+                       +-----------+
    //  | context   |                       | context'  |
    //  +-----------+ --> +-----------+ --> +-----------+
    //  |           |     |           |     |           |
    //
    template <>
    struct es_lazy_closure<ArgEnd>
    : es_leaf_operator
    {
    private:
        typedef es_lazy_closure<ArgEnd> self_t;

    public:
        es_lazy_closure() throw()
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            IPrimitive& arguments = vm.pop();
            IPrimitive& method = vm.pop();
            vm.set_eax(method.call__(vm.pop().get_value__(), arguments));
            vm.push(vm.get_ecx().get_prototype__());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // IPrimitive& arguments = vm.pop();
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << push(ptr[eax])
                // IPrimitive& method = vm.pop();
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)            
                << mov(ecx, &callstack.get_iterator())
                << mov(ptr[ecx], esi)                
                << push(edi)
                << push(ebx)
                << call(&es_machine<IPrimitive>::set_ecx) // +4
                // vm.set_eax(method.call__(vm.pop().get_value__(), arguments));
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << pop(ecx)
                << push(eax)
                << push(ecx)
                << mov(eax, ptr[ecx])
                << call(ptr[eax + IPrimitive::verb_call__ * 4])
                << push(eax)
                << mov(ecx, &callstack.get_iterator())
                << mov(esi, ptr[ecx])                
                << push(ebx)
                << call(&es_machine<IPrimitive>::get_ecx) // +4
                << mov(edi, eax)
                // vm.push(vm.get_ecx().get_prototype__());
                << push(edi)
                << mov(eax, ptr[edi])
                << call(ptr[eax + IPrimitive::verb_get_prototype__ * 4])
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)
                << pop(eax)
                ;
#endif // ES_TEST_JIT
        }
        
        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Primitive;
        }        
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<NewArgEnd>
    //
    //  +-------------+
    //  | arguments   |
    //  +-------------+                         +-------------+
    //  | constructor |                         | result      |
    //  +-------------+     +-------------+     +-------------+
    //  | context     |     | context     |     | context'    |
    //  +-------------+ --> +-------------+ --> +-------------+
    //  |             |     |             |     |             |
    //
    template <>
    struct es_lazy_closure<NewArgEnd>
    : es_leaf_operator
    {
    private:
        typedef es_lazy_closure<NewArgEnd> self_t;
    
    public:
        es_lazy_closure() throw()
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            IPrimitive& arguments = vm.pop();
            IPrimitive& constructor = vm.pop().get_value__();
            vm.pop();
            vm.set_eax(constructor.construct__(arguments));
            vm.push(vm.get_ecx().get_prototype__());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // IPrimitive& arguments = vm.pop();
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << push(ptr[eax])

                // IPrimitive& constructor = vm.pop().get_value__();
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)

                // vm.pop();
                << sub(ptr[ebx], 4)

                // vm.set_eax(constructor.construct__(arguments));
                << pop(eax)
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_construct__ * 4])
                << push(eax)

                // vm.push(vm.get_ecx().get_prototype__());
                << push(edi)
                << mov(eax, ptr[edi])
                << call(ptr[eax + IPrimitive::verb_get_prototype__ * 4])
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                << pop(eax)
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Primitive;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<String>
    //
    //                    +-----------+
    //                    | value     |
    //  +-----------+ --> +-----------+
    //  |           |     |           |
    //
    template <>
    struct es_lazy_closure<String>
    : es_leaf_operator
    {
    private:
        typedef es_lazy_closure<String> self_t;
        
    public:
        template <typename iteratorT>
        explicit es_lazy_closure(
            iteratorT const first,
            iteratorT const last
            ) throw()
        : value_(first, last)
        {
            value_.addref__();
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_eax(value_);
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                << mov(eax, &value_)
                ;
#endif // ES_TEST_JIT
        }
        
        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return value_;
        }
        
    private:
        es_string<IPrimitive::string_t> mutable value_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<SingleQuotedString>
    //
    //                    +-----------+
    //                    | value     |
    //  +-----------+ --> +-----------+
    //  |           |     |           |
    //
    template <>
    struct es_lazy_closure<SingleQuotedString>
    : es_leaf_operator
    {
    private:
        typedef es_lazy_closure<SingleQuotedString> self_t;
        
    public:
        template <typename iteratorT>
        explicit es_lazy_closure(
            iteratorT const first,
            iteratorT const last
            ) throw()
        : value_(first, last)
        {
            value_.addref__();
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_eax(value_);
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                << mov(eax, &value_)
                ;
#endif // ES_TEST_JIT
        }
        
        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return value_;
        }
        
    private:
        es_string<IPrimitive::string_t> mutable value_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<DoubleQuotedString>
    //
    //                    +-----------+
    //                    | value     |
    //  +-----------+ --> +-----------+
    //  |           |     |           |
    //
    template <>
    struct es_lazy_closure<DoubleQuotedString>
    : es_leaf_operator
    {
    private:
        typedef es_lazy_closure<DoubleQuotedString> self_t;
        
    public:
        template <typename iteratorT>
        explicit es_lazy_closure(
            iteratorT const first,
            iteratorT const last
            ) throw()
        : value_(first, last)
        {
            value_.addref__();
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_eax(value_);
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                << mov(eax, &value_)
                ;
#endif // ES_TEST_JIT
        }
        
        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return value_;
        }
        
    private:
        es_string<IPrimitive::string_t> mutable value_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<NullString>
    //
    //                    +-----------+
    //                    | value     |
    //  +-----------+ --> +-----------+
    //  |           |     |           |
    //
    template <>
    struct es_lazy_closure<NullString>
    : es_leaf_operator
    {
    private:
        typedef es_lazy_closure<NullString> self_t;
        
    public:
        es_lazy_closure() throw()
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_eax(es_string<IPrimitive::string_t>::create_null_string());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.set_eax(es_string<IPrimitive::string_t>::create_null_string);
                << mov(eax, &es_string<IPrimitive::string_t>::create_null_string())
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return es_string<IPrimitive::string_t>::create_null_string();
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Null>
    //
    //                    +-----------+
    //                    | value     |
    //  +-----------+ --> +-----------+
    //  |           |     |           |
    //
    template <>
    struct es_lazy_closure<Null>
    : es_leaf_operator
    {
    private:
        typedef es_lazy_closure<Null> self_t;
        
    public:
        es_lazy_closure() throw()
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_eax(es_null<IPrimitive::string_t>::create_instance());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.set_eax(es_null<IPrimitive::string_t>::create_instance());
                << mov(eax, &es_null<IPrimitive::string_t>::create_instance())
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return es_null<IPrimitive::string_t>::create_instance();
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<RegExp>
    //
    // @brief binded functor with 0 arguments.
    //
    // grammer:
    //   '/', ( '\', . | [^*/] ), ( '\', . | [^/] )* , '/' , !(
    //      'g', !('i', !'m' | 'm', !'i')
    //      | 'i', !('m', !'g' | 'g', !'m')
    //      | 'm', !('g', !'i' | 'i', !'g') )
    //
    // arguments:
    //   no arguments
    //
    // action:
    //   precondition:
    //     none.
    //   postcondition:
    //     The address to the regexp object is stored in the A-register.
    //
    // state of the stack:
    //
    //  +-----------+ --> +-----------+
    //  |           |     |           |
    //
    // affected register:
    //  A-register.
    //
    template <>
    struct es_lazy_closure<RegExp>
    : es_leaf_operator
    {
        template <typename T1, typename T2>
        explicit es_lazy_closure(
            T1 first1,
            T1 last1,
            T2 first2,
            T2 last2
            ) throw()
        : value_(es_const_string<wchar_t>(first1, last1)
                , es_const_string<wchar_t>(first2 + 1, last2))
        {
            value_.addref__();
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        template <typename iteratorT>
        explicit es_lazy_closure(
            iteratorT first1,
            iteratorT last1,
            iteratorT first2,
            iteratorT last2
            ) throw()
        : value_(es_const_string<wchar_t>(first1, last1)
                , es_const_string<wchar_t>(first2 + 1, last2))
        {
            value_.addref__();
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_eax(value_);
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.set_eax(value_);
                << mov(eax, &value_)
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return value_;
        }
        
    private:
        mutable es_regexp<IPrimitive::string_t> value_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Undefined>
    //
    // @brief binded functor with 0 arguments.
    //
    // grammer:
    //   'undefined'
    //
    // arguments:
    //   no arguments
    //
    // action:
    //   precondition:
    //     none.
    //   postcondition:
    //     The address to the undefined object is stored in the A-register.
    //
    // state of the stack:
    //
    //  +-----------+ --> +-----------+
    //  |           |     |           |
    //
    // affected register:
    //  A-register.
    //
    template <>
    struct es_lazy_closure<Undefined>
    : es_leaf_operator
    {
        es_lazy_closure() throw()
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            static IUndefined& value_
                = es_undefined<IPrimitive::string_t>::create_instance();
            vm.set_eax(value_);
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.set_eax(value_);
                << mov(eax, &es_undefined<IPrimitive::string_t>::create_instance())
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return es_undefined<IPrimitive::string_t>::create_instance();
        }
        
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Number>
    //
    // @brief binded functor with 0 arguments.
    //
    // arguments:
    //   no arguments
    //
    // action:
    //   precondition:
    //     none.
    //   postcondition:
    //     The address to the number object is stored in the A-register.
    //
    // state of the stack:
    //
    //  +-----------+ --> +-----------+
    //  |           |     |           |
    //
    // affected register:
    //  A-register.
    //
    template <>
    struct es_lazy_closure<Number>
    : es_leaf_operator
    {
        explicit es_lazy_closure(ecmascript::uint32_t d) throw()
        : value_(d)
        {
            value_.addref__();
        }

        explicit es_lazy_closure(double d) throw()
        : value_(d)
        {
            value_.addref__();
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_eax(value_);
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.set_eax(es_number<IPrimitive::string_t>::create_instance(value_));
                << mov(eax, &value_)
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return value_;
        }
        
    private:
        es_number<IPrimitive::string_t> mutable value_;
    };
    
    struct es_small_integer 
    {
        
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<SmallInteger>
    //
    // @brief binded functor with 0 arguments.
    //
    // arguments:
    //   no arguments
    //
    // action:
    //   precondition:
    //     none.
    //   postcondition:
    //     The address to the number object is stored in the A-register.
    //
    // state of the stack:
    //
    //  +-----------+ --> +-----------+
    //  |           |     |           |
    //
    // affected register:
    //  A-register.
    //
    template <>
    struct es_lazy_closure<SmallInteger>
    : es_leaf_operator
    {
        explicit es_lazy_closure(ecmascript::uint32_t d) throw()
        : value_((value_ << 2) | 0x3)
        {
        }

        explicit es_lazy_closure(double d) throw()
        : value_(ecmascript::uint32_t(value_ << 2) | 0x3)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_eax(*p_value_);
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.set_eax(es_number<IPrimitive::string_t>::create_instance(value_));
                << mov(eax, value_)
                ;
#endif // ES_TEST_JIT
        }

    private:
        union {
            ecmascript::int32_t value_;    
            IPrimitive * p_value_;    
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<True>
    //
    // @brief binded functor with 0 arguments.
    //
    // grammer:
    //   'true'
    //
    // arguments:
    //   no arguments
    //
    // action:
    //   precondition:
    //     none.
    //   postcondition:
    //     The address to the true object is stored in the A-register.
    //
    // state of the stack:
    //
    //  +-----------+ --> +-----------+
    //  |           |     |           |
    //
    // affected register:
    //  A-register.
    //
    template <>
    struct es_lazy_closure<True>
    : es_leaf_operator
    {
        es_lazy_closure() throw()
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_eax(value_);
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.set_eax(value_);
                << mov(eax, &value_)
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return value_;
        }
        
    private:
        static IBoolean& value_;
    };

    IBoolean& es_lazy_closure<True>::value_
        = es_boolean<IPrimitive::string_t>::create_instance(true);

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<False>
    //
    // @brief binded functor with 0 arguments.
    //
    // grammer:
    //   'false'
    //
    // arguments:
    //   no arguments
    //
    // action:
    //   precondition:
    //     none.
    //   postcondition:
    //     The address to the false object is stored in the A-register.
    //
    // state of the stack:
    //
    //  +-----------+ --> +-----------+
    //  |           |     |           |
    //
    // affected register:
    //  A-register.
    //
    template <>
    struct es_lazy_closure<False>
    : es_leaf_operator
    {
        es_lazy_closure() throw()
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_eax(value_);
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.set_eax(value_);
                << mov(eax, &value_)
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return value_;
        }
        
    private:
        static IBoolean& value_;
    };

    IBoolean& es_lazy_closure<False>::value_
        = es_boolean<IPrimitive::string_t>::create_instance(false);


    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Parameter>
    //
    template <>
    struct es_lazy_closure<Parameter>
    : es_leaf_operator
    {
    private:
        typedef es_lazy_closure<Parameter> self_t;

    public:
        template <typename iteratorT>
        explicit es_lazy_closure(
            iteratorT const first,
            iteratorT const last
            ) throw()
        : value_(first, last)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.get_ecx().put__(value_, vm.pop());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.get_ecx().put__(value_, vm.pop());
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << push(ptr[eax])
                << push(&value_)
                << push(edi)
                << mov(eax, ptr[edi])
                << call(ptr[eax + IPrimitive::verb_put__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
        
    private:
        const_string_t const value_;
    };
    
    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<FunctionRoot>
    //
    // @brief binded functor with 1 arguments.
    //
    // arguments:
    //   no arguments
    //
    // action:
    //   precondition:
    //     none.
    //   postcondition:
    //     The address to the managed function object is stored
    //     in the A-register.
    //
    // state of the stack:
    //
    //  +-----------+ --> +-----------+
    //  |           |     |           |
    //
    // affected register:
    //  A-register.
    //
    template <>
    struct es_lazy_closure<FunctionRoot>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<FunctionRoot> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& closure,
            IClosure const& arguments
            ) throw()
        : es_binary_operator(closure, arguments)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
#ifdef ES_TEST_JIT
            compile_and_run(*this, vm);
#else
            typedef IPrimitive::string_t string_t;
            
            this->op1(vm);
            
            IPrimitive **p_primitive = vm.get_esp();
            IPrimitive& context = vm.get_ecx();
            
            jmp_buf jmp_buf_;
            vm.push_context__(jmp_buf_);
            
            switch (setjmp(jmp_buf_))
            {
            case CT_Normal:
                this->op2(vm);
                vm.set_eax(es_undefined<string_t>::create_instance());
                goto end;
            case CT_Return:
                vm.set_esp(p_primitive);
                vm.set_ecx(context);
                vm.set_eax(vm.get_signal());
                return;
            case CT_Continue:
                vm.set_signal(*new es_syntax_error<string_t>(L"bad continue statement."));
                vm.throw__();
            case CT_Break:
                vm.set_signal(*new es_syntax_error<string_t>(L"bad break statement."));
                vm.throw__();
            case CT_Throw:
                vm.throw__();
            }
        end:
            vm.pop_context__();
#endif // ES_TEST_JIT
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            long_absolute_label__ l1;
            long_relative_label__ l2, l3;
            code
                << actor1()
                << push(ptr[ebx]) // -4                                
                << push(edi) // -4                
                << push(ebx) // -4
                << push(l1)  // -4
                << mov(ptr[esi], esp)
                << add(esi, 0x4)
                << actor2()
                << push(&es_undefined<IPrimitive::string_t>::create_instance()) 
                << push(ebx) 
                << call(&es_machine<IPrimitive>::set_eax)
                << sub(esi, 0x4)               
                << add(esp, 0x10)
                << jmp(l3)
                
            << l1
                << cmp(al, CT_Return)
                << jne(l2)
                << pop(ebx)  // +4                
                << pop(edi)
                << pop(ecx)
                << mov(ptr[ebx], ecx)
                << push(edx)
                << push(ebx) 
                << call(&es_machine<IPrimitive>::set_eax)
                << jmp(l3)
                
            << l2
                << sub(esi, 0x4)  
                << mov(esp, ptr[esi])
                << pop(ecx)
                << jmp(ecx)
                
            << l3                
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Func>
    //
    // @brief binded functor with 2 arguments.
    //
    // arguments:
    //   no arguments
    //
    // action:
    //   precondition:
    //     none.
    //   postcondition:
    //     The address to the managed function object is stored
    //     in the A-register.
    //
    // state of the stack:
    //
    //  +-----------+ --> +-----------+
    //  |           |     |           |
    //
    // affected register:
    //  A-register.
    //
    template <>
    struct es_lazy_closure<Func>
    : es_leaf_operator
    {
        explicit es_lazy_closure(
            IClosure const& closure
        ) throw()
        : closure_(closure)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_eax(
                es_managed_function<IPrimitive::string_t>::create_instance(
                    vm, vm.get_ecx(), closure_));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code                
                // vm.set_eax(
                //     es_managed_function<IPrimitive::string_t>::create_instance(
                //         vm, vm.get_ecx(), closure_, arguments_));
                << push(&closure_)
                << push(edi)
                << push(ebx)
                << call(es_managed_function<IPrimitive::string_t>::create_instance)
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Function;
        }
        
    private:
        IClosure const& closure_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<TryCatch>
    //    
    template <>
    struct es_lazy_closure<TryCatch>
    : es_trinary_operator
    {
    private:
        typedef es_lazy_closure<TryCatch> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2,
            IClosure const& arg3
            ) throw()
        : es_trinary_operator(arg1, arg2, arg3)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            IPrimitive **p_primitive = vm.get_esp();
            IPrimitive& context = vm.get_ecx();
            
            jmp_buf jmp_buf_;
            vm.push_context__(jmp_buf_);
            switch (setjmp(jmp_buf_))
            {
            case CT_Normal:
                this->op1(vm);
                goto end;
            case CT_Return:
                vm.return__();
            case CT_Continue:
                vm.continue__();
            case CT_Break:
                vm.break__();
            case CT_Throw:
                vm.set_esp(p_primitive);
                vm.set_ecx(context);
                vm.push(vm.get_signal());
                this->op2(vm);
                vm.get_eax().assign__(vm.pop());
                this->op3(vm);
                return;
            }
        end:
            vm.pop_context__();
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            long_absolute_label__ l1;
            long_relative_label__ l2, l3;
            code                
                << push(ptr[ebx]) // -4                                
                << push(edi) // -4                
                << push(ebx) // -4
                << push(l1)  // -4                         
                << mov(ptr[esi], esp)                
                << add(esi, 0x4)                
                << actor1()                
                << sub(esi, 0x4)                               
                << add(esp, 0x10)
                << jmp(l3)

            << l1
                << cmp(al, CT_Throw)
                << jne(l2)                
                << pop(ebx)  // +4                
                << pop(edi)
                << pop(ecx)
                << mov(ptr[ebx], ecx)
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], edx)
                << add(ptr[ebx], 4)                                
                // this->op2(vm);
                << actor2()             
                // vm.get_eax().assign__(vm.pop());             
                << sub(ptr[ebx], 4)
                << mov(ecx, ptr[ebx])
                << push(ptr[ecx])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_assign__ * 4])                
                // this->op3(vm);
                << actor3()                
                << jmp(l3)

            << l2
                << sub(esi, 0x4)                  
                << mov(esp, ptr[esi])                
                << ret
                
            << l3
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<TryFinally>
    //
    template <>
    struct es_lazy_closure<TryFinally>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<TryFinally> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.collect();
            jmp_buf jmp_buf_;
            
            IPrimitive **p_primitive = vm.get_esp();
            IPrimitive& context = vm.get_ecx();
            
            vm.push_context__(jmp_buf_);
            switch (setjmp(jmp_buf_))
            {
            case CT_Normal:
                this->op1(vm);
                break;
            case CT_Return:
                vm.return__();
            case CT_Continue:
                vm.continue__();
            case CT_Break:
                vm.break__();
            case CT_Throw:
                vm.set_esp(p_primitive);
                vm.set_ecx(context);
                this->op2(vm);
                vm.throw__();
            }
            vm.pop_context__();
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            long_absolute_label__ l1;
            long_relative_label__ l2, l3;
            code                
                << push(ptr[ebx]) // -4                                
                << push(edi) // -4           
                << push(ebx) // -4
                << push(l1)  // -4                         
                << mov(ptr[esi], esp)                
                << add(esi, 0x4)
                << actor1()                
                << sub(esi, 0x4)               
                << add(esp, 0x10)
                << jmp(l3)
                
            << l1
                << cmp(al, CT_Throw)
                << jne(l2)                
                << pop(ebx)  // +4                
                << pop(edi)                
                << pop(ecx)
                << mov(ptr[ebx], ecx)
                << push(edx)
                << actor2() 
                << pop(edx)
                << mov(eax, CT_Throw)

            << l2
                << sub(esi, 0x4)
                << mov(esp, ptr[esi])
                << ret
                
            << l3
                ;
            //*/
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Label>
    //
    template <>
    struct es_lazy_closure<Label>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<Label> self_t;

    public:
        template <typename iteratorT>
        explicit es_lazy_closure(
            IClosure const& arg1,
            iteratorT const first,
            iteratorT const last
            ) throw()
        : es_unary_operator(arg1)
        , value_(first, last)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            jmp_buf jmp_buf_;

            IPrimitive **p_primitive = vm.get_esp();
            IPrimitive& context = vm.get_ecx();

            vm.push_context__(jmp_buf_);
        loop:
            switch (setjmp(jmp_buf_))
            {
            case CT_Normal:
                this->op1(vm);
                goto end;
            case CT_Return:
                vm.return__();
            case CT_Continue:
                if (value_.strict_ne__(vm.get_signal()).operator bool())
                    vm.continue__();
                vm.set_esp(p_primitive);
                vm.set_ecx(context);
                vm.push_context__(jmp_buf_);
                goto loop;
            case CT_Break:
                if (value_.strict_ne__(vm.get_signal()).operator bool())
                    vm.break__();
                vm.set_esp(p_primitive);
                vm.set_ecx(context);
                return;
            case CT_Throw:
                vm.throw__();
            }
        end:
            vm.pop_context__();
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            long_absolute_label__ l2;
            long_relative_label__ l1, l3, l4, l5, l6;
            code
                
            << l1
                << push(ptr[ebx]) // -4                                
                << push(edi) // -4                
                << push(ebx) // -4
                << push(l2)  // -4       
                  
                << mov(ptr[esi], esp)
                
                << add(esi, 0x4)            
                << actor1()
                << sub(esi, 0x4)               
                << add(esp, 0x10)
                << jmp(l5)
                
            << l2
                << cmp(al, CT_Break)
                << jne(l3)            
                << push(eax)                
                << push(edx)
                << push(&value_)
                << push(edx)
                << mov(eax, ptr[edx])
                << call(ptr[eax + IPrimitive::verb_strict_ne__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_operator_bool * 4])
                << test(al, al)
                << pop(edx)
                << pop(eax)
                << jne(l4)                
                << pop(ebx)  // +4                
                << pop(edi)                
                << pop(ecx)
                << mov(ptr[ebx], ecx)                
                << jmp(l5)
                
            << l3
                << cmp(al, CT_Continue)
                << jne(l4)
                << push(eax)                
                << push(edx)
                << push(&value_)
                << push(edx)
                << mov(eax, ptr[edx])
                << call(ptr[eax + IPrimitive::verb_strict_ne__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_operator_bool * 4])
                << test(al, al)
                << pop(edx)
                << pop(eax)
                << jne(l4)                                                          
                << pop(ebx)  // +4                
                << pop(edi)
                << pop(ecx)
                << mov(ptr[ebx], ecx)
                << jmp(l1)
                                
            << l4
                << sub(esi, 0x4)  
                << mov(esp, ptr[esi])
                << ret
                
            << l5
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
        
    private:
        es_string<IPrimitive::string_t> const value_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<New>
    //
    template <>
    struct es_lazy_closure<New>
    : es_unary_operator
    {
        typedef es_lazy_closure<New> self_t;

        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            IPrimitive& constructor = vm.get_eax().get_value__();
            vm.set_eax(constructor.construct__(
                es_arguments<IPrimitive::string_t>::create_instance(constructor)));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // IPrimitive& constructor = vm.get_eax().get_value__();
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)

                // vm.set_eax(constructor.construct__(
                //     es_arguments<IPrimitive::string_t>::create_instance(constructor)));
                << push(eax)
                << call(&es_arguments<IPrimitive::string_t>::create_instance)
                << pop(ecx)
                << push(eax)
                << push(ecx)
                << mov(eax, ptr[ecx])
                << call(ptr[eax + IPrimitive::verb_construct__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Object;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Throw>
    //
    template <>
    struct es_lazy_closure<Throw>
    : es_unary_operator
    {
        typedef es_lazy_closure<Throw> self_t;

        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.set_signal(vm.get_eax());
            vm.throw__();
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.set_signal(vm.get_eax());
                << mov(edx, eax)

                // vm.throw__();
                << sub(esi, 0x4)
                << mov(esp, ptr[esi])
                << mov(eax, CT_Throw)
                << ret
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Var>
    //
    template <>
    struct es_lazy_closure<Var>
    : es_leaf_operator
    {
        template <typename iteratorT>
        explicit es_lazy_closure(
            iteratorT const first,
            iteratorT const last) throw()
        : value_(first, last)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.get_ecx().put__(
                value_, es_undefined<IPrimitive::string_t>::create_instance());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.get_ecx().put__(
                //     value_, es_undefined<IPrimitive::string_t>::create_instance());
                << push(&es_undefined<IPrimitive::string_t>::create_instance())
                << push(&value_)
                << push(edi)
                << mov(eax, ptr[edi])
                << call(ptr[eax + IPrimitive::verb_put__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
        
    private:
        es_const_string<wchar_t> const value_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<VarInit>
    //
    template <>
    struct es_lazy_closure<VarInit>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<VarInit> self_t;

    public:
        template <typename iteratorT>
        explicit es_lazy_closure(
            IClosure const& arg1,
            iteratorT const first,
            iteratorT const last
            ) throw()
        : es_unary_operator(arg1)
        , value_(first, last)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.get_ecx().put__(value_, vm.get_eax().get_value__());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.get_ecx().put__(value_, vm.get_eax().get_value__());
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << push(&value_)
                << push(edi)
                << mov(eax, ptr[edi])
                << call(ptr[eax + IPrimitive::verb_put__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
        
    private:
        es_const_string<wchar_t> const value_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<ForInVar>
    //
    template <>
    struct es_lazy_closure<ForInVar>
    : es_leaf_operator
    {
    private:
        typedef es_lazy_closure<ForInVar> self_t;

    public:
        template <typename iteratorT>
        explicit es_lazy_closure(
            iteratorT const first,
            iteratorT const last
            ) throw()
        : value_(first, last)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            IPrimitive& context = vm.get_ecx();
            context.put__(
                value_, es_undefined<IPrimitive::string_t>::create_instance());
            vm.set_eax(context.get__(value_));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // IPrimitive& context = vm.get_ecx();

                // context.put__(
                //     value_, es_undefined<IPrimitive::string_t>::create_instance());
                << push(&es_undefined<IPrimitive::string_t>::create_instance())
                << push(&value_)
                << push(edi)
                << mov(eax, ptr[edi])
                << call(ptr[eax + IPrimitive::verb_put__ * 4])

                // vm.set_eax(context.get__(value_));
                << push(&value_)
                << push(edi)
                << mov(eax, ptr[edi])
                << call(ptr[eax + IPrimitive::verb_get__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Primitive;
        }
        
    private:
        es_const_string<wchar_t> const value_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<ForInVarInit>
    //
    template <>
    struct es_lazy_closure<ForInVarInit>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<ForInVarInit> self_t;

    public:
        template <typename iteratorT>
        explicit es_lazy_closure(
            IClosure const& arg1,
            iteratorT const first,
            iteratorT const last
            ) throw()
        : es_unary_operator(arg1)
        , value_(first, last)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            IPrimitive& context = vm.get_ecx();
            context.put__(value_, vm.get_eax().get_value__());
            vm.set_eax(context.get__(value_));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()
                
                // IPrimitive& eax = vm.get_eax().get_value__()
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)

                // IPrimitive& context = vm.get_ecx();
                << pop(ecx)
                << push(edi)

                // context.put__(value_, eax);
                << push(ecx)
                << push(&value_)
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_put__ * 4])

                // vm.set_eax(context.get__(value_));
                << pop(ecx)
                << push(ecx)
                << push(&value_)
                << push(ecx)
                << mov(eax, ptr[ecx])
                << call(ptr[eax + IPrimitive::verb_get__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Primitive;
        }
        
    private:
        es_const_string<wchar_t> const value_;
    };


    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Array>
    //
    template <>
    struct es_lazy_closure<Array>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<Array> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.push(es_array<IPrimitive::string_t>::create_instance());
            this->op1(vm);
            vm.set_eax(vm.pop());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.push(es_array<IPrimitive::string_t>::create_instance());
                << call(&es_array<IPrimitive::string_t>::create_instance)
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op1(vm);
                << actor1()

                // vm.set_eax(vm.pop());
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Array;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<NullArray>
    //
    template <>
    struct es_lazy_closure<NullArray>
    : es_leaf_operator
    {
        es_lazy_closure() throw()
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_eax(es_array<IPrimitive::string_t>::create_instance());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.set_eax(es_array<IPrimitive::string_t>::create_instance());
                << call(&es_array<IPrimitive::string_t>::create_instance)
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return es_array<IPrimitive::string_t>::create_instance();
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<ArrayElement>
    //
    template <>
    struct es_lazy_closure<ArrayElement>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<ArrayElement> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.top().push__(vm.get_eax());
            this->op2(vm);
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.top().push__(vm.get_eax());
                << push(eax)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax - 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_push__ * 4])

                // this->op2(vm);
                << actor2()
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Primitive;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Object>
    //
    template <>
    struct es_lazy_closure<Object>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<Object> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.push(es_object<IPrimitive::string_t>::create_instance());
            this->op1(vm);
            vm.set_eax(vm.pop());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.push(es_object<IPrimitive::string_t>::create_instance());
                << call(&es_object<IPrimitive::string_t>::create_instance)
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op1(vm);
                << actor1()

                // vm.set_eax(vm.pop());
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Object;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<ObjectElement>
    //
    template <>
    struct es_lazy_closure<ObjectElement>
    : es_trinary_operator
    {
    private:
        typedef es_lazy_closure<ObjectElement> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2,
            IClosure const& arg3
            ) throw()
        : es_trinary_operator(arg1, arg2, arg3)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            IPrimitive& key = vm.pop();
            vm.top().put_by_value__(key, vm.get_eax());
            this->op3(vm);
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // IPrimitive& key = vm.pop();
                // vm.top().put_by_value__(key, vm.get_eax());
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << push(ptr[eax])
                << mov(ecx, ptr[ebx])
                << mov(eax, ptr[ecx - 4])
                << push(eax)
                << mov(ecx, ptr[eax])
                << call(ptr[ecx + IPrimitive::verb_put_by_value__ * 4])

                // this->op3(vm);
                << actor3()
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Primitive;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<This>
    //
    template <>
    struct es_lazy_closure<This>
    : es_leaf_operator
    {
    private:
        typedef es_lazy_closure<This> self_t;

    public:
        es_lazy_closure() throw()
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_eax(vm.get_ecx().get_prototype__());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.set_eax(vm.get_ecx().get_prototype__())
                << push(edi)
                << mov(eax, ptr[edi])
                << call(ptr[eax + IPrimitive::verb_get_prototype__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Primitive;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<PostInc>
    //
    template <>
    struct es_lazy_closure<PostInc>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<PostInc> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.set_eax(vm.get_eax().postfix_inc__());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.set_eax(vm.get_eax().postfix_inc__());
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_postfix_inc__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * value = actor1().hint__();
            if (value)
                throw *new es_syntax_error<IPrimitive::string_t>(
                    L"es_lazy_closure<PostInc>::hint__");
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<PostDec>
    //
    template <>
    struct es_lazy_closure<PostDec>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<PostDec> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.set_eax(vm.get_eax().postfix_dec__());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.set_eax(vm.get_eax().postfix_dec__());
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_postfix_dec__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * value = actor1().hint__();
            if (value)
                throw *new es_syntax_error<IPrimitive::string_t>(
                    L"es_lazy_closure<PostDec>::hint__");
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Delete>
    //
    template <>
    struct es_lazy_closure<Delete>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<Delete> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.set_eax(vm.get_eax().delete__());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.set_eax(vm.get_eax().delete__());
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_delete__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * value = actor1().hint__();
            if (value)
                return value->get_value__().delete__();
            return TH_Boolean;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Void>
    //
    template <>
    struct es_lazy_closure<Void>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<Void> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
//            vm.get_eax().get_value__();
            vm.set_eax(es_undefined<IPrimitive::string_t>::create_instance());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.get_eax().get_value__();
//                << push(eax)
//                << mov(eax, ptr[eax])
//                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])

                // vm.set_eax(es_undefined<IPrimitive::string_t>::create_instance());
                << mov(eax, &es_undefined<IPrimitive::string_t>::create_instance())
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return es_undefined<IPrimitive::string_t>::create_instance();
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<TypeOf>
    //
    template <>
    struct es_lazy_closure<TypeOf>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<TypeOf> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.set_eax(vm.get_eax().get_value__().typeof__());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.set_eax(vm.get_eax().get_value__().typeof__());
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_typeof__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {            
            IPrimitive * value = actor1().hint__();
            if (value)
                return value->get_value__().typeof__();
            return TH_String;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Inc>
    //
    template <>
    struct es_lazy_closure<Inc>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<Inc> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.set_eax(vm.get_eax().prefix_inc__());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.set_eax(vm.get_eax().prefix_inc__());
                << push(eax)
                << mov(ecx, ptr[eax])
                << call(ptr[ecx + IPrimitive::verb_prefix_inc__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * value = actor1().hint__();
            if (value)
                throw *new es_syntax_error<IPrimitive::string_t>(
                    L"es_lazy_closure<Inc>::hint__");
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Dec>
    //
    template <>
    struct es_lazy_closure<Dec>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<Dec> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.set_eax(vm.get_eax().prefix_dec__());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.set_eax(vm.get_eax().prefix_dec__());
                << push(eax)
                << mov(ecx, ptr[eax])
                << call(ptr[ecx + IPrimitive::verb_prefix_dec__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * value = actor1().hint__();
            if (value)
                throw *new es_syntax_error<IPrimitive::string_t>(
                    L"es_lazy_closure<Dec>::hint__");
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<UnaryPlus>
    //
    template <>
    struct es_lazy_closure<UnaryPlus>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<UnaryPlus> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.set_eax(vm.get_eax().get_value__().unary_plus__());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.set_eax(vm.get_eax().get_value__().unary_plus__());
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_unary_plus__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * value = actor1().hint__();
            if (value)
                return value->get_value__().unary_plus__();
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<UnaryMinus>
    //
    template <>
    struct es_lazy_closure<UnaryMinus>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<UnaryMinus> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.set_eax(vm.get_eax().get_value__().unary_minus__());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.set_eax(vm.get_eax().get_value__().unary_minus__());
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_unary_minus__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * value = actor1().hint__();
            if (value)
                return value->get_value__().unary_minus__();
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Tilde>
    //
    template <>
    struct es_lazy_closure<Tilde>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<Tilde> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.set_eax(vm.get_eax().get_value__().tilde__());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.set_eax(vm.get_eax().get_value__().tilde__());
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_tilde__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * value = actor1().hint__();
            if (value)
                return value->get_value__().tilde__();
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Not>
    //
    template <>
    struct es_lazy_closure<Not>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<Not> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.set_eax(vm.get_eax().get_value__().not__());
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.set_eax(vm.get_eax().get_value__().not__());
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_not__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * value = actor1().hint__();
            if (value)
                return value->get_value__().not__();
            return TH_Boolean;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Mul>
    //
    template <>
    struct es_lazy_closure<Mul>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Mul> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1, IClosure const& arg2) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().mul__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().mul__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_mul__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().mul__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Div>
    //
    template <>
    struct es_lazy_closure<Div>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Div> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().div__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().div__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_div__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().div__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Mod>
    //
    template <>
    struct es_lazy_closure<Mod>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Mod> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().mod__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().mod__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_mod__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().mod__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Plus>
    //
    template <>
    struct es_lazy_closure<Plus>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Plus> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1, IClosure const& arg2) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().binary_plus__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().binary_plus__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_binary_plus__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().binary_plus__(rhs->get_value__());
            if (
                (
                    lhs ? L"number" == lhs->typeof__().operator const_string_t const()
                        : TH_Number == actor1().hint__()
                )
                || (
                    rhs ? L"number" == rhs->typeof__().operator const_string_t const()
                        : TH_Number == actor2().hint__()
                )
            )
                return TH_Number;
            if (lhs && L"string" == lhs->typeof__().operator const_string_t const())
                return TH_String;
            if (lhs && L"string" == rhs->typeof__().operator const_string_t const())
                return TH_String;                
            if (TH_String == actor1().hint__() || TH_String == actor2().hint__())
                return TH_String;
            return TH_Primitive;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Minus>
    //
    template <>
    struct es_lazy_closure<Minus>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Minus> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1, IClosure const& arg2) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().binary_minus__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().binary_minus__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_binary_minus__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().binary_minus__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Shl>
    //
    template <>
    struct es_lazy_closure<Shl>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Shl> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1, IClosure const& arg2) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().shl__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().shl__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_shl__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().shl__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Sar>
    //
    template <>
    struct es_lazy_closure<Sar>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Sar> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1, IClosure const& arg2) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().sar__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().sar__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_sar__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().sar__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Shr>
    //
    template <>
    struct es_lazy_closure<Shr>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Shr> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1, IClosure const& arg2) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().shr__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().shr__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_shr__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().shr__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Lt>
    //
    template <>
    struct es_lazy_closure<Lt>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Lt> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().lt__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().lt__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_lt__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().lt__(rhs->get_value__());
            return TH_Boolean;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Gt>
    //
    template <>
    struct es_lazy_closure<Gt>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Gt> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().gt__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().gt__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_gt__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().gt__(rhs->get_value__());
            return TH_Boolean;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Le>
    //
    template <>
    struct es_lazy_closure<Le>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Le> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().le__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().le__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_le__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().le__(rhs->get_value__());
            return TH_Boolean;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Ge>
    //
    template <>
    struct es_lazy_closure<Ge>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Ge> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().ge__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().ge__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_ge__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().ge__(rhs->get_value__());
            return TH_Boolean;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<InstanceOf>
    //
    template <>
    struct es_lazy_closure<InstanceOf>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<InstanceOf> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.get_eax().get_value__().instanceof__(
                vm.pop().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().instanceof__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_instanceof__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().instanceof__(rhs->get_value__());
            return TH_Boolean;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<In>
    //
    template <>
    struct es_lazy_closure<In>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<In> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.get_eax().get_value__().in__(
                vm.pop().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().in__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_in__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().in__(rhs->get_value__());
            return TH_Boolean;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Eq>
    //
    template <>
    struct es_lazy_closure<Eq>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Eq> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().eq__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().eq__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_eq__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().eq__(rhs->get_value__());
            return TH_Boolean;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Ne>
    //
    template <>
    struct es_lazy_closure<Ne>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Ne> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().ne__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().ne__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_ne__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().ne__(rhs->get_value__());
            return TH_Boolean;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<StrictEq>
    //
    template <>
    struct es_lazy_closure<StrictEq>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<StrictEq> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().strict_eq__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().strict_eq__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_strict_eq__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().strict_eq__(rhs->get_value__());
            return TH_Boolean;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<StrictNe>
    //
    template <>
    struct es_lazy_closure<StrictNe>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<StrictNe> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().strict_ne__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().strict_ne__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_strict_ne__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().strict_ne__(rhs->get_value__());
            return TH_Boolean;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<BitAnd>
    //
    template <>
    struct es_lazy_closure<BitAnd>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<BitAnd> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().bitwise_and__(
                vm.pop().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().bitwise_and__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_bitwise_and__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().bitwise_and__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<BitXor>
    //
    template <>
    struct es_lazy_closure<BitXor>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<BitXor> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().bitwise_xor__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().bitwise_xor__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_bitwise_xor__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().bitwise_xor__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<BitOr>
    //
    template <>
    struct es_lazy_closure<BitOr>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<BitOr> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().get_value__().bitwise_or__(
                vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().get_value__().bitwise_or__(
                //     vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_bitwise_or__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().bitwise_or__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Assign>
    //
    template <>
    struct es_lazy_closure<Assign>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<Assign> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().assign__(vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()
                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)
                // this->op2(vm);
                << actor2()
                // vm.set_eax(vm.pop().assign__(vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_assign__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->assign__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<AssignMul>
    //z
    template <>
    struct es_lazy_closure<AssignMul>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<AssignMul> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().assign_mul__(vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().assign_mul__(vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_assign_mul__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().assign_mul__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<AssignDiv>
    //
    template <>
    struct es_lazy_closure<AssignDiv>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<AssignDiv> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().assign_div__(vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().assign_div__(vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_assign_div__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().assign_div__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<AssignMod>
    //
    template <>
    struct es_lazy_closure<AssignMod>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<AssignMod> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().assign_mod__(vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().assign_mod__(vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_assign_mod__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().assign_mod__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<AssignPlus>
    //
    template <>
    struct es_lazy_closure<AssignPlus>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<AssignPlus> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().assign_plus__(vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().assign_plus__(vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_assign_plus__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().assign_plus__(rhs->get_value__());
            if (lhs && L"string" == lhs->typeof__().operator const_string_t const())
                return TH_String;
            if (lhs && L"string" == rhs->typeof__().operator const_string_t const())
                return TH_String;                
            if (TH_String == actor1().hint__() || TH_String == actor2().hint__())
                return TH_String;
            return TH_Primitive;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<AssignMinus>
    //
    template <>
    struct es_lazy_closure<AssignMinus>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<AssignMinus> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().assign_minus__(vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().assign_minus__(vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_assign_minus__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().assign_minus__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<AssignShl>
    //
    template <>
    struct es_lazy_closure<AssignShl>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<AssignShl> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().assign_shl__(vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().assign_shl__(vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_assign_shl__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().assign_shl__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<AssignSar>
    //
    template <>
    struct es_lazy_closure<AssignSar>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<AssignSar> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().assign_sar__(vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().assign_sar__(vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_assign_sar__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().assign_sar__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<AssignShr>
    //
    template <>
    struct es_lazy_closure<AssignShr>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<AssignShr> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().assign_shr__(vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().assign_shr__(vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_assign_shr__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().assign_shr__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<AssignAnd>
    //
    template <>
    struct es_lazy_closure<AssignAnd>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<AssignAnd> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1,
            IClosure const& arg2
            ) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().assign_and__(vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().assign_and__(vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_assign_and__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().assign_and__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<AssignXor>
    //
    template <>
    struct es_lazy_closure<AssignXor>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<AssignXor> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1, IClosure const& arg2) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().assign_xor__(vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().assign_xor__(vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_assign_xor__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().assign_xor__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<AssignOr>
    //
    template <>
    struct es_lazy_closure<AssignOr>
    : es_binary_operator
    {
    private:
        typedef es_lazy_closure<AssignOr> self_t;

    public:
        explicit es_lazy_closure(
            IClosure const& arg1, IClosure const& arg2) throw()
        : es_binary_operator(arg1, arg2)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.push(vm.get_eax());
            this->op2(vm);
            vm.set_eax(vm.pop().assign_or__(vm.get_eax().get_value__()));
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.push(vm.get_eax());
                << mov(ecx, ptr[ebx])
                << mov(ptr[ecx], eax)
                << add(ptr[ebx], 4)

                // this->op2(vm);
                << actor2()

                // vm.set_eax(vm.pop().assign_or__(vm.get_eax().get_value__()));
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_get_value__ * 4])
                << push(eax)
                << sub(ptr[ebx], 4)
                << mov(eax, ptr[ebx])
                << mov(eax, ptr[eax])
                << push(eax)
                << mov(eax, ptr[eax])
                << call(ptr[eax + IPrimitive::verb_assign_or__ * 4])
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            IPrimitive * lhs = actor1().hint__();
            IPrimitive * rhs = actor2().hint__();
            if (lhs && rhs)
                return lhs->get_value__().assign_or__(rhs->get_value__());
            return TH_Number;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Continue>
    //
    template <>
    struct es_lazy_closure<Continue>
    : es_leaf_operator
    {
    private:
        typedef es_lazy_closure<Continue> self_t;

    public:
        template <typename iteratorT>
        explicit es_lazy_closure(
            iteratorT const first,
            iteratorT const last
            ) throw()
        : value_(first, last)
        {
            value_.addref__();
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_signal(value_);
            vm.continue__();
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                << mov(edx, &value_)

                // vm.continue__();
                << sub(esi, 0x4)
                << mov(esp, ptr[esi])
                << mov(eax, CT_Continue)
                << ret
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    private:
        es_string<IPrimitive::string_t> mutable value_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<ContinueNoArg>
    //
    template <>
    struct es_lazy_closure<ContinueNoArg>
    : es_leaf_operator
    {
    private:
        typedef es_lazy_closure<ContinueNoArg> self_t;

    public:
        es_lazy_closure() throw()
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_signal(es_undefined<IPrimitive::string_t>::create_instance());
            vm.continue__();
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.set_signal(es_undefined<IPrimitive::string_t>::create_instance());
                << mov(edx, &es_undefined<IPrimitive::string_t>::create_instance())

                // vm.continue__();
                << sub(esi, 0x4)
                << mov(esp, ptr[esi])
                << mov(eax, CT_Continue)
                << ret
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Break>
    //
    template <>
    struct es_lazy_closure<Break>
    : es_leaf_operator
    {
    private:
        typedef es_lazy_closure<Break> self_t;

    public:
        template <typename iteratorT>
        explicit es_lazy_closure(
            iteratorT const first, iteratorT const last) throw()
        : value_(first, last)
        {
            value_.addref__();
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_signal(value_);
            vm.break__();
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.set_signal(es_string<IPrimitive::string_t>::create_instance(value_));
                << mov(edx, &value_)

                // vm.break__();
                << sub(esi, 0x4)
                << mov(esp, ptr[esi])
                << mov(eax, CT_Break)
                << ret
                ;
#endif // ES_TEST_JIT
        }


        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    private:
        es_string<IPrimitive::string_t> mutable value_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<BreakNoArg>
    //
    template <>
    struct es_lazy_closure<BreakNoArg>
    : es_leaf_operator
    {
    private:
        typedef es_lazy_closure<BreakNoArg> self_t;

    public:
        es_lazy_closure()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_signal(es_undefined<IPrimitive::string_t>::create_instance());
            vm.break__();
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.set_signal(es_undefined<IPrimitive::string_t>::create_instance());
                << mov(edx, &es_undefined<IPrimitive::string_t>::create_instance())

                // vm.break__();
                << sub(esi, 0x4)
                << mov(esp, ptr[esi])
                << mov(eax, CT_Break)
                << ret
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<Return>
    //
    template <>
    struct es_lazy_closure<Return>
    : es_unary_operator
    {
    private:
        typedef es_lazy_closure<Return> self_t;

    public:
        explicit es_lazy_closure(IClosure const& arg1) throw()
        : es_unary_operator(arg1)
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            this->op1(vm);
            vm.set_signal(vm.get_eax());
            vm.return__();
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // this->op1(vm);
                << actor1()

                // vm.set_signal(vm.get_eax());
                << mov(edx, eax)

                // vm.return__();
                << sub(esi, 0x4)
                << mov(esp, ptr[esi])
                << mov(eax, CT_Return)
                << ret
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_lazy_closure<ReturnNoArg>
    //
    template <>
    struct es_lazy_closure<ReturnNoArg>
    : es_leaf_operator
    {
    private:
        typedef es_lazy_closure<ReturnNoArg> self_t;

    public:
        es_lazy_closure() throw()
        {
        }

        virtual ~es_lazy_closure() throw()
        {
        }

        void ES_CLOSURECALLTYPE operator () (es_machine<IPrimitive>& vm) const
        {
            vm.set_signal(es_undefined<IPrimitive::string_t>::create_instance());
            vm.return__();
        }

        void assemble(es_codestream<IPrimitive>& code) const
        {
#if ES_TEST_JIT
            using namespace generator::x86;
            code
                // vm.set_signal(es_undefined<IPrimitive::string_t>::create_instance());
                << mov(edx, &es_undefined<IPrimitive::string_t>::create_instance())
                // vm.return__();
                << sub(esi, 0x4)
                << mov(esp, ptr[esi])
                << mov(eax, CT_Return)
                << ret
                ;
#endif // ES_TEST_JIT
        }

        es_value_or_hint<IPrimitive> const hint__() const 
        {
            return TH_Empty;
        }
    };

} // namespace ecmascript
