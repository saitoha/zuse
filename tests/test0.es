
/*****************************************************************************
*
*  @file   test.es
*  @brief  test cases of ecmascript runtime environment
*
****************************************************************************/

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

result.push(
    a == 1 &&
    b == 1 &&
    c == 1001E-3 &&
    d == 330 &&
    e == 0.0045 &&
    f == 4660
    );

/*
*  case 1-2
*  NaN and Infinity
*/

a = 3 / 0;
b = -94E-2 / 0;
c = NaN;

result.push(
    a == Infinity &&
    b == -Infinity &&
    isNaN(c)
);


/*
*  case 1-3
*  four arithmetic operations
*/
a = 3 + 1 * 4 - 9 / 3;
b = ((1 + 3) / 2 - (4) * (((3))) / 2) * 1.2;

result.push(
    a == 4 &&
    b == -4.8
);

/*
*  case 1-4
*  EqualityExpression
*/

a = 40;
b = 40;
result.push(
    a == b &&
    a == b == true &&
    a == b + 1 == false &&
    a != b == false
);

/*
*  case 1-5
*  PostfixExpression
*/

a = 10;
b = 10;
a++;
b--;

result.push(
    a == 11 &&
    a++ == 11 &&
    a == 12 &&
    a++ == 12 &&
    b == 9 &&
    b-- == 9 &&
    b == 8 &&
    b-- == 8
);

/*
*  case 1-6
*  RelationalExpression
*/

a = 10;
b = -20;

result.push(
    a > b &&
    a > b == true &&
    a < b == false &&
    a + b > -a == false &&
    a + b >= -a &&
    a <= b == false &&
    a * 2 <= -b
)


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

a = '';
b = '';
c = '';

if (1 + 2 == 3) {
    a = 'ok';
}

if (1 + 2 == 3) b = 'ok';

if ('a' + 'bc' == 'abc')
    if ('abc' + 123 == 'abc123')
    c = 'ok';

result.push(
    a == 'ok' &&
    b == 'ok' &&
    c == 'ok'
);

/*
*  case 5-2-2
*  if/else statement - [ if ( ... ) ~ else ~ ]
*/

a = '';
b = '';
c = '';
d = '';

if (123 == false) {
    a = 'equal';
}
else {
    b = 'not equal';
}

if (true != false)
    c = 'equal';
else
    d = 'not equal';

result.push(
    a == '' && b == 'not equal' &&
    c == 'equal' && d == ''
);

//////////////////////////////////////////////////////////////////////////////
//
// 5-3  iteration statement
//

/*
*  case 5-3-1
*  iteration statement - [ while ( ... ) ~ ]
*/
var repeat_count = 1000000;
i = repeat_count + 0;
j = 0;
while (j < i) {
    --i;
    ++j;
}

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/*
*  case 5-3-2
*  iteration statement - [ do ~ while ]
*/
repeat_count = 1000000;
i = repeat_count + 0;
j = 0;

do {
    --i;
    ++j;
}
while (j < i);

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/****************************************************************************
*
* category 6 Function Definition
*
****************************************************************************/



__debug.print(result);

/*****************************************************************************
*
*  @file   test.es
*  @brief  test cases of ecmascript runtime environment
*
****************************************************************************/

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

result.push(
    a == 1 &&
    b == 1 &&
    c == 1001E-3 &&
    d == 330 &&
    e == 0.0045 &&
    f == 4660
    );

/*
*  case 1-2
*  NaN and Infinity
*/

a = 3 / 0;
b = -94E-2 / 0;
c = NaN;

result.push(
    a == Infinity &&
    b == -Infinity &&
    isNaN(c)
);


/*
*  case 1-3
*  four arithmetic operations
*/
a = 3 + 1 * 4 - 9 / 3;
b = ((1 + 3) / 2 - (4) * (((3))) / 2) * 1.2;

result.push(
    a == 4 &&
    b == -4.8
);

/*
*  case 1-4
*  EqualityExpression
*/

a = 40;
b = 40;
result.push(
    a == b &&
    a == b == true &&
    a == b + 1 == false &&
    a != b == false
);

/*
*  case 1-5
*  PostfixExpression
*/

a = 10;
b = 10;
a++;
b--;

result.push(
    a == 11 &&
    a++ == 11 &&
    a == 12 &&
    a++ == 12 &&
    b == 9 &&
    b-- == 9 &&
    b == 8 &&
    b-- == 8
);

/*
*  case 1-6
*  RelationalExpression
*/

a = 10;
b = -20;

result.push(
    a > b &&
    a > b == true &&
    a < b == false &&
    a + b > -a == false &&
    a + b >= -a &&
    a <= b == false &&
    a * 2 <= -b
)


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

a = '';
b = '';
c = '';

if (1 + 2 == 3) {
    a = 'ok';
}

if (1 + 2 == 3) b = 'ok';

if ('a' + 'bc' == 'abc')
    if ('abc' + 123 == 'abc123')
    c = 'ok';

result.push(
    a == 'ok' &&
    b == 'ok' &&
    c == 'ok'
);

/*
*  case 5-2-2
*  if/else statement - [ if ( ... ) ~ else ~ ]
*/

a = '';
b = '';
c = '';
d = '';

if (123 == false) {
    a = 'equal';
}
else {
    b = 'not equal';
}

if (true != false)
    c = 'equal';
else
    d = 'not equal';

result.push(
    a == '' && b == 'not equal' &&
    c == 'equal' && d == ''
);

//////////////////////////////////////////////////////////////////////////////
//
// 5-3  iteration statement
//

/*
*  case 5-3-1
*  iteration statement - [ while ( ... ) ~ ]
*/
var repeat_count = 1000;
i = repeat_count + 0;
j = 0;
while (j < i) {
    --i;
    ++j;
}

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/*
*  case 5-3-2
*  iteration statement - [ do ~ while ]
*/
repeat_count = 1000;
i = repeat_count + 0;
j = 0;

do {
    --i;
    ++j;
}
while (j < i);

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/****************************************************************************
*
* category 6 Function Definition
*
****************************************************************************/



