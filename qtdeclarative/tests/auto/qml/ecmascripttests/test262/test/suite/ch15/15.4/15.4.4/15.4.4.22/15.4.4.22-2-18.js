/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.22/15.4.4.22-2-18.js
 * @description Array.prototype.reduceRight applied to String object, which implements its own property get method
 */


function testcase() {

        var accessed = false;
        var str = new String("432");

        function callbackfn(preVal, curVal, idx, obj) {
            accessed = true;
            return obj.length === 3;
        }

        try {
            String.prototype[3] = "1";
            return Array.prototype.reduceRight.call(str, callbackfn, 111) && accessed;
        } finally {
            delete String.prototype[3];
        }
    }
runTestCase(testcase);
