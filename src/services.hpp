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



#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <sys/stat.h>
#include <locale>

// moduels

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <setjmp.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/param.h>
#endif

#include <limits>

#ifdef _WIN32
#    include <shlwapi.h>
#    ifdef _MSC_VER
#        pragma comment(lib, "shlwapi.lib")
#    endif // _MSC_VER
#    include <direct.h> // _getcwd
#    include <wincon.h>
#    include <mbstring.h>
#else
#    include <sys/param.h>
#    include <libgen.h>
#    include <string.h>
#    include <dlfcn.h>
#endif
#    include <dlfcn.h>

#include <set>
#include <cfloat>
#include <iomanip>
#include <stdexcept>
#include <cstdlib>
#include <float.h>
#include <cmath>
#include <cassert>
#include <sstream>
//#include <vector>
#include "vector.hpp"

//#define ES_TEST_JIT 1

#if ES_TRACE_PARSING_TIME || ES_TRACE_RUNNING_TIME || ES_TRACE_COMPILE_TIME
#include <ctime>
#endif // ES_TRACE_PARSING_TIME || ES_TRACE_RUNNING_TIME || ES_TRACE_COMPILE_TIME

#if _WIN32
    const int ES_MAX_PATH = _MAX_PATH;
#else
    const int ES_MAX_PATH = PATH_MAX;
#endif

#ifndef _MSC_VER
#    define __assume(expr)
#endif

#ifndef __GNUC__
#    define __builtin_expect(expr, value) (expr)
#endif

#ifdef __GNUC__
#  define ES_NOVTABLE
#else
#    define ES_NOVTABLE __declspec(novtable)
#endif // __GNUC__

#if _MSC_VER > 1300
#    define ES_ALIGN(x) __declspec(align(x))
#elif defined __GNUC__
#    define ES_ALIGN(x) __attribute__((aligned(x)))
#endif

#if defined(__APPLE__)
#    define ES_THREADLOCAL
#elif defined(__GNUC__)
#    define ES_THREADLOCAL __thread
#elif _MSC_VER
#    define ES_THREADLOCAL __declspec(thread)
#else
#    define ES_THREADLOCAL
#endif


#if defined(__x86_64__) || defined(__x86_64)
#    define __stdcall
#    define __cdecl
#else
#    if defined(__GNUC__)
#        define __stdcall __attribute__((__stdcall__))
#        define __cdecl __attribute__((__cdecl__))
#    endif
#endif

//#ifdef _MSC_VER
//#    define ES_CLOSURECALLTYPE __thiscall
//#else
#    define ES_CLOSURECALLTYPE __stdcall
//#endif

//////////////////////////////////////////////////////////////////////////////
//
//  ES_STATIC_ASSERT
//
#ifdef ES_DEBUG
#    define ES_STATIC_ASSERT(expr) \
        typedef ::ecmascript::es_size_to_type< \
            sizeof(::ecmascript::es_static_assert<expr>)> \
                es_static_assert_typedef__;
#else
#    define ES_STATIC_ASSERT(expr)
#endif // _DEBUG

//////////////////////////////////////////////////////////////////////////////
//
//  ES_ASSERT
//
#ifdef ES_DEBUG
#    define ES_ASSERT(expr) try { assert(expr); } catch (...) { assert(0); }
#else
#    ifdef _MSC_VER
#        define ES_ASSERT(expr) ( __assume(expr) )
#    else
#        define ES_ASSERT(expr) 
#    endif // _MSC_VER
#endif // _DEBUG

//////////////////////////////////////////////////////////////////////////////
//
//  ES_VERIFY
//
#ifdef ES_DEBUG
#    define ES_VERIFY(expr) try { assert(expr); } catch (...) { assert(0); }
#else
#    ifdef _MSC_VER
#        define ES_VERIFY(expr) ( __assume(expr) )
#    else
#        define ES_VERIFY(expr) ( __assume(expr) )
#    endif // _MSC_VER
#endif // _DEBUG


//////////////////////////////////////////////////////////////////////////////
//
//  ES_TRACE
//
#ifdef ES_DEBUG
#    define ES_TRACE(str) \
        ::ecmascript::es_trace(str)
#    define ES_TRACE1(str, val1) \
        ::ecmascript::es_trace(str, val1)
#    define ES_TRACE2(str, val1, val2) \
        ::ecmascript::es_trace(str, val1, val2)
#    define ES_TRACE3(str, val1, val2, val3) \
        ::ecmascript::es_trace(str, val1, val2, val3)
#else
#    define ES_TRACE(str)
#    define ES_TRACE1(str, val1)
#    define ES_TRACE2(str, val1, val2)
#    define ES_TRACE3(str, val1, val2, val3)
#endif // _DEBUG


namespace ecmascript {

    typedef unsigned char uint8_t;
    typedef unsigned short uint16_t;
    typedef long int32_t;
    typedef unsigned long uint32_t;
    typedef long long int64_t;
    typedef int integer_t;
    typedef int32_t es_result;
    es_result const es_success = 0;
    es_result const es_fail = -1;