__debug.print(result);

/*****************************************************************************
*
*  @file   test.es
*  @brief  test cases of ecmascript runtime environment
*
****************************************************************************/

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

result.push(
    a == 1 &&
    b == 1 &&
    c == 1001E-3 &&
    d == 330 &&
    e == 0.0045 &&
    f == 4660
    );

/*
*  case 1-2
*  NaN and Infinity
*/

a = 3 / 0;
b = -94E-2 / 0;
c = NaN;

result.push(
    a == Infinity &&
    b == -Infinity &&
    isNaN(c)
);


/*
*  case 1-3
*  four arithmetic operations
*/
a = 3 + 1 * 4 - 9 / 3;
b = ((1 + 3) / 2 - (4) * (((3))) / 2) * 1.2;

result.push(
    a == 4 &&
    b == -4.8
);

/*
*  case 1-4
*  EqualityExpression
*/

a = 40;
b = 40;
result.push(
    a == b &&
    a == b == true &&
    a == b + 1 == false &&
    a != b == false
);

/*
*  case 1-5
*  PostfixExpression
*/

a = 10;
b = 10;
a++;
b--;

result.push(
    a == 11 &&
    a++ == 11 &&
    a == 12 &&
    a++ == 12 &&
    b == 9 &&
    b-- == 9 &&
    b == 8 &&
    b-- == 8
);

/*
*  case 1-6
*  RelationalExpression
*/

a = 10;
b = -20;

result.push(
    a > b &&
    a > b == true &&
    a < b == false &&
    a + b > -a == false &&
    a + b >= -a &&
    a <= b == false &&
    a * 2 <= -b
)


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

a = '';
b = '';
c = '';

if (1 + 2 == 3) {
    a = 'ok';
}

if (1 + 2 == 3) b = 'ok';

if ('a' + 'bc' == 'abc')
    if ('abc' + 123 == 'abc123')
    c = 'ok';

result.push(
    a == 'ok' &&
    b == 'ok' &&
    c == 'ok'
);

/*
*  case 5-2-2
*  if/else statement - [ if ( ... ) ~ else ~ ]
*/

a = '';
b = '';
c = '';
d = '';

if (123 == false) {
    a = 'equal';
}
else {
    b = 'not equal';
}

if (true != false)
    c = 'equal';
else
    d = 'not equal';

result.push(
    a == '' && b == 'not equal' &&
    c == 'equal' && d == ''
);

//////////////////////////////////////////////////////////////////////////////
//
// 5-3  iteration statement
//

/*
*  case 5-3-1
*  iteration statement - [ while ( ... ) ~ ]
*/
var repeat_count = 1000;
i = repeat_count + 0;
j = 0;
while (j < i) {
    --i;
    ++j;
}

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/*
*  case 5-3-2
*  iteration statement - [ do ~ while ]
*/
repeat_count = 1000;
i = repeat_count + 0;
j = 0;

do {
    --i;
    ++j;
}
while (j < i);

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/****************************************************************************
*
* category 6 Function Definition
*
****************************************************************************/



__debug.print(result);

/*****************************************************************************
*
*  @file   test.es
*  @brief  test cases of ecmascript runtime environment
*
****************************************************************************/

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

result.push(
    a == 1 &&
    b == 1 &&
    c == 1001E-3 &&
    d == 330 &&
    e == 0.0045 &&
    f == 4660
    );

/*
*  case 1-2
*  NaN and Infinity
*/

a = 3 / 0;
b = -94E-2 / 0;
c = NaN;

result.push(
    a == Infinity &&
    b == -Infinity &&
    isNaN(c)
);


/*
*  case 1-3
*  four arithmetic operations
*/
a = 3 + 1 * 4 - 9 / 3;
b = ((1 + 3) / 2 - (4) * (((3))) / 2) * 1.2;

result.push(
    a == 4 &&
    b == -4.8
);

/*
*  case 1-4
*  EqualityExpression
*/

a = 40;
b = 40;
result.push(
    a == b &&
    a == b == true &&
    a == b + 1 == false &&
    a != b == false
);

/*
*  case 1-5
*  PostfixExpression
*/

a = 10;
b = 10;
a++;
b--;

result.push(
    a == 11 &&
    a++ == 11 &&
    a == 12 &&
    a++ == 12 &&
    b == 9 &&
    b-- == 9 &&
    b == 8 &&
    b-- == 8
);

/*
*  case 1-6
*  RelationalExpression
*/

a = 10;
b = -20;

result.push(
    a > b &&
    a > b == true &&
    a < b == false &&
    a + b > -a == false &&
    a + b >= -a &&
    a <= b == false &&
    a * 2 <= -b
)


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

a = '';
b = '';
c = '';

if (1 + 2 == 3) {
    a = 'ok';
}

if (1 + 2 == 3) b = 'ok';

if ('a' + 'bc' == 'abc')
    if ('abc' + 123 == 'abc123')
    c = 'ok';

result.push(
    a == 'ok' &&
    b == 'ok' &&
    c == 'ok'
);

/*
*  case 5-2-2
*  if/else statement - [ if ( ... ) ~ else ~ ]
*/

a = '';
b = '';
c = '';
d = '';

if (123 == false) {
    a = 'equal';
}
else {
    b = 'not equal';
}

if (true != false)
    c = 'equal';
else
    d = 'not equal';

result.push(
    a == '' && b == 'not equal' &&
    c == 'equal' && d == ''
);

//////////////////////////////////////////////////////////////////////////////
//
// 5-3  iteration statement
//

/*
*  case 5-3-1
*  iteration statement - [ while ( ... ) ~ ]
*/
var repeat_count = 1000;
i = repeat_count + 0;
j = 0;
while (j < i) {
    --i;
    ++j;
}

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/*
*  case 5-3-2
*  iteration statement - [ do ~ while ]
*/
repeat_count = 1000;
i = repeat_count + 0;
j = 0;

do {
    --i;
    ++j;
}
while (j < i);

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/****************************************************************************
*
* category 6 Function Definition
*
****************************************************************************/



