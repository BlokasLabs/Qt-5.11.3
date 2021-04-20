/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.7/15.2.3.7-6-a-58.js
 * @description Object.defineProperties - desc.[[Get]] and P.[[Get]] are two objects which refer to the different objects (8.12.9 step 6)
 */


function testcase() {

        var obj = {};

        function get_Func1() {
            return 10;
        }

        Object.defineProperty(obj, "foo", {
            get: get_Func1,
            configurable: true
        });

        function get_Func2() {
            return 20;
        }

        Object.defineProperties(obj, {
            foo: {
                get: get_Func2
            }
        });

        var verifyEnumerable = false;
        for (var p in obj) {
            if (p === "foo") {
                verifyEnumerable = true;
            }
        }

        var verifyValue = false;
        verifyValue = (obj.foo === 20);

        var desc = Object.getOwnPropertyDescriptor(obj, "foo");

        var verifyConfigurable = false;
        delete obj.foo;
        verifyConfigurable = obj.hasOwnProperty("foo");

        return !verifyConfigurable && !verifyEnumerable && verifyValue && typeof (desc.set) === "undefined" && desc.get === get_Func2;
    }
runTestCase(testcase);
