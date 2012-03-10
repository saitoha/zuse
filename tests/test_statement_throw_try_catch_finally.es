
var result1 = [];
var e = undefined;

var r = null;

// try - catch
try {
	throw 'abc';
} catch (e) {
    r = e;
}

result1.push({
title: 'Try - Catch'
    , result: [
		r === 'abc'
    ]
})

var r = null;
var r2 = null;

// try - catch - finally
try {
	throw 'abc';
} catch (e) {
    r = e;
} finally {
    r2 = 'def'
}

result1.push({
    title: 'Try - Catch - Finally'
    , result: [
		r === 'abc'
		, r2 === 'def'
    ]
})

var r = null;
var r1 = null;

// try - finally
try {
	try {
		throw 'abc'
    } finally {
		r = 'def';
	}
} catch (e) {
    r1 = e;
}

result1.push({
    title: 'Try - Finally'
    , result: [
		r === 'def'
		, r1 === 'abc'
    ]
})

print_result(result1);