__debug.print(result);

/*****************************************************************************
*
*  @file   test.es
*  @brief  test cases of ecmascript runtime environment
*
****************************************************************************/

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

result.push(
    a == 1 &&
    b == 1 &&
    c == 1001E-3 &&
    d == 330 &&
    e == 0.0045 &&
    f == 4660
    );

/*
*  case 1-2
*  NaN and Infinity
*/

a = 3 / 0;
b = -94E-2 / 0;
c = NaN;

result.push(
    a == Infinity &&
    b == -Infinity &&
    isNaN(c)
);


/*
*  case 1-3
*  four arithmetic operations
*/
a = 3 + 1 * 4 - 9 / 3;
b = ((1 + 3) / 2 - (4) * (((3))) / 2) * 1.2;

result.push(
    a == 4 &&
    b == -4.8
);

/*
*  case 1-4
*  EqualityExpression
*/

a = 40;
b = 40;
result.push(
    a == b &&
    a == b == true &&
    a == b + 1 == false &&
    a != b == false
);

/*
*  case 1-5
*  PostfixExpression
*/

a = 10;
b = 10;
a++;
b--;

result.push(
    a == 11 &&
    a++ == 11 &&
    a == 12 &&
    a++ == 12 &&
    b == 9 &&
    b-- == 9 &&
    b == 8 &&
    b-- == 8
);

/*
*  case 1-6
*  RelationalExpression
*/

a = 10;
b = -20;

result.push(
    a > b &&
    a > b == true &&
    a < b == false &&
    a + b > -a == false &&
    a + b >= -a &&
    a <= b == false &&
    a * 2 <= -b
)


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

a = '';
b = '';
c = '';

if (1 + 2 == 3) {
    a = 'ok';
}

if (1 + 2 == 3) b = 'ok';

if ('a' + 'bc' == 'abc')
    if ('abc' + 123 == 'abc123')
    c = 'ok';

result.push(
    a == 'ok' &&
    b == 'ok' &&
    c == 'ok'
);

/*
*  case 5-2-2
*  if/else statement - [ if ( ... ) ~ else ~ ]
*/

a = '';
b = '';
c = '';
d = '';

if (123 == false) {
    a = 'equal';
}
else {
    b = 'not equal';
}

if (true != false)
    c = 'equal';
else
    d = 'not equal';

result.push(
    a == '' && b == 'not equal' &&
    c == 'equal' && d == ''
);

//////////////////////////////////////////////////////////////////////////////
//
// 5-3  iteration statement
//

/*
*  case 5-3-1
*  iteration statement - [ while ( ... ) ~ ]
*/
var repeat_count = 1000;
i = repeat_count + 0;
j = 0;
while (j < i) {
    --i;
    ++j;
}

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/*
*  case 5-3-2
*  iteration statement - [ do ~ while ]
*/
repeat_count = 1000;
i = repeat_count + 0;
j = 0;

do {
    --i;
    ++j;
}
while (j < i);

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/****************************************************************************
*
* category 6 Function Definition
*
****************************************************************************/



__debug.print(result);

/*****************************************************************************
*
*  @file   test.es
*  @brief  test cases of ecmascript runtime environment
*
****************************************************************************/

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

result.push(
    a == 1 &&
    b == 1 &&
    c == 1001E-3 &&
    d == 330 &&
    e == 0.0045 &&
    f == 4660
    );

/*
*  case 1-2
*  NaN and Infinity
*/

a = 3 / 0;
b = -94E-2 / 0;
c = NaN;

result.push(
    a == Infinity &&
    b == -Infinity &&
    isNaN(c)
);


/*
*  case 1-3
*  four arithmetic operations
*/
a = 3 + 1 * 4 - 9 / 3;
b = ((1 + 3) / 2 - (4) * (((3))) / 2) * 1.2;

result.push(
    a == 4 &&
    b == -4.8
);

/*
*  case 1-4
*  EqualityExpression
*/

a = 40;
b = 40;
result.push(
    a == b &&
    a == b == true &&
    a == b + 1 == false &&
    a != b == false
);

/*
*  case 1-5
*  PostfixExpression
*/

a = 10;
b = 10;
a++;
b--;

result.push(
    a == 11 &&
    a++ == 11 &&
    a == 12 &&
    a++ == 12 &&
    b == 9 &&
    b-- == 9 &&
    b == 8 &&
    b-- == 8
);

/*
*  case 1-6
*  RelationalExpression
*/

a = 10;
b = -20;

result.push(
    a > b &&
    a > b == true &&
    a < b == false &&
    a + b > -a == false &&
    a + b >= -a &&
    a <= b == false &&
    a * 2 <= -b
)


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

a = '';
b = '';
c = '';

if (1 + 2 == 3) {
    a = 'ok';
}

if (1 + 2 == 3) b = 'ok';

if ('a' + 'bc' == 'abc')
    if ('abc' + 123 == 'abc123')
    c = 'ok';

result.push(
    a == 'ok' &&
    b == 'ok' &&
    c == 'ok'
);

/*
*  case 5-2-2
*  if/else statement - [ if ( ... ) ~ else ~ ]
*/

a = '';
b = '';
c = '';
d = '';

if (123 == false) {
    a = 'equal';
}
else {
    b = 'not equal';
}

if (true != false)
    c = 'equal';
else
    d = 'not equal';

result.push(
    a == '' && b == 'not equal' &&
    c == 'equal' && d == ''
);

//////////////////////////////////////////////////////////////////////////////
//
// 5-3  iteration statement
//

/*
*  case 5-3-1
*  iteration statement - [ while ( ... ) ~ ]
*/
var repeat_count = 1000;
i = repeat_count + 0;
j = 0;
while (j < i) {
    --i;
    ++j;
}

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/*
*  case 5-3-2
*  iteration statement - [ do ~ while ]
*/
repeat_count = 1000;
i = repeat_count + 0;
j = 0;

do {
    --i;
    ++j;
}
while (j < i);

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/****************************************************************************
*
* category 6 Function Definition
*
****************************************************************************/



__debug.print(result);

