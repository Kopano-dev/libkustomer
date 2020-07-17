PHP_ARG_ENABLE(php_kustomer, whether to enable my extension,
[ --enable-kustomer-php  Enable Kustomer PHP])

if test "$PHP_KUSTOMER" != "no"; then
	AC_DEFINE(HAVE_KUSTOMER, 1, [Wether you have Kustomer PHP extension])
	PHP_NEW_EXTENSION(php_kustomer, ext/php_kustomer.c, $ext_shared)

	AC_SUBST(CPPFLAGS, "$CPPFLAGS -I../.libs/include/kustomer -lkustomer")
	AC_SUBST(LDFLAGS, "$LDFLAGS -L../.libs")
fi
