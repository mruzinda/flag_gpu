#serial 1 flagpfb.m4
AC_DEFUN([AX_CHECK_FLAGPFB],
[AC_PREREQ([2.63])dnl
AC_ARG_WITH([flagpfb],
            AC_HELP_STRING([--with-flagpfb=DIR],
                           [Location of flagpfb headers/libs (/opt/local)]),
            [FLAGPFBDIR="$withval"],
            [FLAGPFBDIR=/opt/local])

orig_LDFLAGS="${LDFLAGS}"
LDFLAGS="${orig_LDFLAGS} -L${FLAGPFBDIR}/lib -L/opt/local/NVIDIA/cuda-10.0/lib64"
AC_CHECK_LIB([flagpfb],[runPFB],
dnl              # Found
             AC_SUBST(FLAGPFB_LIBDIR,${FLAGPFBDIR}/lib),
dnl              # Not found there, check FLAGPFBDIR
             AS_UNSET(ac_cv_lib_flagpfb_runPFB)
             LDFLAGS="${orig_LDFLAGS} -L${FLAGPFBDIR}"
             AC_CHECK_LIB([flagpfb], [runPFB],
                          # Found
                          AC_SUBST(FLAGPFB_LIBDIR,${FLAGPFBDIR}),
                          # Not found there, error
                          AC_MSG_ERROR([flagpfb library not found]), -lcufft), -lcufft)
LDFLAGS="${orig_LDFLAGS}"

AC_CHECK_FILE([${FLAGPFBDIR}/include/pfb.h],
              # Found
              AC_SUBST(FLAGPFB_INCDIR,${FLAGPFBDIR}/include),
              # Not found there, check FLAGPFBDIR
              AC_CHECK_FILE([${FLAGPFBDIR}/pfb.h],
                            # Found
                            AC_SUBST(FLAGPFB_INCDIR,${FLAGPFBDIR}),
                            # Not found there, error
                            AC_MSG_ERROR([pfb.h header file not found])))
])
