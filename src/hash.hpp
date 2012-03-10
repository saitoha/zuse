/* ***** BEGIN LICENSE BLOCK Version: GPL 3.0 ***** 
 * Copyright (C) 2008-2011  zuse <user@zuse.jp>
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



namespace ecmascript {

///////////////////////////////////////////////////////////////////////////////
//
// @class map
//
template <typename stringT>
inline unsigned hash_value(stringT const& v)
{
    unsigned seed = 0;
    typedef typename stringT::const_iterator iterator;
    for (iterator it = v.begin(); it != v.end(); ++it)
        seed ^= static_cast<unsigned>(*it)
            + /*0x9e3779b9 +*/ 0x9e370001UL + (seed << 6) + (seed >> 2);
    return seed;
}

///////////////////////////////////////////////////////////////////////////////
//
// @class map
//
template <typename keyT, typename valueT>
struct map
{
    struct value_type 
    { 
        explicit value_type(value_type * const prev, keyT key)
        : prev_(prev), next_(0), first(key) {}
        value_type *prev_;
        value_type *next_;
        keyT const first; 
        valueT second; 
    };

    struct iterator
    {
        explicit iterator(): p_value_((value_type *)-1) {}

        explicit iterator(value_type * p_value): p_value_(p_value) {}

        value_type * operator ->() const { return p_value_; }

        iterator operator =(value_type * p_value) { return p_value_ = p_value; } 

        bool operator ==(iterator const& other) const 
        { 
            return p_value_ == other.p_value_; 
        } 

        bool operator !=(iterator const& other) const
        { 
            return p_value_ != other.p_value_; 
        } 

        iterator operator ++() const
        { 
            return iterator(p_value_ = p_value_->prev_); 
        } 

        iterator operator ++(int) 
        { 
            iterator result(p_value_); 
            p_value_ = p_value_->prev_;
            return result;
        } 
    private:
        mutable value_type * p_value_;
    };
    typedef iterator const const_iterator;

public:
    map() : table_(0), table_size(100), current_((value_type *)-1) { }
    ~map() { }

public:
    iterator begin() { return iterator(current_); }
    const_iterator begin() const { return const_iterator(current_); }
    iterator end() { return iterator(); }
    const_iterator end() const { return const_iterator(); }
    iterator find(keyT const& key) { return iterator(find_impl(key)); }
    valueT& operator [](keyT const& key) { return find_impl(key)->second; }

private:
    void initialize()
    {
        //table_ = new value_type[500]();
        table_ = (value_type *)malloc(sizeof(value_type) * table_size);
        memset(table_, 0, sizeof(value_type) * table_size);
    }

    value_type * find_impl(keyT const& key) 
    { 
        if (0 == table_)
            initialize();
        value_type *p = table_ + hash_value(key) % table_size;
        if (0 == p->prev_)
            return current_ = new (p) value_type(current_, key);
    loop:
        if (p->first == key)
            return p;
        if (0 == p->next_)
            return current_ = p->next_ = new value_type(current_, key);
        p = p->next_;
//        printf("collision detected.\n");
        goto loop;
    }

private:
    value_type *table_;
    unsigned table_size;
    value_type *current_;
}; 

} // namespace ecmascript

