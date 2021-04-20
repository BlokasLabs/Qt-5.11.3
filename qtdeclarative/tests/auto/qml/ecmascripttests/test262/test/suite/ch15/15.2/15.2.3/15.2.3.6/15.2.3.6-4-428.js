/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.6/15.2.3.6-4-428.js
 * @description ES5 Attributes - success to update [[Enumerable]] attribute of accessor property ([[Get]] is undefined, [[Set]] is undefined, [[Enumerable]] is true, [[Configurable]] is true) to different value
 */


function testcase() {
        var obj = {};

        Object.defineProperty(obj, "prop", {
            get: undefined,
            set: undefined,
            enumerable: true,
            configurable: true
        });
        var result1 = false;
        var desc1 = Object.getOwnPropertyDescriptor(obj, "prop");
        for (var p1 in obj) {
            if (p1 === "prop") {
                result1 = true;
            }
        }

        Object.defineProperty(obj, "prop", {
            enumerable: false
        });
        var result2 = false;
        var desc2 = Object.getOwnPropertyDescriptor(obj, "prop");
        for (var p2 in obj) {
            if (p2 === "prop") {
                result2 = true;
            }
        }

        return result1 && !result2 && desc1.enumerable === true && desc2.enumerable === false;
    }
runTestCase(testcase);
