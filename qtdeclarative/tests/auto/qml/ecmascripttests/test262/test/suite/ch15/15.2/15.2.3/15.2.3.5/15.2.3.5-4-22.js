/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.5/15.2.3.5-4-22.js
 * @description Object.create -  own enumerable data property that overrides an enumerable inherited data property in 'Properties' is defined in 'obj' (15.2.3.7 step 5.a)
 */


function testcase() {

        var proto = {};
        proto.prop = {
            value: "abc"
        };

        var ConstructFun = function () { };
        ConstructFun.prototype = proto;

        var child = new ConstructFun();
        child.prop = {
            value: "bbq"
        };
        var newObj = Object.create({}, child);

        return newObj.hasOwnProperty("prop") && newObj.prop === "bbq";
    }
runTestCase(testcase);
