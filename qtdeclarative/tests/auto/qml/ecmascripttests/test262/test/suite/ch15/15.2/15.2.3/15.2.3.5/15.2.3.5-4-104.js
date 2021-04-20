/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.5/15.2.3.5-4-104.js
 * @description Object.create - 'configurable' property of one property in 'Properties' is own data property that overrides an inherited accessor property (8.10.5 step 4.a)
 */


function testcase() {

        var proto = {};
        Object.defineProperty(proto, "configurable", {
            get: function () {
                return true;
            }
        });

        var ConstructFun = function () { };
        ConstructFun.prototype = proto;
        var descObj = new ConstructFun();

        Object.defineProperty(descObj, "configurable", {
            value: false
        });

        var newObj = Object.create({}, {
            prop: descObj 
        });
        var result1 = newObj.hasOwnProperty("prop");
        delete newObj.prop;
        var result2 = newObj.hasOwnProperty("prop");

        return result1 === true && result2 === true;
    }
runTestCase(testcase);
