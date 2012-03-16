/* ***** BEGIN LICENSE BLOCK Version: GPL 3.0 ***** 
 * This code (sprit.hpp) is derived from boost::spirit (http://boost-spirit.com), 
 * written by Joel de Guzman, distributed under http://www.boost.org/LICENSE_1_0.txt.
 * Here is modified and partically rewited by Hayaki Saito <user@zuse.jp>.
 * ***** END LICENSE BLOCK ***** */

namespace ecmascript { namespace spirit {

    template <typename T>
    struct call_traits
    {
       typedef T value_type;
       typedef T& reference;
       typedef T const& const_reference;
       typedef T const& param_type;
    };

    struct nil_t
    {
    };

    template <typename T = nil_t>
    struct match
    {
        typedef T optional_type;
        typedef T attr_t;

        match(std::size_t length, T val_)
        : len(length)
        , val(val_)
        {
        }

        std::ptrdiff_t length() const
        {
            return len;
        }

        T value() const
        {
            return val;
        }

        template <typename T2>
        match(match<T2> const& other)
        : len(other.length())
        , val()
        {
        }

        operator bool() const
        {
            return len >= 0;
        }

    private:
        std::ptrdiff_t len;
        T val;
    };

    template <>
    struct match<nil_t>
    {
        typedef nil_t attr_t;
        typedef nil_t return_t;

        match()
        : len(-1)
        {
        }

        match(std::size_t length, nil_t)
        : len(length)
        {
        }

        std::ptrdiff_t length() const
        {
            return len;
        }

        nil_t value() const
        {
            return nil_t();
        }

        template <typename T>
        match(match<T> const& other)
        : len(other.length())
        {
        }

        template <typename T>
        void
        concat(match<T> const& other)
        {
            len += other.length();
        }

        operator bool() const
        {
            return len >= 0;
        }

    private:
        std::ptrdiff_t len;
    };

    template <typename MatchPolicyT, typename T>
    struct match_result
    {
        typedef typename MatchPolicyT::template result<T>::type type;
    };

    template <
        typename IterationPolicyT,
        typename MatchPolicyT,
        typename ActionPolicyT>
    struct scanner_policies :
        public IterationPolicyT,
        public MatchPolicyT,
        public ActionPolicyT
    {
        typedef IterationPolicyT iteration_policy_t;
        typedef MatchPolicyT match_policy_t;
        typedef ActionPolicyT action_policy_t;

        scanner_policies(
            IterationPolicyT const& i_policy = IterationPolicyT())
        : IterationPolicyT(i_policy)
        {
        }
    };

    template <
        typename IteratorT,
        typename PoliciesT>
    struct scanner
    : public PoliciesT
    {
        typedef IteratorT iterator_t;
        typedef PoliciesT policies_t;

        typedef typename std::iterator_traits<IteratorT>::value_type value_t;
        typedef typename std::iterator_traits<IteratorT>::reference ref_t;
        typedef typename call_traits<IteratorT>::param_type iter_param_t;

        scanner(
            IteratorT& first_,
            iter_param_t last_,
            PoliciesT const& policies)
        : PoliciesT(policies)
        , first(first_)
        , last(last_)
        {
        }

        template <typename PoliciesT1>
        scanner(scanner<IteratorT, PoliciesT1> const& other)
        : PoliciesT(other), first(other.first), last(other.last)
        {
        }

        bool at_end() const
        {
            return PoliciesT::iteration_policy_t::at_end(*this);
        }

        value_t operator*() const
        {
            typedef typename PoliciesT::iteration_policy_t iteration_policy_type;
            return iteration_policy_type::filter(iteration_policy_type::get(*this));
        }

        scanner const&
        operator++() const
        {
            PoliciesT::iteration_policy_t::advance(*this);
            return *this;
        }

        template <typename PoliciesT2>
        scanner<IteratorT, PoliciesT2>
        change_policies(PoliciesT2 const& policies) const
        {
            return scanner<IteratorT, PoliciesT2>(first, last, policies);
        }

        IteratorT& first;
        IteratorT const last;

    private:
        scanner&
        operator=(scanner const& other);
    };

