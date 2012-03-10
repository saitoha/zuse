
var result = [];

with (1)
{
	__debug.print(toString() === '1');
}

var a = 123
with (a)
{
	result.push( {
		title: 'With statement'
	    , result: [
			toString() === '123'
			, typeof this === 'number'
	    ]
	} )
	debug;
	this.aaa=123;
}
result.push( {
	title: 'With statement 2'
    , result: [
		typeof a.debug === 'undefined'
    ]
} )

print_result(result);

/*
//[0xc6,0x42,0x01,0x57]
with( {
	ret: 0x03
} )
{
	__ar.push([
		0xc6,0x42,0x00,0x99,
		0xc6,0x42,0x01,0x57,
		0xc6,0x42,0x02,0x57,
		0xc6,0x42,0x03,0x57,
		0xc6,0x42,0x04,0x00,
		0xc6,0x42,0x04,0x00,
		0xc3,
	]);
}
[]
//__ar.execute()*/
