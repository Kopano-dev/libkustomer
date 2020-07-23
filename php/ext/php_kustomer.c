/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

#include <php.h>
#include <zend_exceptions.h>

#ifdef HAVE_SPL
#include "ext/spl/spl_exceptions.h"
#endif

#include <kustomer.h>
#include <kustomer_errors.h>

#include "config.h"
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_kustomer_begin_ensure, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_kustomer_end_ensure, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, transaction, IS_OBJECT, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_kustomer_ensure_ok, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, transaction, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, productName, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_kustomer_ensure_get, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, transaction, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, productName, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, claim, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_kustomer_ensure_get_int64, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, transaction, IS_OBJECT, 1)
	ZEND_ARG_TYPE_INFO(0, productName, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, claim, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_kustomer_ensure_ensure_bool, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, transaction, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, productName, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, claim, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_kustomer_ensure_ensure_string, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, transaction, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, productName, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, claim, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_kustomer_ensure_ensure_int64, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, transaction, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, productName, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, claim, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_kustomer_ensure_ensure_float64, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, transaction, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, productName, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, claim, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

/* }}} */

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

	if ((res = kustomer_initialize(productName)) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}
}

PHP_FUNCTION(kustomer_uninitialize)
{
	ZEND_PARSE_PARAMETERS_START(0, 0)
	ZEND_PARSE_PARAMETERS_END();

	int res;

	if ((res = kustomer_uninitialize()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
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

	if ((res = kustomer_wait_until_ready((long long unsigned int)timeout)) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}
}

// Implement our objects.
const zend_function_entry phpkustomer_KopanoProductClaims_functions[] = {
    PHP_FE_END
};

zend_object *phpkustomer_KopanoProductClaims_create_handler(zend_class_entry *ce)
{

	phpkustomer_KopanoProductClaims_t *intern = (phpkustomer_KopanoProductClaims_t *)ecalloc(1, sizeof(phpkustomer_KopanoProductClaims_t) + zend_object_properties_size(ce));
	memset(intern, 0, sizeof(phpkustomer_KopanoProductClaims_t));

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);

	intern->std.handlers = &phpkustomer_KopanoProductClaims_object_handlers;

	return &intern->std;
}

PHP_FUNCTION(kustomer_begin_ensure)
{
	ZEND_PARSE_PARAMETERS_START(0, 0)
	ZEND_PARSE_PARAMETERS_END();

	struct kustomer_begin_ensure_return res;

	res = kustomer_begin_ensure();

	if (res.r0 != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res.r0);
		return;
	}

	zend_object *obj;
	obj = phpkustomer_KopanoProductClaims_create_handler(phpkustomer_KopanoProductClaims_ce);

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(obj);
	kpc->kpc_ptr = res.r1;

#ifdef PHPKUSTOMER_ALLOW_UNTRUSTED
	// NOTE(longsleep): The next line might be handy for development.
	kustomer_ensure_set_allow_untrusted(kpc->kpc_ptr, 1);
#endif

	RETURN_OBJ(obj);
}

PHP_FUNCTION(kustomer_end_ensure)
{
	zval *kpc_zv;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJECT_OF_CLASS(kpc_zv, phpkustomer_KopanoProductClaims_ce)
	ZEND_PARSE_PARAMETERS_END();

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	int res;
	res = kustomer_end_ensure(kpc->kpc_ptr);

	kpc->kpc_ptr = NULL;

	if (res != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}
}

PHP_FUNCTION(kustomer_ensure_ok)
{
	zval *kpc_zv;
	zend_string *productName;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_OBJECT_OF_CLASS(kpc_zv, phpkustomer_KopanoProductClaims_ce)
		Z_PARAM_STR(productName)
	ZEND_PARSE_PARAMETERS_END();

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	int res;

	res = kustomer_ensure_ok(kpc->kpc_ptr, ZSTR_VAL(productName));

	zend_string_release(productName);

	if (res != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}
}

