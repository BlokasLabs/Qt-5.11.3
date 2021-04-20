/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.15/15.4.4.15-2-6.js
 * @description Array.prototype.lastIndexOf - 'length' is an inherited data property on an Array-like object
 */


function testcase() {

        var proto = { length: 2 };

        var Con = function () {};
        Con.prototype = proto;

        var child = new Con();
        child[1] = "x";
        child[2] = "y";

        return Array.prototype.lastIndexOf.call(child, "x") === 1 &&
             Array.prototype.lastIndexOf.call(child, "y") === -1;
    }
runTestCase(testcase);
