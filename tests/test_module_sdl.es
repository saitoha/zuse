var result1 = [];

//////////////////////////////////////////////////////////////////////////////
//
// Function Prototype Chain
//
var a = (function() {
	this.xyz = 456;
})
var b = (function(arg) {
	this.pqr = arg;
})
var c = new a;
a.prototype.abc = 123;
var d = new b('zzz');
var e = new a;
e.xyz = 789;

result1.push( {
    title: 'Function Prototype Chain'
    , result: [
		a.abc == undefined
		, c.abc + c.xyz == 123 + 456
		, d.abc == undefined
		, d.xyz == undefined
		, d.pqr == 'zzz'
		, e.abc + e.xyz == 123 + 789
    ]
} )
alert(d.pqr)

print_result(result1);
