#ifndef COROUTINEGEN_H
#define COROUTINEGEN_H
#include "coroutine_runtime.h"


//------------------------
// generated code

struct loopThroughFlts_Data {
	int _case;
	const float* _arr_36;
	int _count_39;

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
