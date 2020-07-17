/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

#define PHP_KUSTOMER_EXTNAME "php_kustomer"
#define PHP_KUSTOMER_VERSION "0.0.1"

// Exports.
PHP_FUNCTION(kustomer_initialize);
PHP_FUNCTION(kustomer_uninitialize);
PHP_FUNCTION(kustomer_wait_until_ready);
