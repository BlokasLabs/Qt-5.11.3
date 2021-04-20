/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.7/15.2.3.7-6-a-9.js
 * @description Object.defineProperties - 'P' is own accessor property without a get function (8.12.9 step 1 ) 
 */


function testcase() {
        var obj = {};
        Object.defineProperty(obj, "prop", {
            set: function () { },
            configurable: false
        });

        try {
            Object.defineProperties(obj, {
                prop: {
                    get: function () { },
                    configurable: true
                }
            });
            return false;
        } catch (e) {
            return (e instanceof TypeError);
        }
    }
runTestCase(testcase);
