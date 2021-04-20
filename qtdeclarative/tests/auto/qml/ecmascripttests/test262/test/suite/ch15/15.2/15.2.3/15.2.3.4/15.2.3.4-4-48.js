/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.4/15.2.3.4-4-48.js
 * @description Object.getOwnPropertyNames - own accessor property of Array object 'O' is pushed into the returned array.
 */


function testcase() {
        var arr = [0, 1, 2];

        Object.defineProperty(arr, "ownProperty", {
            get: function () {
                return "ownArray";
            },
            configurable: true
        });

        var result = Object.getOwnPropertyNames(arr);

        for (var p in result) {
            if (result[p] === "ownProperty") {
                return true;
            }
        }

        return false;
    }
runTestCase(testcase);
