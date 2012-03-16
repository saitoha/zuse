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


#include "unicode.hpp"

//////////////////////////////////////////////////////////////////////////////
//
// regexp object
//

namespace ecmascript { namespace regular_expression {

    //////////////////////////////////////////////////////////////////////////
    //
    // node_id
    //
    enum node_id
    {
        Unknown, Character, GreedyStar, LazyStar, Union,
        Combine, Epsilon, Group, Dot, Final,
        Fail, Digit, NonDigit, Space, NonSpace,
        Word, NonWord, LineEnd, LineStart, Break,
        NonBreak, ClassRange, Repeat, RepeatMin, RepeatMinMax,
        GroupEnd, GroupFail, Root,
    };

    int isdigit(int c) { return ::isdigit(c); }

    int isalnum(int c) { return ::isalnum(c); }

    int isspace(int c) { return ::isspace(c); }

    int isalpha(int c) 
    {
        switch (c)
        {
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
            case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
            case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
            case 's': case 't': case 'u': case 'v': case 'w': case 'x':
            case 'y': case 'z':
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
            case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
            case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
            case 'Y': case 'Z':
                return 1;
            default:
                return 0;
        }
    }
 

    //////////////////////////////////////////////////////////////////////////
    //
    // re_parse_error
    //
    struct re_parse_error
    {
        re_parse_error(char const* message) throw() : message_(message) {}

        char const* what() const throw() { return message_; }

    private:
        const char *message_;
    };

} } // namespace ecmascript::regular_expression


namespace ecmascript { namespace regular_expression { namespace scanner {

    //////////////////////////////////////////////////////////////////////////
    //
    // re_scanner
    //
    template <typename iteratorT>
    struct re_scanner
    {
        typedef iteratorT iterator_t;
        typedef re_scanner<iteratorT> self_t;
        typedef typename std::iterator_traits<iterator_t>::value_type
            value_type;

        explicit re_scanner(iteratorT begin, iteratorT end)
        : first_(begin), current_(begin), last_(end)
        {
        }

        template <typename stringT>
        explicit re_scanner(stringT const& source)
        : first_(source.begin()), current_(source.begin()), last_(source.end())
        {
        }

        self_t& operator ++() { return ++current_, *this; }

        self_t& operator --() { return --current_, *this; }

        value_type const& operator *() const { return *current_; }

        bool is_first() const { return current_ == first_; }

        bool is_end() const { return current_ == last_; }

        bool is_word() const
        {
            return isalnum(*current_) || '_' == *current_ ;
        }

        iteratorT first_, current_, last_;
    };

    template <typename iteratorT>
    re_scanner<iteratorT> re_scanner_gen(iteratorT begin, iteratorT end)
    {
        return re_scanner<iteratorT>(begin, end);
    }

} } } // namespace ecmascript::regular_expression::scanner

//////////////////////////////////////////////////////////////////////////////
//
//  re_node
//
namespace ecmascript { namespace regular_expression { namespace node {

    //////////////////////////////////////////////////////////////////////////
    //
    // re_iterator
    //
    template <typename nodeT>
    struct re_iterator
    {
        typedef nodeT value_type;
        typedef re_iterator<value_type> self_t;

        explicit re_iterator(value_type *p_value) : p_value_(p_value) { }

        self_t const operator ++()
        {
            return self_t(p_value_ = p_value_->next_);
        }

        value_type* const operator ->() const { return p_value_; }

        value_type* const* const operator &() const { return &p_value_; }

        value_type const& operator *() const { return *p_value_; }

        operator value_type const* const() const { return p_value_; }

        bool is_end() const { return 0 == p_value_; }

    private:
        value_type * p_value_;
    };

    namespace action {

        namespace {

        //////////////////////////////////////////////////////////////////////
        //
        // re_character_action
        //
        struct re_character_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                if (scan.is_end())
                    return it->go_to_false_node(scan, matcher);
                if (*scan == it->letter())
                    return it->go_to_true_node(++scan, matcher);
                return it->go_to_false_node(scan, matcher);
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_greedy_star_action
        //
        struct re_greedy_star_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                if (scan.is_end())
                    return it->go_to_true_node(scan, matcher);
                if (-1 == it->children_begin()->action(scan, matcher))
                    return it->go_to_true_node(scan, matcher);
                return 0;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_lazy_star_action
        //
        struct re_lazy_star_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                if (scan.is_end())
                    return it->go_to_true_node(scan, matcher);
                if (-1 == it->go_to_true_node(scan, matcher)
                    && -1 != it->children_begin()->action(scan, matcher))
                        return it->go_to_true_node(scan, matcher);
                return 0;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_union_action
        //
        struct re_union_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                return it->children_begin()->action(scan, matcher);
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_combine_action
        //
        struct re_combine_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                if (-1 != it->children_begin()->action(scan, matcher))
                    return it->go_to_true_node(scan, matcher);
                return it->go_to_false_node(scan, matcher);
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_epsilon_action
        //
        struct re_epsilon_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                return it->go_to_true_node(scan, matcher);
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_group_action
        //
        struct re_group_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                matcher.next_ = new matchT(scan.current_, scan.current_);
                matcher.next_->prev_ = &matcher;
                return it->children_begin()->action(scan, *matcher.next_);
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_dot_action
        //
        struct re_dot_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                if (scan.is_end())
                    return -1;
                return it->go_to_true_node(++scan, matcher);
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_final_action
        //
        struct re_final_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                if (matcher.first_ == scan.current_)
                    return -1;
                matcher.last_ = scan.current_;
                throw scan;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_fail_action
        //
        struct re_fail_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                return -1;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_digit_action
        //
        struct re_digit_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                if (scan.is_end())
                    return -1;
                if (isdigit(*scan))
                    return it->go_to_true_node(++scan, matcher);
                return it->go_to_false_node(scan, matcher);
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_non_digit_action
        //
        struct re_non_digit_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                if (scan.is_end())
                    return it->go_to_false_node(scan, matcher);
                if (!isdigit(*scan))
                    return it->go_to_true_node(++scan, matcher);
                return it->go_to_false_node(scan, matcher);
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_space_action
        //
        struct re_space_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                if (scan.is_end())
                    return it->go_to_false_node(scan, matcher);
                if (isspace(*scan))
                    return it->go_to_true_node(++scan, matcher);
                return it->go_to_false_node(scan, matcher);
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_non_space_action
        //
        struct re_non_space_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                if (scan.is_end())
                    return -1;
                if (!isspace(*scan))
                    return it->go_to_true_node(++scan, matcher);
                return it->go_to_false_node(scan, matcher);
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_word_action
        //
        struct re_word_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                if (scan.is_end())
                    return -1;
                if (isalnum(*scan) || *scan == '_')
                    return it->go_to_true_node(++scan, matcher);
                return it->go_to_false_node(scan, matcher);
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_non_word_action
        //
        struct re_non_word_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                if (scan.is_end())
                    return -1;
                if (!isalnum(*scan) && *scan != '_')
                    return it->go_to_true_node(++scan, matcher);
                return it->go_to_false_node(scan, matcher);
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_line_start_action
        //
        struct re_line_start_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                if (scan.is_first())
                    return it->true_->action(scan, matcher);
                switch (*--scan)
                {
                    case '\r':
                    case '\n':
                    case 0x2028: //	Line Separator
                    case 0x2029: // Paragraph Separator
                        return ++scan, it->go_to_true_node(scan, matcher);
                    default:
                        return ++scan, it->go_to_false_node(scan, matcher);
                }
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_line_end_action
        //
        struct re_line_end_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                if (scan.is_end())
                    return it->true_->action(scan, matcher);
                switch (*scan)
                {
                    case '\r':
                    case '\n':
                    case 0x2028: //	Line Separator
                    case 0x2029: // Paragraph Separator
                        return it->go_to_true_node(++scan, matcher);
                    default:
                        return it->go_to_false_node(scan, matcher);
                }
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_break_action
        //
        struct re_break_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                return current_character_is_word(scan)
                    != previous_character_is_word(scan) ?
                    it->go_to_true_node(scan, matcher):
                    it->go_to_false_node(scan, matcher);
            }

