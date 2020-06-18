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
