PHP_ARG_ENABLE(kustomer, for kustomer support, [  --enable-kustomer       Enable Kustomer support])

AC_ARG_ENABLE([kustomer_allow_untrusted],
	AS_HELP_STRING([--enable-kustomer-allow-untrusted], [Enable Kustomer to allow untrusted ensuring, use for development only]))

if test "$PHP_KUSTOMER" != "no"; then
	if test -z "$kustomer_INCLUDE" ; then
		AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
		AC_MSG_CHECKING(for libkustomer)
		if test -x "$PKG_CONFIG" && $PKG_CONFIG --exists libkustomer ; then
			kustomer_INCLUDE=`$PKG_CONFIG libkustomer --variable=includedir`
			kustomer_VERSION=`$PKG_CONFIG libkustomer --modversion`
			AC_MSG_RESULT($kustomer_VERSION)
		else
			AC_MSG_ERROR(libkustomer not found)
		fi
	fi

	PHP_ADD_INCLUDE($kustomer_INCLUDE)

	AC_DEFINE(HAVE_KUSTOMER, 1, [ ])
	PHP_NEW_EXTENSION(kustomer, ext/php_kustomer.c, $ext_shared)
fi

if test "$enable_kustomer_allow_untrusted" = "yes"; then
	AC_DEFINE(PHPKUSTOMER_ALLOW_UNTRUSTED, 1, [ ])
fi
