
var result = [];

var acc = '';
for (a in {a:444, b:545, c:455})
	acc += a;

result.push( {
    title: '9-1. for-instatement'
    , result: [
        acc == 'abc'
    ]
} )

acc = '';
s= ['4', 'df', 434];
s.df='fdf';
s.gg='fggg';
s.gg='dfaf';
for (var a in s) {
    if (s[a] == 'fdf') break;
	acc += s[a];
}

result.push( {
    title: '9-2. for-instatement 2'
    , result: [
        acc == '4df434'
    ]
} )
__debug.print("ok 2 : ");

acc = '';
for (var a in s) {
    if (s[a] == 'fdf') continue;
	acc += s[a];
}
result.push( {
    title: '9-3. for-instatement 3'
    , result: [
        acc == '4df434dfaf'
    ]
} )

print_result(result);


