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

type Claims struct {
	response *api.ClaimsResponse
}

func (c *Claims) Dump() map[string]interface{} {
	return map[string]interface{}{
		"payload": c.response,
	}
}

type KopanoProductClaims struct {
	response *api.ClaimsKopanoProductsResponse

	mustBeOnline   bool
	allowUntrusted bool
}

func (kpc *KopanoProductClaims) Dump() map[string]interface{} {
	return map[string]interface{}{
		"mustBeOnline":   kpc.mustBeOnline,
		"allowUntrusted": kpc.allowUntrusted,
		"payload":        kpc.response,
	}
}

func (kpc *KopanoProductClaims) SetMustBeOnline(flag bool) {
	kpc.mustBeOnline = flag
}

func (kpc *KopanoProductClaims) SetAllowUntrusted(flag bool) {
	kpc.allowUntrusted = flag
}

func (kpc *KopanoProductClaims) EnsureOnline() (err error) {
	if kpc.response.Offline {
		return ErrEnsureOnlineFailed
	}
	return
}

func (kpc *KopanoProductClaims) EnsureTrusted() (err error) {
	if kpc.response.Trusted {
		return ErrEnsureTrustedFailed
	}
	return
}

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

func (kpc *KopanoProductClaims) GetInt64(product, claim string) (int64, error) {
	v, err := kpc.ensureValue(product, claim)
	if err != nil {
		return 0, err
	}

	tv, ok := v.(int64)
	if !ok {
		return 0, ErrEnsureProductClaimValueTypeMismatch
	}
	return tv, nil
}

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

func (kpc *KopanoProductClaims) EnsureInt64WithOperator(product, claim string, value int64, op OperatorType) error {
	v, err := kpc.ensureValue(product, claim)
	if err != nil {
		return err
	}

	tv, ok := v.(int64)
	if !ok {
		return ErrEnsureProductClaimValueTypeMismatch
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

func (kpc *KopanoProductClaims) EnsureFloat64WithOperator(product, claim string, value float64, op OperatorType) error {
	v, err := kpc.ensureValue(product, claim)
	if err != nil {
		return err
	}

	tv, ok := v.(float64)
	if !ok {
		return ErrEnsureProductClaimValueTypeMismatch
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