        private:
            template <typename scannerT>
            bool current_character_is_word(scannerT scan) const
            {
                return scan.is_word();
            }

            template <typename scannerT>
            bool previous_character_is_word(scannerT scan) const
            {
                if (scan.is_first())
                    return false;
                bool result = (--scan).is_word();
                return ++scan, result;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_non_break_action
        //
        struct re_non_break_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                return current_character_is_word(scan)
                    == previous_character_is_word(scan) ?
                    it->go_to_true_node(++scan, matcher):
                    it->go_to_false_node(++scan, matcher);
            }

        private:
            template <typename scannerT>
            bool current_character_is_word(scannerT scan) const
            {
                return scan.is_word();
            }

            template <typename scannerT>
            bool previous_character_is_word(scannerT scan) const
            {
                if (scan.is_first())
                    return false;
                bool result = (--scan).is_word();
                return ++scan, result;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_class_range_action
        //
        struct re_class_range_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                if (it->child_->letter_ > *scan)
                    return it->go_to_false_node(scan, matcher);
                if (it->child_->next_->letter_ < *scan)
                    return it->go_to_false_node(scan, matcher);
                return it->go_to_true_node(++scan, matcher);
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_group_end_action
        //
        struct re_group_end_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                matcher.last_ = scan.current_;
                return it->go_to_true_node(scan, *matcher.prev_);
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_group_fail_action
        //
        struct re_group_fail_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                return it->go_to_false_node(scan, *matcher.prev_);
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // re_root_action
        //
        struct re_root_action
        {
            template <typename scannerT, typename iteratorT, typename matchT>
            int action(iteratorT it, scannerT scan, matchT& matcher) const
            {
                return it->children_begin()->action(scan, matcher);
            }
        };

        } // anonymous namespace

        //////////////////////////////////////////////////////////////////////
        //
        // call_action
        //
        template <typename scannerT, typename iteratorT, typename matchT>
        int call_action(iteratorT it, scannerT scan, matchT& matcher)
        {
            switch (it->id_)
            {
                case Character:
                    return re_character_action().action(it, scan, matcher);
                case GreedyStar:
                    return re_greedy_star_action().action(it, scan, matcher);
                case LazyStar:
                    return re_lazy_star_action().action(it, scan, matcher);
                case Union:
                    return re_union_action().action(it, scan, matcher);
                case Combine:
                    return re_combine_action().action(it, scan, matcher);
                case Epsilon:
                    return re_epsilon_action().action(it, scan, matcher);
                case Group:
                    return re_group_action().action(it, scan, matcher);
                case Dot:
                    return re_dot_action().action(it, scan, matcher);
                case Final:
                    return re_final_action().action(it, scan, matcher);
                case Fail:
                    return re_fail_action().action(it, scan, matcher);
                case Digit:
                    return re_digit_action().action(it, scan, matcher);
                case NonDigit:
                    return re_non_digit_action().action(it, scan, matcher);
                case Space:
                    return re_space_action().action(it, scan, matcher);
                case NonSpace:
                    return re_non_space_action().action(it, scan, matcher);
                case Word:
                    return re_word_action().action(it, scan, matcher);
                case NonWord:
                    return re_non_word_action().action(it, scan, matcher);
                case LineStart:
                    return re_line_start_action().action(it, scan, matcher);
                case LineEnd:
                    return re_line_end_action().action(it, scan, matcher);
                case Break:
                    return re_break_action().action(it, scan, matcher);
                case NonBreak:
                    return re_non_break_action().action(it, scan, matcher);
                case ClassRange:
                    return re_class_range_action().action(it, scan, matcher);
                case GroupEnd:
                    return re_group_end_action().action(it, scan, matcher);
                case GroupFail:
                    return re_group_fail_action().action(it, scan, matcher);
                case Root:
                    return re_root_action().action(it, scan, matcher);
                default:
                    throw;
            }
        }

    } // namespace action


    //////////////////////////////////////////////////////////////////////////
    //
    // re_node
    //
    template <typename charT>
    struct re_node
    {
        typedef charT char_t;
        typedef re_node<char_t> self_t;
        typedef re_iterator<self_t> iterator;