PHP_FUNCTION(kustomer_ensure_get_bool)
{
	zval *kpc_zv;
	zend_string *productName;
	zend_string *claim;

	ZEND_PARSE_PARAMETERS_START(3, 3)
		Z_PARAM_OBJECT_OF_CLASS(kpc_zv, phpkustomer_KopanoProductClaims_ce)
		Z_PARAM_STR(productName)
		Z_PARAM_STR(claim)
	ZEND_PARSE_PARAMETERS_END();

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	struct kustomer_ensure_get_bool_return res;

	res = kustomer_ensure_get_bool(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim));

	zend_string_release(productName);
	zend_string_release(claim);

	if (res.r0 != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res.r0);
		return;
	}

	RETURN_BOOL(res.r1);
}

PHP_FUNCTION(kustomer_ensure_ensure_bool)
{
	zval *kpc_zv;
	zend_string *productName;
	zend_string *claim;
	zend_bool value;

	ZEND_PARSE_PARAMETERS_START(4, 4)
		Z_PARAM_OBJECT_OF_CLASS(kpc_zv, phpkustomer_KopanoProductClaims_ce)
		Z_PARAM_STR(productName)
		Z_PARAM_STR(claim)
		Z_PARAM_BOOL(value)
	ZEND_PARSE_PARAMETERS_END();

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	int res;

	res = kustomer_ensure_ensure_bool(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim), (int)value);

	zend_string_release(productName);
	zend_string_release(claim);

	if (res != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}
}

PHP_FUNCTION(kustomer_ensure_get_string)
{
	zval *kpc_zv;
	zend_string *productName;
	zend_string *claim;

	ZEND_PARSE_PARAMETERS_START(3, 3)
		Z_PARAM_OBJECT_OF_CLASS(kpc_zv, phpkustomer_KopanoProductClaims_ce)
		Z_PARAM_STR(productName)
		Z_PARAM_STR(claim)
	ZEND_PARSE_PARAMETERS_END();

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	struct kustomer_ensure_get_string_return res;

	res = kustomer_ensure_get_string(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim));

	zend_string_release(productName);
	zend_string_release(claim);

	if (res.r0 != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res.r0);
		return;
	}

	RETURN_STRING(res.r1);
}

PHP_FUNCTION(kustomer_ensure_ensure_string)
{
	zval *kpc_zv;
	zend_string *productName;
	zend_string *claim;
	zend_string *value;

	ZEND_PARSE_PARAMETERS_START(4, 4)
		Z_PARAM_OBJECT_OF_CLASS(kpc_zv, phpkustomer_KopanoProductClaims_ce)
		Z_PARAM_STR(productName)
		Z_PARAM_STR(claim)
		Z_PARAM_STR(value)
	ZEND_PARSE_PARAMETERS_END();

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	int res;

	res = kustomer_ensure_ensure_string(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim), ZSTR_VAL(value));

	zend_string_release(productName);
	zend_string_release(claim);
	zend_string_release(value);

	if (res != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}
}

PHP_FUNCTION(kustomer_ensure_get_int64)
{
	zval *kpc_zv;
	zend_string *productName;
	zend_string *claim;

	ZEND_PARSE_PARAMETERS_START(3, 3)
		Z_PARAM_OBJECT_OF_CLASS(kpc_zv, phpkustomer_KopanoProductClaims_ce)
		Z_PARAM_STR(productName)
		Z_PARAM_STR(claim)
	ZEND_PARSE_PARAMETERS_END();

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	struct kustomer_ensure_get_int64_return res;

	res = kustomer_ensure_get_int64(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim));

	zend_string_release(productName);
	zend_string_release(claim);

	if (res.r0 != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res.r0);
		return;
	}

	RETURN_LONG(res.r1);
}

