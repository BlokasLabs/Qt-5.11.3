/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.6/15.2.3.6-4-440.js
 * @description ES5 Attributes - [[Get]] attribute of accessor property ([[Get]] is undefined, [[Set]] is undefined, [[Enumerable]] is false, [[Configurable]] is true) is undefined
 */


function testcase() {
        var obj = {};

        Object.defineProperty(obj, "prop", {
            get: undefined,
            set: undefined,
            enumerable: false,
            configurable: true
        });

        var propertyDefineCorrect = obj.hasOwnProperty("prop");
        var desc = Object.getOwnPropertyDescriptor(obj, "prop");

        return propertyDefineCorrect && typeof desc.get === "undefined";
    }
runTestCase(testcase);