    template <typename ParserT, typename ScannerT>
    struct parser_result
    {
        typedef typename ParserT::template result<ScannerT>::type type;
    };

    template <typename S, typename BaseT>
    struct unary
    : public BaseT
    {
        typedef BaseT base_t;
        typedef typename call_traits<S>::param_type param_t;
        typedef typename call_traits<S>::const_reference return_t;
        typedef typename S::embed_t subject_embed_t;

        unary(param_t subj_)
        : base_t()
        , subj(subj_)
        {
        }

        return_t
        subject() const
        {
            return subj;
        }

    private:
        subject_embed_t subj;
    };

    template <typename A, typename B, typename BaseT>
    struct binary
    : public BaseT
    {
        typedef BaseT base_t;
        typedef typename call_traits<A>::param_type left_param_t;
        typedef typename call_traits<A>::const_reference left_return_t;
        typedef typename call_traits<B>::param_type right_param_t;
        typedef typename call_traits<B>::const_reference right_return_t;
        typedef typename A::embed_t left_embed_t;
        typedef typename B::embed_t right_embed_t;

        binary(left_param_t a, right_param_t b)
        : base_t()
        , first_(a)
        , second_(b)
        {
        }

        left_return_t left() const
        {
            return first_;
        }

        right_return_t right() const
        {
            return second_;
        }

    private:
        left_embed_t first_;
        right_embed_t second_;
    };

    template <typename DerivedT>
    struct parser
    {
        template <typename ParserT, typename ActionT>
        struct action
        : public unary<ParserT, parser<action<ParserT, ActionT> > >
        {
            typedef action<ParserT, ActionT> self_t;
            typedef unary<ParserT, parser<self_t> > base_t;
            typedef ActionT predicate_t;

            template <typename ScannerT>
            struct result
            {
                typedef typename parser_result<ParserT, ScannerT>::type type;
            };

            action(ParserT const& p, ActionT const& a)
            : base_t(p)
            , actor(a)
            {
            }

            template <typename ScannerT>
            typename parser_result<self_t, ScannerT>::type
            parse(ScannerT const& scan) const
            {
                typedef typename ScannerT::iterator_t iterator_t;
                typedef typename parser_result<self_t, ScannerT>::type result_t;
                scan.at_end();
                iterator_t save = scan.first;
                result_t hit = this->subject().parse(scan);
                if (!hit)
                    return hit;
                typename result_t::attr_t val = hit.value();
                scan.do_action(actor, val, save, scan.first);
                return hit;
            }

        private:
            ActionT actor;
        };

        typedef DerivedT embed_t;

        template <typename ScannerT>
        struct result
        {
            typedef typename match_result<ScannerT, nil_t>::type type;
        };

        DerivedT const& derived() const
        {
            return *static_cast<DerivedT const*>(this);
        }

        template <typename ActionT>
        action<DerivedT, ActionT>
        operator[](ActionT const& actor) const
        {
            return action<DerivedT, ActionT>(derived(), actor);
        }
    };

    template <typename BaseT>
    struct skipper_iteration_policy
    : public BaseT
    {
        typedef BaseT base_t;

        skipper_iteration_policy()
        : BaseT()
        {
        }

        template <typename ScannerT>
        void
        advance(ScannerT const& scan) const
        {
            BaseT::advance(scan);
            scan.skip(scan);
        }

        template <typename ScannerT>
        bool
        at_end(ScannerT const& scan) const
        {
            scan.skip(scan);
            return BaseT::at_end(scan);
        }
    };

    template <typename BaseT>
    struct no_skipper_iteration_policy
    : public BaseT
    {
        typedef BaseT base_t;

        template <typename PolicyT>
        no_skipper_iteration_policy(PolicyT const& other)
        : BaseT(other)
        {
        }

        template <typename ScannerT>
        void
        skip(ScannerT const& /*scan*/) const
        {
        }
    };

