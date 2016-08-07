Coroutines
=====================
Preprocessor for coroutines in C++ with only O(∞) caveats.

I got tired of messing with setjmp/longjmp/asm and just wrote a preprocessor that converts slightly marked-up C++ functions into something that can be iterated.

For each coroutine function, a data struct is created that holds all parameters and local variables.  Then, the function is converted to take a pointer to the data and reference its fields instead of local vars and parameters.  This wrapper function is also generated, which also convers any yield's made into jumps using a switch statement.

Basically:

 * Write the coroutine function.
 * Write the usage code that uses the coroutines.
 * Run the preprocessor on your code.
 * Compile everything together.
 
You *could* call the coroutine function directly.  It is a valid function after all.  It would just run all at once, without any of the coroutine goodness.

Known caveats:
 - This is not a full compiler.  It mostly assumes my style, so some other stuff won't do.
 - All delcarations become assignments, so initalizer-lists don't work.
 - Template types with multiple parameters, or with multiple tokens in its parameter (i.e. Vector<int*> or Vector<const IntPtr> won't work).
 - Arrays with a length that is not either a literal, or a global constant.
 
Licensed under MIT License, see LICENSE.txt for more info.

Example
--------------

Write this:

```cpp
/*[Coroutine]*/
void loopThroughFlts(const float* arr, int count) {
	float total = 0;
	for (int i = 0; i < count; i++) {
		total += arr[i];
		printf("num: %f, total: %f\n", arr[i], total);
		BNS_YIELD();
	}
}
```

Use like this:

```cpp
	float list[5] = {5.23f, 51.1f, -34.55f, 0.0f, 43.21f};

	START_CR_ARGS(list_inst, loopThroughFlts, list, 5);

	while(list_inst.Next()){
		printf("Taking a break here.\n");
		float cc = 20.4f;
		printf("Here's another number: %f\n", cc);
	}
```

Code generated looks like this:

```cpp
struct loopThroughFlts_Data {
	int _case;
	const float* _arr_306;
	int _count_309;
	float _total_313;
	int _i_320;

	loopThroughFlts_Data(const float* arrParam, int countParam) {
		_case = 0;
		_arr_306 = arrParam;
		_count_309 = countParam;
	}
};

CoroutineResult loopThroughFlts_Func(void* ptr){
	loopThroughFlts_Data* _data = (loopThroughFlts_Data*)ptr;
	switch (_data->_case) {
	case 0:
	_data->_total_313 = 0 ;
	for ( _data->_i_320 = 0 ; _data->_i_320 < _data->_count_309 ; _data->_i_320 ++ ) {
		_data->_total_313 += _data->_arr_306 [ _data->_i_320 ] ;
		printf ( "num: %f, total: %f\n" , _data->_arr_306 [ _data->_i_320 ] , _data->_total_313 ) ;
		_data->_case = 1;
		return CoroutineResult::CR_Yield;
		case 1: ;

	}

	default:
	break;}
	return CoroutineResult::CR_Return;
}
```

And the output looks like this:

```
num: 5.230000, total: 5.230000
Taking a break here.
Here's another number: 20.400000
num: 51.099998, total: 56.329998
Taking a break here.
Here's another number: 20.400000
num: -34.549999, total: 21.779999
Taking a break here.
Here's another number: 20.400000
num: 0.000000, total: 21.779999
Taking a break here.
Here's another number: 20.400000
num: 43.209999, total: 64.989998
Taking a break here.
Here's another number: 20.400000
```
