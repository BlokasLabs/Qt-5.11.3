/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.6/15.2.3.6-4-583.js
 * @description ES5 Attributes - [[Get]] field of inherited property of [[Prototype]] internal property is correct (Error Instance)
 */


function testcase() {
        var data = "data";
        try {
            Object.defineProperty(Error.prototype, "prop", {
                get: function () {
                    return data;
                },
                set: function (value) {
                    data = value;
                },
                enumerable: true,
                configurable: true
            });
            var errObj = new Error();

            return !errObj.hasOwnProperty("prop") && errObj.prop === "data";
        } finally {
            delete Error.prototype.prop;
        }
    }
runTestCase(testcase);
