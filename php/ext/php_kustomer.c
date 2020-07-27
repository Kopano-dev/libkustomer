/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

#include <php.h>
#include <zend_exceptions.h>

#ifdef HAVE_SPL
#include "ext/spl/spl_exceptions.h"
#endif

#include "ext/standard/info.h"

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

ZEND_BEGIN_ARG_INFO_EX(arginfo_kustomer_instant_ensure, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, productName, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, productUserAgent, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 1)
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_kustomer_ensure_ensure_int64_op, 0, 0, 5)
	ZEND_ARG_TYPE_INFO(0, transaction, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, productName, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, claim, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, opCode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_kustomer_ensure_ensure_float64, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, transaction, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, productName, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, claim, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_kustomer_ensure_ensure_float64_op, 0, 0, 5)
	ZEND_ARG_TYPE_INFO(0, transaction, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, productName, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, claim, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, opCode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_kustomer_ensure_ensure_stringArray_value, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, transaction, IS_OBJECT, 0)
	ZEND_ARG_TYPE_INFO(0, productName, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, claim, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

/* }}} */

// Externals from libkustomer.
typedef char* (*kustomer_err_numeric_text_dynamic_t)(unsigned long long errNum);
typedef char* (*kustomer_version_dynamic_t)();
typedef char* (*kustomer_build_date_dynamic_t)();
typedef long long unsigned int (*kustomer_initialize_dynamic_t)(char *productName);
typedef long long unsigned int (*kustomer_uninitialize_dynamic_t)();
typedef long long unsigned int (*kustomer_wait_until_ready_dynamic_t)(unsigned long long timeout);
typedef struct kustomer_begin_ensure_return (*kustomer_begin_ensure_dynamic_t)();
typedef struct kustomer_instant_ensure_return (*kustomer_instant_ensure_dynamic_t)(char *productName, char *productUserAgent, unsigned long long timeout);
typedef long long unsigned int (*kustomer_end_ensure_dynamic_t)(void *transactionPtr);
typedef long long unsigned int (*kustomer_ensure_set_allow_untrusted_dynamic_t)(void *transactionPtr, int flag);
typedef long long unsigned int (*kustomer_ensure_ok_dynamic_t)(void *transactionPtr, char *productName);
typedef struct kustomer_ensure_get_bool_return (*kustomer_ensure_get_bool_dynamic_t)(void *transactionPtr, char *productName, char *claim);
typedef long long unsigned int (*kustomer_ensure_ensure_bool_dynamic_t)(void *transactionPtr, char *productName, char *claim, int value);
typedef struct kustomer_ensure_get_string_return (*kustomer_ensure_get_string_dynamic_t)(void *transactionPtr, char *productName, char *claim);
typedef long long unsigned int (*kustomer_ensure_ensure_string_dynamic_t)(void *transactionPtr, char *productName, char *claim, char *value);
typedef struct kustomer_ensure_get_int64_return (*kustomer_ensure_get_int64_dynamic_t)(void *transactionPtr, char *productName, char *claim);
typedef long long unsigned int (*kustomer_ensure_ensure_int64_dynamic_t)(void *transactionPtr, char *productName, char *claim, long long value);
typedef long long unsigned int (*kustomer_ensure_ensure_int64_op_dynamic_t)(void *transactionPtr, char *productName, char *claim, long long value, int opCode);
typedef struct kustomer_ensure_get_float64_return (*kustomer_ensure_get_float64_dynamic_t)(void *transactionPtr, char *productName, char *claim);
typedef long long unsigned int (*kustomer_ensure_ensure_float64_dynamic_t)(void *transactionPtr, char *productName, char *claim, double value);
typedef long long unsigned int (*kustomer_ensure_ensure_float64_op_dynamic_t)(void *transactionPtr, char *productName, char *claim, double value, int opCode);
typedef long long unsigned int (*kustomer_ensure_ensure_stringArray_value_dynamic_t)(void *transactionPtr, char *productName, char *claim, char *value);
kustomer_err_numeric_text_dynamic_t kustomer_err_numeric_text_dynamic = NULL;
kustomer_version_dynamic_t kustomer_version_dynamic = NULL;
kustomer_build_date_dynamic_t kustomer_build_date_dynamic = NULL;
kustomer_initialize_dynamic_t kustomer_initialize_dynamic = NULL;
kustomer_uninitialize_dynamic_t kustomer_uninitialize_dynamic = NULL;
kustomer_wait_until_ready_dynamic_t kustomer_wait_until_ready_dynamic = NULL;
kustomer_begin_ensure_dynamic_t kustomer_begin_ensure_dynamic = NULL;
kustomer_instant_ensure_dynamic_t kustomer_instant_ensure_dynamic = NULL;
kustomer_end_ensure_dynamic_t kustomer_end_ensure_dynamic = NULL;
kustomer_ensure_set_allow_untrusted_dynamic_t kustomer_ensure_set_allow_untrusted_dynamic = NULL;
kustomer_ensure_ok_dynamic_t kustomer_ensure_ok_dynamic = NULL;
kustomer_ensure_get_bool_dynamic_t kustomer_ensure_get_bool_dynamic = NULL;
kustomer_ensure_ensure_bool_dynamic_t kustomer_ensure_ensure_bool_dynamic = NULL;
kustomer_ensure_get_string_dynamic_t kustomer_ensure_get_string_dynamic = NULL;
kustomer_ensure_ensure_string_dynamic_t kustomer_ensure_ensure_string_dynamic = NULL;
kustomer_ensure_get_int64_dynamic_t kustomer_ensure_get_int64_dynamic = NULL;
kustomer_ensure_ensure_int64_dynamic_t kustomer_ensure_ensure_int64_dynamic = NULL;
kustomer_ensure_ensure_int64_op_dynamic_t kustomer_ensure_ensure_int64_op_dynamic = NULL;
kustomer_ensure_get_float64_dynamic_t kustomer_ensure_get_float64_dynamic = NULL;
kustomer_ensure_ensure_float64_dynamic_t kustomer_ensure_ensure_float64_dynamic = NULL;
kustomer_ensure_ensure_float64_op_dynamic_t kustomer_ensure_ensure_float64_op_dynamic = NULL;
kustomer_ensure_ensure_stringArray_value_dynamic_t kustomer_ensure_ensure_stringArray_value_dynamic = NULL;

// Global signleton to remember dlopen.
int kustomer_so_loaded = 0;

// Module initializer.
int load_so()
{
	if (kustomer_so_loaded == 1) {
		return KUSTOMER_ERRSTATUSSUCCESS;
	}

	void* libkustomer_library_handle = dlopen(PHP_KUSTOMER_SO, RTLD_NOW);
	if (libkustomer_library_handle == NULL) {
		zend_throw_exception_ex(phpkustomer_NumericException_ce, KUSTOMER_ERRSTATUSTIMEOUT, "%s", "Could not load libkustomer.so.0 library");
		return KUSTOMER_ERRSTATUSTIMEOUT;
	}
	dlerror();

	kustomer_err_numeric_text_dynamic = (kustomer_err_numeric_text_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_err_numeric_text");
	if (kustomer_err_numeric_text_dynamic == NULL) {
		zend_throw_exception_ex(phpkustomer_NumericException_ce, KUSTOMER_ERRSTATUSTIMEOUT, "Could not load expected function from library");
		return KUSTOMER_ERRSTATUSTIMEOUT;
	}

	kustomer_version_dynamic = (kustomer_version_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_version");
	if (kustomer_version_dynamic == NULL) {
		zend_throw_exception_ex(phpkustomer_NumericException_ce, KUSTOMER_ERRSTATUSTIMEOUT, "Could not load expected function from library");
		return KUSTOMER_ERRSTATUSTIMEOUT;
	}

	kustomer_build_date_dynamic = (kustomer_build_date_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_build_date");
	if (kustomer_build_date_dynamic == NULL) {
		zend_throw_exception_ex(phpkustomer_NumericException_ce, KUSTOMER_ERRSTATUSTIMEOUT, "Could not load expected function from library");
		return KUSTOMER_ERRSTATUSTIMEOUT;
	}

	kustomer_initialize_dynamic = (kustomer_initialize_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_initialize");
	if (kustomer_initialize_dynamic == NULL) {
		zend_throw_exception_ex(phpkustomer_NumericException_ce, KUSTOMER_ERRSTATUSTIMEOUT, "Could not load expected function from library");
		return KUSTOMER_ERRSTATUSTIMEOUT;
	}
	kustomer_uninitialize_dynamic = (kustomer_uninitialize_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_uninitialize");
	kustomer_wait_until_ready_dynamic = (kustomer_wait_until_ready_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_wait_until_ready");
	kustomer_begin_ensure_dynamic = (kustomer_begin_ensure_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_begin_ensure");
	kustomer_instant_ensure_dynamic = (kustomer_instant_ensure_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_instant_ensure");
	kustomer_end_ensure_dynamic = (kustomer_end_ensure_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_end_ensure");
	kustomer_ensure_set_allow_untrusted_dynamic = (kustomer_ensure_set_allow_untrusted_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_ensure_set_allow_untrusted");
	kustomer_ensure_ok_dynamic = (kustomer_ensure_ok_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_ensure_ok");
	kustomer_ensure_get_bool_dynamic = (kustomer_ensure_get_bool_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_ensure_get_bool");
	kustomer_ensure_ensure_bool_dynamic = (kustomer_ensure_ensure_bool_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_ensure_ensure_bool");
	kustomer_ensure_get_string_dynamic = (kustomer_ensure_get_string_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_ensure_get_string");
	kustomer_ensure_ensure_string_dynamic = (kustomer_ensure_ensure_string_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_ensure_ensure_string");
	kustomer_ensure_get_int64_dynamic = (kustomer_ensure_get_int64_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_ensure_get_int64");
	kustomer_ensure_ensure_int64_dynamic = (kustomer_ensure_ensure_int64_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_ensure_ensure_int64");
	kustomer_ensure_ensure_int64_op_dynamic = (kustomer_ensure_ensure_int64_op_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_ensure_ensure_int64_op");
	kustomer_ensure_get_float64_dynamic = (kustomer_ensure_get_float64_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_ensure_get_float64");
	kustomer_ensure_ensure_float64_dynamic = (kustomer_ensure_ensure_float64_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_ensure_ensure_float64");
	kustomer_ensure_ensure_float64_op_dynamic = (kustomer_ensure_ensure_float64_op_dynamic_t)dlsym(libkustomer_library_handle, "ustomer_ensure_ensure_float64_op");
	kustomer_ensure_ensure_stringArray_value_dynamic = (kustomer_ensure_ensure_stringArray_value_dynamic_t)dlsym(libkustomer_library_handle, "kustomer_ensure_ensure_stringArray_value");

	kustomer_so_loaded = 1;
	return KUSTOMER_ERRSTATUSSUCCESS;
}

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

	if ((res = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}

	if ((res = kustomer_initialize_dynamic(productName)) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}
}

PHP_FUNCTION(kustomer_uninitialize)
{
	ZEND_PARSE_PARAMETERS_START(0, 0)
	ZEND_PARSE_PARAMETERS_END();

	int res;

	if ((res = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}

	if ((res = kustomer_uninitialize_dynamic()) != KUSTOMER_ERRSTATUSSUCCESS) {
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

	if ((res = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}

	if ((res = kustomer_wait_until_ready_dynamic((long long unsigned int)timeout)) != KUSTOMER_ERRSTATUSSUCCESS) {
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

	int so;

	if ((so = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(so);
		return;
	}

	struct kustomer_begin_ensure_return res;

	res = kustomer_begin_ensure_dynamic();

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
	kustomer_ensure_set_allow_untrusted_dynamic(kpc->kpc_ptr, 1);
#endif

	RETURN_OBJ(obj);
}

PHP_FUNCTION(kustomer_instant_ensure)
{

	zval *productNameVal;
	zval *productUserAgentVal;
	zend_long timeout;

	ZEND_PARSE_PARAMETERS_START(3, 3)
		Z_PARAM_ZVAL(productNameVal)
		Z_PARAM_ZVAL(productUserAgentVal)
		Z_PARAM_LONG(timeout)
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

	char *productUserAgent;
	if (Z_TYPE_P(productUserAgentVal) == IS_STRING) {
		productUserAgent = Z_STRVAL_P(productUserAgentVal);
	} else if(Z_TYPE_P(productUserAgentVal) == IS_NULL) {
		productUserAgent = NULL;
	} else {
		php_error_docref(NULL, E_WARNING, "unexpected argument type");
		return;
	}

	int so;

	if ((so = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(so);
		return;
	}

	struct kustomer_instant_ensure_return res;

	res = kustomer_instant_ensure_dynamic(productName, productUserAgent, (long long unsigned int)timeout);

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
	kustomer_ensure_set_allow_untrusted_dynamic(kpc->kpc_ptr, 1);
#endif

	RETURN_OBJ(obj);
}



PHP_FUNCTION(kustomer_end_ensure)
{
	zval *kpc_zv;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJECT_OF_CLASS(kpc_zv, phpkustomer_KopanoProductClaims_ce)
	ZEND_PARSE_PARAMETERS_END();

	int res;

	if ((res = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	res = kustomer_end_ensure_dynamic(kpc->kpc_ptr);

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

	int res;

	if ((res = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	res = kustomer_ensure_ok_dynamic(kpc->kpc_ptr, ZSTR_VAL(productName));

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

	int so;

	if ((so = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(so);
		return;
	}

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	struct kustomer_ensure_get_bool_return res;

	res = kustomer_ensure_get_bool_dynamic(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim));

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

	int res;

	if ((res = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	res = kustomer_ensure_ensure_bool_dynamic(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim), (int)value);

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

	int so;

	if ((so = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(so);
		return;
	}

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	struct kustomer_ensure_get_string_return res;

	res = kustomer_ensure_get_string_dynamic(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim));

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

	int res;

	if ((res = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	res = kustomer_ensure_ensure_string_dynamic(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim), ZSTR_VAL(value));

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

	int so;

	if ((so = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(so);
		return;
	}

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	struct kustomer_ensure_get_int64_return res;

	res = kustomer_ensure_get_int64_dynamic(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim));

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

	int res;

	if ((res = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	res = kustomer_ensure_ensure_int64_dynamic(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim), (long long int)value);

	zend_string_release(productName);
	zend_string_release(claim);

	if (res != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}
}

PHP_FUNCTION(kustomer_ensure_ensure_int64_op)
{
	zval *kpc_zv;
	zend_string *productName;
	zend_string *claim;
	zend_long value;
	zend_long opCode;

	ZEND_PARSE_PARAMETERS_START(5, 5)
		Z_PARAM_OBJECT_OF_CLASS(kpc_zv, phpkustomer_KopanoProductClaims_ce)
		Z_PARAM_STR(productName)
		Z_PARAM_STR(claim)
		Z_PARAM_LONG(value)
		Z_PARAM_LONG(opCode)
	ZEND_PARSE_PARAMETERS_END();

	int res;

	if ((res = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	res = kustomer_ensure_ensure_int64_op_dynamic(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim), (long long int)value, (int)opCode);

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

	int so;

	if ((so = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(so);
		return;
	}

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	struct kustomer_ensure_get_float64_return res;

	res = kustomer_ensure_get_float64_dynamic(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim));

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

	int res;

	if ((res = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	res = kustomer_ensure_ensure_float64_dynamic(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim), value);

	zend_string_release(productName);
	zend_string_release(claim);

	if (res != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}
}

PHP_FUNCTION(kustomer_ensure_ensure_float64_op)
{
	zval *kpc_zv;
	zend_string *productName;
	zend_string *claim;
	double value;
	zend_long opCode;

	ZEND_PARSE_PARAMETERS_START(5, 5)
		Z_PARAM_OBJECT_OF_CLASS(kpc_zv, phpkustomer_KopanoProductClaims_ce)
		Z_PARAM_STR(productName)
		Z_PARAM_STR(claim)
		Z_PARAM_DOUBLE(value)
		Z_PARAM_LONG(opCode)
	ZEND_PARSE_PARAMETERS_END();

	int res;

	if ((res = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	res = kustomer_ensure_ensure_float64_op_dynamic(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim), value, (int)opCode);

	zend_string_release(productName);
	zend_string_release(claim);

	if (res != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}
}

PHP_FUNCTION(kustomer_ensure_ensure_stringArray_value)
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

	int res;

	if ((res = load_so()) != KUSTOMER_ERRSTATUSSUCCESS) {
		PHPKUSTOMER_THROW(res);
		return;
	}

	phpkustomer_KopanoProductClaims_t *kpc;
	kpc = Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(kpc_zv);

	res = kustomer_ensure_ensure_stringArray_value_dynamic(kpc->kpc_ptr, ZSTR_VAL(productName), ZSTR_VAL(claim), ZSTR_VAL(value));

	zend_string_release(productName);
	zend_string_release(claim);
	zend_string_release(value);

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
	PHPKUSTOMER_DECLARE_ENSURE_CONSTANT("OperatorGreaterThan", KUSTOMER_OPERATOR_GT);
	PHPKUSTOMER_DECLARE_ENSURE_CONSTANT("OperatorGreaterThanOrEqual", KUSTOMER_OPERATOR_GE);
	PHPKUSTOMER_DECLARE_ENSURE_CONSTANT("OperatorLesserThan", KUSTOMER_OPERATOR_LT);
	PHPKUSTOMER_DECLARE_ENSURE_CONSTANT("OperatorLesserThanOrEqual", KUSTOMER_OPERATOR_LE);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(kustomer_php) {
	int res = load_so();

	char buf[128];
	php_info_print_table_start();
	php_info_print_table_row(2, "kustomer support", "enabled");
	php_info_print_table_row(2, "kustomer EXT version", PHP_KUSTOMER_VERSION);
	if (res == KUSTOMER_ERRSTATUSSUCCESS) {
		snprintf(buf, sizeof(buf), "%s (%s)", kustomer_version_dynamic(), kustomer_build_date_dynamic());
	} else {
		snprintf(buf, sizeof(buf), "failed to load");
	}
	php_info_print_table_row(2, "libkustomer library version", buf);
	php_info_print_table_end();
}
/* }}} */

/* {{{ kustomer_php_functions[] */
zend_function_entry kustomer_php_functions[] = {
	PHP_FE(kustomer_initialize, arginfo_kustomer_initialize)
	PHP_FE(kustomer_uninitialize, arginfo_kustomer_uninitialize)
	PHP_FE(kustomer_wait_until_ready, arginfo_kustomer_wait_until_ready)
	PHP_FE(kustomer_begin_ensure, arginfo_kustomer_begin_ensure)
	PHP_FE(kustomer_instant_ensure, arginfo_kustomer_instant_ensure)
	PHP_FE(kustomer_end_ensure, arginfo_kustomer_end_ensure)
	PHP_FE(kustomer_ensure_ok, arginfo_kustomer_ensure_ok)
	PHP_FE(kustomer_ensure_get_bool, arginfo_kustomer_ensure_get)
	PHP_FE(kustomer_ensure_ensure_bool, arginfo_kustomer_ensure_ensure_bool)
	PHP_FE(kustomer_ensure_get_string, arginfo_kustomer_ensure_get)
	PHP_FE(kustomer_ensure_ensure_string, arginfo_kustomer_ensure_ensure_string)
	PHP_FE(kustomer_ensure_get_int64, arginfo_kustomer_ensure_get)
	PHP_FE(kustomer_ensure_ensure_int64, arginfo_kustomer_ensure_ensure_int64)
	PHP_FE(kustomer_ensure_ensure_int64_op, arginfo_kustomer_ensure_ensure_int64_op)
	PHP_FE(kustomer_ensure_get_float64, arginfo_kustomer_ensure_get)
	PHP_FE(kustomer_ensure_ensure_float64, arginfo_kustomer_ensure_ensure_float64)
	PHP_FE(kustomer_ensure_ensure_float64_op, arginfo_kustomer_ensure_ensure_float64_op)
	PHP_FE(kustomer_ensure_ensure_stringArray_value, arginfo_kustomer_ensure_ensure_stringArray_value)
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
	PHP_MINFO(kustomer_php),
	PHP_KUSTOMER_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

ZEND_GET_MODULE(kustomer_php)
