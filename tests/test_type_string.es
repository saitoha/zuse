
var result = [];

//////////////////////////////////////////////////////////////////////////////
//
// string constructor
//
result.push( {
    title: '7-1-1. properties of string constructor'
    , result: [
		String.fromCharCode(88, 89, 90) == 'XYZ'
		, String.fromCharCode() == ''
		, String.fromCharCode(88, 0, 90) != 'X'
		, String.fromCharCode(88, 0, 90).length == 3
    ]
});

result.push({
    title: '7-1-2. string constructor as function'
    , result: [
		String("XYZ") === 'XYZ'
		, String(123) === "123"
    ]
} );

result.push({
    title: '7-1-3. construct new string object'
    , result: [
		new String('XYZ') == 'XYZ'
    ]
});

result.push({
    title: '7-2-1. method: charAt(pos)'
    , result: [
		"abcde".charAt(1) == 'b'
		, "abcde".charAt(0) == 'a'
		, "abcde".charAt(NaN) == 'a'
		, "abcde".charAt(-1) == ''
		, "abcde".charAt(6) == ''
		, "".charAt(0) == ''
		, "".charAt(-10) == ''
    ]
});

result.push({
    title: '7-2-2. method: charCodeAt(pos)'
    , result: [
		"abcde".charCodeAt(1) == 98
		, "abcde".charCodeAt(0) == 97
		, "abcde".charCodeAt(NaN) == 97
		, isNaN("abcde".charCodeAt(-1))
		, isNaN("abcde".charCodeAt(6))
		, isNaN("".charCodeAt(0))
		, isNaN("".charCodeAt(-10))
		, "??".charCodeAt(1) == 23383
    ]
});

result.push({
    title: '7-2-3. method: concat([string1[, string2[, ...]]])'
    , result: [
		"a".concat("b") == "ab"
		, "a".concat("b").concat("c") == "abc"
		, "a".concat("b", "c", "d") == "abcd"
		, "".concat("b", "c", "d") == "bcd"
		, "a".concat("", "", "") == "a"
		, "a".concat() == "a"
    ]
});

result.push({
    title: '7-2-4. method: indexOf(searchString, position)'
    , result: [
		"abcde".indexOf("bcd") == 1
		, "abcde".indexOf("bcd", 4) == -1
		, "abcde".indexOf("b", 7) == -1
		, "abcde".indexOf("a", -50) == -1
		, "abcde".indexOf("a", NaN) == 0
		, "abcde".indexOf("", 3) == 3
    ]
});

result.push({
    title: '7-2-5. method: lastIndexOf(searchString, position)'
    , result: [
		"abcde".lastIndexOf("bcd") == 1
		, "abcde".lastIndexOf("bcd", 4) == 1
		, "abcde".lastIndexOf("b", 7) == 1
		, "abcde".lastIndexOf("a", -50) == 0
		, "abcde".lastIndexOf("a", NaN) == 0
		, "abcde".lastIndexOf("", 3) == 3
    ]
});

result.push({
title: '7-2-6. method: localeCompare(that)'
    , result: [
		"abcde".localeCompare("") == 1
		, "abcde".localeCompare("abcde") == 0
		, "abcde".localeCompare("abcdefg") == -1
		, "?".localeCompare("?") == -1
    ]
});

result.push({
    title: '7-2-7. method: match(regexp)'
    , result: [
		"abcde".match(/bcd/) == "bcd"
		, "abcde".match("bcd") == "bcd"
		, "aaaaa".match(/a/g) == 'a,a,a,a,a'

    ]
});

result.push({
    title: '7-2-8. method: replace(searchString, replaceString)'
    , result: [
		"abcde".replace("bcd", "---") == "a---e"
		, "abcde".replace("bcd") == "aundefinede"
		, "asbcdde".replace("d", "-") == "asbc-de"
		, "asbcdde".replace("d", function() { return '-'; }) == "asbc-de"
		, "asbcdde".replace(/(asb|d)/, "-") == "-cdde"
		, "asbcdde".replace(/(asb|d)/g, "-") == "-c--e"
		, "asbcdde".replace(/d/g, function() { return '-'; }) == "asbc--e"
		, function(i) {
            return "aaaaa".replace(/a/g, function() { return ++i })
		} (0) == '12345'
    ]
});

result.push({
    title: '7-2-9. method: search(regexp)'
    , result: [
		"abcde".search(/bcd/) == 1
		, "abcde".search("b*") == 1
		, "aaaaa".search(/a/g) == 0
		, "aaaaab".search(/b/) == 5
    ]
});

result.push({
    title: '7-2-10. method: slice(start, end)'
    , result: [
		"abcde".slice(1, 3) === 'bc'
		, "abcde".slice("1", "3") === 'bc'
		, "abcde".slice("1", "a") === ''
		, "abcde".slice(0) === 'abcde'
		, "abcde".slice(-1) === 'e'
		, "abcde".slice(0, 99) === 'abcde'
		, "".slice.apply("abcde", [1, 3]) === 'bc'
		, String.prototype.slice.apply("abcde", [1, 3]) === 'bc'
		, String.prototype.slice.length == 2
    ]
});

result.push({
    title: '7-2-11. method: split(separator, limit)'
    , result: [
		"abcabca".split("ab") == ",c,ca"
		, "abcabcab".split("ab") == ',c,c,'
		, String.prototype.split.length === 2
    ]
});

result.push({
    title: '7-2-12. method: substring(start, end)'
    , result: [
		"abcde".substring(1, 3) === 'bc'
		, "abcde".substring("1", "3") === 'bc'
		, "abcde".substring("1", "a") === 'a'
		, "abcde".substring(0) === 'abcde'
		, "abcde".substring(-1) === 'abcde'
		, "abcde".substring(0, 99) === 'abcde'
		, "".substring.apply("abcde", [1, 3]) === 'bc'
		, String.prototype.substring.apply("abcde", [1, 3]) === 'bc'
		, String.prototype.substring.length == 2
    ]
});

__debug.print("abcde".match("bcd"))

print_result(result);