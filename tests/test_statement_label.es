
var result = [];

var a =1
var buffer = [];
buffer.push(1)
aaa: {
    buffer.push(2)
    bbb: {
        while (++a < 3)
            continue aaa;
        buffer.push(3)
    }
}
buffer.push(4)
result.push( {
    title: '10-1. labeled continue statement'
    , result: [
        buffer == '1,2,2,3,4'
    ]
} )
print(buffer)

var buffer = [];
buffer.push(1)
aaa: {
    buffer.push(2)
    bbb: {
        while (true)
            break aaa;
        buffer.push(3)
    }
}
buffer.push(4)
result.push( {
    title: '10-2. labeled break statement'
    , result: [
        buffer == '1,2,4'
    ]
} )
print(buffer)

print_result(result);