PHP_FUNCTION(kustomer_ensure_ensure_int64)
{
	zval *kpc_zv;
	zend_string *productName;
	zend_string *claim;
	zend_long value;

	ZEND_PARSE_PARAMETERS_START(4, 4)
		Z_PARAM_OBJECT_OF_CLASS(kpc_zv, phpkustomer_KopanoProductClaims_ce)
		Z_PARAM_STR(productName)
		Z_PARAM_STR(claim)
		Z_PARAM_LONG(value)
	ZEND_PARSE_PARAMETERS_END();

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	int res;

	res = kustomer_ensure_ensure_int64(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim), (long long int)value);

	zend_string_release(productName);
	zend_string_release(claim);

	if (res != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}
}

PHP_FUNCTION(kustomer_ensure_get_float64)
{
	zval *kpc_zv;
	zend_string *productName;
	zend_string *claim;

	ZEND_PARSE_PARAMETERS_START(3, 3)
		Z_PARAM_OBJECT_OF_CLASS(kpc_zv, phpkustomer_KopanoProductClaims_ce)
		Z_PARAM_STR(productName)
		Z_PARAM_STR(claim)
	ZEND_PARSE_PARAMETERS_END();

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	struct kustomer_ensure_get_float64_return res;

	res = kustomer_ensure_get_float64(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim));

	zend_string_release(productName);
	zend_string_release(claim);

	if (res.r0 != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res.r0);
		return;
	}

	RETURN_DOUBLE(res.r1);
}

