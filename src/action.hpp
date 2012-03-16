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




#include "actor.hpp"

namespace ecmascript {

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_heap_range
    //
    template <typename T>
    struct es_heap_range
    {
        typedef T value_type;

        explicit es_heap_range(value_type *first, value_type *last) throw()
        : min_(first)
        , max_(last)
        {
        }

        value_type const* begin() const throw() { return min_; }

        value_type const* end() const throw() { return max_; }

        void resize()
        {
            size_t const size = (max_ - min_) * 2;
            max_ = min_ + size;
            min_ = reinterpret_cast<value_type *>(
                std::realloc(min_, sizeof(value_type *) * size));
        }

    private:
        value_type * min_;
        value_type * max_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_parse_stack
    //
    template <typename T, unsigned int initial_size = 23280>
    struct es_parse_stack
    {
        typedef T value_type;

        es_parse_stack() throw()
        : sp_(reinterpret_cast<value_type *>(
            std::malloc(initial_size * sizeof(value_type *))))
        , range_(sp_, sp_ + initial_size)
        {
        }
        
        ~es_parse_stack() throw() { std::free(sp_); }

        void push(value_type value) throw()
        {
            if (sp_ == range_.end() - 1)
                range_.resize();
            *++sp_ = value;
        }

        value_type const top() const throw() { return *sp_; }

        value_type const pop() throw() { return *(sp_--); }

    private:
        value_type * sp_;
        es_heap_range<T> range_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_parsed_element
    //
    template <typename iteratorT>
    struct es_parsed_element
    {
        typedef es_parsed_element self_t;

        es_parsed_element(es_parsed_element const& rhs) throw()
        : p_closure_(rhs.p_closure_)
        {
        }

        es_parsed_element(IClosure * const p_closure) throw()
        : p_closure_(p_closure)
        {
        }

        es_parsed_element(iteratorT const& it) throw()
        : it_(it)
        {
        }

        es_parsed_element const& operator =(self_t const& rhs) throw()
        {
            return p_closure_ = rhs.p_closure_, *this;
        }

        IClosure * const& operator =(IClosure * const p_closure) throw()
        {
            return p_closure_ = p_closure;
        }

        iteratorT const& operator =(iteratorT const it) throw()
        {
            return it_ = it;
        }

        operator IClosure &() const throw()
        {
            return *p_closure_;
        }

        operator iteratorT const() const throw() { return it_; }

    private:
        union {
            IClosure * p_closure_;
            iteratorT it_;
        };

        ES_STATIC_ASSERT (sizeof(IClosure *) == sizeof(iteratorT));

    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_semantic_action_base
    //
    struct es_semantic_action_base
    {
        typedef es_parsed_element<wchar_t const*> value_type;
        typedef es_parse_stack<value_type> stack_type;

        void push(value_type value) const throw()
        {
            parse_stack_.push(value);
        }

        value_type const top() const throw()
        {
            return parse_stack_.top();
        }

        value_type const pop() const throw()
        {
            return parse_stack_.pop();
        }

    private:
        static stack_type parse_stack_;
    };

