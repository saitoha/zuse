
var result = [];

var acc = '';
for (var i = 0; i < 10; ++i)
{
    if (i == 5) 
	    continue;
	if (i == 8) 
	    break;
	acc += i;
}

result.push({
    title: '14-1. continue and break'
    , result: [
        acc === '0123467'
    ]
})

print_result(result);


