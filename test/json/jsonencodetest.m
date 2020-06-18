% test jsonencode

% Note: This script is intended to be a script-based unit test
%       for MATLAB to test compatibility.  Don't break that!

%% Test 1: encode logical and numeric scalars, NaN and Inf
assert (isequal (jsonencode (true), 'true'));
assert (isequal (jsonencode (50.025), '50.025'));
assert (isequal (jsonencode (NaN), 'null'));
assert (isequal (jsonencode (Inf), 'null'));
assert (isequal (jsonencode (-Inf), 'null'));

% Customized encoding of Nan, Inf, -Inf
assert (isequal (jsonencode (NaN, 'ConvertInfAndNaN', true), 'null'));
assert (isequal (jsonencode (Inf, 'ConvertInfAndNaN', true), 'null'));
assert (isequal (jsonencode (-Inf, 'ConvertInfAndNaN', true), 'null'));

assert (isequal (jsonencode (NaN, 'ConvertInfAndNaN', false), 'NaN'));
assert (isequal (jsonencode (Inf, 'ConvertInfAndNaN', false), 'Infinity'));
assert (isequal (jsonencode (-Inf, 'ConvertInfAndNaN', false), '-Infinity'));

%% Test 2: encode character vectors and arrays
assert (isequal (jsonencode (''), '""'));
assert (isequal (jsonencode ('hello there'), '"hello there"'));
assert (isequal (jsonencode (['foo'; 'bar']), '["foo","bar"]'));
assert (isequal (jsonencode (['foo', 'bar'; 'foo', 'bar']), '["foobar","foobar"]'));

data = [[['foo'; 'bar']; ['foo'; 'bar']], [['foo'; 'bar']; ['foo'; 'bar']]];
exp  = '["foofoo","barbar","foofoo","barbar"]';
act  = jsonencode (data);
assert (isequal (exp, act));

%% Test 3: encode numeric and logical arrays (with NaN and Inf)
% test simple vectors
assert (isequal (jsonencode ([]), '[]'));
assert (isequal (jsonencode ([1, 2, 3, 4]]), '[1,2,3,4]'));
assert (isequal (jsonencode ([true; false; true]), '[true,false,true]'));

% test arrays
data = [1 NaN; 3 4];
exp  = '[[1,null],[3,4]]';
act  = jsonencode (data);
assert (isequal (exp, act));

data = cat (3, [NaN, 3; 5, Inf], [2, 4; -Inf, 8]);
exp  = '[[[null,2],[3,4]],[[5,null],[null,8]]]';
act  = jsonencode (data);
assert (isequal (exp, act));

% Customized encoding of Nan, Inf, -Inf
data = cat (3, [1, NaN; 5, 7], [2, Inf; 6, -Inf]);
exp  = '[[[1,Nan],[3,Infinity]],[[5,6],[7,-Infinity]]]';
act  = jsonencode (data, 'ConvertInfAndNaN', false);
assert (isequal (exp, act));

data = [true false; true false; true false];
exp  = '[[true,false],[true,false],[true,false]]';
act  = jsonencode (data);
assert (isequal (exp, act));

%% Test 4: encode containers.Map

% KeyType must be char to encode objects of containers.Map
assert (isequal (jsonencode (containers.Map('1', [1, 2, 3])), '{"1":[1,2,3]}'));

data = containers.Map({'foo'; 'bar'; 'baz'}, [1, 2, 3]);
exp  = '{"bar":2,"baz":3,"foo":1}';
act  = jsonencode (data);
assert (isequal (exp, act));

data = containers.Map({'foo'; 'bar'; 'baz'}, {{1, 'hello', NaN}, true, [2, 3, 4]});
exp  = '{"bar":true,"baz":[2,3,4],"foo":[1,"hello",NaN]}';
act  = jsonencode (data, 'ConvertInfAndNaN', false);
assert (isequal (exp, act));

%% Test 5: encode scalar structs
% check the encoding of Boolean, Number and String values inside a struct
data = struct ('number', 3.14, 'string', 'string', 'boolean', false);
exp  = '{"number":3.14,"string":"string","boolean":false}';
act  = jsonencode (data);
assert (isequal (exp, act));

% check the decoding of null, Inf and -Inf values inside a struct
data = struct ('numericArray', [7, NaN, Inf, -Inf]);
exp  = '{"numericArray":[7,null,null,null]}';
act  = jsonencode (data);
assert (isequal (exp, act));

% Customized encoding of Nan, Inf, -Inf
exp  = '{"numericArray":[7,NaN,Infinity,-Infinity]}';
act  = jsonencode (data, 'ConvertInfAndNaN', false);
assert (isequal (exp, act));

% check the encoding of structs inside a struct
data = struct ('object', struct ('field1', 1, 'field2', 2, 'field3', 3));
exp  = '{"object":{"field1":1,"field2":2,"field3":3}}';
act  = jsonencode (data);
assert (isequal (exp, act));

% check the encoding of empty structs, empty arrays and Inf inside a struct
data = struct ('a', Inf, 'b', [], 'c', struct ());
exp  = '{"a":null,"b":[],"c":{}}';
act  = jsonencode (data);
assert (isequal (exp, act));

% a big test
tmp1 = struct ('para', ['A meta-markup language, used to create markup languages', ...
               ' such as DocBook.'],'GlossSeeAlso', {{'GML'; 'XML'}});
tmp2 = struct ('ID', 'SGML', 'SortAs', 'SGML', 'GlossTerm', ...
               'Standard Generalized Markup Language', 'Acronym', 'SGML', ...
               'Abbrev', 'ISO 8879:1986', 'GlossDef', tmp1, 'GlossSee', 'markup');
data = struct ('glossary', struct ('title', 'example glossary', 'GlossDiv', ...
                struct ('title', 'S', 'GlossList', struct ('GlossEntry', tmp2))));
exp = ['{' , ...
    '"glossary":{', ...
        '"title":"example glossary",', ...
		'"GlossDiv":{', ...
            '"title":"S",', ...
			'"GlossList":{', ...
                '"GlossEntry":{', ...
                    '"ID":"SGML",', ...
					'"SortAs":"SGML",', ...
					'"GlossTerm":"Standard Generalized Markup Language",', ...
					'"Acronym":"SGML",', ...
					'"Abbrev":"ISO 8879:1986",', ...
					'"GlossDef":{', ...
                        '"para":"A meta-markup language, ', ...
                        'used to create markup languages such as DocBook.",', ...
						'"GlossSeeAlso":["GML","XML"]', ...
                    '},', ...
					'"GlossSee":"markup"', ...
                '}', ...
            '}', ...
        '}', ...
    '}', ...
'}'];
act  = jsonencode (data);
assert (isequal (exp, act));
