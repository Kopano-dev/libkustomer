/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package kustomer

import (
	api "stash.kopano.io/kgol/kustomer/server/api-v1"
)

// OperatorsType is a special type of strring which can be used as operator.
type OperatorType string

// Operators to use with the operator comparisons.
const (
	OperatorGreaterThan        OperatorType = "gt"
	OperatorGreaterThanOrEqual OperatorType = "ge"
	OperatorLesserThan         OperatorType = "lt"
	OperatorLesserThanOrEqual  OperatorType = "le"
)

// Claims represent a set of active claim key value pairs.
type Claims struct {
	response *api.ClaimsResponse
}

// Dump exports the associated Claims payload data.
func (c *Claims) Dump() map[string]interface{} {
	return map[string]interface{}{
		"payload": c.response,
	}
}

// KopanoProductClaims represent a set of all active claims as aggregated
// values.
type KopanoProductClaims struct {
	response *api.ClaimsKopanoProductsResponse

	mustBeOnline   bool
	allowUntrusted bool
}

// Dump exports the associated KopanoProductClaims data.
func (kpc *KopanoProductClaims) Dump() map[string]interface{} {
	return map[string]interface{}{
		"mustBeOnline":   kpc.mustBeOnline,
		"allowUntrusted": kpc.allowUntrusted,
		"payload":        kpc.response,
	}
}

// SetMustBeOnline sets the mustBeOnline flag value of the associated claims
// to the provided flag value. If true, any ensure check of the associated
// claims will fail if the claims data was produced without online verification.
func (kpc *KopanoProductClaims) SetMustBeOnline(flag bool) {
	kpc.mustBeOnline = flag
}

// SetAllowUntrusted sets the allowUntrusted flag value of the associated claims
// to the provided flag value. If true, any ensure check of the associated
// claims will ignore the trusted state of the claims data.
func (kpc *KopanoProductClaims) SetAllowUntrusted(flag bool) {
	kpc.allowUntrusted = flag
}

// EnsureOnline returns ErrEnsureOnlineFailed error if the associated claims
// data was validated with offline. This function returns the error even if
// the associated claims mustBeOnline flag was is false.
func (kpc *KopanoProductClaims) EnsureOnline() (err error) {
	if kpc.response.Offline {
		return ErrEnsureOnlineFailed
	}
	return
}

// EnsureTrusted returns ErrEnsureTrustedFailed error if the associated claims
// data is not trusted. This function will return the error even if the
// associated claims SetAllowUntrusted was set to true.
func (kpc *KopanoProductClaims) EnsureTrusted() (err error) {
	if !kpc.response.Trusted {
		return ErrEnsureTrustedFailed
	}
	return
}

// EnsureOnlineAndTrusted is the combination of EnsureOnline and EnsureOnline
// for convinience. Samle rules apply as described in those two functions.
func (kpc *KopanoProductClaims) EnsureOnlineAndTrusted() (err error) {
	if err := kpc.EnsureOnline(); err != nil {
		return err
	}
	if err := kpc.EnsureTrusted(); err != nil {
		return err
	}
	return
}

func (kpc *KopanoProductClaims) getProduct(product string) (*api.ClaimsKopanoProductsResponseProduct, error) {
	if err := kpc.EnsureOnline(); kpc.mustBeOnline && err != nil {
		return nil, err
	}
	if err := kpc.EnsureTrusted(); !kpc.allowUntrusted && err != nil {
		return nil, err
	}

	p, ok := kpc.response.Products[product]
	if !ok {
		return nil, ErrEnsureProductNotFound
	}
	return p, nil
}

func (kpc *KopanoProductClaims) ensureValue(product, claim string) (interface{}, error) {
	p, err := kpc.getProduct(product)
	if err != nil {
		return nil, err
	}
	if !p.OK {
		return nil, ErrEnsureProductNotLicensed
	}

	v, ok := p.Claims[claim]
	if !ok {
		return nil, ErrEnsureProductClaimNotFound
	}

	return v, nil
}

// EnsureOK returns an error if the provided product is not found in the
// associated claims data or if that product is found but the OK flag of the
// active product is false.
func (kpc *KopanoProductClaims) EnsureOK(product string) (err error) {
	p, err := kpc.getProduct(product)
	if err != nil {
		return err
	}
	if !p.OK {
		return ErrEnsureProductNotLicensed
	}
	return nil
}

// GetBool returns the prodvided prodcut claim bool value. If the product or
// the claim is not found, an the returned error describes the reason why the
// claim value is not available.
func (kpc *KopanoProductClaims) GetBool(product, claim string) (bool, error) {
	v, err := kpc.ensureValue(product, claim)
	if err != nil {
		return false, err
	}

	tv, ok := v.(bool)
	if !ok {
		return false, ErrEnsureProductClaimValueTypeMismatch
	}
	return tv, nil
}

// EnsureBool returns an error if the provided product or the claim value is not
// found. Furthermore the claim value is compared to the provided value and if
// it is not a match, an error is returned as well.
func (kpc *KopanoProductClaims) EnsureBool(product, claim string, value bool) error {
	tv, err := kpc.GetBool(product, claim)
	if err != nil {
		return err
	}

	if value != tv {
		return ErrEnsureProductClaimValueMismatch
	}
	return nil
}