PHP_FUNCTION(kustomer_ensure_ensure_float64)
{
	zval *kpc_zv;
	zend_string *productName;
	zend_string *claim;
	double value;

	ZEND_PARSE_PARAMETERS_START(4, 4)
		Z_PARAM_OBJECT_OF_CLASS(kpc_zv, phpkustomer_KopanoProductClaims_ce)
		Z_PARAM_STR(productName)
		Z_PARAM_STR(claim)
		Z_PARAM_DOUBLE(value)
	ZEND_PARSE_PARAMETERS_END();

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	int res;

	res = kustomer_ensure_ensure_float64(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim), value);

	zend_string_release(productName);
	zend_string_release(claim);

	if (res != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}
}

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(kustomer_php)
{
	zend_class_entry tmp_ne_ce;
	INIT_CLASS_ENTRY(tmp_ne_ce, "KUSTOMER\\NumericException", NULL);
#ifdef HAVE_SPL
	phpkustomer_NumericException_ce = zend_register_internal_class_ex(
		&tmp_ne_ce, spl_ce_RuntimeException
	);
#else
	pykustomer_NumericException_ce = zend_register_internal_class_ex(
		&tmp_ce, zend_ce_exception
	);
#endif
	// Attach all errors as class constants to our exception.
	PHPKUSTOMER_DECLARE_ERROR_CONSTANT("ErrStatusUnknown", KUSTOMER_ERRSTATUSUNKNOWN);
	PHPKUSTOMER_DECLARE_ERROR_CONSTANT("ErrStatusInvalidProductName", KUSTOMER_ERRSTATUSINVALIDPRODUCTNAME);
	PHPKUSTOMER_DECLARE_ERROR_CONSTANT("ErrStatusAlreadyInitialized", KUSTOMER_ERRSTATUSALREADYINITIALIZED);
	PHPKUSTOMER_DECLARE_ERROR_CONSTANT("ErrStatusNotInitialized", KUSTOMER_ERRSTATUSNOTINITIALIZED);
	PHPKUSTOMER_DECLARE_ERROR_CONSTANT("ErrStatusTimeout", KUSTOMER_ERRSTATUSTIMEOUT);
	PHPKUSTOMER_DECLARE_ERROR_CONSTANT("ErrEnsureOnlineFailed", KUSTOMER_ERRENSUREONLINEFAILED);
	PHPKUSTOMER_DECLARE_ERROR_CONSTANT("ErrEnsureTrustedFailed", KUSTOMER_ERRENSURETRUSTEDFAILED);
	PHPKUSTOMER_DECLARE_ERROR_CONSTANT("ErrEnsureProductNotFound", KUSTOMER_ERRENSUREPRODUCTNOTFOUND);
	PHPKUSTOMER_DECLARE_ERROR_CONSTANT("ErrEnsureProductNotLicensed", KUSTOMER_ERRENSUREPRODUCTNOTLICENSED);
	PHPKUSTOMER_DECLARE_ERROR_CONSTANT("ErrEnsureProductClaimNotFound", KUSTOMER_ERRENSUREPRODUCTCLAIMNOTFOUND);
	PHPKUSTOMER_DECLARE_ERROR_CONSTANT("ErrEnsureProductClaimValueTypeMismatch", KUSTOMER_ERRENSUREPRODUCTCLAIMVALUETYPEMISMATCH);
	PHPKUSTOMER_DECLARE_ERROR_CONSTANT("ErrEnsureProductClaimValueMismatch", KUSTOMER_ERRENSUREPRODUCTCLAIMVALUEMISMATCH);
	PHPKUSTOMER_DECLARE_ERROR_CONSTANT("ErrEnsureUnknownOperator", KUSTOMER_ERRENSUREUNKNOWNOPERATOR);
	PHPKUSTOMER_DECLARE_ERROR_CONSTANT("ErrEnsureInvalidTransaction", KUSTOMER_ERRENSUREINVALIDTRANSACTION);

	zend_class_entry tmp_kpc_ce;
	INIT_CLASS_ENTRY(tmp_kpc_ce, "KUSTOMER\\KopanoProductClaims", phpkustomer_KopanoProductClaims_functions);
	phpkustomer_KopanoProductClaims_ce = zend_register_internal_class(&tmp_kpc_ce);
	phpkustomer_KopanoProductClaims_ce->create_object = phpkustomer_KopanoProductClaims_create_handler;

	memcpy(&phpkustomer_KopanoProductClaims_object_handlers,
		zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	phpkustomer_KopanoProductClaims_object_handlers.offset = XtOffsetOf(phpkustomer_KopanoProductClaims_t, std);

	return SUCCESS;
}
/* }}} */

/* {{{ kustomer_php_functions[] */
zend_function_entry kustomer_php_functions[] = {
	PHP_FE(kustomer_initialize, arginfo_kustomer_initialize)
	PHP_FE(kustomer_uninitialize, arginfo_kustomer_uninitialize)
	PHP_FE(kustomer_wait_until_ready, arginfo_kustomer_wait_until_ready)
	PHP_FE(kustomer_begin_ensure, arginfo_kustomer_begin_ensure)
	PHP_FE(kustomer_end_ensure, arginfo_kustomer_end_ensure)
	PHP_FE(kustomer_ensure_ok, arginfo_kustomer_ensure_ok)
	PHP_FE(kustomer_ensure_get_bool, arginfo_kustomer_ensure_get)
	PHP_FE(kustomer_ensure_ensure_bool, arginfo_kustomer_ensure_ensure_bool)
	PHP_FE(kustomer_ensure_get_string, arginfo_kustomer_ensure_get)
	PHP_FE(kustomer_ensure_ensure_string, arginfo_kustomer_ensure_ensure_string)
	PHP_FE(kustomer_ensure_get_int64, arginfo_kustomer_ensure_get_int64)
	PHP_FE(kustomer_ensure_ensure_int64, arginfo_kustomer_ensure_ensure_int64)
	PHP_FE(kustomer_ensure_get_float64, arginfo_kustomer_ensure_get)
	PHP_FE(kustomer_ensure_ensure_float64, arginfo_kustomer_ensure_ensure_float64)
	PHP_FE_END
};
/* }}} */

/* {{{ kustomer_php_module_entry
 */
zend_module_entry kustomer_php_module_entry = {
	STANDARD_MODULE_HEADER,
	PHP_KUSTOMER_EXTNAME,
	kustomer_php_functions,
	PHP_MINIT(kustomer_php),
	NULL,
	NULL,
	NULL,
	NULL,
	PHP_KUSTOMER_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

ZEND_GET_MODULE(kustomer_php)
