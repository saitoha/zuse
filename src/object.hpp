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


//////////////////////////////////////////////////////////////////////////////
//
// Object object definition
//
namespace ecmascript {

    //////////////////////////////////////////////////////////////////////////
    //
    // 15.2 Object Objects
    //
    template <typename stringT>
    struct es_object
    : public base_classes::es_collectable_object<
        base_classes::es_object_impl<IObject, stringT> >
    {
        typedef stringT string_t;
        typedef es_object<string_t> self_t;

// constructor
        static IFunction& static_get_constructor()
        {
            static struct constructor
            : public base_classes::es_function_impl<IObjectConstructor, stringT>
            {
                constructor() throw()
                {
                }

                ~constructor() throw()
                {
                }

                IPrimitive& __stdcall call__(
                    IPrimitive& /*this_arg*/,
                    IPrimitive& arguments
                    )
                {
                    return (arguments.length__() == 0 ?
                        es_undefined<string_t>::create_instance():
                        arguments[0]
                    ).ToObject();
                }

                IPrimitive& __stdcall construct__(IPrimitive& arguments)
                {
                    IObject& object = *new es_object<string_t>();
                    IObject& object2 = *new es_object<string_t>();
                    if (arguments.length__() > 0)
                        object2.put__(L"value", arguments[0]);
                    return object.set_prototype__(object2), object;
                }

                IPrimitive& __stdcall prototype()
                {
                    return es_object<string_t>::static_prototype();
                }

            } constructor_;
            return constructor_;
        }

        static IObject& __stdcall create_instance()
        {
            return *new self_t;
        }

        es_object() throw()
        {
        }

        virtual ~es_object() throw()
        {
        }

        const_string_t const class__() const throw()
        {
            return L"Object";
        }

        IFunction const& __stdcall constructor() const
        {
            return static_get_constructor();
        }
    };

} // namespcae ecmascript
