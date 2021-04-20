/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.5/15.2.3.5-4-269.js
 * @description Object.create - 'set' property of one property in 'Properties' is an inherited data property (8.10.5 step 8.a)
 */


function testcase() {
        var data = "data";
        var proto = {
            set: function (value) {
                data = value;
            }
        };

        var ConstructFun = function () { };
        ConstructFun.prototype = proto;
        var child = new ConstructFun();

        var newObj = Object.create({}, {
            prop: child 
        });

        var hasProperty = newObj.hasOwnProperty("prop");

        newObj.prop = "overrideData";

        return hasProperty && data === "overrideData";
    }
runTestCase(testcase);
