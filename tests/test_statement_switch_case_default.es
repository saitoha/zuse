
var result = [];

var switch_test = function(key)
{
	var acc = '';
	switch (key)
	{
		case acc+='abc':
			acc+='def';

		case acc+='ghi', 'jkl':
			acc+='mno';

		default:
			acc+='pqr';

		case 'stu':
			acc+='vwx';

		case 'yza',acc+='bcd':
			acc+='efg';
	}
	return acc;
}
__debug.print(switch_test('jkl'));
__debug.print(switch_test('jkl123'));
result.push( {
	title: 'Switch - Case - Default'
    , result: [
		switch_test('jkl') === 'abcghimnopqrvwxefg'
		, switch_test('jkl123') === 'abcghibcdpqrvwxefg'
    ]
} )

var switch_test2 = function(key)
{
	var acc = '';
	switch (key)
	{
		case acc+='abc':
			acc+='def';
			break;

		case acc+='ghi', 'jkl':
			acc+='mno';
			break;

		default:
			acc+='pqr';
			break;

		case 'stu':
			acc+='vwx';
			break;

		case 'yza',acc+='bcd':
			acc+='efg';
			break;
	}
	return acc;
}
result.push( {
	title: 'Switch - Case - Default with break'
    , result: [
		switch_test2('abc') === 'abcdef'
		, switch_test2('stu') === 'abcghivwx'
		, switch_test2('stuwww') === 'abcghibcdpqr'
    ]
} )
__debug.print('012')


var switch_test3 = function(key)
{
	var acc = '';
	switch (key)
	{
		case '111':
			acc+='def';
			break;

		case 222:
			acc+='mno';
			break;

		default:
			acc+='pqr';
			break;

		case 333:
			acc+='vwx';
			break;

	}
	return acc;
}
result.push( {
	title: 'Switch - C'
    , result: [
		switch_test3('111') === 'def'
		, switch_test3(222) === 'mno'
		, switch_test3(333) === 'vwx'
    ]
} );



switch (3)
{
    case 1:
    case 3: 
    case 2: b;
    default: c;
    case 1: a;
    case 3:
    case 2: b;
}


var switch_test = function(key) {
    for (i = 0; i < 10;  i++) {
        switch (key) {
        case 2:
        case 1:
            key = 1;
            break;
        case 3:
        case 4:
            key = 2;
            break;
        default:
            key = 3;
            break;

        }
        switch (key) {
            case 2:
            case 1:
                key = 1;
                break;
            case 3:
            case 4:
                key = 2;
                break;
            default:
                key = 3;
                break;

        }
    }
    return key;
}

result.push({
    title: 'Switch - D'
    , result: [
        switch_test(2) === 1
    ]
});

print_result(result);

/*
__debug.print(switch_test('jkl'))
__debug.print(switch_test('jkl') === 'abcghimnopqrvwxefg')
__debug.print(switch_test('jkl123'))
__debug.print(switch_test('jkl123') === 'abcghibcdpqrvwxefg')
*/