/*****************************************************************************
*
*  @file   test.es
*  @brief  test cases of ecmascript runtime environment
*
****************************************************************************/

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

result.push(
    a == 1 &&
    b == 1 &&
    c == 1001E-3 &&
    d == 330 &&
    e == 0.0045 &&
    f == 4660
    );

/*
*  case 1-2
*  NaN and Infinity
*/

a = 3 / 0;
b = -94E-2 / 0;
c = NaN;

result.push(
    a == Infinity &&
    b == -Infinity &&
    isNaN(c)
);


/*
*  case 1-3
*  four arithmetic operations
*/
a = 3 + 1 * 4 - 9 / 3;
b = ((1 + 3) / 2 - (4) * (((3))) / 2) * 1.2;

result.push(
    a == 4 &&
    b == -4.8
);

/*
*  case 1-4
*  EqualityExpression
*/

a = 40;
b = 40;
result.push(
    a == b &&
    a == b == true &&
    a == b + 1 == false &&
    a != b == false
);

/*
*  case 1-5
*  PostfixExpression
*/

a = 10;
b = 10;
a++;
b--;

result.push(
    a == 11 &&
    a++ == 11 &&
    a == 12 &&
    a++ == 12 &&
    b == 9 &&
    b-- == 9 &&
    b == 8 &&
    b-- == 8
);

/*
*  case 1-6
*  RelationalExpression
*/

a = 10;
b = -20;

result.push(
    a > b &&
    a > b == true &&
    a < b == false &&
    a + b > -a == false &&
    a + b >= -a &&
    a <= b == false &&
    a * 2 <= -b
)


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

a = '';
b = '';
c = '';

if (1 + 2 == 3) {
    a = 'ok';
}

if (1 + 2 == 3) b = 'ok';

if ('a' + 'bc' == 'abc')
    if ('abc' + 123 == 'abc123')
    c = 'ok';

result.push(
    a == 'ok' &&
    b == 'ok' &&
    c == 'ok'
);

/*
*  case 5-2-2
*  if/else statement - [ if ( ... ) ~ else ~ ]
*/

a = '';
b = '';
c = '';
d = '';

if (123 == false) {
    a = 'equal';
}
else {
    b = 'not equal';
}

if (true != false)
    c = 'equal';
else
    d = 'not equal';

result.push(
    a == '' && b == 'not equal' &&
    c == 'equal' && d == ''
);

//////////////////////////////////////////////////////////////////////////////
//
// 5-3  iteration statement
//

/*
*  case 5-3-1
*  iteration statement - [ while ( ... ) ~ ]
*/
var repeat_count = 1000;
i = repeat_count + 0;
j = 0;
while (j < i) {
    --i;
    ++j;
}

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/*
*  case 5-3-2
*  iteration statement - [ do ~ while ]
*/
repeat_count = 1000;
i = repeat_count + 0;
j = 0;

do {
    --i;
    ++j;
}
while (j < i);

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/****************************************************************************
*
* category 6 Function Definition
*
****************************************************************************/



__debug.print(result);

/*****************************************************************************
*
*  @file   test.es
*  @brief  test cases of ecmascript runtime environment
*
****************************************************************************/

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

result.push(
    a == 1 &&
    b == 1 &&
    c == 1001E-3 &&
    d == 330 &&
    e == 0.0045 &&
    f == 4660
    );

/*
*  case 1-2
*  NaN and Infinity
*/

a = 3 / 0;
b = -94E-2 / 0;
c = NaN;

result.push(
    a == Infinity &&
    b == -Infinity &&
    isNaN(c)
);


/*
*  case 1-3
*  four arithmetic operations
*/
a = 3 + 1 * 4 - 9 / 3;
b = ((1 + 3) / 2 - (4) * (((3))) / 2) * 1.2;

result.push(
    a == 4 &&
    b == -4.8
);

/*
*  case 1-4
*  EqualityExpression
*/

a = 40;
b = 40;
result.push(
    a == b &&
    a == b == true &&
    a == b + 1 == false &&
    a != b == false
);

/*
*  case 1-5
*  PostfixExpression
*/

a = 10;
b = 10;
a++;
b--;

result.push(
    a == 11 &&
    a++ == 11 &&
    a == 12 &&
    a++ == 12 &&
    b == 9 &&
    b-- == 9 &&
    b == 8 &&
    b-- == 8
);

/*
*  case 1-6
*  RelationalExpression
*/

a = 10;
b = -20;

result.push(
    a > b &&
    a > b == true &&
    a < b == false &&
    a + b > -a == false &&
    a + b >= -a &&
    a <= b == false &&
    a * 2 <= -b
)


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

a = '';
b = '';
c = '';

if (1 + 2 == 3) {
    a = 'ok';
}

if (1 + 2 == 3) b = 'ok';

if ('a' + 'bc' == 'abc')
    if ('abc' + 123 == 'abc123')
    c = 'ok';

result.push(
    a == 'ok' &&
    b == 'ok' &&
    c == 'ok'
);

/*
*  case 5-2-2
*  if/else statement - [ if ( ... ) ~ else ~ ]
*/

a = '';
b = '';
c = '';
d = '';

if (123 == false) {
    a = 'equal';
}
else {
    b = 'not equal';
}

if (true != false)
    c = 'equal';
else
    d = 'not equal';

result.push(
    a == '' && b == 'not equal' &&
    c == 'equal' && d == ''
);

//////////////////////////////////////////////////////////////////////////////
//
// 5-3  iteration statement
//

/*
*  case 5-3-1
*  iteration statement - [ while ( ... ) ~ ]
*/
var repeat_count = 1000;
i = repeat_count + 0;
j = 0;
while (j < i) {
    --i;
    ++j;
}

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/*
*  case 5-3-2
*  iteration statement - [ do ~ while ]
*/
repeat_count = 1000;
i = repeat_count + 0;
j = 0;

do {
    --i;
    ++j;
}
while (j < i);

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/****************************************************************************
*
* category 6 Function Definition
*
****************************************************************************/



__debug.print(result);