// GetString returns the prodvided product claim string value. If the product
// or the claim is not found, an the returned error describes the reason why the
// claim value is not available.
func (kpc *KopanoProductClaims) GetString(product, claim string) (string, error) {
	v, err := kpc.ensureValue(product, claim)
	if err != nil {
		return "", err
	}

	tv, ok := v.(string)
	if !ok {
		return "", ErrEnsureProductClaimValueTypeMismatch
	}
	return tv, nil
}

// EnsureString returns an error if the provided product or the claim value is
// not found. Furthermore the claim value is compared to the provided value and
// if it is not a match, an error is returned as well.
func (kpc *KopanoProductClaims) EnsureString(product, claim, value string) error {
	tv, err := kpc.GetString(product, claim)
	if err != nil {
		return err
	}

	if value != tv {
		return ErrEnsureProductClaimValueMismatch
	}
	return nil
}

// GetInt64 returns the prodvided product claim numeric value. If the product
// or the claim is not found, an the returned error describes the reason why the
// claim value is not available.
func (kpc *KopanoProductClaims) GetInt64(product, claim string) (int64, error) {
	v, err := kpc.ensureValue(product, claim)
	if err != nil {
		return 0, err
	}

	tv, ok := v.(float64)
	if !ok {
		return 0, ErrEnsureProductClaimValueTypeMismatch
	}
	return int64(tv), nil
}

// EnsureInt64 returns an error if the provided product or the claim value is
// not found. Furthermore the claim value is compared to the provided value and
// if it is not a match, an error is returned as well.
func (kpc *KopanoProductClaims) EnsureInt64(product, claim string, value int64) error {
	tv, err := kpc.GetInt64(product, claim)
	if err != nil {
		return err
	}

	if value != tv {
		return ErrEnsureProductClaimValueMismatch
	}
	return nil
}

// EnsureInt64WithOperator returns an error if the provided product or the claim
// value is not found. Furthermore the claim value is compared to the provided
// value using the provided comparison operator and if it is not a match, an
// error is returned as well.
func (kpc *KopanoProductClaims) EnsureInt64WithOperator(product, claim string, value int64, op OperatorType) error {
	tv, err := kpc.GetInt64(product, claim)
	if err != nil {
		return err
	}

	switch op {
	case OperatorGreaterThan:
		if tv > value {
			return nil
		}
	case OperatorGreaterThanOrEqual:
		if tv >= value {
			return nil
		}
	case OperatorLesserThan:
		if tv < value {
			return nil
		}
	case OperatorLesserThanOrEqual:
		if tv <= value {
			return nil
		}
	default:
		return ErrEnsureUnknownOperator
	}
	return ErrEnsureProductClaimValueMismatch
}

// GetFloat64 returns the prodvided product claim float value. If the product
// or the claim is not found, an the returned error describes the reason why the
// claim value is not available.
func (kpc *KopanoProductClaims) GetFloat64(product, claim string) (float64, error) {
	v, err := kpc.ensureValue(product, claim)
	if err != nil {
		return 0, err
	}

	tv, ok := v.(float64)
	if !ok {
		return 0, ErrEnsureProductClaimValueTypeMismatch
	}
	return tv, nil
}

// EnsureFloat64 returns an error if the provided product or the claim value is
// not found. Furthermore the claim value is compared to the provided value and
// if it is not a match, an error is returned as well.
func (kpc *KopanoProductClaims) EnsureFloat64(product, claim string, value float64) error {
	tv, err := kpc.GetFloat64(product, claim)
	if err != nil {
		return err
	}

	if value != tv {
		return ErrEnsureProductClaimValueMismatch
	}
	return nil
}

// EnsureFloat64WithOperator returns an error if the provided product or the
// claim value is not found. Furthermore the claim value is compared to the
// provided value using the provided comparison operator and if it is not a
// match, an error is returned as well.
func (kpc *KopanoProductClaims) EnsureFloat64WithOperator(product, claim string, value float64, op OperatorType) error {
	tv, err := kpc.GetFloat64(product, claim)
	if err != nil {
		return err
	}

	switch op {
	case OperatorGreaterThan:
		if tv > value {
			return nil
		}
	case OperatorGreaterThanOrEqual:
		if tv >= value {
			return nil
		}
	case OperatorLesserThan:
		if tv < value {
			return nil
		}
	case OperatorLesserThanOrEqual:
		if tv <= value {
			return nil
		}
	default:
		return ErrEnsureUnknownOperator
	}
	return ErrEnsureProductClaimValueMismatch
}

// GetStringArrayValues returns the prodvided product claim string array value.
// If the product  or the claim is not found, an the returned error describes
// the reason why the claim value is not available.
func (kpc *KopanoProductClaims) GetStringArrayValues(product, claim string) ([]string, error) {
	v, err := kpc.ensureValue(product, claim)
	if err != nil {
		return nil, err
	}

	tv, ok := v.([]string)
	if !ok {
		return nil, ErrEnsureProductClaimValueTypeMismatch
	}
	return tv, nil
}

// EnsureStringArrayValues returns an error if the provided product or the claim
// value is not found. Furthermore if not all of the provided value prameters
// are present in the claim value n error is returned as well.
func (kpc *KopanoProductClaims) EnsureStringArrayValues(product, claim string, value ...string) error {
	tv, err := kpc.GetStringArrayValues(product, claim)
	if err != nil {
		return err
	}

	for _, v := range value {
		found := false
		for _, e := range tv {
			if v == e {
				found = true
				break
			}
		}
		if !found {
			return ErrEnsureProductClaimValueMismatch
		}
	}

	return nil
}
