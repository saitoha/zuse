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


#include "interface.hpp"

namespace ecmascript {

    template <typename stringT> struct es_reference;
    template <typename stringT> struct es_undefined;
    template <typename stringT> struct es_null;
    template <typename stringT> struct es_boolean;
    template <typename stringT> struct es_number;
    template <typename stirngT> struct es_string;
    template <typename stringT> struct es_array;
    template <typename stringT> struct es_object;
    template <typename stringT> struct es_native_function;
    template <typename stringT> struct es_native_property;
    template <typename stringT> struct es_internal_object;
    template <typename stringT> struct es_regexp;
    template <typename stringT> struct es_arguments;
    template <typename stringT> struct es_native_error;
    template <typename stringT> struct es_type_error;

} // namespce ecmascript

#include "reference.hpp"
#include "completion.hpp"
#include "base.hpp"
#include "internalobject.hpp"
#include "activation.hpp"
#include "delegate.hpp"
#include "undefined.hpp"
#include "error.hpp"
#include "null.hpp"
#include "boolean.hpp"
#include "string.hpp"
#include "number.hpp"
#include "object.hpp"
#include "array.hpp"
#include "arguments.hpp"
#include "function.hpp"
#include "regexp.hpp"
#include "math.hpp"
#include "date.hpp"
#include "global.hpp"
#include "modules.hpp"

namespace ecmascript {

namespace {
typedef const_string_t string_t;
typedef es_reference<string_t> reference_t; 
typedef es_undefined<string_t> undefined_t;
typedef es_null<string_t> null_t;
typedef es_boolean<string_t> boolean_t;
typedef es_number<string_t> number_t;
typedef es_string<string_t> string_object_t;
typedef es_array<string_t> array_t;
typedef es_object<string_t> object_t;
typedef es_native_property<string_t> native_function_t;
typedef es_native_property<string_t> native_property_t;
typedef es_internal_object<string_t> internal_object_t;
typedef es_regexp<string_t> regexp_t;
typedef es_arguments<string_t> arguments_t;
typedef es_native_error<string_t> native_error_t;
} // anonymous namespace

IReference& reference_(IPrimitive& primitive)
{
    return *new reference_t(primitive);
}

IUndefined& undefined_()
{
    return undefined_t::create_instance();
}

IBoolean& true_() 
{
    return boolean_t::create_instance(true);
}

IBoolean& false_()
{
    return boolean_t::create_instance(false);
}

INull& null_()
{
    return null_t::create_instance();
}

} // namespace ecmascript