        re_node()
        : letter_(0), id_(Unknown), next_(0), child_(0), true_(0), false_(0)
        {
        }

        explicit re_node(node_id id)
        : letter_(0), id_(id), next_(0), child_(0), true_(0), false_(0)
        {
        }

        explicit re_node(char_t c)
        : letter_(c), id_(Character), next_(0), child_(0), true_(0), false_(0)
        {
        }

        self_t const& insert(self_t& node)
        {
            return next_ == 0 ? *(next_ = &node): next_->insert(node);
        }

        self_t const& add(self_t& node)
        {
            return child_ == 0 ? *(child_ = &node): child_->insert(node);
        }

        self_t& clone() const { return *new self_t(*this); }

        template <typename scannerT, typename matchT>
        int go_to_true_node(scannerT scan, matchT& matcher) const
        {
            return true_->action(scan, matcher);
        }

        template <typename scannerT, typename matchT>
        int go_to_false_node(scannerT scan, matchT& matcher) const
        {
            return false_ ? false_->action(scan, matcher): -1;
        }

        template <typename scannerT, typename matchT>
        int action(scannerT scan, matchT& matcher)
        {
            return action::call_action(iterator(this), scan, matcher);
        }

        char_t const letter() const { return letter_; }

        iterator children_begin() const { return iterator(child_); }

        char_t const letter_;
        node_id id_;
        self_t * next_;
        self_t * child_;
        self_t * true_;
        self_t * false_;
    };

} } } // namespace ecmascript::regular_expression::node

//////////////////////////////////////////////////////////////////////////
//
//  re_match
//  re_match_test
//
namespace ecmascript { namespace regular_expression { namespace match {

    //////////////////////////////////////////////////////////////////////////
    //
    // @struct re_match
    //
    template <typename iteratorT>
    struct re_match
    {
        typedef iteratorT iterator_t;
        typedef re_match<iterator_t> self_t;
        explicit re_match(iterator_t const begin, const iterator_t end)
        : first_(begin), last_(end), next_(0), prev_(0)
        {
        }

        bool success() const { return first_ != last_; }

//    private:
        iterator_t first_;
        iterator_t last_;
        self_t *next_;
        self_t *prev_;
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // @fn re_match_test
    //
    template <typename scannerT, typename nodeT>
    re_match<typename scannerT::iterator_t>
    re_match_test(scannerT& scan, nodeT& node)
    {
        typedef typename scannerT::iterator_t iterator_t;
        for (; !scan.is_end(); ++scan)
        {
            re_match<iterator_t> matcher(scan.current_, scan.current_);
            try {
                node.action(scan, matcher);
            } catch(scannerT&) {
                return matcher;
            }
        }
        return re_match<iterator_t>(scan.current_, scan.current_);
    }

} } } // namespace ecmascript::regular_expression::match

//////////////////////////////////////////////////////////////////////////////
//
// -*- Pattern
// -*- Disjunction
// -*- Alternative
// -*- Term
// -*- Assertion
// -*- Quantifier
// -*- QuantifierPrefix
// -*- Atom
// -*- PatternCharacter
// -*- AtomEscape
// -*- CharacterEscape
// -*- ControlEscape
// -*- ControlLetter
// -*- IdentityEscape
// -*- DecimalEscape
// -*- CharacterClassEscape
// -*- CharacterClass
// -*- ClassRanges
// -*- NonemptyClassRanges
// -*- NonemptyClassRangesNoDash
// -*- ClassAtom
// -*- ClassAtomNoDash
// -*- ClassEscape
//
namespace ecmascript { namespace regular_expression { namespace parser {

    //////////////////////////////////////////////////////////////////////////
    //
    // re_parser
    //
    struct re_parser
    {
        template <typename scannerT, typename nodeT>
        bool parse(scannerT& scan, nodeT& node) const
        {
            return re_pattern_parser().parse(scan, node);
        }

    private:
        //////////////////////////////////////////////////////////////////////
        //
        // Pattern ::
        // -*- Disjunction
        //
        struct re_pattern_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                if (scan.is_end())
                    return false;
                if (!re_disjunction_parser().parse(scan, node))
                    return false;
                return true;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // Disjunction ::
        // -*- Alternative
        // -*- Alternative | Disjunction
        //
        struct re_disjunction_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                if (scan.is_end())
                    return false;
                nodeT& new_node = *new nodeT(Union);
                if (!parse_impl(scan, new_node))
                    return false;
                return node.add(new_node), true;
            }

        private:
            template <typename scannerT, typename nodeT>
            bool parse_impl(scannerT& scan, nodeT& node) const
            {
                if (!re_alternative_parser().parse(scan, node))
                    return false;
                if (scan.is_end())
                    return true;
                if (*scan != '|')
                    return true;
                return parse_impl(++scan, node), true;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // Alternative ::
        // -*- [empty]
        // -*- Alternative Term
        //
        //   -> [empty] | +Term
        //
        struct re_alternative_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                if (scan.is_end())
                    return node.add(*new nodeT(Epsilon)), true;
                nodeT& new_node = *new nodeT(Combine);
                if (!parse_impl(scan, new_node))
                    return node.add(*new nodeT(Epsilon)), true;
                return node.add(new_node), true;
            }

        private:
            // +Term
            template <typename scannerT, typename nodeT>
            bool parse_impl(scannerT& scan, nodeT& node) const
            {
                if (re_term_parser().parse(scan, node))
                    return parse_impl(scan, node), true;
                return false;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // Term ::
        // -*- Assertion
        // -*- Atom
        // -*- Atom Quantifier
        //
        struct re_term_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                if (scan.is_end())
                    return false;
                if (re_assertion_parser().parse(scan, node))
                    return true;
                nodeT& new_node = *new nodeT();
                if (!re_atom_parser().parse(scan, new_node))
                    return false;
                if (re_quantifier_parser().parse(scan, new_node))
                    return node.add(new_node), true;
                node.add(*new_node.child_);
                delete &new_node;
                return true;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // Assertion ::
        // -*-    ^
        // -*-    $
        // -*-    \ b
        // -*-    \ B
        //
        struct re_assertion_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                switch (*scan)
                {
                    case '^': return parse_hat_impl(++scan, node);
                    case '$': return parse_dollar_impl(++scan, node);
                    case '\\': return parse_backslash_impl(++scan, node);
                    default: return false;
                }
            }

#if _MSC_VER >= 1400
#pragma region // implementation
#endif // _MSC_VER >= 1400

