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

// Operators to use with the operator comparions.
const (
	OperatorGreaterThan        OperatorType = "gt"
	OperatorGreaterThanOrEqual OperatorType = "ge"
	OperatorLesserThan         OperatorType = "lt"
	OperatorLesserThanOrEqual  OperatorType = "le"
)

type KopanoProductClaims struct {
	response *api.ClaimsKopanoProductsResponse

	mustBeOnline   bool
	allowUntrusted bool
}

func (kpc *KopanoProductClaims) MustBeOnline(flag bool) {
	kpc.mustBeOnline = flag
}

func (kpc *KopanoProductClaims) AllowUntrusted(flag bool) {
	kpc.allowUntrusted = flag
}

func (kpc *KopanoProductClaims) EnsureOnline() (err error) {
	if kpc.response.Offline == true {
		return ErrEnsureOnlineFailed
	}
	return
}

func (kpc *KopanoProductClaims) EnsureTrusted() (err error) {
	if kpc.response.Trusted == false {
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

func (kpc *KopanoProductClaims) ensureValue(product string, claim string) (interface{}, error) {
	if err := kpc.EnsureOnline(); kpc.mustBeOnline && err != nil {
		return nil, err
	}
	if err := kpc.EnsureTrusted(); kpc.allowUntrusted && err != nil {
		return nil, err
	}

	p, ok := kpc.response.Products[product]
	if !ok {
		return nil, ErrEnsureProductNotFound
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

func (kpc *KopanoProductClaims) EnsureBool(product string, claim string, value bool) error {
	tv, err := kpc.GetBool(product, claim)
	if err != nil {
		return err
	}

	if value != tv {
		return ErrEnsureProductClaimValueMismatch
	}
	return nil
}

func (kpc *KopanoProductClaims) GetString(product string, claim string) (string, error) {
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

func (kpc *KopanoProductClaims) EnsureString(product string, claim string, value string) error {
	tv, err := kpc.GetString(product, claim)
	if err != nil {
		return err
	}

	if value != tv {
		return ErrEnsureProductClaimValueMismatch
	}
	return nil
}

func (kpc *KopanoProductClaims) GetInt64(product string, claim string) (int64, error) {
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

func (kpc *KopanoProductClaims) EnsureInt64(product string, claim string, value int64) error {
	tv, err := kpc.GetInt64(product, claim)
	if err != nil {
		return err
	}

	if value != tv {
		return ErrEnsureProductClaimValueMismatch
	}
	return nil
}

func (kpc *KopanoProductClaims) EnsureInt64WithOperator(product string, claim string, value int64, op OperatorType) error {
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

func (kpc *KopanoProductClaims) GetFloat64(product string, claim string) (float64, error) {
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

func (kpc *KopanoProductClaims) EnsureFloat64(product string, claim string, value float64) error {
	tv, err := kpc.GetFloat64(product, claim)
	if err != nil {
		return err
	}

	if value != tv {
		return ErrEnsureProductClaimValueMismatch
	}
	return nil
}

func (kpc *KopanoProductClaims) EnsureFloat64WithOperator(product string, claim string, value float64, op OperatorType) error {
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
