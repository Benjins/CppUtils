#include "idbase.h"
#include "macros.h"

#if defined(IDBASE_TEST_MAIN)

static int allocCount = 0;

struct TestEntity : IDBase{
	char name[64];
	
	float pos[3];
	TestEntity(){
		allocCount++;
	}
	TestEntity(const TestEntity& ent){
		BNS_UNUSED(ent);
		allocCount++;
	}
	
	~TestEntity(){
		allocCount--;
	}
};

CREATE_TEST_CASE("IDbase + IDTracker basic") {
	ASSERT(allocCount == 0);
	
	{
		IDTracker<TestEntity> ents(20);
		
		for(int i = 0; i < 20; i++){
			TestEntity* add = ents.CreateAndAdd();
			ASSERT(add != nullptr);
		}
		
		for(int i = 0; i < 20; i++){
			for(int j = i+1; j < 20; j++){
				ASSERT(ents.vals[i].id != ents.vals[j].id);
			}
		}
		
		for(int i = 0; i < 10; i++){
			ASSERT(ents.GetByIdNum(i) != nullptr);
			ASSERT(ents.GetById(IDHandle<TestEntity>(i)) != nullptr);
			ents.RemoveByIdNum(i);
			ASSERT(ents.GetByIdNum(i) == nullptr);
			ASSERT(ents.GetById(IDHandle<TestEntity>(i)) == nullptr);
		}
		
		for(int i = 0; i < 10; i++){
			TestEntity* add = ents.CreateAndAdd();
			ASSERT(add != nullptr);
		}
		
		ASSERT(ents.currentCount == 20);

		for(int i = 0; i < 20; i++){
			for(int j = i+1; j < 20; j++){
				ASSERT(ents.vals[i].id != ents.vals[j].id);
			}
		}
		
		for(int i = 10; i < 20; i++){
			ASSERT(ents.GetByIdNum(i) != nullptr);
			ents.RemoveById(IDHandle<TestEntity>(i));
			ASSERT(ents.GetByIdNum(i) == nullptr);
		}
	}

	{
		IDTracker<TestEntity> ents(20);

		for(int i = 1; i < 50; i++){
			ents.SetSize(30  * i);
		}
	}
	
	ASSERT(allocCount == 0);
	
	return 0;
}



#endif