/*****************************************************************************
*
*  @file   test.es
*  @brief  test cases of ecmascript runtime environment
*
****************************************************************************/

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

result.push(
    a == 1 &&
    b == 1 &&
    c == 1001E-3 &&
    d == 330 &&
    e == 0.0045 &&
    f == 4660
    );

/*
*  case 1-2
*  NaN and Infinity
*/

a = 3 / 0;
b = -94E-2 / 0;
c = NaN;

result.push(
    a == Infinity &&
    b == -Infinity &&
    isNaN(c)
);


/*
*  case 1-3
*  four arithmetic operations
*/
a = 3 + 1 * 4 - 9 / 3;
b = ((1 + 3) / 2 - (4) * (((3))) / 2) * 1.2;

result.push(
    a == 4 &&
    b == -4.8
);

/*
*  case 1-4
*  EqualityExpression
*/

a = 40;
b = 40;
result.push(
    a == b &&
    a == b == true &&
    a == b + 1 == false &&
    a != b == false
);

/*
*  case 1-5
*  PostfixExpression
*/

a = 10;
b = 10;
a++;
b--;

result.push(
    a == 11 &&
    a++ == 11 &&
    a == 12 &&
    a++ == 12 &&
    b == 9 &&
    b-- == 9 &&
    b == 8 &&
    b-- == 8
);

/*
*  case 1-6
*  RelationalExpression
*/

a = 10;
b = -20;

result.push(
    a > b &&
    a > b == true &&
    a < b == false &&
    a + b > -a == false &&
    a + b >= -a &&
    a <= b == false &&
    a * 2 <= -b
)


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

a = '';
b = '';
c = '';

if (1 + 2 == 3) {
    a = 'ok';
}

if (1 + 2 == 3) b = 'ok';

if ('a' + 'bc' == 'abc')
    if ('abc' + 123 == 'abc123')
    c = 'ok';

result.push(
    a == 'ok' &&
    b == 'ok' &&
    c == 'ok'
);

/*
*  case 5-2-2
*  if/else statement - [ if ( ... ) ~ else ~ ]
*/

a = '';
b = '';
c = '';
d = '';

if (123 == false) {
    a = 'equal';
}
else {
    b = 'not equal';
}

if (true != false)
    c = 'equal';
else
    d = 'not equal';

result.push(
    a == '' && b == 'not equal' &&
    c == 'equal' && d == ''
);

//////////////////////////////////////////////////////////////////////////////
//
// 5-3  iteration statement
//

/*
*  case 5-3-1
*  iteration statement - [ while ( ... ) ~ ]
*/
var repeat_count = 1000;
i = repeat_count + 0;
j = 0;
while (j < i) {
    --i;
    ++j;
}

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/*
*  case 5-3-2
*  iteration statement - [ do ~ while ]
*/
repeat_count = 1000;
i = repeat_count + 0;
j = 0;

do {
    --i;
    ++j;
}
while (j < i);

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/****************************************************************************
*
* category 6 Function Definition
*
****************************************************************************/



__debug.print(result);

/*****************************************************************************
*
*  @file   test.es
*  @brief  test cases of ecmascript runtime environment
*
****************************************************************************/

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

result.push(
    a == 1 &&
    b == 1 &&
    c == 1001E-3 &&
    d == 330 &&
    e == 0.0045 &&
    f == 4660
    );

/*
*  case 1-2
*  NaN and Infinity
*/

a = 3 / 0;
b = -94E-2 / 0;
c = NaN;

result.push(
    a == Infinity &&
    b == -Infinity &&
    isNaN(c)
);


/*
*  case 1-3
*  four arithmetic operations
*/
a = 3 + 1 * 4 - 9 / 3;
b = ((1 + 3) / 2 - (4) * (((3))) / 2) * 1.2;

result.push(
    a == 4 &&
    b == -4.8
);

/*
*  case 1-4
*  EqualityExpression
*/

a = 40;
b = 40;
result.push(
    a == b &&
    a == b == true &&
    a == b + 1 == false &&
    a != b == false
);

/*
*  case 1-5
*  PostfixExpression
*/

a = 10;
b = 10;
a++;
b--;

result.push(
    a == 11 &&
    a++ == 11 &&
    a == 12 &&
    a++ == 12 &&
    b == 9 &&
    b-- == 9 &&
    b == 8 &&
    b-- == 8
);

/*
*  case 1-6
*  RelationalExpression
*/

a = 10;
b = -20;

result.push(
    a > b &&
    a > b == true &&
    a < b == false &&
    a + b > -a == false &&
    a + b >= -a &&
    a <= b == false &&
    a * 2 <= -b
)


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

a = '';
b = '';
c = '';

if (1 + 2 == 3) {
    a = 'ok';
}

if (1 + 2 == 3) b = 'ok';

if ('a' + 'bc' == 'abc')
    if ('abc' + 123 == 'abc123')
    c = 'ok';

result.push(
    a == 'ok' &&
    b == 'ok' &&
    c == 'ok'
);

/*
*  case 5-2-2
*  if/else statement - [ if ( ... ) ~ else ~ ]
*/

a = '';
b = '';
c = '';
d = '';

if (123 == false) {
    a = 'equal';
}
else {
    b = 'not equal';
}

if (true != false)
    c = 'equal';
else
    d = 'not equal';

result.push(
    a == '' && b == 'not equal' &&
    c == 'equal' && d == ''
);

//////////////////////////////////////////////////////////////////////////////
//
// 5-3  iteration statement
//

/*
*  case 5-3-1
*  iteration statement - [ while ( ... ) ~ ]
*/
var repeat_count = 1000;
i = repeat_count + 0;
j = 0;
while (j < i) {
    --i;
    ++j;
}

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/*
*  case 5-3-2
*  iteration statement - [ do ~ while ]
*/
repeat_count = 1000;
i = repeat_count + 0;
j = 0;

do {
    --i;
    ++j;
}
while (j < i);

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/****************************************************************************
*
* category 6 Function Definition
*
****************************************************************************/



__debug.print(result);

