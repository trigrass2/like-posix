#!/usr/bin/env bash

greenlight 																																					\
-s ../../cutensils/strutils/strutils.c,fixture.cpp,../stdio.c,test_stdio_printf.cpp,test_stdio_sprintf.cpp,test_stdio_fprintf.cpp,test_stdio.cpp 		\
-i ./,../,../../cutensils/strutils/ 																									\
--cflags="-DMINLIBC_BUILD_FOR_TEST"