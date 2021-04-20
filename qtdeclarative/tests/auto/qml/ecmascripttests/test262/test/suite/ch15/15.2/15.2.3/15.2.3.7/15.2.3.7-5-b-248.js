/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.7/15.2.3.7-5-b-248.js
 * @description Object.defineProperties - 'descObj' is an Error object which implements its own [[Get]] method to get 'set' property (8.10.5 step 8.a)
 */


function testcase() {

        var data = "data";
        var descObj = new Error("test");
        descObj.description = { value: 11 };
        descObj.message = { value: 11 };
        descObj.name = { value: 11 };

        var setFun = function (value) {
            data = value;
        };
        descObj.prop = {
            set: setFun
        };

        var obj = {};
        Object.defineProperties(obj, descObj);
        obj.prop = "errorData";
        return obj.hasOwnProperty("prop") && data === "errorData";
    }
runTestCase(testcase);