/*****************************************************************************
*
*  @file   test.es
*  @brief  test cases of ecmascript runtime environment
*
****************************************************************************/

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

result.push(
    a == 1 &&
    b == 1 &&
    c == 1001E-3 &&
    d == 330 &&
    e == 0.0045 &&
    f == 4660
    );

/*
*  case 1-2
*  NaN and Infinity
*/

a = 3 / 0;
b = -94E-2 / 0;
c = NaN;

result.push(
    a == Infinity &&
    b == -Infinity &&
    isNaN(c)
);


/*
*  case 1-3
*  four arithmetic operations
*/
a = 3 + 1 * 4 - 9 / 3;
b = ((1 + 3) / 2 - (4) * (((3))) / 2) * 1.2;

result.push(
    a == 4 &&
    b == -4.8
);

/*
*  case 1-4
*  EqualityExpression
*/

a = 40;
b = 40;
result.push(
    a == b &&
    a == b == true &&
    a == b + 1 == false &&
    a != b == false
);

/*
*  case 1-5
*  PostfixExpression
*/

a = 10;
b = 10;
a++;
b--;

result.push(
    a == 11 &&
    a++ == 11 &&
    a == 12 &&
    a++ == 12 &&
    b == 9 &&
    b-- == 9 &&
    b == 8 &&
    b-- == 8
);

/*
*  case 1-6
*  RelationalExpression
*/

a = 10;
b = -20;

result.push(
    a > b &&
    a > b == true &&
    a < b == false &&
    a + b > -a == false &&
    a + b >= -a &&
    a <= b == false &&
    a * 2 <= -b
)


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

a = '';
b = '';
c = '';

if (1 + 2 == 3) {
    a = 'ok';
}

if (1 + 2 == 3) b = 'ok';

if ('a' + 'bc' == 'abc')
    if ('abc' + 123 == 'abc123')
    c = 'ok';

result.push(
    a == 'ok' &&
    b == 'ok' &&
    c == 'ok'
);

/*
*  case 5-2-2
*  if/else statement - [ if ( ... ) ~ else ~ ]
*/

a = '';
b = '';
c = '';
d = '';

if (123 == false) {
    a = 'equal';
}
else {
    b = 'not equal';
}

if (true != false)
    c = 'equal';
else
    d = 'not equal';

result.push(
    a == '' && b == 'not equal' &&
    c == 'equal' && d == ''
);

//////////////////////////////////////////////////////////////////////////////
//
// 5-3  iteration statement
//

/*
*  case 5-3-1
*  iteration statement - [ while ( ... ) ~ ]
*/
var repeat_count = 1000;
i = repeat_count + 0;
j = 0;
while (j < i) {
    --i;
    ++j;
}

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/*
*  case 5-3-2
*  iteration statement - [ do ~ while ]
*/
repeat_count = 1000;
i = repeat_count + 0;
j = 0;

do {
    --i;
    ++j;
}
while (j < i);

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/****************************************************************************
*
* category 6 Function Definition
*
****************************************************************************/



__debug.print(result);

/*****************************************************************************
*
*  @file   test.es
*  @brief  test cases of ecmascript runtime environment
*
****************************************************************************/

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

result.push(
    a == 1 &&
    b == 1 &&
    c == 1001E-3 &&
    d == 330 &&
    e == 0.0045 &&
    f == 4660
    );

/*
*  case 1-2
*  NaN and Infinity
*/

a = 3 / 0;
b = -94E-2 / 0;
c = NaN;

result.push(
    a == Infinity &&
    b == -Infinity &&
    isNaN(c)
);


/*
*  case 1-3
*  four arithmetic operations
*/
a = 3 + 1 * 4 - 9 / 3;
b = ((1 + 3) / 2 - (4) * (((3))) / 2) * 1.2;

result.push(
    a == 4 &&
    b == -4.8
);

/*
*  case 1-4
*  EqualityExpression
*/

a = 40;
b = 40;
result.push(
    a == b &&
    a == b == true &&
    a == b + 1 == false &&
    a != b == false
);

/*
*  case 1-5
*  PostfixExpression
*/

a = 10;
b = 10;
a++;
b--;

result.push(
    a == 11 &&
    a++ == 11 &&
    a == 12 &&
    a++ == 12 &&
    b == 9 &&
    b-- == 9 &&
    b == 8 &&
    b-- == 8
);

/*
*  case 1-6
*  RelationalExpression
*/

a = 10;
b = -20;

result.push(
    a > b &&
    a > b == true &&
    a < b == false &&
    a + b > -a == false &&
    a + b >= -a &&
    a <= b == false &&
    a * 2 <= -b
)


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

a = '';
b = '';
c = '';

if (1 + 2 == 3) {
    a = 'ok';
}

if (1 + 2 == 3) b = 'ok';

if ('a' + 'bc' == 'abc')
    if ('abc' + 123 == 'abc123')
    c = 'ok';

result.push(
    a == 'ok' &&
    b == 'ok' &&
    c == 'ok'
);

/*
*  case 5-2-2
*  if/else statement - [ if ( ... ) ~ else ~ ]
*/

a = '';
b = '';
c = '';
d = '';

if (123 == false) {
    a = 'equal';
}
else {
    b = 'not equal';
}

if (true != false)
    c = 'equal';
else
    d = 'not equal';

result.push(
    a == '' && b == 'not equal' &&
    c == 'equal' && d == ''
);

//////////////////////////////////////////////////////////////////////////////
//
// 5-3  iteration statement
//

/*
*  case 5-3-1
*  iteration statement - [ while ( ... ) ~ ]
*/
var repeat_count = 1000;
i = repeat_count + 0;
j = 0;
while (j < i) {
    --i;
    ++j;
}

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/*
*  case 5-3-2
*  iteration statement - [ do ~ while ]
*/
repeat_count = 1000;
i = repeat_count + 0;
j = 0;

do {
    --i;
    ++j;
}
while (j < i);

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/****************************************************************************
*
* category 6 Function Definition
*
****************************************************************************/



__debug.print(result);

