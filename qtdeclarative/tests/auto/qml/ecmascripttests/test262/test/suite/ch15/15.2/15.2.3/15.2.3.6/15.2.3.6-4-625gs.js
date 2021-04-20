/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.6/15.2.3.6-4-625gs.js
 * @description Globally declared variable should take precedence over Object.prototype property of the same name
 */

Object.defineProperty(Object.prototype, 
                      "prop", 
                      { value: 1001, writable: false, enumerable: false, configurable: false} 
                      );
var prop = 1002;

if (! (this.hasOwnProperty("prop") && prop === 1002)) {
    throw "this.prop should take precedence over Object.prototype.prop";
}
