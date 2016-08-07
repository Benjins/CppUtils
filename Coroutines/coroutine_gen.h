#ifndef COROUTINEGEN_H
#define COROUTINEGEN_H
#include "coroutine_runtime.h"


//------------------------
// generated code

struct loopThroughFlts_Data {
	int _case;
	const float* _arr_36;
	int _count_39;
	float _total_43;
	int _i_50;

	loopThroughFlts_Data(){
	}
	loopThroughFlts_Data(const float* arrParam, int countParam) {
		_case = 0;
		_arr_36 = arrParam;
		_count_39 = countParam;
	}
};

CoroutineResult loopThroughFlts_Func(void* ptr){
	loopThroughFlts_Data* _data = (loopThroughFlts_Data*)ptr;
	switch (_data->_case) {
	case 0:
	_data->_total_43 = 0.0f ; 
	for ( _data->_i_50 = 0 ; _data->_i_50 < _data->_count_39 ; _data->_i_50 ++ ) { 
		_data->_total_43 += _data->_arr_36 [ _data->_i_50 ] ; 
		printf ( "num: %f, total: %f\n" , _data->_arr_36 [ _data->_i_50 ] , _data->_total_43 ) ; 
		_data->_case = 1;
		return CoroutineResult::CR_Yield;
		case 1: ;
		; 
			} 
	
	default:
	break;}
	_data->_case = -1;
	return CoroutineResult::CR_Return; 
}
//------------------------

//------------------------
// generated code

struct loopThroughIVec_Data {
	int _case;
	const Vector<int>* _iVec_99;
	int _i_105;

	loopThroughIVec_Data(){
	}
	loopThroughIVec_Data(const Vector<int>* iVecParam) {
		_case = 0;
		_iVec_99 = iVecParam;
	}
};

CoroutineResult loopThroughIVec_Func(void* ptr){
	loopThroughIVec_Data* _data = (loopThroughIVec_Data*)ptr;
	switch (_data->_case) {
	case 0:
	for ( _data->_i_105 = 0 ; _data->_i_105 < _data->_iVec_99 -> count ; _data->_i_105 ++ ) { 
		printf ( "idx: %d, val: %d\n" , _data->_i_105 , _data->_iVec_99 -> data [ _data->_i_105 ] ) ; 
		_data->_case = 1;
		return CoroutineResult::CR_Yield;
		case 1: ;
		; 
			} 
	
	default:
	break;}
	_data->_case = -1;
	return CoroutineResult::CR_Return; 
}
//------------------------

//------------------------
// generated code

struct WaitForSeconds_Data {
	int _case;
	float _seconds_146;
	float _time_150;

	WaitForSeconds_Data(){
	}
	WaitForSeconds_Data(float secondsParam) {
		_case = 0;
		_seconds_146 = secondsParam;
	}
};

CoroutineResult WaitForSeconds_Func(void* ptr){
	WaitForSeconds_Data* _data = (WaitForSeconds_Data*)ptr;
	switch (_data->_case) {
	case 0:
	_data->_time_150 = 0 ; 
	while ( _data->_time_150 < _data->_seconds_146 ) { 
		_data->_case = 1;
		return CoroutineResult::CR_Yield;
		case 1: ;
		; 
		_data->_time_150 += 0.01667f ; 
			} 
	
	default:
	break;}
	_data->_case = -1;
	return CoroutineResult::CR_Return; 
}
//------------------------

//------------------------
// generated code

struct PatrolPath_Data {
	int _case;
	const Vector<PatrolPoint>* _path_183;
	int _i_189;
	WaitForSeconds_Data _WaitForSeconds_Data_0;

	PatrolPath_Data(){
	}
	PatrolPath_Data(const Vector<PatrolPoint>* pathParam) {
		_case = 0;
		_path_183 = pathParam;
	}
};

CoroutineResult PatrolPath_Func(void* ptr){
	PatrolPath_Data* _data = (PatrolPath_Data*)ptr;
	switch (_data->_case) {
	case 0:
	for ( _data->_i_189 = 0 ; _data->_i_189 < _data->_path_183 -> count ; _data->_i_189 ++ ) { 
		printf ( "Got to point (%f, %f) on path. Waiting %f seconds.\n" , _data->_path_183 -> data [ _data->_i_189 ] . x , _data->_path_183 -> data [ _data->_i_189 ] . y , _data->_path_183 -> data [ _data->_i_189 ] . delay ) ; 
		_data->_WaitForSeconds_Data_0 = WaitForSeconds_Data(_data->_path_183 -> data [ _data->_i_189 ] . delay );
		while (WaitForSeconds_Func(&_data->_WaitForSeconds_Data_0) != CoroutineResult::CR_Return){
			_data->_case = 1;
			return CoroutineResult::CR_Yield;
			case 1: ;
			}
 	} 
	
	default:
	break;}
	_data->_case = -1;
	return CoroutineResult::CR_Return; 
}
//------------------------

#define COROT_MAX(a, b) ((a) > (b) ? (a) : (b))
const int maxCoroutineDataSize = 
	COROT_MAX(sizeof(loopThroughFlts_Data), 
	COROT_MAX(sizeof(loopThroughIVec_Data), 
	COROT_MAX(sizeof(WaitForSeconds_Data), 
	COROT_MAX(sizeof(PatrolPath_Data), 
	0))));


#undef COROT_MAX

#endif
