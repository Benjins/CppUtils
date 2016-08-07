#include <stdio.h>
#include "../vector.h"

struct PatrolPoint{
	float x;
	float y;
	float delay;
};

#include "coroutine_gen.h"

/*[Coroutine]*/
void loopThroughFlts(const float* arr, int count) {
	float total = 0.0f;
	for (int i = 0; i < count; i++) {
		total += arr[i];
		printf("num: %f, total: %f\n", arr[i], total);
		BNS_YIELD();
	}
}

/*[Coroutine]*/
void loopThroughIVec(const Vector<int>* iVec) {
	for (int i = 0; i < iVec->count; i++) {
		printf("idx: %d, val: %d\n", i, iVec->data[i]);
		BNS_YIELD();
	}
}

/*[Coroutine]*/
void WaitForSeconds(float seconds){
	float time = 0;
	while (time < seconds){
		BNS_YIELD();
		time += 0.01667f;
	}
}

/*[Coroutine]*/
void PatrolPath(const Vector<PatrolPoint>* path){
	for (int i = 0; i < path->count; i++){
		printf("Got to point (%f, %f) on path. Waiting %f seconds.\n", path->data[i].x, path->data[i].y, path->data[i].delay);
		BNS_YIELD_FROM_ARGS(WaitForSeconds, path->data[i].delay);
	}
}

#if defined(COROUTINE_TEST_MAIN)

int main(int argc, char** argv) {
	
	float list[5] = {5.23f, 51.1f, -34.55f, 0.0f, 43.21f};
	
	START_CR_ARGS(list_inst, loopThroughFlts, list, 5);
	
	while(list_inst.Next()){
		printf("Taking a break here.\n");
		float cc = 20.4f;
		printf("Here's another number: %f\n", cc);
	}

	Vector<int> iVec;
	iVec.PushBack(23);
	iVec.PushBack(34);
	iVec.PushBack(51);
	iVec.PushBack(87);

	START_CR_ARGS(vec_inst, loopThroughIVec, &iVec);

	while (vec_inst.Next()) {
		printf("In between vec iteration.\n");
	}

	Vector<PatrolPoint> patrolPoints;
	PatrolPoint p1 = {14, 15, 0.08f};
	patrolPoints.PushBack(p1);
	PatrolPoint p2 = { -34, 5, 0.14f };
	patrolPoints.PushBack(p2);

	START_CR_ARGS(pat_inst, PatrolPath, &patrolPoints);
	while (pat_inst.Next()) {
		printf("---End frame---\n");
	}

	return 0;
}

#endif
