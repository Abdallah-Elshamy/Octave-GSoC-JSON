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