        private:
            template <typename scannerT, typename nodeT>
            bool parse_hat_impl(scannerT& scan, nodeT& node) const
            {
                return node.add(*new nodeT(LineStart)), true;
            }

            template <typename scannerT, typename nodeT>
            bool parse_dollar_impl(scannerT& scan, nodeT& node) const
            {
                return node.add(*new nodeT(LineEnd)), true;
            }

            template <typename scannerT, typename nodeT>
            bool parse_backslash_impl(scannerT& scan, nodeT& node) const
            {
                switch (*scan)
                {
                    case 'b': return ++scan, node.add(*new nodeT(Break)), true;
                    case 'B': return ++scan, node.add(*new nodeT(NonBreak)), true;
                    default: return --scan, false;
                }
            }

#if _MSC_VER >= 1400
#pragma endregion // implementation
#endif // _MSC_VER >= 1400

        };

        //////////////////////////////////////////////////////////////////////
        //
        // Quantifier ::
        // -*- QuantifierPrefix
        // //QuantifierPrefix ?
        //
        struct re_quantifier_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                if (!re_quantifier_prefix_parser().parse(scan, node))
                    return false;
                return true;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // QuantifierPrefix ::
        // -*- * >> !?
        // -*- + >> !?
        // -*- ?  >> !?
        // -*- { DecimalDigits } >> !?
        // -*- { DecimalDigits , } >> !?
        // -*- { DecimalDigits , DecimalDigits } >> !?
        //
        struct re_quantifier_prefix_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                if (scan.is_end())
                    return false;
                switch (*scan)
                {
                    case '*': return parse_star_impl(++scan, node);
                    case '+': return parse_plus_impl(++scan, node);
                    case '?': return parse_question_impl(++scan, node);
                    case '{': return parse_bracket_impl(++scan, node);
                    default: return false;
                }
            }

#if _MSC_VER >= 1400
#pragma region // implementation
#endif // _MSC_VER >= 1400

        private:
            template <typename scannerT, typename nodeT>
            bool parse_star_impl(scannerT& scan, nodeT& node) const
            {
                if (scan.is_end() || *scan != '?')
                    return node.id_ = GreedyStar, true;
                ++scan;
                node.id_ = LazyStar;
                return true;
            }

            template <typename scannerT, typename nodeT>
            bool parse_plus_impl(scannerT& scan, nodeT& node) const
            {
                nodeT& star_node = *new nodeT(GreedyStar);
                parse_star_impl(scan, star_node);
                star_node.add(*new nodeT(*node.child_));
                node.id_ = Combine;
                node.add(star_node);
                return true;
            }

            template <typename scannerT, typename nodeT>
            bool parse_question_impl(scannerT& scan, nodeT& node) const
            {
                node.id_ = Union;
                nodeT& epsilon_node = *new nodeT(Epsilon);
                if (*scan != '?')
                    return node.add(epsilon_node), true;
                ++scan;
                epsilon_node.next_ = node.child_;
                node.child_ = &epsilon_node;
                return true;
            }

            template <typename scannerT, typename nodeT>
            bool parse_bracket_impl(scannerT& scan, nodeT& node) const
            {
                unsigned int first, second;
                node.id_ = Combine;
                if (!re_decimal_digits_parser().parse(scan, node, first))
                    throw re_parse_error("illegal quantifier: digits are expected.");
                if (*scan == '}') // match - { decimaldigits }
                    return parse_repeat_impl(++scan, node, first);
                if (*scan != ',' || (++scan).is_end())
                    throw re_parse_error("illegal quantifier: [,}] is expected.");
                if (*scan == '}') // match - { DecimalDigits , }
                    return parse_repeat_min_impl(++scan, node, first);
                if (!re_decimal_digits_parser().parse(scan, node, second))
                    throw re_parse_error("illegal quantifier: digits are expected.");
                if (*scan == '}') // match - { DecimalDigits , DecimalDigits }
                    return parse_repeat_min_max_impl(++scan, node, first, second);
                throw re_parse_error("illegal quantifier: '}' is expected.");
            }

            template <typename scannerT, typename nodeT>
            bool parse_repeat_impl(
                scannerT& scan, nodeT& node, unsigned int repeat_count) const
            {
                if (repeat_count == 0)
                    return false;
                for (nodeT *p_repeat_node = new nodeT(*node.child_);
                    --repeat_count;
                    p_repeat_node = new nodeT(*p_repeat_node))
                    node.add(*p_repeat_node);
                return true;
            }

            template <typename scannerT, typename nodeT>
            bool parse_repeat_min_impl(
                scannerT& scan, nodeT& node,
                unsigned int repeat_min_count) const
            {
                nodeT& star_node = *new nodeT;
                parse_star_impl(scan, star_node);
                star_node.add(node.child_->clone());
                for (nodeT *p_repeat_node = &node.child_->clone();
                    --repeat_min_count > 0;)
                    p_repeat_node = &node.add(*p_repeat_node).clone();
                node.add(star_node);
                return true;
            }

            template <typename scannerT, typename nodeT>
            bool parse_repeat_min_max_impl(
                scannerT& scan, nodeT& node,
                unsigned int repeat_min_count,
                unsigned int repeat_max_count) const
            {
                if (repeat_min_count > repeat_max_count)
                    return false;
                if (0 == repeat_max_count)
                    return false;
                size_t diff = repeat_max_count - repeat_min_count;
                nodeT *p_repeat_node = &node.child_->clone();
                if (0 == repeat_min_count)
                    delete node.child_, node.child_ = 0;
                else
                    while (--repeat_min_count)
                        p_repeat_node = &node.add(*p_repeat_node).clone();
                while (diff--)
                {
                    nodeT& non_deterministic_node = *new nodeT(Union);
                    non_deterministic_node.add(p_repeat_node->clone());
                    non_deterministic_node.add(*new nodeT(Epsilon));
                    node.add(non_deterministic_node);
                }
                return true;
            }

#if _MSC_VER >= 1400
#pragma endregion // implementation
#endif // _MSC_VER >= 1400

        };

