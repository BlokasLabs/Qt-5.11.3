/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.22/15.4.4.22-2-3.js
 * @description Array.prototype.reduceRight applied to Array-like object, 'length' is an own data property that overrides an inherited data property
 */


function testcase() {

        var accessed = true;

        function callbackfn(prevVal, curVal, idx, obj) {
            accessed = true;
            return obj.length === 2;
        }

        var proto = { length: 3 };

        var Con = function () { };
        Con.prototype = proto;

        var child = new Con();
        child.length = 2;
        child[0] = 12;
        child[1] = 11;
        child[2] = 9;

        return Array.prototype.reduceRight.call(child, callbackfn) && accessed;
    }
runTestCase(testcase);
