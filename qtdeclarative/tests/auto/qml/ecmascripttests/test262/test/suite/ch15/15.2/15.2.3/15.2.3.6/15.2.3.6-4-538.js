/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.6/15.2.3.6-4-538.js
 * @description ES5 Attributes - success to update the accessor property ([[Get]] is a Function, [[Set]] is a Function, [[Enumerable]] is true, [[Configurable]] is true) to a data property
 */


function testcase() {
        var obj = {};

        var getFunc = function () {
            return 1001;
        };

        var verifySetFunc = "data";
        var setFunc = function (value) {
            verifySetFunc = value;
        };

        Object.defineProperty(obj, "prop", {
            get: getFunc,
            set: setFunc,
            enumerable: true,
            configurable: true
        });
        var desc1 = Object.getOwnPropertyDescriptor(obj, "prop");

        Object.defineProperty(obj, "prop", {
            value: 1001
        });
        var desc2 = Object.getOwnPropertyDescriptor(obj, "prop");

        return desc1.hasOwnProperty("get") && desc2.hasOwnProperty("value") &&
            typeof desc2.get === "undefined" && typeof desc2.get === "undefined" &&
            dataPropertyAttributesAreCorrect(obj, "prop", 1001, false, true, true);
    }
runTestCase(testcase);