        //////////////////////////////////////////////////////////////////////
        //
        // Atom ::
        // -*- PatternCharacter
        // -*-    .
        // -*-    \ AtomEscape
        // -*-    CharacterClass
        // -*-    ( Disjunction )
        //     ( ? : Disjunction )
        //     ( ? = Disjunction )
        //     ( ? ! Disjunction )
        //
        struct re_atom_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                if (scan.is_end())
                    return false;
                if (re_pattern_character_parser().parse(scan, node))
                    return true;
                if (parse_impl_dot(scan, node))
                    return true;
                if (parse_impl_atom_escape(scan, node))
                    return true;
                if (re_character_class_parser().parse(scan, node))
                    return true;
                if (parse_impl_grouped(scan, node))
                    return true;
                return false;
            }

#if _MSC_VER >= 1400
#pragma region // implementation
#endif // _MSC_VER >= 1400

        private:
            template <typename scannerT, typename nodeT>
            int parse_impl_atom_escape(scannerT& scan, nodeT& node) const
            {
                if (*scan != '\\')
                    return false;
                ++scan;
                if (!re_atom_escape_parser().parse(scan, node))
                    return false;
                return true;
            }

            template <typename scannerT, typename nodeT>
            int parse_impl_grouped(scannerT& scan, nodeT& node) const
            {
                if (*scan != '(')
                    return false;
                ++scan;
                nodeT& new_node = *new nodeT(Group);
                if (!re_disjunction_parser().parse(scan, new_node))
                    throw re_parse_error("atom: a disjunction is expected.");
                if (*scan != ')')
                    throw re_parse_error("atom: ')' is expected.");
                ++scan;
                new_node.add(*new nodeT(GroupFail));
                new_node.add(*new nodeT(GroupEnd));
                node.add(new_node);
                return true;
            }

            template <typename scannerT, typename nodeT>
            int parse_impl_dot(scannerT& scan, nodeT& node) const
            {
                if (scan.is_end())
                    return false;
                if (*scan != '.')
                    return false;
                ++scan;
                node.add(*new nodeT(Dot));
                return true;
            }

