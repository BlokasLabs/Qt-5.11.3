/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.14/15.2.3.14-5-2.js
 * @description Object.keys - own enumerable accessor property of 'O' is defined in returned array
 */


function testcase() {
        var obj = { };

        Object.defineProperty(obj, "prop", {
            get: function () {
                return 1003;
            },
            enumerable: true,
            configurable: true
        });

        var arr = Object.keys(obj);

        return arr.hasOwnProperty(0) && arr[0] === "prop";
    }
runTestCase(testcase);