    es_semantic_action_base::stack_type es_semantic_action_base::parse_stack_;

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_unary
    //
    struct es_action_unary
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_unary_operator(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_binary
    //
    struct es_action_binary
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_binary_operator(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_nop
    //
    struct es_action_nop
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            this->push(new es_lazy_closure<Nop>());
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_function
    //
    struct es_action_function
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Func>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_functionbody
    //
    struct es_action_functionbody
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<FunctionRoot>(arg1, arg2));
        }

    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_parameterlist
    //
    struct es_action_parameterlist
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            this->push(static_cast<wchar_t *>(0));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_parameter
    //
    struct es_action_parameter
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT first, iteratorT last) const
        {
            this->push(new es_lazy_closure<Parameter>(first, last));
        }

    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_identifier
    //
    struct es_action_identifier
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT first, iteratorT last) const
        {
            this->push(new es_lazy_closure<Identifier>(first, last));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_var
    //
    struct es_action_var
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            iteratorT const arg1 = this->pop();
            iteratorT const arg2 = this->pop();
            this->push(new es_lazy_closure<Var>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_varinit
    //
    struct es_action_varinit
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            iteratorT const arg2 = this->pop();
            iteratorT const arg3 = this->pop();
            this->push(new es_lazy_closure<VarInit>(arg1, arg2, arg3));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_forinvar
    //
    struct es_action_forinvar
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            wchar_t const* const arg1 = this->pop();
            wchar_t const* const arg2 = this->pop();
            this->push(new es_lazy_closure<ForInVar>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_forinvarinit
    //
    struct es_action_forinvarinit
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            wchar_t const* const arg2 = this->pop();
            wchar_t const* const arg3 = this->pop();
            this->push(new es_lazy_closure<ForInVarInit>(arg1, arg2, arg3));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_member
    //
    struct es_action_member
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT first, iteratorT last) const
        {
            this->push(new es_lazy_closure<Member>(first, last));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_bracket
    //
    struct es_action_bracket
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Bracket>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_undefined
    //
    struct es_action_undefined
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            this->push(new es_lazy_closure<Undefined>());
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_number
    //
    struct es_action_number
    : es_semantic_action_base
    {
        template <typename valueT>
        void
        operator () (valueT d) const
        {
//            if (ecmascript::uint32_t(d) == d)
//                this->push(new es_lazy_closure<SmallInteger>(double(d)));
//            else
                this->push(new es_lazy_closure<Number>(double(d)));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_singlequotedstring
    //
    struct es_action_singlequotedstring
    : es_semantic_action_base
    {
        void
        operator () (const_string_t const& str) const
        {
            this->push(new es_lazy_closure<SingleQuotedString>(
                str.begin(), str.end()));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_doublequotedstring
    //
    struct es_action_doublequotedstring
    : es_semantic_action_base
    {
        void
        operator () (const_string_t const& str) const
        {
            this->push(new es_lazy_closure<DoubleQuotedString>(
                str.begin(), str.end()));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_string
    //
    struct es_action_string
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT first, iteratorT last) const
        {
            this->push(new es_lazy_closure<String>(first, last));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_args
    //
    struct es_action_args
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Args>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_arg
    //
    struct es_action_arg
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Arg>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_argend
    //
    struct es_action_argend
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            this->push(new es_lazy_closure<ArgEnd>());
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_newargend
    //
    struct es_action_newargend
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            this->push(new es_lazy_closure<NewArgEnd>());
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_nativestring
    //
    struct es_action_nativestring
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT first, iteratorT last) const
        {
            this->push(last);
            this->push(first);
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_regexp
    //
    struct es_action_regexp
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT first, iteratorT last) const
        {
            iteratorT const arg1 = this->pop();
            iteratorT const arg2 = this->pop();
            this->push(new es_lazy_closure<RegExp>(arg1, arg2, first, last));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_nullstring
    //
    struct es_action_nullstring
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            this->push(new es_lazy_closure<NullString>());
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_true
    //
    struct es_action_true
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            this->push(new es_lazy_closure<True>());
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_false
    //
    struct es_action_false
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            this->push(new es_lazy_closure<False>());
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_null
    //
    struct es_action_null
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator ()(iteratorT first, iteratorT last) const
        {
            this->push(new es_lazy_closure<Null>());
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_this
    //
    struct es_action_this
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator ()(iteratorT first, iteratorT last) const
        {
            this->push(new es_lazy_closure<This>());
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_call
    //
    struct es_action_call
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Call>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_array
    //
    struct es_action_array
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Array>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_arrayelement
    //
    struct es_action_arrayelement
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            IClosure& arg2 = this->pop();
            this->push(new es_lazy_closure<ArrayElement>(arg2, arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_object
    //
    struct es_action_object
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Object>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_objectelement
    //
    struct es_action_objectelement
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            IClosure& arg2 = this->pop();
            IClosure& arg3 = this->pop();
            this->push(new es_lazy_closure<ObjectElement>(arg3, arg2, arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_new
    //
    struct es_action_new
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<New>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_postinc
    //
    struct es_action_postinc
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<PostInc>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_postdec
    //
    struct es_action_postdec
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<PostDec>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_inc
    //
    struct es_action_inc
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Inc>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_dec
    //
    struct es_action_dec
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Dec>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_unaryplus
    //
    struct es_action_unaryplus
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<UnaryPlus>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_unaryminus
    //
    struct es_action_unaryminus
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<UnaryMinus>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_tilde
    //
    struct es_action_tilde
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Tilde>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_not
    //
    struct es_action_not
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Not>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_void
    //
    struct es_action_void
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Void>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_delete
    //
    struct es_action_delete
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Delete>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_typeof
    //
    struct es_action_typeof
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<TypeOf>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_mul
    //
    struct es_action_mul
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Mul>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_div
    //
    struct es_action_div
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Div>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_mod
    //
    struct es_action_mod
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Mod>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_plus
    //
    struct es_action_plus
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Plus>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_minus
    //
    struct es_action_minus
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Minus>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_shl
    //
    struct es_action_shl
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Shl>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_sar
    //
    struct es_action_sar
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Sar>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_shr
    //
    struct es_action_shr
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Shr>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_le
    //
    struct es_action_le
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Le>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_ge
    //
    struct es_action_ge
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Ge>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_lt
    //
    struct es_action_lt
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Lt>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_gt
    //
    struct es_action_gt
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Gt>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_instanceof
    //
    struct es_action_instanceof
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<InstanceOf>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_in
    //
    struct es_action_in
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<In>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_stricteq
    //
    struct es_action_stricteq
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<StrictEq>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_strictne
    //
    struct es_action_strictne
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<StrictNe>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_eq
    //
    struct es_action_eq
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Eq>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_ne
    //
    struct es_action_ne
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Ne>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_bitand
    //
    struct es_action_bitand
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<BitAnd>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_bitxor
    //
    struct es_action_bitxor
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<BitXor>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_bitor
    //
    struct es_action_bitor
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<BitOr>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_and
    //
    struct es_action_and
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<And>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_or
    //
    struct es_action_or
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Or>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_alternative
    //
    struct es_action_alternative
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Alternative>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_if
    //
    struct es_action_if
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<If>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_ifelse
    //
    struct es_action_ifelse
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<IfElse>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_assign
    //
    struct es_action_assign
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Assign>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_assignmul
    //
    struct es_action_assignmul
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<AssignMul>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_assigndiv
    //
    struct es_action_assigndiv
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<AssignDiv>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_assignmod
    //
    struct es_action_assignmod
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<AssignMod>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_assignplus
    //
    struct es_action_assignplus
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<AssignPlus>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_assignminus
    //
    struct es_action_assignminus
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<AssignMinus>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_assignshl
    //
    struct es_action_assignshl
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<AssignShl>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_assignshr
    //
    struct es_action_assignshr
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<AssignShr>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_assignsar
    //
    struct es_action_assignsar
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<AssignSar>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_assignand
    //
    struct es_action_assignand
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<AssignAnd>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_assignxor
    //
    struct es_action_assignxor
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<AssignXor>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_assignor
    //
    struct es_action_assignor
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<AssignOr>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_dowhile
    //
    struct es_action_dowhile
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<DoWhile>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_while
    //
    struct es_action_while
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<While>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_for
    //
    struct es_action_for
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg3 = this->pop();
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<For>(arg1, arg2, arg3));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_forin
    //
    struct es_action_forin
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg3 = this->pop();
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push((new es_lazy_closure<ForIn>(arg1, arg2, arg3)));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_switch
    //
    struct es_action_switch
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Switch>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_case
    //
    struct es_action_case
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Case>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_default
    //
    struct es_action_default
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Default>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_continue
    //
    struct es_action_continue
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT first, iteratorT last) const
        {
            this->push(new es_lazy_closure<Continue>(first, last));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_continuenoarg
    //
    struct es_action_continuenoarg
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            this->push(new es_lazy_closure<ContinueNoArg>());
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_break
    //
    struct es_action_break
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT first, iteratorT last) const
        {
            this->push(new es_lazy_closure<Break>(first, last));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_breaknoarg
    //
    struct es_action_breaknoarg
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            this->push(new es_lazy_closure<BreakNoArg>());
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_return
    //
    struct es_action_return
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Return>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_returnnoarg
    //
    struct es_action_returnnoarg
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            this->push(new es_lazy_closure<ReturnNoArg>());
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_throw
    //
    struct es_action_throw
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<Throw>(arg1));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_with
    //
    struct es_action_with
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<With>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_trycatch
    //
    struct es_action_trycatch
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg3 = this->pop();
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<TryCatch>(arg1, arg2, arg3));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_tryfinally
    //
    struct es_action_tryfinally
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg2 = this->pop();
            IClosure& arg1 = this->pop();
            this->push(new es_lazy_closure<TryFinally>(arg1, arg2));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct es_action_label
    //
    struct es_action_label
    : es_semantic_action_base
    {
        template <typename iteratorT>
        void
        operator () (iteratorT /*first*/, iteratorT /*last*/) const throw()
        {
            IClosure& arg1 = this->pop();
            iteratorT const arg2 = this->pop();
            iteratorT const arg3 = this->pop() - 1;
            this->push(new es_lazy_closure<Label>(arg1, arg2, arg3));
        }
    };

} // namespace ecmascript