    template <typename ParserT, typename BaseT>
    struct skip_parser_iteration_policy
    : public skipper_iteration_policy<BaseT>
    {
        typedef skipper_iteration_policy<BaseT> base_t;

        skip_parser_iteration_policy(ParserT const& skip_parser)
        : base_t()
        , subject(skip_parser)
        {
        }

        template <typename ScannerT>
        void
        skip(ScannerT const& scan) const
        {
            typedef typename ScannerT::iterator_t iterator_t;
            typedef scanner_policies<
                no_skipper_iteration_policy<
                    typename ScannerT::iteration_policy_t>,
                typename ScannerT::match_policy_t,
                typename ScannerT::action_policy_t
            > policies_t;
            scanner<iterator_t, policies_t>
                scan2(scan.first, scan.last, policies_t(scan));
        loop:
            iterator_t save = scan.first;
            if (subject.parse(scan2))
                goto loop;
            scan.first = save;
        }

        ParserT const&
        skipper() const
        {
            return subject;
        }

    private:
        ParserT const& subject;
    };

    template <typename DerivedT>
    struct char_parser
    : public parser<DerivedT>
    {
        template <typename ScannerT>
        typename parser_result<DerivedT, ScannerT>::type
        parse(ScannerT const& scan) const
        {
            if (scan.at_end())
                return scan.no_match();
            if (!this->derived().test(*scan))
                return scan.no_match();
            ++scan.first;
            return scan.empty_match();
        }
    };

    template <typename CharT = char>
    struct chlit
    : public char_parser<chlit<CharT> >
    {
        chlit(CharT ch_)
        : ch(ch_)
        {
        }

        template <typename T>
        bool test(T ch_) const
        {
            return ch_ == ch;
        }

    private:
        CharT   ch;
    };

    template <typename IteratorT>
    inline IteratorT
    get_last(IteratorT first)
    {
        while (*first)
            first++;
        return first;
    }

    template <typename IteratorT = char const*>
    struct chseq
    : public parser<chseq<IteratorT> >
    {
        typedef chseq<IteratorT> self_t;

        chseq(IteratorT first_)
        : first(first_)
        , last(get_last(first_))
        {
        }

        chseq(IteratorT first_, IteratorT last_)
        : first(first_)
        , last(last_)
        {
        }

        template <typename ScannerT>
        typename parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const
        {
            IteratorT str_first = first;
        loop:
            if (str_first == last)
                return scan.empty_match();
            if (scan.at_end() || (*str_first != *scan))
                return scan.no_match();
            ++str_first;
            ++scan;
            goto loop;
        }

    private:
        IteratorT first;
        IteratorT last;
    };

    template <typename RT, typename ST, typename ScannerT, typename BaseT>
    inline RT
    contiguous_parser_parse(
        ST const& s,
        ScannerT const& scan,
        skipper_iteration_policy<BaseT> const&);

    template <typename IteratorT = char const*>
    struct strlit
    : public parser<strlit<IteratorT> >
    {
        typedef strlit<IteratorT> self_t;

        strlit(IteratorT first)
        : seq(first)
        {
        }

        strlit(IteratorT first, IteratorT last)
        : seq(first, last)
        {
        }

        template <typename ScannerT>
        typename parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const
        {
            typedef typename parser_result<self_t, ScannerT>::type result_t;
            return contiguous_parser_parse<result_t>(seq, scan, scan);
        }

    private:
        chseq<IteratorT> seq;
    };

    template<typename T>
    struct as_parser
    {
        typedef T type;
    };

    template<>
    struct as_parser<char>
    {
        typedef chlit<char> type;
    };

    template<int N>
    struct as_parser<char const[N]>
    {
        typedef strlit<char const*> type;
    };

    struct difference_parser_gen;

