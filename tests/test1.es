
/*****************************************************************************
 *
 *  @file   test.es
 *  @brief  test cases of ecmascript runtime environment
 *
 ****************************************************************************/
//debug.alert(Infinity);
//debug.alert('ok');
var result = [];
/*
 *  case 1-1
 *  assignment expression
 *
 */
var a = 1;
var b = a;
var c = 1.001;
var d = 3.3E2;
var e = 4.5E-3;
var f = 0x1234;

result.push( {
    title: '1-1. assignment expression'
    , result: [
        a == 1
        && b == 1
        && c == 1001E-3
        && d == 330
        && e == 0.0045
        && f == 4660
    ]
} );

print ("--1");


/*
 *  case 1-2
 *  NaN and Infinity
 */

var a = 3 / 0;
var b = -94E-2 / 0;
var c = NaN;

result.push( {
    title: '1-2. NaN and Infinity'
    , result: [
        a == Infinity
        && b == -Infinity
        && isNaN( c )
    ]
} );

print ("--2");
/*
 *  case 1-3
 *  four arithmetic operations
 */
var a = 3 + 1 * 4 - 9 / 3;
var b = ( ( 1 + 3 ) / 2 - ( 4 ) * ( ( ( 3 ) ) ) / 2 ) * 1.2;

result.push( {
    title: '1-3. four arithmatic operations'
    , result: [
        a == 4
        , b == -4.8
    ]
} );

print ("--3");
/*
 *  case 1-4
 *  EqualityExpression
 */

var a = 40;
var b = 40;

result.push( {
    title: '1-4. EqualityExpression'
    , result: [
        a == b
        , a == b == true
        , a == b + 1 == false
        , a != b == false
    ]
} )

/*
 *  case 1-5
 *  PostfixExpression
 */

var a = 10;
var b = 10;
a++;
b --;

result.push( {
    title: '1-5. PostfixExpression'
    , result: [
        a == 11
        , a++ == 11
        , a == 12
        , a++ == 12
        , b == 9
        , b-- == 9
        , b == 8
        , b-- == 8
    ]
} )

/*
 *  case 1-6
 *  UnaryExpression - Increment, Decrement
 */

var a = 10;
var b = 10;
++a;
-- b;

result.push( {
    title: '1-6. UnaryExpression - Increment, Decrement'
    , result: [
        a == 11
        , --a == 10
        , a == 10
        , ++a == 11
        , b == 9
        , --b == 8
        , b == 8
        , --b == 7
    ]
} )

/*
 *  case 1-10
 *  RelationalExpression
 */

var a = 10;
var b = -20;
result.push( {
    title: '1-10. RelationalExpression'
    , result: [
        a > b
        , a > b == true
        , a < b == false
        , a + b > - a == false
        , a + b >= -a
        , a <= b == false
        , a * 2 <= -b
    ]
} )

/****************************************************************************
 *
 * category 2 Array
 *
 ****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
//
// 2-1  Array Literal
//

var a = [];
var b = [ 1, 2, 3 ]
var c = [ "abc", "def", "ghi" ]
var d = [ a, b, c, [ a, b, c ], [ a.length ] ];

result.push( {
    title: '2-1. Array Literal'
    , result: [
        a.length == 0
        , b.length == 3
        , b[1] + b[2] == 5
        , c[ b[0] - 1 ] + c[ b[1] ] == "abcghi"
        , d[2][1] == 'def'
        , d[3][2][0] == c[0]
        , d[4][0] == 0
        , d[3][2].length == 3
    ]
} )

//////////////////////////////////////////////////////////////////////////////
//
// 2-2  Array Constructor
//

var a = new Array;
var b = new Array();
var c = new Array(10);
var d = new Array('a');
var e = new Array('abc', 123, {abc: 123});

result.push( {
    title: '2-2. Array Constructor'
    , result: [
        a.length == 0
        , a == a
        , a != c
        , c.length == 10
        , d[0] == 'a'
        , e[0] == 'abc'
        , e[1] == e[2][e[0]]
    ]
} )

/****************************************************************************
 *
 * category 5 statement
 *
 ****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
//
// 5-2  if statement
//

/*
 *  case 5-2-1
 *  if statement - [ if ( ... ) ~ ]
 */

var a = '';
var b = '';
var c = '';

 if ( 1 + 2 == 3 )
 {
    a = 'ok';
 }

 if ( 1 + 2 == 3 ) b = 'ok';

 if ( 'a' + 'bc' == 'abc' )
     if ( 'abc' + 123 == 'abc123' )
         c = 'ok';

result.push( {
    title: '5-2-1. if statement - [ if ( ... ) ~ ]'
    , result: [
        a == 'ok'
        , b == 'ok'
        , c == 'ok'
    ]
} );

/*
 *  case 5-2-2
 *  if/else statement - [ if ( ... ) ~ else ~ ]
 */

var a = '';
var b = '';
var c = '';
var d = '';

 if ( 123 == false )
 {
    a = 'equal';
 }
 else
 {
    b = 'not equal';
 }

 if ( true != false )
    c = 'equal';
 else
    d = 'not equal';

result.push( {
    title: '5-2-2. if/else statement - [ if ( ... ) ~ else ~ ]'
    , result: [
        a == ''
        , b == 'not equal'
        , c == 'equal'
        , d == ''
    ]
} );

//////////////////////////////////////////////////////////////////////////////
//
// 5-3  iteration statement
//

/*
 *  case 5-3-1
 *  iteration statement - [ while ( ... ) ~ ]
 */
var repeat_count = 1000;
var i = repeat_count + 0;
var j = 0;
--i;
++j;
print((j < i) + "--4")
while (j < i)
{
    --i;
    ++j;
}

result.push ( {
    title: '5-3-1. iteration statement - [ while ( ... ) ~ ]'
    , result: [
        i == repeat_count / 2
        , j == repeat_count / 2
    ]
} );

/*
*  case 5-3-2
*  iteration statement - [ do ~ while ]
*/
var repeat_count = 1000;
var i = repeat_count + 0;
var j = 0;

do
{
    --i;
    ++j;
}
while ( j < i );

result.push ( {
    title: '5-3-2. iteration statement - [ do ~ while ]'
    , result: [
        i == repeat_count / 2
        , j == repeat_count / 2
    ]
} );

/****************************************************************************
 *
 * category 6 Function Definition
 *
 ****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
//
// 6-1  Function Declaration
//

/*
 *  case 6-1-1
 *  function with no argument
 *
 */
function fun1() {
    return("ok");
}
result.push( {
    title: '6-1-1. function with no argument'
    , result: fun1() == 'ok'
} );

/*
 *  case 6-1-2
 *  function with 1 argument
 */
function fun2( arg1 ) {
    return arg1;
}
result.push( {
    title: '6-1-2. function with 1 argument'
    , result: fun2("ok") == "ok"
} );

/*
 *  case 6-1-3
 *  function with 2 argument
 */
function fun3( arg1, arg2 ) {
    return arg1 + arg2;
}
result.push( {
    title: '6-1-3. function with 2 argument '
    , result: fun3("ok5", "--") == "ok5--"
} );

/*
 *  case 6-1-4
 *  function with 3 argument
 */
function fun4( arg1, arg2, arg3 ) {
    return arg1 + arg2 + arg3;
}
result.push( {
    title: '6-1-4. function with 3 argument'
    , result: fun3(fun2("-"), fun4("a", "b", "c")) == '-abc'
} );

//////////////////////////////////////////////////////////////////////////////
//
// 6-2  Scope
//

print_result(result);
print(i);




