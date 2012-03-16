################################################################################
#
# zuse 
# $Id$
#

bin_PROGRAMS = zuse
zuse_CXXFLAGS = -g -O0 -Wall $(genericCPPFLAGS)
zuse_LDLIBS = $(genericLDLIBS)
zuse_LDFLAGS = -lreadline $(genericLDFLAGS) -ldl -export-dynamic libzuse.a
zuse_SOURCES = zuse.cpp
zuse_HEADERS = zuse.hpp
zusedir = .
zuse_DEPENDENCIES = libzuse.a

lib_LIBRARIES = libzuse.a
libzuse_a_CXXFLAGS = -c -g -O0 -Wall $(genericCPPFLAGS)
libzuse_a_LDLIBS   = $(genericLDLIBS)
libzuse_a_LIBADD   = $(genericLDFLAGS) 
libzuse_a_SOURCES  = libzuse.cpp \
				     action.hpp \
                     activation.hpp \
                     activescript.hpp \
                     activex.hpp \
                     actor.hpp \
                     arguments.hpp \
                     array.hpp \
                     base.hpp \
                     boolean.hpp \
                     bytearray.hpp \
                     charparser.hpp \
                     completion.hpp \
                     date.hpp \
                     debug.hpp \
                     decimalparser.hpp \
                     delegate.hpp \
                     epsilonparser.hpp \
                     error.hpp \
                     errorparser.hpp \
                     function.hpp \
                     global.hpp \
                     grammer.hpp \
                     hash.hpp \
                     hexparser.hpp \
                     identifierparser.hpp \
                     interface.hpp \
                     internalobject.hpp \
                     libzuse.cpp \
                     math.hpp \
                     modules.hpp \
                     null.hpp \
                     number.hpp \
                     object.hpp \
                     reference.hpp \
                     regexp.hpp  \
                     services.hpp \
                     skipparser.hpp \
                     spirit.hpp \
                     string.hpp \
                     sys.hpp \
                     tags \
                     types.hpp \
                     zuse.def \
                     undefined.hpp \
                     unicode.hpp \
                     vector.hpp \
                     x86.hpp 
                   
#libzuse_a_DEPENDENCIES = libzusesdl.la

if BUILD_EXTRA_MODULES
if HAVE_LIBSDL
lib_LTLIBRARIES = libzusesdl.la
libzusesdl_la_CXXFLAGS = -c -g -O0 -Wall $(genericCPPFLAGS) \
						 -I/opt/local/include \
						 `/opt/local/bin/sdl-config --cflags`
libzusesdl_la_LIBADD = -L/opt/local/lib -lSDLmain -lSDL \
					   #`/opt/local/bin/sdl-config --libs` 
libzusesdl_la_LDFLAGS = -module 
libzusesdl_la_SOURCES = sdl.cpp
endif # HAVE_LIBSDL
endif # BUILD_EXTRA_MODULES

CTAGS=ctags
#			--c++-kinds=+p 
#			--fields=+iaS 
#			--extra=+q 
#			--language-force=C++ 

doc:
	doxygen

tags: 
	$(CTAGS) -R \
			--language=c++ \
			* \
			/usr/include/*


