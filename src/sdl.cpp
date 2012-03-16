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



#include <SDL/SDL.h>
#include "types.hpp"

namespace ecmascript {

    namespace {

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class  ISdlModule
    //  @brief
    //
    struct ISdl : IObject
    {
        typedef IObject base_t;
        enum { id = VT::Object };
        enum { offset = base_t::vtsize };
        enum { vtsize = offset + 13 };

        virtual INumber& __stdcall Init(IPrimitive const&) const = 0;
        virtual INumber& __stdcall InitSubSystem(IPrimitive const&) const = 0;
        virtual IUndefined& __stdcall QuitSubSystem(IPrimitive const&) const = 0;
        virtual INumber& __stdcall WasInit(IPrimitive const&) const = 0;
        virtual IUndefined& __stdcall Quit() const = 0;
        virtual INumber& __stdcall SDL_INIT_TIMER_() const = 0;
        virtual INumber& __stdcall SDL_INIT_AUDIO_() const = 0;
        virtual INumber& __stdcall SDL_INIT_VIDEO_() const = 0;
        virtual INumber& __stdcall SDL_INIT_CDROM_() const = 0;
        virtual INumber& __stdcall SDL_INIT_JOYSTICK_() const = 0;
        virtual INumber& __stdcall SDL_INIT_NOPARACHUTE_() const = 0;
        virtual INumber& __stdcall SDL_INIT_EVENTTHREAD_() const = 0;
        virtual INumber& __stdcall SDL_INIT_EVERYTHING_() const = 0;
        virtual IPrimitive& __stdcall SetVideoMode(
            IPrimitive const&, IPrimitive const&, 
            IPrimitive const&, IPrimitive const&) const = 0; 
        static es_attributes const& get_typelib__()
        {
            static es_attributes const funcdata[] =
            {
                { L"Init",                  offset +  0, Stdcall| Method,    1 },
                { L"InitSubSystem",         offset +  1, Stdcall| Method,    1 },
                { L"QuitSubSystem",         offset +  2, Stdcall| Method,    1 },
                { L"WasInit",               offset +  3, Stdcall| Method,    1 },
                { L"Quit",                  offset +  4, Stdcall| Method,    1 },
                { L"SDL_INIT_TIMER",        offset +  5, Stdcall| Property,  0 },
                { L"SDL_INIT_AUDIO",        offset +  6, Stdcall| Property,  0 },
                { L"SDL_INIT_VIDEO",        offset +  7, Stdcall| Property,  0 },
                { L"SDL_INIT_CDROM",        offset +  8, Stdcall| Property,  0 },
                { L"SDL_INIT_JOYSTICK",     offset +  9, Stdcall| Property,  0 },
                { L"SDL_INIT_NOPARACHUTE",  offset + 10, Stdcall| Property,  0 },
                { L"SDL_INIT_EVENTTHREAD",  offset + 11, Stdcall| Property,  0 },
                { L"SDL_INIT_EVERYTHING",   offset + 12, Stdcall| Property,  0 },
                { L"SetVideoMode",          offset + 13, Stdcall| Method,    4 },
                { 0, 0, 0, 0, &base_t::get_typelib__() },
            };
            return *funcdata;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //  @class es_sdl
    //  @brief
    //
    struct es_sdl
    : public base_classes::es_collectable_object<
        base_classes::es_object_impl<ISdl, std::wstring> >
    {
        typedef std::wstring string_t;
        typedef string_t::value_type char_type;

// constructor
        static IFunction& static_get_constructor()
        {
            static struct constructor
            : public base_classes::es_function_impl<IObjectConstructor, string_t>
            {
                constructor() throw()
                {
                }

                ~constructor() throw()
                {
                }

                IPrimitive& __stdcall call__(IPrimitive&, IPrimitive& arguments)
                {
                    return es_undefined<string_t>::create_instance();
                }

                IPrimitive& __stdcall construct__(IPrimitive& arguments)
                {
                    static es_sdl instance;
                    return instance;
                }

                IPrimitive& __stdcall prototype()
                {
                    return es_sdl::static_prototype();
                }

            } constructor_;
            return constructor_;
        }

 
        es_sdl() throw()
        {
        }

        ~es_sdl() throw()
        {
        }

        const_string_t const class__() const throw()
        {
            return L"SDL";
        }

        INumber& __stdcall Init(IPrimitive const& arg1) const
        {
            return es_number<string_t>::create_instance(
                SDL_Init(arg1.operator ecmascript::uint32_t()));
        }

        INumber& __stdcall InitSubSystem(IPrimitive const& arg1) const
        {
            return es_number<string_t>::create_instance(
                SDL_InitSubSystem(arg1.operator ecmascript::uint32_t()));
        }
 
        IUndefined& __stdcall QuitSubSystem(IPrimitive const& arg1) const
        {
            SDL_QuitSubSystem(arg1.operator ecmascript::uint32_t());
			return es_undefined<string_t>::create_instance();
        }

        INumber& __stdcall WasInit(IPrimitive const& arg1) const 
        {
            return es_number<string_t>::create_instance(
				SDL_WasInit(arg1.operator ecmascript::uint32_t()));
        }

        IUndefined& __stdcall Quit() const
        {
            SDL_Quit();
			return es_undefined<string_t>::create_instance();
        }

        INumber& __stdcall SDL_INIT_TIMER_() const
        {
            return es_number<string_t>::create_instance(SDL_INIT_TIMER);
        }

        INumber& __stdcall SDL_INIT_AUDIO_() const
        {
            return es_number<string_t>::create_instance(SDL_INIT_AUDIO);
        }

        INumber& __stdcall SDL_INIT_VIDEO_() const 
        {
            return es_number<string_t>::create_instance(SDL_INIT_VIDEO);
        }

        INumber& __stdcall SDL_INIT_CDROM_() const
        {
            return es_number<string_t>::create_instance(SDL_INIT_CDROM);
        }

        INumber& __stdcall SDL_INIT_JOYSTICK_() const
        {
            return es_number<string_t>::create_instance(SDL_INIT_JOYSTICK);
        }

        INumber& __stdcall SDL_INIT_NOPARACHUTE_() const
        {
            return es_number<string_t>::create_instance(SDL_INIT_NOPARACHUTE);
        }

        INumber& __stdcall SDL_INIT_EVENTTHREAD_() const
        {
            return es_number<string_t>::create_instance(SDL_INIT_EVENTTHREAD);
        }

        INumber& __stdcall SDL_INIT_EVERYTHING_() const
        {
            return es_number<string_t>::create_instance(SDL_INIT_EVERYTHING);
        }
/*
        IObject& SurfaceToObject(SDL_Surface& surface) const
        {
            IObject& result = *new es_object<string_t>;
            result.put__(L"flags", es_number<string_t>::create_instance<string_t>(surface.flags));
            {
                IObject& pixel_format = *new es_object<string_t>;
            result.put__(L"flags", es_number<string_t>::create_instance<string_t>(surface.flags));
            
        }
*/ 
 
        IPrimitive& __stdcall SetVideoMode(
            IPrimitive const& width, IPrimitive const& height, 
            IPrimitive const& bpp, IPrimitive const& flags) const
        {
            SDL_Surface *p_surface = SDL_SetVideoMode(width, height, bpp, flags.operator uint32_t());
            if (NULL == p_surface)
                return es_null<string_t>::create_instance(); 
            return es_number<string_t>::create_instance(SDL_INIT_EVERYTHING);
        }
     };

    } // anonymous namespace

} // namespace ecmascript

extern "C" ecmascript::IPrimitive * sdl()
{
    return &ecmascript::es_sdl::static_get_constructor();
}