    template <typename A, typename B>
    struct difference
    : public binary<A, B, parser<difference<A, B> > >
    {
        typedef difference<A, B>                self_t;
        typedef binary<A, B, parser<self_t> >   base_t;

        difference(A const& a, B const& b)
        : base_t(a, b)
        {
        }

        template <typename ScannerT>
        typename parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const
        {
            typedef typename parser_result<self_t, ScannerT>::type result_t;
            typename ScannerT::iterator_t save = scan.first;
            result_t hl = this->left().parse(scan);
            if (!hl)
                return scan.no_match();
            std::swap(save, scan.first);
            result_t hr = this->right().parse(scan);
            if (!hr || (hr.length() < hl.length()))
                return scan.first = save, hl;
            return scan.no_match();
        }
    };

    struct difference_parser_gen
    {
        template <typename A, typename B>
        struct result
        {
            typedef
                difference<
                    typename as_parser<A>::type
                  , typename as_parser<B>::type
                >
            type;
        };
    };

    template <typename A, typename B>
    inline difference<A, B>
    operator-(parser<A> const& a, parser<B> const& b)
    {
        return difference<A, B>(a.derived(), b.derived());
    }

    template <typename A>
    inline difference<A, chlit<char> >
    operator-(parser<A> const& a, char b)
    {
        return difference<A, chlit<char> >(a.derived(), b);
    }

    struct sequence_parser_gen;

    template <typename A, typename B>
    struct sequence
    : public binary<A, B, parser<sequence<A, B> > >
    {
        typedef sequence<A, B>                  self_t;
        typedef binary<A, B, parser<self_t> >   base_t;

        sequence(A const& a, B const& b)
        : base_t(a, b)
        {
        }

        template <typename ScannerT>
        typename parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const
        {
            typedef typename parser_result<self_t, ScannerT>::type result_t;
            if (result_t ma = this->left().parse(scan))
                if (this->right().parse(scan))
                    return ma;
            return scan.no_match();
        }
    };

    struct sequence_parser_gen
    {
        template <typename A, typename B>
        struct result
        {
            typedef
                sequence<
                    typename as_parser<A>::type
                  , typename as_parser<B>::type
                >
            type;
        };
    };

    template <typename A, typename B>
    inline sequence<A, B>
    operator>>(parser<A> const& a, parser<B> const& b)
    {
        return sequence<A, B>(a.derived(), b.derived());
    }

    template <typename A>
    inline sequence<A, chlit<char> >
    operator>>(parser<A> const& a, char b)
    {
        return sequence<A, chlit<char> >(a.derived(), b);
    }

    template <typename B>
    inline sequence<chlit<char>, B>
    operator>>(char a, parser<B> const& b)
    {
        return sequence<chlit<char>, B>(a, b.derived());
    }

    template <typename A>
    inline sequence<A, strlit<char const*> >
    operator>>(parser<A> const& a, char const* b)
    {
        return sequence<A, strlit<char const*> >(a.derived(), b);
    }

    template <typename B>
    inline sequence<strlit<char const*>, B>
    operator>>(char const* a, parser<B> const& b)
    {
        return sequence<strlit<char const*>, B>(a, b.derived());
    }

    struct alternative_parser_gen;

    template <typename A, typename B>
    struct alternative
    : public binary<A, B, parser<alternative<A, B> > >
    {
        typedef alternative<A, B> self_t;
        typedef binary<A, B, parser<self_t> > base_t;

        alternative(A const& a, B const& b)
        : base_t(a, b)
        {
        }

        template <typename ScannerT>
        typename parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const
        {
            typedef typename parser_result<self_t, ScannerT>::type result_t;
            typename ScannerT::iterator_t save = scan.first;
            if (result_t hit = this->left().parse(scan))
                return hit;
            scan.first = save;
            return this->right().parse(scan);
        }
    };