/*****************************************************************************
*
*  @file   test.es
*  @brief  test cases of ecmascript runtime environment
*
****************************************************************************/

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

result.push(
    a == 1 &&
    b == 1 &&
    c == 1001E-3 &&
    d == 330 &&
    e == 0.0045 &&
    f == 4660
    );

/*
*  case 1-2
*  NaN and Infinity
*/

a = 3 / 0;
b = -94E-2 / 0;
c = NaN;

result.push(
    a == Infinity &&
    b == -Infinity &&
    isNaN(c)
);


/*
*  case 1-3
*  four arithmetic operations
*/
a = 3 + 1 * 4 - 9 / 3;
b = ((1 + 3) / 2 - (4) * (((3))) / 2) * 1.2;

result.push(
    a == 4 &&
    b == -4.8
);

/*
*  case 1-4
*  EqualityExpression
*/

a = 40;
b = 40;
result.push(
    a == b &&
    a == b == true &&
    a == b + 1 == false &&
    a != b == false
);

/*
*  case 1-5
*  PostfixExpression
*/

a = 10;
b = 10;
a++;
b--;

result.push(
    a == 11 &&
    a++ == 11 &&
    a == 12 &&
    a++ == 12 &&
    b == 9 &&
    b-- == 9 &&
    b == 8 &&
    b-- == 8
);

/*
*  case 1-6
*  RelationalExpression
*/

a = 10;
b = -20;

result.push(
    a > b &&
    a > b == true &&
    a < b == false &&
    a + b > -a == false &&
    a + b >= -a &&
    a <= b == false &&
    a * 2 <= -b
)


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

a = '';
b = '';
c = '';

if (1 + 2 == 3) {
    a = 'ok';
}

if (1 + 2 == 3) b = 'ok';

if ('a' + 'bc' == 'abc')
    if ('abc' + 123 == 'abc123')
    c = 'ok';

result.push(
    a == 'ok' &&
    b == 'ok' &&
    c == 'ok'
);

/*
*  case 5-2-2
*  if/else statement - [ if ( ... ) ~ else ~ ]
*/

a = '';
b = '';
c = '';
d = '';

if (123 == false) {
    a = 'equal';
}
else {
    b = 'not equal';
}

if (true != false)
    c = 'equal';
else
    d = 'not equal';

result.push(
    a == '' && b == 'not equal' &&
    c == 'equal' && d == ''
);

//////////////////////////////////////////////////////////////////////////////
//
// 5-3  iteration statement
//

/*
*  case 5-3-1
*  iteration statement - [ while ( ... ) ~ ]
*/
var repeat_count = 1000;
i = repeat_count + 0;
j = 0;
while (j < i) {
    --i;
    ++j;
}

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/*
*  case 5-3-2
*  iteration statement - [ do ~ while ]
*/
repeat_count = 1000;
i = repeat_count + 0;
j = 0;

do {
    --i;
    ++j;
}
while (j < i);

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/****************************************************************************
*
* category 6 Function Definition
*
****************************************************************************/



__debug.print(result);

/*****************************************************************************
*
*  @file   test.es
*  @brief  test cases of ecmascript runtime environment
*
****************************************************************************/

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

result.push(
    a == 1 &&
    b == 1 &&
    c == 1001E-3 &&
    d == 330 &&
    e == 0.0045 &&
    f == 4660
    );

/*
*  case 1-2
*  NaN and Infinity
*/

a = 3 / 0;
b = -94E-2 / 0;
c = NaN;

result.push(
    a == Infinity &&
    b == -Infinity &&
    isNaN(c)
);


/*
*  case 1-3
*  four arithmetic operations
*/
a = 3 + 1 * 4 - 9 / 3;
b = ((1 + 3) / 2 - (4) * (((3))) / 2) * 1.2;

result.push(
    a == 4 &&
    b == -4.8
);

/*
*  case 1-4
*  EqualityExpression
*/

a = 40;
b = 40;
result.push(
    a == b &&
    a == b == true &&
    a == b + 1 == false &&
    a != b == false
);

/*
*  case 1-5
*  PostfixExpression
*/

a = 10;
b = 10;
a++;
b--;

result.push(
    a == 11 &&
    a++ == 11 &&
    a == 12 &&
    a++ == 12 &&
    b == 9 &&
    b-- == 9 &&
    b == 8 &&
    b-- == 8
);

/*
*  case 1-6
*  RelationalExpression
*/

a = 10;
b = -20;

result.push(
    a > b &&
    a > b == true &&
    a < b == false &&
    a + b > -a == false &&
    a + b >= -a &&
    a <= b == false &&
    a * 2 <= -b
)


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

a = '';
b = '';
c = '';

if (1 + 2 == 3) {
    a = 'ok';
}

if (1 + 2 == 3) b = 'ok';

if ('a' + 'bc' == 'abc')
    if ('abc' + 123 == 'abc123')
    c = 'ok';

result.push(
    a == 'ok' &&
    b == 'ok' &&
    c == 'ok'
);

/*
*  case 5-2-2
*  if/else statement - [ if ( ... ) ~ else ~ ]
*/

a = '';
b = '';
c = '';
d = '';

if (123 == false) {
    a = 'equal';
}
else {
    b = 'not equal';
}

if (true != false)
    c = 'equal';
else
    d = 'not equal';

result.push(
    a == '' && b == 'not equal' &&
    c == 'equal' && d == ''
);

//////////////////////////////////////////////////////////////////////////////
//
// 5-3  iteration statement
//

/*
*  case 5-3-1
*  iteration statement - [ while ( ... ) ~ ]
*/
var repeat_count = 1000;
i = repeat_count + 0;
j = 0;
while (j < i) {
    --i;
    ++j;
}

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/*
*  case 5-3-2
*  iteration statement - [ do ~ while ]
*/
repeat_count = 1000;
i = repeat_count + 0;
j = 0;

do {
    --i;
    ++j;
}
while (j < i);

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/****************************************************************************
*
* category 6 Function Definition
*
****************************************************************************/



__debug.print(result);

