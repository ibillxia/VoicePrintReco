dnl $RCSfile: gccwarn.m4,v $
dnl $Author: ggravier $
dnl
dnl $Date: 2002/07/25 14:14:31 $
dnl $Revison$
dnl
dnl Check if the compiler supports useful warning options.  There's a few that
dnl we don't use, simply because they're too noisy:
dnl
dnl     -Wconversion (useful in older versions of gcc, but not in gcc 2.7.x)
dnl     -Wredundant-decls (system headers make this too noisy)
dnl     -Wtraditional (combines too many unrelated messages, only a few useful)
dnl     -Wcast-qual because with char * cp; const char * ccp;
dnl                 cp = (char *) ccp;
dnl                 is the only portable way to do cp = ccp;
dnl     -pedantic
dnl
AC_DEFUN([CF_GCC_WARNINGS],
[EXTRA_CFLAGS=""
if test -n "$GCC"
then
        changequote(,)dnl
        cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
int main(int argc, char *argv[]) { return argv[argc-1] == 0; }
EOF
        changequote([,])dnl
        AC_CHECKING([for gcc warning options])
        cf_save_CFLAGS="$CFLAGS"
        EXTRA_CFLAGS="-W -Wall"
        for cf_opt in \
                Wbad-function-cast \
                Wcast-align \
                Wmissing-declarations \
                Wmissing-prototypes \
                Wnested-externs \
                Wpointer-arith \
                Wshadow \
                Wstrict-prototypes \
		Wwrite-strings
        do
                CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
                if AC_TRY_EVAL(ac_compile); then
                        test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
                        EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
                        test "$cf_opt" = Wcast-qual && EXTRA_CFLAGS="$EXTRA_CFLAGS -DXTSTRINGDEFINES"
                fi
        done
        rm -f conftest*
        CFLAGS="$cf_save_CFLAGS"
fi
AC_SUBST(EXTRA_CFLAGS)
])dnl
