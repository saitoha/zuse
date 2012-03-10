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

    template <typename T>
    void swap(T lhs, T rhs) throw()
    {
        T temp(lhs);
        lhs = rhs;
        rhs = temp;
    }

    template <typename T>
    struct vector 
    {
        typedef vector<T> self_t;
        typedef T value_type;
        typedef T * iterator;
        typedef T const* const_iterator;
        typedef size_t size_type;
    private:
        enum { initial_size = 16 };

    public:
        vector() { initialize(initial_size); }
        explicit vector(size_type size) { initialize(size); }
        ~vector() { delete [] bottom_; }

    public:
        T& operator [] (size_type index) { return bottom_[index]; }
        iterator begin() const { return bottom_; }
        iterator end() const { return current_; }
        size_type size() const { return current_ - bottom_; }
        void push_back(value_type const& value) { *current_++ = value; }
        void resize(size_type size) 
        { 
            self_t other(size);
            swap(bottom_, other.bottom_); 
            current_ = other.current_;
            top_ = other.top_;
        }

    private:
        void initialize(size_type size) { current_ = bottom_ = new T[size]; }

    private:
        T *bottom_, *current_, *top_;
    };
    
} // namespace ecmascript