/*****************************************************************************
*
*  @file   test.es
*  @brief  test cases of ecmascript runtime environment
*
****************************************************************************/

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

result.push(
    a == 1 &&
    b == 1 &&
    c == 1001E-3 &&
    d == 330 &&
    e == 0.0045 &&
    f == 4660
    );

/*
*  case 1-2
*  NaN and Infinity
*/

a = 3 / 0;
b = -94E-2 / 0;
c = NaN;

result.push(
    a == Infinity &&
    b == -Infinity &&
    isNaN(c)
);


/*
*  case 1-3
*  four arithmetic operations
*/
a = 3 + 1 * 4 - 9 / 3;
b = ((1 + 3) / 2 - (4) * (((3))) / 2) * 1.2;

result.push(
    a == 4 &&
    b == -4.8
);

/*
*  case 1-4
*  EqualityExpression
*/

a = 40;
b = 40;
result.push(
    a == b &&
    a == b == true &&
    a == b + 1 == false &&
    a != b == false
);

/*
*  case 1-5
*  PostfixExpression
*/

a = 10;
b = 10;
a++;
b--;

result.push(
    a == 11 &&
    a++ == 11 &&
    a == 12 &&
    a++ == 12 &&
    b == 9 &&
    b-- == 9 &&
    b == 8 &&
    b-- == 8
);

/*
*  case 1-6
*  RelationalExpression
*/

a = 10;
b = -20;

result.push(
    a > b &&
    a > b == true &&
    a < b == false &&
    a + b > -a == false &&
    a + b >= -a &&
    a <= b == false &&
    a * 2 <= -b
)


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

a = '';
b = '';
c = '';

if (1 + 2 == 3) {
    a = 'ok';
}

if (1 + 2 == 3) b = 'ok';

if ('a' + 'bc' == 'abc')
    if ('abc' + 123 == 'abc123')
    c = 'ok';

result.push(
    a == 'ok' &&
    b == 'ok' &&
    c == 'ok'
);

/*
*  case 5-2-2
*  if/else statement - [ if ( ... ) ~ else ~ ]
*/

a = '';
b = '';
c = '';
d = '';

if (123 == false) {
    a = 'equal';
}
else {
    b = 'not equal';
}

if (true != false)
    c = 'equal';
else
    d = 'not equal';

result.push(
    a == '' && b == 'not equal' &&
    c == 'equal' && d == ''
);

//////////////////////////////////////////////////////////////////////////////
//
// 5-3  iteration statement
//

/*
*  case 5-3-1
*  iteration statement - [ while ( ... ) ~ ]
*/
var repeat_count = 1000;
i = repeat_count + 0;
j = 0;
while (j < i) {
    --i;
    ++j;
}

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/*
*  case 5-3-2
*  iteration statement - [ do ~ while ]
*/
repeat_count = 1000;
i = repeat_count + 0;
j = 0;

do {
    --i;
    ++j;
}
while (j < i);

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/****************************************************************************
*
* category 6 Function Definition
*
****************************************************************************/



__debug.print(result);

/*****************************************************************************
*
*  @file   test.es
*  @brief  test cases of ecmascript runtime environment
*
****************************************************************************/

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

result.push(
    a == 1 &&
    b == 1 &&
    c == 1001E-3 &&
    d == 330 &&
    e == 0.0045 &&
    f == 4660
    );

/*
*  case 1-2
*  NaN and Infinity
*/

a = 3 / 0;
b = -94E-2 / 0;
c = NaN;

result.push(
    a == Infinity &&
    b == -Infinity &&
    isNaN(c)
);


/*
*  case 1-3
*  four arithmetic operations
*/
a = 3 + 1 * 4 - 9 / 3;
b = ((1 + 3) / 2 - (4) * (((3))) / 2) * 1.2;

result.push(
    a == 4 &&
    b == -4.8
);

/*
*  case 1-4
*  EqualityExpression
*/

a = 40;
b = 40;
result.push(
    a == b &&
    a == b == true &&
    a == b + 1 == false &&
    a != b == false
);

/*
*  case 1-5
*  PostfixExpression
*/

a = 10;
b = 10;
a++;
b--;

result.push(
    a == 11 &&
    a++ == 11 &&
    a == 12 &&
    a++ == 12 &&
    b == 9 &&
    b-- == 9 &&
    b == 8 &&
    b-- == 8
);

/*
*  case 1-6
*  RelationalExpression
*/

a = 10;
b = -20;

result.push(
    a > b &&
    a > b == true &&
    a < b == false &&
    a + b > -a == false &&
    a + b >= -a &&
    a <= b == false &&
    a * 2 <= -b
)


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

a = '';
b = '';
c = '';

if (1 + 2 == 3) {
    a = 'ok';
}

if (1 + 2 == 3) b = 'ok';

if ('a' + 'bc' == 'abc')
    if ('abc' + 123 == 'abc123')
    c = 'ok';

result.push(
    a == 'ok' &&
    b == 'ok' &&
    c == 'ok'
);

/*
*  case 5-2-2
*  if/else statement - [ if ( ... ) ~ else ~ ]
*/

a = '';
b = '';
c = '';
d = '';

if (123 == false) {
    a = 'equal';
}
else {
    b = 'not equal';
}

if (true != false)
    c = 'equal';
else
    d = 'not equal';

result.push(
    a == '' && b == 'not equal' &&
    c == 'equal' && d == ''
);

//////////////////////////////////////////////////////////////////////////////
//
// 5-3  iteration statement
//

/*
*  case 5-3-1
*  iteration statement - [ while ( ... ) ~ ]
*/
var repeat_count = 1000;
i = repeat_count + 0;
j = 0;
while (j < i) {
    --i;
    ++j;
}

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/*
*  case 5-3-2
*  iteration statement - [ do ~ while ]
*/
repeat_count = 1000;
i = repeat_count + 0;
j = 0;

do {
    --i;
    ++j;
}
while (j < i);

result.push(
    i == repeat_count / 2 &&
    j == repeat_count / 2
);

/****************************************************************************
*
* category 6 Function Definition
*
****************************************************************************/



__debug.print(result);
