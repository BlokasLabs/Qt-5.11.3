// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/**
 * Function declaration within an "if" statement in strict code is not
 * allowed
 *
 * @path bestPractice/Sbp_A2_T1.js
 * @description Declaring function within a strict "if" statement
 * @onlyStrict
 * @negative SyntaxError
 * @bestPractice http://wiki.ecmascript.org/doku.php?id=conventions:no_non_standard_strict_decls
 */

"use strict";
if (true) {
    function __func(){};
} else {
    function __func(){};
}