    struct alternative_parser_gen
    {
        template <typename A, typename B>
        struct result
        {
            typedef
                alternative<
                    typename as_parser<A>::type
                  , typename as_parser<B>::type
                >
            type;
        };
    };

    template <typename A, typename B>
    inline alternative<A, B>
    operator|(parser<A> const& a, parser<B> const& b)
    {
        return alternative<A, B>(a.derived(), b.derived());
    }

    template <typename A>
    inline alternative<A, chlit<char> >
    operator|(parser<A> const& a, char b)
    {
        return alternative<A, chlit<char> >(a.derived(), b);
    }

    template <typename B>
    inline alternative<chlit<char>, B>
    operator|(char a, parser<B> const& b)
    {
        return alternative<chlit<char>, B>(a, b.derived());
    }

    struct kleene_star_parser_gen;

    template <typename S>
    struct kleene_star
    : public unary<S, parser<kleene_star<S> > >
    {
        typedef kleene_star<S> self_t;
        typedef unary<S, parser<self_t> > base_t;

        kleene_star(S const& a)
        : base_t(a)
        {
        }

        template <typename ScannerT>
        typename parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const
        {
        loop:
            typename ScannerT::iterator_t save = scan.first;
            if (!this->subject().parse(scan))
                return scan.first = save, scan.empty_match();
            goto loop;
        }
    };

    struct kleene_star_parser_gen
    {
        template <typename S>
        struct result
        {
            typedef kleene_star<S> type;
        };
    };

    template <typename S>
    inline kleene_star<S>
    operator*(parser<S> const& a)
    {
        return kleene_star<S>(a.derived());
    }

    struct optional_parser_gen;

    template <typename S>
    struct optional
    : public unary<S, parser<optional<S> > >
    {
        typedef optional<S>                 self_t;
        typedef unary<S, parser<self_t> >   base_t;

        optional(S const& a)
        : base_t(a)
        {
        }

        template <typename ScannerT>
        typename parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const
        {
            typedef typename parser_result<self_t, ScannerT>::type result_t;
            typename ScannerT::iterator_t save = scan.first;
            if (result_t r = this->subject().parse(scan))
                return r;
            scan.first = save;
            return scan.empty_match();
        }
    };

    struct optional_parser_gen
    {
        template <typename S>
        struct result
        {
            typedef optional<S> type;
        };
    };

    template <typename S>
    optional<S>
    operator!(parser<S> const& a)
    {
        return optional<S>(a.derived());
    }

    struct es_lexeme_parser_gen;

    template <typename RT, typename ST, typename ScannerT, typename BaseT>
    inline RT
    contiguous_parser_parse(
        ST const& s,
        ScannerT const& scan,
        skipper_iteration_policy<BaseT> const&)
    {
        typedef scanner_policies<
            no_skipper_iteration_policy<
                typename ScannerT::iteration_policy_t>,
            typename ScannerT::match_policy_t,
            typename ScannerT::action_policy_t
        > policies_t;
        scan.skip(scan);
        return s.parse(scan.change_policies(policies_t(scan)));
    }

    template <typename ParserT>
    struct contiguous
    : public unary<ParserT, parser<contiguous<ParserT> > >
    {
        typedef contiguous<ParserT> self_t;
        typedef unary<ParserT, parser<self_t> > base_t;

        contiguous(ParserT const& p) throw()
        : base_t(p)
        {
        }

        template <typename ScannerT>
        typename parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const
        {
            typedef typename parser_result<self_t, ScannerT>::type result_t;
            return contiguous_parser_parse<result_t>(this->subject(), scan, scan);
        }
    };

    struct es_lexeme_parser_gen
    {
        template <typename ParserT>
        contiguous<ParserT>
        operator[](parser<ParserT> const& subject) const throw()
        {
            return contiguous<ParserT>(subject.derived());
        }
    };

    es_lexeme_parser_gen const es_lexeme_d = es_lexeme_parser_gen();

