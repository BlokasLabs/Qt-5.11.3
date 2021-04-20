/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.5/15.2.3.5-4-165.js
 * @description Object.create - one property in 'Properties' is a Function object which implements its own [[Get]] method to access the 'value' property (8.10.5 step 5.a)
 */


function testcase() {

        var Func = function (a, b) {
            return a + b;
        };

        var fun = new Func();
        fun.value = "FunValue";

        var newObj = Object.create({}, {
            prop: fun
        });
        return newObj.prop === "FunValue";
    }
runTestCase(testcase);
