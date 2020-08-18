# CHANGELOG

## Unreleased

- Build with all hardening flags turned on


## v0.6.2 (2020-08-17)

- Allow to set additional EXTLDFLAGS
- Ensure to create pkg-config folder on install


## v0.6.1 (2020-08-12)

- Fix string array value ensure type conversion


## v0.6.0 (2020-08-05)

- Move Jenkins pipepline to warnings-ng plugin
- Fix unit test Jenkins reporting
- Fix some cosmetics in Makefile
- Make version package internal


## v0.5.1 (2020-07-30)

- Pass along product name to watch request
- Add link to Godocs
- Fix license text to match the template exactly


## v0.5.0 (2020-07-29)

- Include kustomer-dump utility in dist tarball
- Add commandline parameter to dump tool
- Add basic unit test
- Bump API model dependency to latest version
- Add doc strings to all public functions


## v0.4.2 (2020-07-29)

- Fix ensure check copy/paste error


## v0.4.1 (2020-07-28)

- Fix build when building without SPL
- Fix a bunch of compiler warnings and copy/paste errors


## v0.4.0 (2020-07-28)

- Add debug log support to PHP extension
- Display additional information in PHP info
- Bump PHP API version
- Add support for ensure stringArray to PHP API
- Add support for instant ensure to PHP API
- Add support for instant ensure in Go and C API
- Add support to ensure string array values for Go and C API


## v0.3.0 (2020-07-27)

- Export version and build data API for C
- Dynamically load libkustomer in PHP extension


## v0.2.1 (2020-07-26)

- Do not strip C library so by default, leave that to packaging


## v0.2.0 (2020-07-24)

- Expose operator compare functions to PHP API
- Simplify ensure operator compare functions
- Expose comparison operators to C API
- Bump exposed PHP version string to 0.2.0
- Use consistent php module name
- Fix fetch of int64 values
- Implement initial set of ensure support for PHP
- Add pkg-config support to build php extension


## v0.1.0 (2020-07-21)

- Expose update notify watch in C API
- Leave it to the c logger implementation to add new lines
- Fix trusted comparison check


## v0.0.1 (2020-07-20)

- Use correct folder name in example
- Build with proper version information
- Expose all Go ensure functions to C API
- Expose errors as preprocessor constants with C header
- Fix Jenkins build
- Fix linter errors
- Implement ensure support in Python module
- Add PHP extension minimal stub
- Fix debug logging crash when initialize was not called
- Fix logging crash when initializing with null product name
- Do not try to uninitialize when not initialized in c lib
- Add Python module minimal stub
- Improve cpp API
- Add basic cpp example
- Return correct errors when timing out context
- Add WaitUntilReady to the c API
- Add API to fetch claims (with cache)
- Implement API to get notified when claims are updated
- Implement ensure comparison functions
- Load product license claims from kustomerd api
- Update license ranger
- Add kustomer_available API for testing
- Add Jenkins builds via Dockerfile.build
- Initial commit

