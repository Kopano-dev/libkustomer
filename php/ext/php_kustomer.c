/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

#include <php.h>
#include <zend_exceptions.h>

#include <kustomer.h>

#include "php_kustomer.h"

//* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_kustomer_initialize, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, productName, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_kustomer_uninitialize, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_kustomer_wait_until_ready, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 1)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ kustomer_php_functions[] */
zend_function_entry kustomer_php_functions[] = {
	PHP_FE(kustomer_initialize, arginfo_kustomer_initialize)
	PHP_FE(kustomer_uninitialize, arginfo_kustomer_uninitialize)
	PHP_FE(kustomer_wait_until_ready, arginfo_kustomer_wait_until_ready)
	PHP_FE_END
};
/* }}} */

// Module meta data.
zend_module_entry kustomer_php_module_entry = {
	STANDARD_MODULE_HEADER,
	PHP_KUSTOMER_EXTNAME,
	kustomer_php_functions,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	PHP_KUSTOMER_VERSION,
	STANDARD_MODULE_PROPERTIES
};

// Make extension dynamically loadable.
ZEND_GET_MODULE(kustomer_php)

// Implement our functions.
PHP_FUNCTION(kustomer_initialize)
{
	zval *productNameVal;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ZVAL(productNameVal)
	ZEND_PARSE_PARAMETERS_END();

	char *productName;
	if (Z_TYPE_P(productNameVal) == IS_STRING) {
		productName = Z_STRVAL_P(productNameVal);
	} else if(Z_TYPE_P(productNameVal) == IS_NULL) {
		productName = NULL;
	} else {
		php_error_docref(NULL, E_WARNING, "unexpected argument type");
		return;
	}

	int res;

	if ((res = kustomer_initialize(productName)) != 0) {
		zend_throw_exception_ex(zend_ce_exception, res, "Error: %s (%d)",kustomer_err_numeric_text(res), res);
		return;
	}
}

PHP_FUNCTION(kustomer_uninitialize)
{
	ZEND_PARSE_PARAMETERS_START(0, 0)
	ZEND_PARSE_PARAMETERS_END();

	int res;

	if ((res = kustomer_uninitialize()) != 0) {
		zend_throw_exception_ex(zend_ce_exception, res, "Error: %s (%d)",kustomer_err_numeric_text(res), res);
		return;
	}
}

PHP_FUNCTION(kustomer_wait_until_ready)
{
	zend_long timeout;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(timeout)
	ZEND_PARSE_PARAMETERS_END();

	int res;

	if ((res = kustomer_wait_until_ready((long long unsigned int)timeout)) != 0) {
		zend_throw_exception_ex(zend_ce_exception, res, "Error: %s (%d)",kustomer_err_numeric_text(res), res);
		return;
	}
}