#if _MSC_VER >= 1400
#pragma endregion // implementation
#endif // _MSC_VER >= 1400

        };

        //////////////////////////////////////////////////////////////////////
        //
        // PatternCharacter :: SourceCharacter but not any of:
        // -*- ^$\.*+?()[]{}|
        //
        struct re_pattern_character_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                switch (*scan)
                {
                    case '^': case '$': case '\\': case '.': case '*': case '+':
                    case '?': case '(': case ')': case '[': case ']': case '{':
                    case '}': case '|':
                        return false;
                    default:
                        node.add(*new nodeT(*scan));
                        ++scan;
                        return true;
                }
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // AtomEscape ::
        // -*-    DecimalEscape
        // -*-    CharacterEscape
        // -*-    CharacterClassEscape
        //
        struct re_atom_escape_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                if (re_decimal_escape_parser().parse(scan, node))
                    return true;
                if (re_character_escape_parser().parse(scan, node))
                    return true;
                if (re_character_class_escape_parser().parse(scan, node))
                    return true;
                return false;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // CharacterEscape ::
        // -*-    ControlEscape
        // -*-    c ControlLetter
        //     HexEscapeSequence
        //     UnicodeEscapeSequence
        // -*-    IdentityEscape
        //
        struct re_character_escape_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                unsigned int value = 0;
                if (re_control_escape_parser().parse(scan, node, value))
                    return node.add(*new nodeT(value)), true;
                if (parse_control_letter_impl(scan, node, value))
                    return node.add(*new nodeT(value)), true;
                if (re_identity_escape_parser().parse(scan, node, value))
                    return node.add(*new nodeT(value)), true;
                return false;
            }

        private:
            // c ControlLetter
            template <typename scannerT, typename nodeT>
            bool parse_control_letter_impl(
                scannerT& scan, nodeT& node, unsigned int& value) const
            {
                if (*scan != 'c')
                    return false;
                ++scan;
                if (re_control_letter_parser().parse(scan, node, value))
                    return true;
                return false;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // ControlEscape :: one of
        // -*-    f n r t v
        //
        struct re_control_escape_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node, unsigned int& value) const
            {
                switch (*scan)
                {
                    case 'f': return value = (unsigned int)('\f'), ++scan, true;
                    case 'n': return value = (unsigned int)('\n'), ++scan, true;
                    case 'r': return value = (unsigned int)('\r'), ++scan, true;
                    case 't': return value = (unsigned int)('\t'), ++scan, true;
                    case 'v': return value = (unsigned int)('\v'), ++scan, true;
                    default:  return false;
                }
                return false;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // IdentityEscape ::
        // -*- SourceCharacter but not IdentifierPart
        //
        struct re_identity_escape_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node, unsigned int& value) const
            {
                if (!unicode::test<IdentifierChar>(*scan))
                    return value = (unsigned int)(*scan), ++scan, true;
                return false;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // ControlLetter :: one of
        // -*-    a b c d e f g h i j k l m n o p q r s t u v w x y z
        //     A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
        //
        struct re_control_letter_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node, unsigned int& value) const
            {
                if (!isalpha(*scan))
                    return false;
                value = (unsigned int)(*scan);
                ++scan;
                return true;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // DecimalEscape ::
        // -*-   DecimalIntegerLiteral [lookahead ( DecimalDigit]
        //
        struct re_decimal_escape_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(
                scannerT& scan, nodeT& node, unsigned int value = 0) const
            {
                if (re_decimal_integer_literal_parser().parse(
                    scan, node, value))
                    return true;
                return false;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // CharacterClassEscape :: one of
        // -*-    d D s S w W
        //
        struct re_character_class_escape_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                switch (*scan)
                {
                    case 'd': node.add(*new nodeT(Digit)); break;
                    case 'D': node.add(*new nodeT(NonDigit)); break;
                    case 's': node.add(*new nodeT(Space)); break;
                    case 'S': node.add(*new nodeT(NonSpace)); break;
                    case 'w': node.add(*new nodeT(Word)); break;
                    case 'W': node.add(*new nodeT(NonWord)); break;
                    default: return false;
                }
                ++scan;
                return true;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // CharacterClass ::
        //     [ [lookahead ( {^} ] ClassRanges ]
        //     [ ^ ClassRanges ]
        //
        struct re_character_class_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                if (*scan != '[')
                    return false;
                ++scan;
                if (*scan == '^')
                    ++scan;
                nodeT& new_node = *new nodeT(Union);
                if (!re_class_ranges_parser().parse(scan, new_node))
                    throw re_parse_error(
                        "illegal character struct: illegal range.");
                if (*scan != ']')
                    throw re_parse_error(
                        "illegal character struct: ']' is expected.");
                node.add(new_node);
                ++scan;
                return true;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // ClassRanges ::
        //     [empty]
        //     NonemptyClassRanges
        //
        struct re_class_ranges_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                if (!re_nonempty_class_ranges_parser().parse(scan, node))
                    return true;
                return true;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        //  NonemptyClassRanges ::
        //      ClassAtom
        //      ClassAtom NonemptyClassRangesNoDash
        //      ClassAtom - ClassAtom ClassRanges
        //
        struct re_nonempty_class_ranges_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                nodeT& new_node = *new nodeT(ClassRange);
                if (!re_class_atom_parser().parse(scan, new_node))
                    return false;
                if (re_nonempty_class_ranges_no_dash_parser().parse(scan, node))
                    return node.add(*new_node.child_), true;
                if (*scan != '-')
                    return node.add(*new_node.child_), true;
                ++scan;
                if (*scan == ']')
                    return node.add(*new_node.child_), node.add(*new nodeT('-')), true;
                if (!re_class_atom_parser().parse(scan, new_node))
                    throw re_parse_error(
                        "illegal struct ranges: a class atom exprected.");
                node.add(new_node);
                if (!re_class_ranges_parser().parse(scan, node))
                    throw re_parse_error(
                        "illegal struct ranges: a struct ranges exprected.");
                return true;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        //  NonemptyClassRangesNoDash ::
        //      ClassAtomNoDash
        //      ClassAtomNoDash NonemptyClassRangesNoDash
        //      ClassAtomNoDash - ClassAtom ClassRanges
        //
        struct re_nonempty_class_ranges_no_dash_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                nodeT& new_node = *new nodeT(ClassRange);
                if (!re_class_atom_no_dash_parser().parse(scan, new_node))
                    return false;
                if (re_nonempty_class_ranges_no_dash_parser().parse(scan, node))
                    return node.add(*new_node.child_), true;
                if (*scan != '-')
                    return node.add(*new_node.child_), true;
                ++scan;
                if (*scan == ']')
                    return node.add(*new_node.child_), node.add(*new nodeT('-')), true;
                if (!re_class_atom_no_dash_parser().parse(scan, new_node))
                    throw re_parse_error(
                        "illegal struct ranges: a struct atom is expected.");
                node.add(new_node);
                if (!re_class_ranges_parser().parse(scan, node))
                    throw re_parse_error(
                        "illegal struct ranges: a struct ranges exprected.");
                return true;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        //  ClassAtom ::
        //      -
        //      ClassAtomNoDash
        //
        struct re_class_atom_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                if (*scan == '-')
                    return ++scan, true;
                if (re_class_atom_no_dash_parser().parse(scan, node))
                    return true;
                return false;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        //  ClassAtomNoDash ::
        //      SourceCharacter but not one of \ ] -
        //      \ ClassEscape
        //
        struct re_class_atom_no_dash_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                if (scan.is_end())
                    return false;
                switch (*scan)
                {
                    case ']':
                    case '-':
                        return false;
                    case '\\':
                        ++scan;
                        return re_class_escape_parser().parse(scan, node);
                    default:
                        node.add(*new nodeT(*scan));
                        ++scan;
                        return true;
                }
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        //  ClassEscape ::
        //      DecimalEscape
        //      b
        //      CharacterEscape
        //      CharacterClassEscape
        //
        struct re_class_escape_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node) const
            {
                if (scan.is_end())
                    return false;
                if (re_decimal_escape_parser().parse(scan, node))
                    return true;
                if (*scan == 'b')
                    return ++scan, true;
                if (re_character_escape_parser().parse(scan, node))
                    return true;
                if (re_character_class_escape_parser().parse(scan, node))
                    return true;
                return false;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // DecimalDigit :: one of
        // -*-    0 1 2 3 4 5 6 7 8 9
        //
        struct re_decimal_digit_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node, unsigned int& value) const
            {
                if (*scan < '0' || *scan > '9')
                    return false;
                value = (unsigned int)(*scan) - '0';
                ++scan;
                return true;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // NonZeroDigit :: one of
        // -*-    1 2 3 4 5 6 7 8 9
        //
        struct re_non_zero_digit_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node, unsigned int& value) const
            {
                if (*scan < '1' || *scan > '9')
                    return false;
                value = (unsigned int)(*scan) - '0';
                ++scan;
                return true;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // DecimalDigits ::
        // -*- DecimalDigit
        // -*- DecimalDigits DecimalDigit
        //
        struct re_decimal_digits_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node, unsigned int& value) const
            {
                if (!re_decimal_digit_parser().parse(scan, node, value))
                    return false;
                unsigned int value_of_next_digit = 0;
                if (parse(scan, node, value_of_next_digit))
                    value = value * 10 + value_of_next_digit;
                return true;
            }
        };

        //////////////////////////////////////////////////////////////////////
        //
        // DecimalIntegerLiteral ::
        // -*-    0
        // -*-    NonZeroDigit DecimalDigits opt
        //
        struct re_decimal_integer_literal_parser
        {
            template <typename scannerT, typename nodeT>
            bool parse(scannerT& scan, nodeT& node, unsigned int& value) const
            {
                if (*scan == '0')
                    return ++scan, value = 0, true;
                if (!re_non_zero_digit_parser().parse(scan, node, value))
                    return false;
                unsigned int value_of_2nd_digit_after = 0;
                if (re_decimal_digit_parser().parse(
                    scan, node, value_of_2nd_digit_after))
                    value = value * 10 + value_of_2nd_digit_after;
                return true;
            }
        };
    };

    //////////////////////////////////////////////////////////////////////////
    //
    // re_nfa_converter
    //
    struct re_nfa_converter
    {
        template <typename scannerT, typename nodeT>
        bool re_create_nfa(scannerT& scan, nodeT& node)
        {
            switch (node.id_)
            {
                case Union:
                    for (nodeT *i = node.child_; i != 0; i = i->next_)
                    {
                        i->true_ = node.true_;
                        i->false_ = i->next_ ? i->next_ : node.false_;
                    }
                    break;
                case Combine:
                    for (nodeT *i = node.child_; i != 0; i = i->next_)
                    {
                        i->true_ = i->next_ ? i->next_ : node.true_;
                        i->false_ = 0;//it->false_;
                    }
                    break;
                case GreedyStar:
                    node.child_->true_ = &node;
                    node.child_->false_ = node.true_;
                    break;
                case LazyStar:
                    node.child_->true_ = &node;
                    node.child_->false_ = node.true_;
                    break;
                case Group:
                    node.child_->false_ = node.child_->next_;
                    node.child_->true_ = node.child_->next_->next_;
                    node.child_->next_->false_ = node.false_;
                    node.child_->next_->next_->true_ = node.true_;
                    break;
                case Root:
                    node.child_->true_ = node.true_;
                    node.child_->false_ = node.false_;
                    break;
                default:
                    return true;
            }
            return true;
        }

        template <typename scannerT, typename nodeT>
        bool re_parse_impl(scannerT& scan, nodeT& node)
        {
            re_create_nfa(scan, node);
            for (nodeT *it = node.child_; it != 0; it = it->next_)
            {
                re_parse_impl(scan, *it);
            }
            return true;
        }

    }; // re_parse

    //////////////////////////////////////////////////////////////////////////
    //
    // re_nfa_converter
    //
    template <typename scannerT, typename nodeT>
    bool re_parse(scannerT& scan, nodeT& node)
    {
        if (!re_parser().parse(scan, node))
            throw re_parse_error("unknown error");
        node.true_ = new nodeT(Final);
        node.false_ = new nodeT(Fail);
        return re_nfa_converter().re_parse_impl<scannerT, nodeT>(scan, node);
    }

} } } // namespace ecmascript::regular_expression::parser

