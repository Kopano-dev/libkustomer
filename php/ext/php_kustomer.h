/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

#define PHP_KUSTOMER_EXTNAME "kustomer"
#define PHP_KUSTOMER_VERSION "0.2.0"

// Exports.
PHP_FUNCTION(kustomer_initialize);
PHP_FUNCTION(kustomer_uninitialize);
PHP_FUNCTION(kustomer_wait_until_ready);
PHP_FUNCTION(kustomer_begin_ensure);
PHP_FUNCTION(kustomer_end_ensure);
PHP_FUNCTION(kustomer_ensure_ok);
PHP_FUNCTION(kustomer_ensure_get_bool);
PHP_FUNCTION(kustomer_ensure_ensure_bool);
PHP_FUNCTION(kustomer_ensure_get_string);
PHP_FUNCTION(kustomer_ensure_ensure_string);
PHP_FUNCTION(kustomer_ensure_get_int64);
PHP_FUNCTION(kustomer_ensure_ensure_int64);
PHP_FUNCTION(kustomer_ensure_ensure_int64_op);
PHP_FUNCTION(kustomer_ensure_get_float64);
PHP_FUNCTION(kustomer_ensure_ensure_float64);
PHP_FUNCTION(kustomer_ensure_ensure_float64_op);

// Objects.
zend_class_entry *phpkustomer_NumericException_ce;

#define PHPKUSTOMER_THROW(res) \
	(zend_throw_exception_ex(phpkustomer_NumericException_ce, res, "%s (:0x%x)",kustomer_err_numeric_text(res), res))

static zend_object_handlers phpkustomer_KopanoProductClaims_object_handlers;

typedef struct {
	void *kpc_ptr;
	zend_object std;
} phpkustomer_KopanoProductClaims_t;

#define PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(obj) \
	((phpkustomer_KopanoProductClaims_t*)((char*)(obj) - XtOffsetOf(phpkustomer_KopanoProductClaims_t, std)))

#define Z_PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(zv) \
	(PHPKUSTOMER_KOPANOPRODUCTCLAIMS_P(Z_OBJ_P(zv)))

zend_class_entry *phpkustomer_KopanoProductClaims_ce;

// Constants.
#define PHPKUSTOMER_DECLARE_ERROR_CONSTANT(name, value) \
	(zend_declare_class_constant_long(phpkustomer_NumericException_ce, name, sizeof(name)-1, value))
#define PHPKUSTOMER_DECLARE_ENSURE_CONSTANT(name, value) \
	(zend_declare_class_constant_long(phpkustomer_KopanoProductClaims_ce, name, sizeof(name)-1, value))
