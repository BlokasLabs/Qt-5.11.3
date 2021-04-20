/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.7/15.2.3.7-6-a-203.js
 * @description Object.defineProperties - 'O' is an Array, 'P' is an array index named property, 'P' property doesn't exist in 'O', test [[Enumerable]] of 'P' property in 'Attributes' is set as false value if [[Enumerable]] is absent in accessor descriptor 'desc'  (15.4.5.1 step 4.c)
 */


function testcase() {
        var arr = [];

        Object.defineProperties(arr, {
            "0": {
                set: function () { },
                get: function () { },
                configurable: true
            }
        });

        for (var i in arr) {
            if (i === "0" && arr.hasOwnProperty("0")) {
                return false;
            }
        }
        return true;
    }
runTestCase(testcase);