//////////////////////////////////////////////////////////////////////////////
//
// regexp object
//
namespace ecmascript {

    //////////////////////////////////////////////////////////////////////////
    //
    //  @struct es_regexp
    //  @brief
    //
    template <typename stringT>
    struct es_regexp
    : public base_classes::es_object_impl<IRegExp, stringT>
    {
        typedef stringT string_t;
        typedef typename string_t::value_type char_t;
        typedef typename regular_expression::node::re_node<char_t> node_t;
        static IFunction& static_get_constructor()
        {
            static struct constructor
            : public base_classes::es_function_impl<IRegExpConstructor, stringT>
            {
                constructor() throw() {}

                ~constructor() throw() {}

                IPrimitive& __stdcall prototype()
                {
                    return es_regexp<stringT>::static_prototype();
                }

                IPrimitive& __stdcall construct__(IPrimitive& arguments)
                {
                    return *new es_regexp<stringT>(arguments);
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

        explicit es_regexp(const_string_t body, const_string_t flag)
        : parse_tree_(*new node_t(regular_expression::Root))
        , last_index_(0)
        , flag_global_(false)
        , flag_ignore_case_(false)
        , flag_multiline_(false)
        {
            if (!construct_from_pattern_and_flags(body, flag))
                throw *new es_syntax_error<string_t>(L"es_regexp::es_regexp");
        }

        explicit es_regexp(IPrimitive& arguments)
        : parse_tree_(*new node_t(regular_expression::Root))
        , last_index_(0)
        , flag_global_(false)
        , flag_ignore_case_(false)
        , flag_multiline_(false)
        {
            ecmascript::uint32_t length = arguments.length__();
            if (!construct_from_pattern_and_flags(
                length > 0 ?
                    arguments[0].operator const_string_t const() :
                    const_string_t(),
                length > 1 ?
                    arguments[1].operator const_string_t const() :
                    const_string_t()))
                throw *new es_syntax_error<string_t>(L"es_regexp::es_regexp");
        }

        ~es_regexp() throw() {}

        const_string_t const class__() const throw() { return L"RegExp"; }

// INativeTypeConversion
        operator const_string_t const() const
        {
            return L"/" + source_ + L"/" + (flag_global_ ? L"g": L"")
                + (flag_ignore_case_ ? L"i": L"")
                + (flag_multiline_ ? L"m": L"");
        }

        operator string_t const() const
        {
            return L"/" + source_ + L"/" + (flag_global_ ? L"g": L"")
                + (flag_ignore_case_ ? L"i": L"")
                + (flag_multiline_ ? L"m": L"");
        }
// IRegExp
        IPrimitive& __stdcall internal_print() const
        {
            using namespace regular_expression::node;
            return print(re_node<wchar_t>::iterator(&parse_tree_)),
                es_undefined<string_t>::create_instance();
        }

        IPrimitive& __stdcall exec(IPrimitive& input_string)
        {
            using namespace regular_expression::match;
            using namespace regular_expression::scanner;
            typedef const_string_t::const_iterator iterator_t;
            typedef re_match<iterator_t> match_t;
            typedef re_scanner<iterator_t> scanner_t;
            const_string_t const& str 
                = input_string.operator const_string_t const();
            scanner_t scan(str.begin() + (flag_global_ ? last_index_: 0), str.end());
            match_t match = re_match_test(scan, parse_tree_);
            if (!match.success())
                return last_index_ = 0, es_null<string_t>::create_instance();
            match_t * p_match = &match;
            IArray& result = *new es_array<string_t>();
            do result.push(*new es_string<string_t>(p_match->first_, p_match->last_));
                while (0 != p_match->next_ && (p_match = p_match->next_));
            last_index_ = p_match->last_ - str.begin();
            return result;
        }

// IRegExp
        IBoolean& __stdcall test(IPrimitive& arg1)
        {
            return es_boolean<string_t>::create_instance(
                VT::Null != exec(arg1).type__());
        }

        IString& __stdcall source() const
        {
            return *new es_string<string_t>(source_);
        }

        IBoolean& __stdcall global() const
        {
            return es_boolean<string_t>::create_instance(flag_global_);
        }

        IBoolean& __stdcall ignoreCase() const
        {
            return es_boolean<string_t>::create_instance(flag_ignore_case_);
        }

        IBoolean& __stdcall multiline() const
        {
            return es_boolean<string_t>::create_instance(flag_multiline_);
        }

        INumber& __stdcall lastIndex() const
        {
            return *new es_number<string_t>(ecmascript::int32_t(last_index_));
        }

    private:
        bool construct_from_pattern_and_flags(
            const_string_t const& source,
            const_string_t const& flag)
        {
            source_ = source;
            for (const_string_t::const_iterator it = flag.begin();
                it != flag.end(); ++it)
                *it == 'g' ? (flag_global_ = true):
                *it == 'i' ? (flag_ignore_case_ = true):
                *it == 'm' && (flag_multiline_ = true);
            typedef typename const_string_t::const_iterator iterator_t;
            regular_expression::scanner::re_scanner<iterator_t>
                scan(source.begin(), source.end());
            return regular_expression::parser::re_parse(scan, parse_tree_);
        }

        template <typename charT>
        charT const* const id_str(size_t id) const
        {
            static charT const id_str[] = {
                'u', 'n', 'k', 'n', 'o', 'w', 'n',  0,   0,   0,   0,   0,   0,
                'c', 'h', 'a', 'r', 'a', 'c', 't', 'e', 'r',  0,   0,   0,   0,
                'g', 'l', 'e', 'e', 'd', 'y', 's', 't', 'a', 'r',  0,   0,   0,
                'l', 'a', 'z', 'y', 's', 't', 'a', 'r',  0,   0,   0,   0,   0,
                'u', 'n', 'i', 'o', 'n',  0,   0,   0,   0,   0,   0,   0,   0,
                'c', 'o', 'm', 'b', 'i', 'n', 'e',  0,   0,   0,   0,   0,   0,
                'e', 'p', 's', 'i', 'l', 'o', 'n',  0,   0,   0,   0,   0,   0,
                'g', 'r', 'o', 'u', 'p',  0,   0,   0,   0,   0,   0,   0,   0,
                'd', 'o', 't',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
                'f', 'i', 'n', 'a', 'l',  0,   0,   0,   0,   0,   0,   0,   0,
                'f', 'a', 'i', 'l',  0,   0,   0,   0,   0,   0,   0,   0,   0,
                'd', 'i', 'g', 'i', 't',  0,   0,   0,   0,   0,   0,   0,   0,
                'n', 'o', 'n', 'd', 'i', 'g', 'i', 't',  0,   0,   0,   0,   0,
                's', 'p', 'a', 'c', 'e',  0,   0,   0,   0,   0,   0,   0,   0,
                'n', 'o', 'n', 's', 'p', 'a', 'c', 'e',  0,   0,   0,   0,   0,
                'w', 'o', 'r', 'd',  0,   0,   0,   0,   0,   0,   0,   0,   0,
                'n', 'o', 'n', 'w', 'o', 'r', 'd',  0,   0,   0,   0,   0,   0,
                'l', 'i', 'n', 'e', 's', 't', 'a', 'r', 't',  0,   0,   0,   0,
                'l', 'i', 'n', 'e', 'e', 'n', 'd',  0,   0,   0,   0,   0,   0,
                'b', 'r', 'e', 'a', 'k',  0,   0,   0,   0,   0,   0,   0,   0,
                'n', 'o', 'n', 'b', 'r', 'e', 'a', 'k',  0,   0,   0,   0,   0,
                'c', 'l', 'a', 's', 's', 'r', 'a', 'n', 'g', 'e',  0,   0,   0,
                'r', 'e', 'p', 'e', 'a', 't',  0,   0,   0,   0,   0,   0,   0,
                'r', 'e', 'p', 'e', 'a', 't', 'm', 'i', 'n',  0,   0,   0,   0,
                'r', 'e', 'p', 'e', 'a', 't', 'm', 'i', 'n', 'm', 'a', 'x',  0,
                'g', 'r', 'o', 'u', 'p', 'e', 'e', 'n', 'd',  0,   0,   0,   0,
                'g', 'r', 'o', 'u', 'p', 'e', 'f', 'a', 'i', 'l',  0,   0,   0,
                'r', 'o', 'o', 't',  0,   0,   0,   0,   0,   0,   0,   0,   0,
            };
            return &id_str[id * 13];
        };

        template <typename iteratorT>
        void print_node(iteratorT it, size_t depth) const
        {
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4311)
#endif // _MSC_VER
            ES_STATIC_ASSERT(sizeof(void *) == sizeof(unsigned long));
            wprintf(
                L"%s%x %s  t: %x  f: %x :"
                , std::wstring(depth, L' ').c_str()
                , reinterpret_cast<std::ptrdiff_t>(&*it) % 0x10000
                , id_str<char>(it->id_)
                , reinterpret_cast<std::ptrdiff_t>(it->true_) % 0x10000
                , reinterpret_cast<std::ptrdiff_t>(it->false_) % 0x10000
                );
#ifdef _MSC_VER
#  pragma warning(pop)
#endif // _MSC_VER
            if (it->letter_)
                ecmascript::base_services::es_putchar(it->letter_);
            putchar('\n');
        }

        template <typename iteratorT>
        void print(iteratorT tree, size_t depth = 0) const
        {
            print_node(tree, depth);
            for (iteratorT it = tree->children_begin(); !it.is_end(); ++it)
                print(it, depth + 1);
        }

    private:
        es_regexp();

    private:
        node_t& parse_tree_;
        std::ptrdiff_t last_index_;
        const_string_t source_;
        bool flag_global_;
        bool flag_ignore_case_;
        bool flag_multiline_;
    };

} // namespace ecmascript
