//#!/usr/local/bin/
/*****************************************************************************
*
*  @file   test_start.es
*  @brief  test cases of ecmascript runtime environment
*
****************************************************************************/

//print = __debug.print;
if (typeof alert == 'undefined') {
    alert = __debug.alert;
}
if (typeof echo == 'undefined') {
    print = __debug.print;
}

//debugger;
function print_result(result_)
{
    for (var i = 0; i < result_.length; ++i)
        ( function(o) {
            print(
                o.title
                + '.....'
                + (
                    function(r) {
                        if (typeof r == 'boolean')
                            return r;
                        else if (typeof r != 'array')
                            return false;
                        else
                            for (var j = 0; j < r.length; ++j)
                                if (r[j] == false)
                                    return false;
                        return true;
                    } (o.result) ?
                        (__debug.green(), 'success')
                        : (__debug.red(), 'fail')
                )
            )
        })(result_[i]);
    __debug.white()
}
//\<\< push\(eax\):b*\n:b*\<\< push\(ebx\):b*\n:b*\<\< call\(function_cast\(\&es_machine\<IPrimitive\>\:\:push\)\):b*\n
var execfile = function(testfile) {
    eval(__debug.readtextfile(__sys.abspath(testfile)));
}

//execfile('test0.es');
execfile('test1.es');
execfile('test_type_string.es');
execfile('test_type_array.es');
execfile('test_type_function.es');
execfile('test_type_regexp.es');
execfile('test_statement_throw_try_catch_finally.es');
execfile('test_statement_continue_break.es');
execfile('test_statement_switch_case_default.es');
//execfile('test_statement_for_in.es');

execfile('test_statement_with.es');
execfile('test_statement_label.es');

//execfile('tools.es');

//verbose(1);
//execfile('test_expression_callexpression.es');

//var fs = new ActiveXObject('Scripting.FileSystemObject');
//alert(fs)
//alert(fs.GetAbsolutePathName("./abc"))
/*

a="abc"
print("".charAt.apply(a, [1]))
*/


//var path = fs.BwiuildPath('robots.txt', 'c:/');
//stream = fs.OpenTextFile('c:/robots.txt');
//__debug.print(fs.GetAbsolutePathName("./abc"))

//function(){}
//__debug.alert(stream.ReadAll())
//__debug.print(stream.ReadAll());

//__debug.alert(null)
//verbose(true);
//execfile('test_statement_switch_case_default.es')

//a.b.c.d().d["e"].f();*/
