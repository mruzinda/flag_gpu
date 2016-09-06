# serial 1 flagpower.m4
AC_DEFUN([AX_CHECK_FLAGPOW],
[AC_PREREQ([2.65])dnl
AC_ARG_WITH([flagpower],
            AC_HELP_STRING([--with-flagpower=DIR],
                           [Location of flagpower headers/libs (/usr/local)]),
            [FLAGPOWDIR="$withval"],
            [FLAGPOWDIR=/usr/local])

orig_LDFLAGS="${LDFLAGS}"
LDFLAGS="${orig_LDFLAGS} -L${FLAGPOWDIR}/lib"
dnl AC_SEARCH_LIBS([getTotalPower], [flagpower],
dnl              # Found
             AC_SUBST(FLAGPOW_LIBDIR,${FLAGPOWERDIR}/lib),
dnl              # Not found there, check FLAGPOWERDIR
dnl              AS_UNSET(ac_cv_lib_flagpower_getTotalPower)
dnl              LDFLAGS="${orig_LDFLAGS} -L${FLAGPOWERDIR}"
dnl              AC_CHECK_LIB([FLAGPOWER], [getTotalPower],
dnl                           # Found
dnl                           AC_SUBST(FLAGPOWER_LIBDIR,${FLAGPOWERDIR}),
dnl                           # Not found there, error
dnl                           AC_MSG_ERROR([flagpower library not found])))
LDFLAGS="${orig_LDFLAGS}"

AC_CHECK_FILE([${FLAGPOWDIR}/include/total_power.h],
              # Found
              AC_SUBST(FLAGPOW_INCDIR,${FLAGPOWDIR}/include),
              # Not found there, check FLAGPOWERDIR
              AC_CHECK_FILE([${FLAGPOWDIR}/total_power.h],
                            # Found
                            AC_SUBST(FLAGPOW_INCDIR,${FLAGPOWDIR}),
                            # Not found there, error
                            AC_MSG_ERROR([total_power.h header file not found])))
])