    template <typename scannerT>
    struct es_rule
    : public parser<es_rule<scannerT> >
    {
        template <typename ScannerT>
        struct es_abstract_parser
        {
            es_abstract_parser() throw()
            {
            }

            virtual ~es_abstract_parser() throw()
            {
            }

            virtual typename match_result<ScannerT, nil_t>::type
                do_parse_virtual(ScannerT const& scan) const = 0;
        };

        template <typename ParserT, typename ScannerT>
        struct es_concrete_parser
        : es_abstract_parser<ScannerT>
        {
            es_concrete_parser(ParserT const& p) throw()
            : p(p)
            {
            }

            virtual ~es_concrete_parser() throw()
            {
            }

            virtual typename match_result<ScannerT, nil_t>::type
            do_parse_virtual(ScannerT const& scan) const throw()
            {
                return p.parse(scan);
            }

            typename ParserT::embed_t p;
        };

        typedef es_rule<scannerT> self_t;
        typedef self_t const& embed_t;

        es_rule() throw()
        : ptr(0)
        {
        }

        ~es_rule() throw()
        {
            delete ptr;
        }

        template <typename ParserT>
        es_rule& operator=(ParserT const& p) throw()
        {
            return ptr = new es_concrete_parser<ParserT, scannerT>(p), *this;
        }
        
        template <typename ScannerT>
        typename parser_result<self_t, ScannerT>::type
        parse(ScannerT const& scan) const throw()
        {
            return ptr->do_parse_virtual(scan);
        }

    private:
        es_rule(self_t const& rhs);
        es_rule& operator=(self_t const& rhs);
        es_abstract_parser<scannerT> * ptr;
    };

} } // namespace ecmascript::spirit

namespace ecmascript {

    struct es_iteration_policy
    {
        template <typename ScannerT>
        void
        advance(ScannerT const& scan) const
        {
            ++scan.first;
        }

        template <typename ScannerT>
        bool
        at_end(ScannerT const& scan) const
        {
            return scan.first == scan.last;
        }

        template <typename T>
        T filter(T ch) const
        {
            return ch;
        }

        template <typename ScannerT>
        typename ScannerT::ref_t
        get(ScannerT const& scan) const
        {
            return *scan.first;
        }
    };

    struct match_policy
    {
        template <typename T>
        struct result { typedef spirit::match<T> type; };

        spirit::match<spirit::nil_t> const
        no_match() const
        {
            return spirit::match<spirit::nil_t>();
        }

        spirit::match<spirit::nil_t> const
        empty_match() const
        {
            return spirit::match<spirit::nil_t>(0, spirit::nil_t());
        }

        template <typename AttrT, typename IteratorT>
        spirit::match<AttrT> const
        create_match(
            std::size_t         length,
            AttrT const&        val,
            IteratorT const&    /*first*/,
            IteratorT const&    /*last*/
            ) const
        {
            return spirit::match<AttrT>(length, val);
        }

    };

    template <typename AttrT>
    struct attributed_action_policy
    {
        template <typename ActorT, typename IteratorT>
        static void
        call(
            ActorT const& actor,
            AttrT& val,
            IteratorT const&,
            IteratorT const&
            )
        {
            actor(val);
        }
    };

    template <>
    struct attributed_action_policy<spirit::nil_t>
    {
        template <typename ActorT, typename IteratorT>
        static void
        call(
            ActorT const& actor,
            spirit::nil_t,
            IteratorT const& first,
            IteratorT const& last
            )
        {
            actor(first, last);
        }
    };

    struct action_policy
    {
        typedef es_parsed_element<wchar_t const*> value_type;
        typedef es_parse_stack<value_type> stack_type;

        action_policy()
        {
        }

        template <typename ActorT, typename AttrT, typename IteratorT>
        void
        do_action(
            ActorT const& actor,
            AttrT& val,
            IteratorT const& first,
            IteratorT const& last
            ) const
        {
            attributed_action_policy<AttrT>::call(actor, val, first, last);
        }

    };
} // namespace ecmascript