    //////////////////////////////////////////////////////////////////////////
    //
    //  es_static_assert
    //
    template <bool b> struct es_static_assert;
    template <> struct es_static_assert<true>
    {
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  es_size_to_type
    //
    template <size_t I>
    struct es_size_to_type
    {
        enum { value = I };
    };


    //////////////////////////////////////////////////////////////////////////
    //
    //  es_trace
    //
    void es_trace(char const *str)
    {
        printf("%s", str);
    }

    template <typename T1>
    inline void es_trace(char const *str, T1 val1)
    {
        printf(str, val1);
    }

    template <typename T1, typename T2>
    inline void es_trace(char const *str, T1 val1, T2 val2)
    {
        printf(str, val1, val2);
    }

    template <typename T1, typename T2, typename T3>
    inline void es_trace(char const *str, T1 val1, T2 val2, T3 val3)
    {
        printf(str, val1, val2, val3);
    }

    inline void es_trace(wchar_t const *str)
    {
        wprintf(str);
    }

    template <typename T1>
    inline void es_trace(wchar_t const *str, T1 val1)
    {
        wprintf(str, val1);
    }

    template <typename T1, typename T2>
    inline void es_trace(wchar_t const *str, T1 val1, T2 val2)
    {
        wprintf(str, val1, val2);
    }

    template <typename T1, typename T2, typename T3>
    inline void es_trace(wchar_t const *str, T1 val1, T2 val2, T3 val3)
    {
        wprintf(str, val1, val2, val3);
    }

    //////////////////////////////////////////////////////////////////////
    //
    //  @class  es_chunk
    //
    struct

#if _MSC_VER > 1300
    __declspec(align(256))
#elif defined(__CYGWIN__) || defined(__MINGW32__)
    __attribute__((aligned(16)))
#elif defined __GNUC__
    __attribute__((aligned(256)))
#endif
    es_chunk
    {
        typedef es_chunk self_t;

        enum { chunk_size = 256 };

        es_chunk() throw()
        : mark_(0)
        {
            ES_STATIC_ASSERT(es_chunk::chunk_size == sizeof(es_chunk));
        }

        bool is_available() const throw()
        {
            return 0 != mark_;
        }

        void free() throw()
        {
            mark_ = 0;
        }

        self_t& get_next() const throw()
        {
            return *next_;
        }

        void set_next(self_t& chunk) throw()
        {
            next_ = &chunk;
        }

    private:

        ES_STATIC_ASSERT(1 == sizeof(char));

        union
        {
            int mark_;
            char block_[chunk_size - sizeof(es_chunk *)];
        };
        es_chunk *next_;
    };


    //////////////////////////////////////////////////////////////////////
    //
    //  @class  es_machine_protector
    //
    template <typename machineT>
    struct es_machine_protector
    {
        void __stdcall push_machine(machineT& machine) throw()
        {
            *(it_machine_++) = &machine;
        }

        void __stdcall pop_machine() throw()
        {
            --it_machine_;
        }

    protected:
        es_machine_protector() throw()
        {
            it_machine_ = machines_;
        }

        void __stdcall mark__(int cookie) throw()
        {
            for (machineT** it_machine = machines_;
                it_machine != it_machine_; ++it_machine)
                (**it_machine).mark__(cookie);
        }

    private:
        machineT *machines_[9024];
        machineT **it_machine_;
    };

    //////////////////////////////////////////////////////////////////////
    //
    //  @class  es_object_protector
    //
    template <typename primitiveT>
    struct es_object_protector
    {
        void push_object(primitiveT& object) throw()
        {
            protected_objects_.insert(&object);
        }

    protected:
        es_object_protector() throw()
        {
        }

        void __stdcall mark__(int cookie) throw()
        {
            for (typename std::set<primitiveT *>::iterator it_object
                = protected_objects_.begin();
                it_object != protected_objects_.end(); ++it_object)
            {
                if (!reinterpret_cast<es_chunk&>(**it_object).is_available())
                    protected_objects_.erase(it_object);
                (**it_object).mark__(cookie);
            }
        }

    private:
        std::set<primitiveT *> protected_objects_;
    };

    //////////////////////////////////////////////////////////////////////
    //
    //  @struct  es_allocator
    //


    //////////////////////////////////////////////////////////////////////
    //
    //  @struct  es_memory_pool
    //
    template <typename primitiveT, typename chunkT, int BUFFER_SIZE>
    struct es_memory_pool
    {
    protected:
        es_memory_pool() throw()
        : buffer_(reinterpret_cast<chunkT *>(
            malloc(BUFFER_SIZE * sizeof(chunkT))))
        , it_chunk_(buffer_)
        , reserve(BUFFER_SIZE)
        {
        }

        ~es_memory_pool() throw()
        {
            free(buffer_);
        }

        void __stdcall sweep__(int cookie) throw()
        {
            for (es_chunk *it = buffer_; it != it_chunk_; ++it)
                if (it->is_available())
                    reinterpret_cast<primitiveT *>(it)->sweep__(cookie);
        }

        void * __stdcall allocate() throw()
        {
            if (__builtin_expect(BUFFER_SIZE > it_chunk_ - buffer_, true))
                return it_chunk_++;
            chunkT const * const p = reinterpret_cast<chunkT *>(
                realloc(buffer_, (reserve *= 2) * sizeof(chunkT)));
            (void)p;
            ES_ASSERT(buffer_ == p);
            return it_chunk_++;
        }

    private:
        chunkT *buffer_;
        chunkT *it_chunk_;
        size_t reserve;
    };

    //////////////////////////////////////////////////////////////////////
    //
    //  @struct  es_freelist_manager
    //
    template <typename chunkT>
    struct es_freelist_manager
    {
    protected:
        es_freelist_manager() throw()
        : it_free_list_(0)
        {
        }

        void __stdcall push(es_chunk& free_chunk) throw()
        {
            free_chunk.free();
            free_chunk.set_next(*it_free_list_);
            it_free_list_ = &free_chunk;
        }

        chunkT * __stdcall pop() throw()
        {
            chunkT *result = it_free_list_;
            it_free_list_ = &it_free_list_->get_next();
            return result;
        }

        bool __stdcall is_empty() throw()
        {
            return 0 == it_free_list_;
        }

    private:
        chunkT *it_free_list_;
    };

    //////////////////////////////////////////////////////////////////////
    //
    //  @class  es_root_holder
    //
    template <typename primitiveT>
    struct es_root_holder
    {
        void __stdcall set_root(primitiveT& root) throw()
        {
            p_root_ = &root;
        }

    protected:
        void __stdcall mark__(int cookie) throw()
        {
            if (__builtin_expect(0 != p_root_, true))
                p_root_->mark__(cookie);
        }

    private:
        primitiveT *p_root_;
    };

    template <typename primitiveT>
    struct es_machine;

    //////////////////////////////////////////////////////////////////////
    //
    //  @class  es_gc
    //
    template <typename primitiveT>
    struct es_gc
    : public es_machine_protector<es_machine<primitiveT> >
    , public es_object_protector<primitiveT>
    , public es_memory_pool<primitiveT, es_chunk, 2 << 16>
    , public es_freelist_manager<es_chunk>
    , public es_root_holder<primitiveT>
    {
        typedef es_machine<primitiveT> machine_t;
        typedef es_machine_protector<machine_t> machine_protector_t;
        typedef es_object_protector<primitiveT> object_protector_t;
        typedef es_memory_pool<primitiveT, es_chunk, 2 << 16> memory_pool_t;
        typedef es_freelist_manager<es_chunk> freelist_t;
        typedef es_root_holder<primitiveT> root_holder_t;

        es_gc() throw()
        {
        }

        ~es_gc() throw()
        {
        }

        void __stdcall collect() throw()
        {
            static long cookie = 0;
            cookie = 1 - cookie;
            root_holder_t::mark__(cookie);
            machine_protector_t::mark__(cookie);
            object_protector_t::mark__(cookie);
            memory_pool_t::sweep__(cookie);
        }

        void * __stdcall allocate(size_t) throw()
        {
            return freelist_t::is_empty() ?
                memory_pool_t::allocate():
                freelist_t::pop();
        }

        void __stdcall deallocate(void* p) throw()
        {
            freelist_t::push(*reinterpret_cast<es_chunk *>(p));
        }

    private:
        es_gc(es_gc<primitiveT> const&);
        es_gc& operator=(es_gc<primitiveT> const&);
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // struct es_context_holder
    //
    template <typename primitiveT>
    struct es_context_holder
    {
    private:
        typedef es_context_holder<primitiveT> self_t;
        
    public:
        explicit es_context_holder(primitiveT& this_arg) throw()
        : value_(&this_arg)
        {
            ES_ASSERT(0 != value_);
//            ES_ASSERT(0 == &static_cast<self_t const *const>(0)->value_);
        }

        es_context_holder() throw()
        : value_(0)
        {
//            ES_ASSERT(0 == &static_cast<self_t const *const>(0)->value_);
        }

        primitiveT& get() const throw()
        {
            ES_ASSERT(0 != value_);
            return *value_;
        }

        void set(primitiveT& value) throw()
        {
            value_ = &value;
            ES_ASSERT(0 != value_);
        }

        void mark__(int cookie) throw()
        {
            ES_ASSERT(0 != value_);
            value_->mark__(cookie);
        }

    private:
        primitiveT * value_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // struct es_register_machine
    //
    template <typename primitiveT>
    struct es_register_machine
    {
    private:
        typedef es_register_machine<primitiveT> self_t;
        
    public:
        es_register_machine() throw()
        : value_(0)
        {
//            ES_ASSERT(0 == &static_cast<self_t const *const>(0)->value_);
        }

        primitiveT& get() const throw()
        {
            ES_ASSERT(0 != value_);
            return *value_;
        }

        void set(primitiveT& value) throw()
        {
            value_ = &value;
        }

        void mark__(int cookie) throw()
        {
            if (__builtin_expect(0 != value_, true))
                value_->mark__(cookie);
        }
    private:
        primitiveT * value_;
    };


    //////////////////////////////////////////////////////////////////////////
    //
    // struct es_stack_machine
    //
    template <typename primitiveT>
    struct es_stack_machine
    {
    private:
        typedef es_stack_machine<primitiveT> self_t;
        
    public:
        es_stack_machine() throw()
        : value_(reinterpret_cast<primitiveT **>(&stack_frame_))
        {
//            ES_ASSERT(0 == &static_cast<self_t const *const>(0)->value_);
        }

        primitiveT & __stdcall pop() throw()
        {
            return **(-- value_);
        }

        primitiveT & __stdcall top() const throw()
        {
            return **(value_ - 1);
        }

        void __stdcall push(primitiveT & value) throw()
        {
            *(value_ ++) = &value;
        }

        void __stdcall push() throw()
        {
            ++ value_;
        }

        primitiveT ** __stdcall get_esp() const throw()
        {
            return value_;
        }

        void __stdcall set_esp(primitiveT **pp_primitive) throw()
        {
            value_ = pp_primitive;
        }

        void __stdcall mark__(int cookie) throw()
        {
            for (primitiveT **it = stack_frame_; it != value_; ++it)
                if ((**reinterpret_cast<es_chunk **>(it)).is_available())
                    (**it).mark__(cookie);
        }
        
    private:
        primitiveT **value_;
        primitiveT * stack_frame_[1024];
    };

    enum CompletionType 
    {
        CT_Normal           = 0,
        CT_Return           = 1,
        CT_Continue         = 2,
        CT_Break            = 3,
        CT_Throw            = 4,
        CT_LabeledContinue  = 5,
        CT_LabeledBreak     = 6,
    };


    //////////////////////////////////////////////////////////////////////////
    //
    // struct es_signal_controller
    //
    template <typename primitiveT>
    struct es_signal_controller
    {
        static primitiveT& __stdcall get_signal() throw()
        {
            return *p_signal_;
        }

        static void __stdcall set_signal(primitiveT& value) throw()
        {
            p_signal_ = &value;
        }

        static void __stdcall push_context__(jmp_buf& jmp_buf_) throw()
        {
            context_stack_.push(&jmp_buf_);
        }

        static void __stdcall pop_context__() throw()
        {
            context_stack_.pop();
        }

        static void __stdcall return__() throw()
        {
            longjmp(*context_stack_.pop(), CT_Return);
        }

        static void __stdcall continue__() throw()
        {
            longjmp(*context_stack_.pop(), CT_Continue);
        }

        static void __stdcall break__() throw()
        {
            longjmp(*context_stack_.pop(), CT_Break);
        }

        static void __stdcall throw__() throw()
        {
            longjmp(*context_stack_.pop(), CT_Throw);
        }

    private:
        static struct es_context
        {
            es_context()
            {
                it_ = jmp_buf_stack_;
            }

            void push(jmp_buf * p_jmp_buf) const
            {
                *(it_++) = p_jmp_buf;
            }

            jmp_buf * pop() const
            {
                return *--it_;
            }

        private:
            static ES_THREADLOCAL jmp_buf **it_;
            static ES_THREADLOCAL jmp_buf *jmp_buf_stack_[1000];
        } const context_stack_;
        static ES_THREADLOCAL primitiveT * p_signal_;
    };

    template <typename primitiveT>
    typename es_signal_controller<primitiveT>::es_context const
    es_signal_controller<primitiveT>::context_stack_;

    template <typename primitiveT>
    ES_THREADLOCAL jmp_buf **
    es_signal_controller<primitiveT>::es_context::it_;

    template <typename primitiveT>
    ES_THREADLOCAL jmp_buf *
    es_signal_controller<primitiveT>::es_context::jmp_buf_stack_[1000];

    template <typename primitiveT>
    ES_THREADLOCAL primitiveT *
    es_signal_controller<primitiveT>::p_signal_;

    //////////////////////////////////////////////////////////////////////////
    //
    // struct es_machine
    //
    template <typename primitiveT>
    struct es_machine
    : public es_stack_machine<primitiveT>
    , public es_signal_controller<primitiveT>
    {
    private:
        typedef es_machine<primitiveT> self_t;
        
    public:
        explicit es_machine(
            primitiveT& this_arg
            ) throw()
        : context_holder_(this_arg)
        {
            get_gc().push_machine(*this);
        }

        es_machine() throw()
        {
            get_gc().push_machine(*this);
        }

        ~es_machine() throw()
        {
            get_gc().pop_machine();
        }

        void __stdcall mark__(int cookie) throw()
        {
            register_machine_.mark__(cookie); // protect eax
            context_holder_.mark__(cookie); // protect ecx
            es_stack_machine<primitiveT>::mark__(cookie); // protect stack
        }

        void __stdcall collect() throw()
        {
            get_gc().collect();
        }

        static es_gc<primitiveT>& __stdcall get_gc() throw()
        {
            static es_gc<primitiveT> gc_;
            return gc_;
        }

        primitiveT& __stdcall get_eax() const throw()
        {
            return register_machine_.get();
        }
        
        void __stdcall set_eax(primitiveT& value) throw()
        {
            register_machine_.set(value);
        }

        primitiveT& __stdcall get_ecx() const throw()
        {
            return context_holder_.get();
        }

        void __stdcall set_ecx(primitiveT& value) throw()
        {
            context_holder_.set(value);
        }

    private:
        es_machine(self_t const&);
        es_machine<primitiveT>& operator =(self_t const&);
    private:
        es_register_machine<primitiveT> register_machine_;
        es_context_holder<primitiveT> context_holder_;
    };
    
    //////////////////////////////////////////////////////////////////////////
    //
    //  debug flag
    //
    static bool verbose_ = false;
    void es_set_verbose(bool b)
    {
        verbose_ = b;
    }
} // namespace ecmascript

namespace ecmascript { namespace base_services {

    //////////////////////////////////////////////////////////////////////
    //
    // @fn es_mbscmp
    // @brief compare two multibyte strings.
    //
    int es_mbscmp(unsigned char * lhs, unsigned char * rhs)
    {
#if _MSC_VER
        return _mbscmp(lhs, rhs);
#elif defined(__CYGWIN__) && defined(__MINGW32__)
        return _mbscmp(lhs, rhs);
#else
        return strcmp((char *)lhs, (char *)rhs);
#endif // defined(__CYGWIN__) || defined(__MINGW32__)
    }

    //////////////////////////////////////////////////////////////////////
    //
    // @fn es_strcat
    //
    inline es_result es_strcat(
        wchar_t const * const lhs_it,
        wchar_t const * const lhs_end,
        wchar_t const * const rhs_it,
        wchar_t const * const rhs_end,
        wchar_t * *const p_result_it,
        wchar_t * *const p_result_end
        )
    {
        size_t length1 = lhs_end - lhs_it;
        size_t length2 = rhs_end - rhs_it;
        wchar_t *const buffer = new wchar_t[length1 + length2 + 1];
        std::copy(lhs_it, lhs_end, buffer);
        std::copy(rhs_it, rhs_end, buffer + length1);
        *p_result_it = buffer;
        *p_result_end = buffer + length1 + length2;
        return es_success;
    }

    //////////////////////////////////////////////////////////////////////
    //
    // @fn es_compare
    //
    inline int es_compare(
        wchar_t const * const lhs_it,
        wchar_t const * const lhs_end,
        wchar_t const * const rhs_it,
        wchar_t const * const rhs_end
        )
    {
        return lhs_it == lhs_end ? (rhs_it == rhs_end ? 0: -1):
            rhs_it == rhs_end ? 1 :
            *lhs_it > *rhs_it ? 1 :
            *lhs_it < *rhs_it ? -1 :
            es_compare(lhs_it + 1, lhs_end, rhs_it + 1, rhs_end);
    }

    //////////////////////////////////////////////////////////////////////
    //
    // @fn es_locale_compare
    // @brief compare two strings using locale-specific rules.
    //
    inline int es_locale_compare(
        wchar_t const * const lhs_it,
        wchar_t const * const lhs_end,
        wchar_t const * const rhs_it,
        wchar_t const * const rhs_end
        )
    {
        if (lhs_it == lhs_end)
            return rhs_it == rhs_end ? 0: -1;
        if (rhs_it == rhs_end)
            return 1;
        unsigned char lhs_char[MB_LEN_MAX + 1] = {0};
        unsigned char rhs_char[MB_LEN_MAX + 1] = {0};
        if (__builtin_expect(-1 == wctomb((char *)(lhs_char), *lhs_it), false)
            || __builtin_expect(-1 == wctomb((char *)(rhs_char), *rhs_it), false))
            return *lhs_it > *rhs_it ? 1 : *lhs_it < *rhs_it ? -1 :
                es_locale_compare(lhs_it + 1, lhs_end, rhs_it + 1, rhs_end);
        int result = es_mbscmp(lhs_char, rhs_char);
        return 0 != result ?
            result :
            es_locale_compare(lhs_it + 1, lhs_end, rhs_it + 1, rhs_end);
    }

    //////////////////////////////////////////////////////////////////////
    //
    // es_print
    //
    bool es_print(char const * message)
    {
        printf("%s\n", message);
        return true;
    }

    bool es_print(wchar_t const * message)
    {
        wprintf(L"%s\n", message);
        return true;
    }

    //////////////////////////////////////////////////////////////////////
    //
    // es_strstr
    //
    wchar_t const * es_strstr(
        wchar_t const *str,
        wchar_t const *substr) throw()
    {
        return wcsstr(str, substr);
    }

#if _MSC_VER
    unsigned char const * es_strstr(
        unsigned char const *str,
        unsigned char const *substr) throw()
    {
        return _mbsstr(str, substr);
    }
#else
    char const * es_strstr(
        char const *str,
        char const *substr) throw()
    {
        return strstr(str, substr);
    }
#endif // _MSC_VER

    //////////////////////////////////////////////////////////////////////////
    //
    //  es_puts
    //
    inline int es_puts(char const* s)
    {
        return puts(s);
    }

    inline int es_puts(wchar_t const *s)
    {
#if _WIN32
        return _putws(s);
#elif defined(__CYGWIN__) && defined(__MINGW32__)
        return putws(s);
#elif defined(__APPLE__)
    fputws(s, stdout);
    return fputws(L"\n", stdout);
#else
    return fputws(s, stdout);
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  es_putchar
    //
    inline int es_putchar(char c)
    {
        return putchar(c);
    }

    inline wint_t es_putchar(wchar_t c)
    {
        return putwchar(c);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  es_floor
    //
    inline double es_floor(double d)
    {
#if defined(__CYGWIN__) || defined(__MINGW32__)
        return floor(d);
#else
        return std::floor(d);
#endif // defined(__CYGWIN__) || defined(__MINGW32__)
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  es_abs
    //
    inline double es_abs(double d)
    {
        return d >= 0 ? d : -d;
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  es_max
    //
    template <typename T>
    inline T es_max(T d1, T d2)
    {
        return d1 > d2 ? d1 : d2;
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  es_min
    //
    template <typename T>
    inline T es_min(T d1, T d2)
    {
        return d1 < d2 ? d1 : d2;
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  es_sign
    //
    inline double es_sign(double d)
    {
        return d >= 0 ? 1 : -1;
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  es_lexical_cast
    //
    template <typename T>
    inline bool es_lexical_cast(double, T);

    template <>
    inline bool es_lexical_cast<wchar_t *>(double d, wchar_t *str)
    {
#if defined(__CYGWIN__) || defined(__MINGW32__)
        std::stringstream ss;
        ss.imbue(std::locale::classic());
        ss << std::setprecision(15) << d;
        std::string const bufstr = ss.str();
        mbstowcs(str, bufstr.c_str(), bufstr.size() + 1);
#else
        std::wstringstream wss;
        wss.imbue(std::locale::classic());
        wss << std::setprecision(15) << d;
        wcscpy(str, wss.str().c_str());
#endif // defined(__CYGWIN__) || defined(__MINGW32__)

        return true;
    }

    template <typename T>
    inline T es_lexical_cast(wchar_t const *);

    template <>
    inline double es_lexical_cast<double>(wchar_t const *str)
    {
#if defined(_MSC_VER)
        return _wtof(str);
#else
        return wcstod(str, 0);
#endif // _MSC_VER
    }

    template <>
    inline ecmascript::int32_t
    es_lexical_cast<ecmascript::int32_t>(wchar_t const *str)
    {
#ifdef _MSC_VER
        int result_value = _wtoi(str);
#else
        int result_value = wcstol(str, 0, 10);
#endif // _MSC_VER
        if (result_value > INT_MAX || result_value < INT_MIN)
            throw std::runtime_error("overflow/underflow:");
        return result_value;
    }

    template <>
    inline ecmascript::uint32_t
    es_lexical_cast<ecmascript::uint32_t>(wchar_t const *str)
    {
#ifdef _MSC_VER
        int result_value = _wtoi(str);
#else
        int result_value = wcstol(str, 0, 10);
#endif // _MSC_VER
        return result_value;
    }

    template <>
    inline ecmascript::uint16_t
    es_lexical_cast<ecmascript::uint16_t>(wchar_t const *str)
    {
#ifdef _MSC_VER
        return ecmascript::uint16_t(_wtoi(str));
#else
        return wcstol(str, 0, 10);
#endif // _MSC_VER
    }

    //////////////////////////////////////////////////////////////////////
    //
    // es_isnan
    //
    inline bool es_isnan(double d)
    {
        using namespace std;
#ifdef _MSC_VER
        return 0 != (_isnan)(d);
#elif defined(__APPLE__)
    return 0 != isnan(d);
#elif defined(__CYGWIN__) && defined(__MINGW32__)
        return 0 != (_isnan)(d);
#else
        return 0 != (isnan)(d);
#endif // _MSC_VER
    }

    //////////////////////////////////////////////////////////////////////
    //
    // es_isinf
    //
    inline bool es_isinf(double d)
    {
        using namespace std;
#ifdef _MSC_VER
        return !_finite(d) && !_isnan(d);
#elif defined(__APPLE__)
    return 0 != isinf(d);
#elif defined(__CYGWIN__) && defined(__MINGW32__)
        return (_isnan)(d) != 0;
#else
        return (isnan)(d) != 0;
#endif // _MSC_VER
    }

    //////////////////////////////////////////////////////////////////////
    //
    // es_dirname
    //
    inline bool es_dirname(
        char const * fullpath, char *dirpath, size_t buffer_size)
    {
#ifdef _WIN32
        char drive[_MAX_DRIVE];
        char dir[_MAX_DIR];
        char fname[_MAX_FNAME];
        char ext[_MAX_EXT];
#    ifdef _MSC_VER
#        pragma warning(push)
#        pragma warning(disable:4996)
#    endif // _MSC_VER
        _splitpath(fullpath, drive, dir, fname, ext);
        _makepath(dirpath, drive, dir, NULL, NULL);
#    ifdef _MSC_VER
#        pragma warning(pop)
#    endif // _MSC_VER
        return true;
#else
        memcpy(dirpath, dirname(const_cast<char *>(fullpath)), buffer_size);
        return true;
#endif // _WIN32
    }


    //////////////////////////////////////////////////////////////////////
    //
    // es_abspath
    //
    inline bool es_abspath(
        char const * path, 
        char *resolved_path, 
        size_t buffer_size
        ) throw()
    {
#ifdef _WIN32
        return _fullpath(resolved_path, path, buffer_size) != NULL;
#else
        return realpath(path, resolved_path) != NULL;
#endif // _WIN32
    }

    bool es_abspath(
        wchar_t const* wpath, 
        wchar_t *wresolved_path, 
        size_t buffer_size
        ) throw()
    {
#ifdef _WIN32
        if (wcslen(wpath) >= ES_MAX_PATH)
            return false;
        return _wfullpath(wresolved_path, wpath, buffer_size) != NULL;
#else
        int size = wcstombs(NULL, wpath, 0);
        if (size >= ES_MAX_PATH)
            return false;
        char path[ES_MAX_PATH + 1];
        if (size_t(-1) == wcstombs(path, wpath, size))
            return false;
        char resolved_path[ES_MAX_PATH + 1];
        if (0 == realpath(path, resolved_path))
            return false;
        if (mbstowcs(wresolved_path, resolved_path, buffer_size - 1)
            == size_t(-1))
            return false;
        return true;
#endif // _WIN32
    }

    //////////////////////////////////////////////////////////////////////
    //
    // es_getcwd
    //
    inline bool es_getcwd(char * path, int max_len) throw()
    {
#ifdef _WIN32
        return _getcwd(path, max_len) != NULL;
#else
        return getcwd(path, max_len) != NULL;
#endif // _WIN32
    }

    bool es_getcwd(wchar_t * wpath, int max_len) throw()
    {
#ifdef _WIN32
        return _wgetcwd(wpath, max_len) != NULL;
#else
        char path[ES_MAX_PATH + 1];
        if (getcwd(path, ES_MAX_PATH) == NULL)
            return false;
        if (mbstowcs(wpath, path, max_len - 1) == size_t(-1))
            return false;
        return true;
#endif // _WIN32
    }

    //////////////////////////////////////////////////////////////////////
    //
    // es_dynamic_load
    //
    inline void * es_dynamic_load(char const* name) throw()
    {
#if defined(_WIN32)
        HMODULE module = LoadLibraryA(name);
        if (NULL == module)
            return NULL;
        return GetProcAddress(module, name);
#else
        char module_name[ES_MAX_PATH];
        sprintf(module_name, "libzuse%s.so", name);
        void * module = dlopen(module_name, RTLD_NOW);
        if (0 == module)
            return 0;
        return dlsym(module, name);
#endif
     }

    inline void * es_dynamic_load(wchar_t const* wname) throw()
    {
        char name[ES_MAX_PATH + 1]; 
        if (wcstombs(name, wname, sizeof(name) - 1) == size_t(-1))
            return 0;
        return es_dynamic_load(name); 
    }

} } // namespace ecmascript::base_services

namespace ecmascript {

    inline es_result
    es_con_red()
    {
#ifdef _WIN32
        return SetConsoleTextAttribute(
            GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED) ? 0: -1;
#else
        return 0;
#endif // _WIN32
    }

    inline es_result
    es_con_green()
    {
#ifdef _WIN32
        return SetConsoleTextAttribute(
            GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN) ?
                es_success: es_fail;
#else
        return 0;
#endif // _WIN32
    }

    inline es_result
    es_con_blue()
    {
#ifdef _WIN32
        return SetConsoleTextAttribute(
            GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE) ?
                es_success: es_fail;
#else
        return 0;
#endif // _WIN32
    }

    inline es_result
    es_con_intensity()
    {
#ifdef _WIN32
        return SetConsoleTextAttribute(
            GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY) ?
                es_success: es_fail;
#else
        return 0;
#endif // _WIN32
    }

    inline es_result
    es_con_white()
    {
#ifdef _WIN32
        return SetConsoleTextAttribute(
            GetStdHandle(STD_OUTPUT_HANDLE),
            FOREGROUND_RED| FOREGROUND_GREEN| FOREGROUND_BLUE) ? 0: -1;
#else
        return 0;
#endif // _WIN32
    }

} // ecmascript

namespace ecmascript {

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_heap_alloc
    //
    template <typename valueT>
    valueT const* const es_heap_alloc(valueT initial_value) throw()
    {
        valueT *const p = new (std::nothrow) valueT[1];
        if (0 == p)
            return 0;
        *p = initial_value;
        return p;
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_clone_copy
    //
    template <typename valueT, typename iteratorT>
    inline valueT * es_clone_copy(
        iteratorT const first,
        iteratorT const last) throw()
    {
        int const length = last - first;
        ES_ASSERT(0 <= length);
//        if (__builtin_expect(0 < length, false))
//            return es_clone_copy<valueT>(last, first);
        valueT * p = new (std::nothrow) valueT[length + 1];
        if (0 == p)
            return 0;
//        std::copy(first, last, p);
        memcpy(p, first, sizeof(valueT) * length);
        p[length] = 0;
        return p;
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_range
    //
    template <typename charT, bool sbo = true, bool counter = true>
    struct es_range;

    template <typename charT>
    struct es_range<charT, true, true>
    {
        typedef charT value_type;
        typedef value_type const * const_iterator;
        typedef value_type * iterator;
        typedef size_t size_type;

        struct error {};

        es_range() throw()
        : length_(0)
        , buffer_(0)
        {
            small_buffer_[0] = 0;
        }

        explicit es_range(error e) throw()
        : length_(0)
        , buffer_(0)
        {
        }

        explicit es_range(const_iterator first, const_iterator last) throw()
        : length_(last - first)
        {
            if (length_ >= sbo_size)
            {
                buffer_ = es_clone_copy<value_type>(first, last);
                return;
            }
            memcpy(small_buffer_, first, sizeof(value_type) * length_);
            small_buffer_[length_] = 0;
        }

        es_range(es_range const& rhs) throw()
        {
            assign(rhs);
        }

        es_range& operator =(es_range const& rhs) throw()
        {
            assign(rhs);
            return *this;
        }

        void assign(es_range const& rhs) throw()
        {
            length_ = rhs.length_;
            if (length_ >= sbo_size)
            {
                buffer_ = es_clone_copy<value_type>(rhs.begin(), rhs.end());
                return;
            }
            memcpy(small_buffer_, rhs.small_buffer_, sizeof(small_buffer_));
        }

        const_iterator begin() const throw()
        {
            return length_ < sbo_size ? small_buffer_: buffer_;
        }

        const_iterator end() const throw()
        {
            return begin() + length_;
        }

        value_type const *const c_str() const throw()
        {
            return begin();
        }

        size_type length() const throw()
        {
#if 0
            size_t length = 0;
            ES_ASSERT(begin() <= end());
            for (iterator it = begin() ; it != end(); ++it, ++length)
                if (*it >= 0xd800 && *it <= 0xdbff)
                    ++it, ES_ASSERT(*it >= 0xdc00 && *it <= 0xdfff);
            return length;
#endif
            return length_;
        }
    protected:
        ~es_range() throw()
        {
            if (length_ < sbo_size)// || 0 != --*counter_)
                return;
            ::operator delete [](buffer_, std::nothrow);
        }

    private:
        enum { sbo_size = 8 };
        size_t length_;
        union {
            value_type small_buffer_[sbo_size];
            value_type * buffer_;
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_string_base
    //
    template <typename valueT>
    struct es_string_base
    : es_range<valueT, true, true>
    {
        typedef valueT value_type;
        typedef es_range<valueT, true, true> range_t;
        typedef es_string_base<value_type> self_t;

        explicit es_string_base() throw()
        {
        }

        template <typename iteratorT>
        explicit es_string_base(
            iteratorT const first,
            iteratorT const last) throw()
        : range_t(&*first, &*last)
        {
        }

        explicit es_string_base(
            wchar_t const *const first,
            wchar_t const *const last) throw()
        : range_t(first, last)
        {
        }

        explicit es_string_base(value_type const *const value) throw()
        : range_t(value, value + wcslen(value))
        {
        }

        es_string_base(self_t const& rhs) throw()
        : range_t(rhs)
        {
        }

        ~es_string_base() throw()
        {
        }

        bool empty() const { return range_t::begin() == range_t::end(); }

        bool operator == (self_t const& rhs) const throw()
        {
            return range_t::length() == rhs.length()
                && 0 == base_services::es_compare(
                    range_t::begin(), range_t::end(), rhs.begin(), rhs.end());
        }

        bool operator == (valueT const *const rhs) const throw()
        {
            return 0 == base_services::es_compare(
                range_t::begin(), range_t::end(), rhs, rhs + wcslen(rhs));
        }

        bool operator != (std::wstring const& rhs) const throw()
        {
            return 0 != base_services::es_compare(
                range_t::begin(), range_t::end(), &*rhs.begin(), &*rhs.end());
        }

        bool operator != (valueT const *const rhs) const throw()
        {
            return 0 != base_services::es_compare(
                range_t::begin(), range_t::end(), rhs, rhs + wcslen(rhs));
        }

        bool operator < (std::wstring const& rhs) const throw()
        {
            return 0 > base_services::es_compare(
                range_t::begin(), range_t::end(), &*rhs.begin(), &*rhs.end());
        }

        bool operator <= (std::wstring const& rhs) const throw()
        {
            return 0 >= base_services::es_compare(
                range_t::begin(), range_t::end(), &*rhs.begin(), &*rhs.end());
        }

        bool operator > (std::wstring const& rhs) const throw()
        {
            return 0 < base_services::es_compare(
                range_t::begin(), range_t::end(), &*rhs.begin(), &*rhs.end());
        }

        bool operator >= (std::wstring const& rhs) const throw()
        {
            return 0 <= base_services::es_compare(
                range_t::begin(), range_t::end(), &*rhs.begin(), &*rhs.end());
        }

        operator std::wstring const() const
        {
            return std::wstring(range_t::begin(), range_t::end());
        }
    };

    template <typename valueT>
    inline bool operator == (
        es_string_base<valueT> const& lhs,
        es_string_base<valueT> const& rhs) throw()
    {
        return lhs.length() == rhs.length()
            && 0 == base_services::es_compare(
                lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    template <typename T1, typename valueT>
    inline bool operator == (T1 lhs, es_string_base<valueT> const& rhs) throw()
    {
        return rhs.operator ==(lhs);
    }

    template <typename T1, typename valueT>
    inline bool operator != (T1 lhs, es_string_base<valueT> const& rhs) throw()
    {
        return rhs.operator !=(lhs);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_const_string
    //
    template <typename valueT>
    struct es_const_string
    : es_string_base<valueT>
    {
        typedef valueT value_type;
        typedef es_const_string<valueT> self_t;
        typedef es_string_base<valueT> base_t;
        typedef typename base_t::size_type size_type;
        typedef typename base_t::const_iterator const_iterator;

        es_const_string() throw()
        {
        }

        template <typename iteratorT>
        explicit es_const_string(
            iteratorT const first,
            iteratorT const last) throw()
        : base_t(first, last)
        {
        }

        es_const_string(std::basic_string<valueT> const& value) throw()
        : base_t(&*value.begin(), &*value.end())
        {
        }

        es_const_string(valueT const *const value) throw()
        : base_t(value, value + wcslen(value))
        {
        }

        es_const_string(self_t const& rhs) throw()
        : base_t(rhs)
        {
        }

        virtual ~es_const_string() throw()
        {
        }

        valueT const& at(size_type position) const throw()
        {
            //ES_ASSERT(position < base_t::length());
            return *(base_t::begin() + position);
        }

        es_const_string<valueT> const substr(
            size_type position, size_type length)
             const throw()
        {
            return self_t(
                base_t::begin() + position,
                base_t::begin() + position + length);
        }

        enum { npos = -1 };

        size_type find(
            es_const_string<valueT> const& search_string,
            size_type position
            ) const throw()
        {
            if (0 > position || position >= base_t::length())
                return size_type(npos);
            //const_iterator it_rhs = search_string.begin();
            wchar_t const* p_result
                = base_services::es_strstr(
                    base_t::c_str() + position, search_string.c_str());
            if (0 == p_result)
                return size_type(npos);
            ES_ASSERT(0 <= p_result - base_t::begin());
            return size_type(p_result - base_t::begin());
        }

        template <typename charT>
        self_t const operator + (std::basic_string<charT> const& rhs) const throw()
        {
            value_type * it_begin = 0, * it_end = 0;
            if (es_success != base_services::es_strcat(
                base_t::begin(), base_t::end(),
                &*rhs.begin(), &*rhs.end(),
                &it_begin, &it_end))
                return self_t(0);
            return self_t(it_begin, it_end);
        }

        self_t const operator + (es_const_string const& rhs) const throw()
        {
            value_type * it_begin = 0, * it_end = 0;
            if (es_success != base_services::es_strcat(
                base_t::begin(), base_t::end(),
                rhs.begin(), rhs.end(),
                &it_begin, &it_end))
                return self_t(0);
            return self_t(it_begin, it_end);
        }

    };

    template <typename T1, typename valueT>
    inline es_const_string<valueT> operator + (
        T1 lhs, es_const_string<valueT> const& rhs) throw()
    {
        return es_const_string<valueT>(lhs) + rhs;
    }

    typedef es_const_string<wchar_t> const_string_t;

    inline std::size_t hash_value(const_string_t const& v)
    {
        std::size_t seed = 0;
        for(const_string_t::const_iterator it = v.begin(); it != v.end(); ++it)
            seed ^= static_cast<std::size_t>(*it)
                + /*0x9e3779b9 +*/ 0x9e370001UL + (seed << 6) + (seed >> 2);
        return seed;
    }

    template <typename valueT>
    valueT *mmap_alloc(std::size_t size) 
    {
        valueT * result = static_cast<valueT *>(std::malloc(size));
#if defined(_MSC_VER)
        DWORD dwOldProtect;
        BOOL bret = VirtualProtect(
            result,
            size,
            PAGE_EXECUTE_READWRITE,
            &dwOldProtect);
        ES_ASSERT(bret);
#endif
        return result;
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct  es_value_or_hint
    //
    template <typename primitiveT>
    struct es_value_or_hint
    {
        es_value_or_hint(primitiveT& primitive)
        : p_primitive_(&primitive)
        {
        }
        
        es_value_or_hint(std::ptrdiff_t const hint)
        : hint_(hint)
        {
        }
        
        bool const is_hint() const
        {
            return 0x3 == (0x3 & hint_);
        }
        
        operator primitiveT *() const
        {
            return is_hint() ? 0 : p_primitive_;
        }
        
        operator primitiveT& () const 
        {
            return *p_primitive_;
        }

        operator std::ptrdiff_t const () const 
        {
            return hint_;
        }
        
    private:
        union {
            primitiveT *p_primitive_;
            std::ptrdiff_t hint_;
        };
        
    };

} // namespace ecmascript


