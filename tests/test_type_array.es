
var result = [];

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

//////////////////////////////////////////////////////////////////////////////
//
// 2-3  Array Prototype
//

Array.prototype.abc = 'uu';
Array.prototype.xyz = 123;

result.push( {
    title: '2-3. Array Prototype'
    , result: [
        [].abc == 'uu'
        , (new Array()).xyz == 123
        , (new Array).abc + [].xyz == 'uu123'
    ]
} )

print_result(result)