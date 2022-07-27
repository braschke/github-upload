##########################################################################################
#
#  GMakefile to gmake the octbox library
#
#  C. Dehning, Mar 2017
#
#  $Id: GMakefile 10 2012-12-19 18:23:43Z dehning $
#
##########################################################################################

#include ../../makeutils/gmake_all.inc
SO          =  so
AR          =  ar csurv
LD_FLAGS    =
LD_SO       =  ld -shared
CX_SO       = $(LD_SO)
LIB_DL      = -ldl
LIB_RT      =
LIB_SOCK    =
OMP_CFLAGS  =
OMP_LDFLAGS =
OPT_PTHREAD =

CC          = gcc -m64

GCC_WARNINGS= \
-Wall \
-pedantic \
-Wcast-align \
-Wwrite-strings \
-Wuninitialized \
-Wno-long-long \
-Wunused-function \
-Wunused-label \
-Wunused-value \
-Wunused-variable \
-Wpointer-arith \
-Wshadow \

CC_FLAGS     = -fPIC -O3 $(GCC_WARNINGS)

SPEC         = -64d

PRECDEF=-DDOUBLE_PRECISION=1

INCLUDES =  -I. \
            -I../../utlinclude

SOURCES  = liboctbox.c
OBJECTS  = $(SOURCES:.c=.o)
TARGET   = $(SOURCES:.c=)$(SPEC).a

default: $(TARGET)

$(TARGET): $(MAKEFILE) $(SOURCES)
	@echo "#"
	@echo "# Compiling $(SOURCES) ..."
	@echo "#"
	$(CC) $(CC_FLAGS) $(PRECDEF) $(INCLUDES) -c $(SOURCES)
	@echo "#"
	@echo "# Making archive $@ ..."
	@echo "# "
	$(AR) $@ $(OBJECTS)
