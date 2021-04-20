// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/**
 * FunctionDeclaration within a "do-while" Block in strict code is not
 * allowed
 *
 * @path bestPractice/Sbp_A3_T1.js
 * @description Declaring function within a "do-while" loop
 * @onlyStrict
 * @negative SyntaxError
 * @bestPractice http://wiki.ecmascript.org/doku.php?id=conventions:no_non_standard_strict_decls
 */

"use strict";
do {
    function __func(){};
} while(0);

