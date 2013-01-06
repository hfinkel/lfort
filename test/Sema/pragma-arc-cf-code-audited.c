// RUN: %lfort_cc1 -fsyntax-only -verify %s

#pragma lfort arc_cf_code_audited foo // expected-error {{expected 'begin' or 'end'}}

#pragma lfort arc_cf_code_audited begin foo // expected-warning {{extra tokens at end of #pragma directive}}

#pragma lfort arc_cf_code_audited end
#pragma lfort arc_cf_code_audited end // expected-error {{not currently inside '#pragma lfort arc_cf_code_audited'}}

#pragma lfort arc_cf_code_audited begin // expected-note {{#pragma entered here}}
#pragma lfort arc_cf_code_audited begin // expected-error {{already inside '#pragma lfort arc_cf_code_audited'}} expected-note {{#pragma entered here}}

#include "Inputs/pragma-arc-cf-code-audited.h" // expected-error {{cannot #include files inside '#pragma lfort arc_cf_code_audited'}}

// This is actually on the #pragma line in the header.
// expected-error {{'#pragma lfort arc_cf_code_audited' was not ended within this file}}

#pragma lfort arc_cf_code_audited begin // expected-error {{'#pragma lfort arc_cf_code_audited' was not ended within this file}}
