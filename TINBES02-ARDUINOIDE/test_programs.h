// program hello
byte prog1[] = {STRING, 'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '\n', 0,
                PRINT,
                STOP};

// program test_vars
byte prog2[] = {
    STRING,
    't',
    'e',
    's',
    't',
    0,
    SET,
    's',
    CHAR,
    'a',
    SET,
    'c',
    INT,
    1,
    7,
    SET,
    'i', // 263
    FLOAT,
    66,
    246,
    230,
    102,
    SET,
    'f', // 123.45
    STRING,
    'p',
    'a',
    's',
    's',
    'e',
    'd',
    0,
    SET,
    's'};

// program test_loop
byte prog3[] = {INT, 0, 0, SET, 'i',
                LOOP,
                GET, 'i', INCREMENT, SET, 'i',
                GET, 'i', PRINTLN,
                MILLIS, INT, 3, 232, PLUS, // 1000
                DELAYUNTIL,
                ENDLOOP};

// program test_if
byte prog4[] = {INT, 0, 3, SET, 'a',
                CHAR, 5, SET, 'b',
                GET, 'a', GET, 'b', EQUALS,
                IF, 7,
                STRING, 'T', 'r', 'u', 'e', 0,
                PRINTLN,
                ELSE, 8,
                STRING, 'F', 'a', 'l', 's', 'e', 0,
                PRINTLN,
                ENDIF,
                CHAR, 3, SET, 'b',
                GET, 'a', GET, 'b', EQUALS,
                IF, 7,
                STRING, 'T', 'r', 'u', 'e', 0,
                PRINTLN,
                ELSE, 8,
                STRING, 'F', 'a', 'l', 's', 'e', 0,
                PRINTLN,
                ENDIF,
                STOP};
byte prog5[] = {STRING, 't', 'e', 's', 't', 0, SET, 's',
                CHAR, 'a', SET, 'c',
                INT, 1, 7, SET, 'i',                // 263
                FLOAT, 66, 246, 230, 102, SET, 'f', // 123.45
                STRING, 'p', 'a', 's', 's', 'e', 'd', 0, SET, 's',
                GET, 'c', INCREMENT, SET, 'c',
                GET, 'i', DECREMENT, SET, 'i',
                GET, 'f', INCREMENT, SET, 'f',
                GET, 's', PRINTLN,
                GET, 'c', PRINTLN,
                GET, 'i', PRINTLN,
                GET, 'f', PRINTLN,
                STOP};
