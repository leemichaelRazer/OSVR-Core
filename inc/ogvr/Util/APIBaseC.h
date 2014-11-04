/** @file
    @brief Header

    Must be c-safe!

    @date 2014

    @author
    Ryan Pavlik
    <ryan@sensics.com>
    <http://sensics.com>
*/

/*
// Copyright 2014 Sensics, Inc.
//
// All rights reserved.
//
// (Final version intended to be licensed under
// the Apache License, Version 2.0)
*/

#ifndef INCLUDED_APIBaseC_h_GUID_C5A2E769_2ADC_429E_D250_DF0883E6E5DB
#define INCLUDED_APIBaseC_h_GUID_C5A2E769_2ADC_429E_D250_DF0883E6E5DB

#ifdef __cplusplus
#define OGVR_C_ONLY(X)
#define OGVR_CPP_ONLY(X) X
#define OGVR_EXTERN_C_BEGIN extern "C" {
#define OGVR_EXTERN_C_END }
#else
#define OGVR_C_ONLY(X) X
#define OGVR_CPP_ONLY(X)
#define OGVR_EXTERN_C_BEGIN
#define OGVR_EXTERN_C_END
#endif

#endif