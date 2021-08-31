#include "stdafx.h"
#include "CppUnitTest.h"
#include "GC.h"
#include "GCUtil.h"
#include "Dummy.h"
#include "TestSpy.h"
#include "Managed_Array.h"
#include "Handle_Array.h"
#include "HndlArrHandle.h"
#include "Unmanaged_Array.h"
#include "UnmanagedArrHandle.h"
#include "UnmanagedHndlDummy.h"
#include "UnmanagedDummy.h"
#include "EmptyDummy.h"
#include "LargeDummy.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace HyMMSFinalTest
{		
	TEST_CLASS(UnitTest1)
	{
	public:

		//Test unaligned allocation
		TEST_METHOD(AllocatorTest1)
		{
			GC::CreateHeap(100); // Allocate heap of 100 bytes
			TestSpy spy = TestSpy();

			void* base = spy.ForceAllocate(1);

			Assert::AreEqual((char*)base + 1, (char*)spy.GetHeapTopPtr());

			spy.ForceAllocate(19);

			Assert::AreEqual((char*)base + 20, (char*)spy.GetHeapTopPtr());

			void* marker = spy.ForceAllocate(79);

			Assert::AreEqual((char*)marker + 79, (char*)spy.GetHeapTopPtr());

			GC::DestroyHeap();
		}

		//Test aligned allocation
		TEST_METHOD(AllocatorTest2)
		{
			GC::CreateHeap(100);
			TestSpy spy = TestSpy();

			void* base = spy.ForceAllocate(1);

			void* alignedBase = spy.ForceAllocateAligned(12, 4);

			Assert::AreEqual((char*)base + 4, (char*)alignedBase);

			void* alignedNext = spy.ForceAllocateAligned(12, 4);

			Assert::AreEqual((char*)alignedBase + 12, (char*)alignedNext);

			GC::DestroyHeap();
		}

		//Test that failed allocation requests return NULL (aligned and unaligned)
		TEST_METHOD(AllocatorTest3)
		{
			GC::CreateHeap(100);
			TestSpy spy = TestSpy();

			void* base = spy.ForceAllocate(100);

			Assert::IsNotNull(base);

			void* outOfMem = spy.ForceAllocate(1);

			Assert::IsNull(outOfMem);

			GC::DestroyHeap();

			GC::CreateHeap(100);
			TestSpy spy2 = TestSpy();

			void* baseAligned = spy2.ForceAllocateAligned(100, 4);

			Assert::IsNotNull(baseAligned);

			void* outOfMemAligned = spy2.ForceAllocateAligned(1, 4);

			Assert::IsNull(outOfMemAligned);

			GC::DestroyHeap();
		}

		//Test normal allocation with gcnew
		TEST_METHOD(Managed_ObjectTest1)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			EmptyDummy* ed1 = gcnew(EmptyDummy)();

			EmptyDummy* ed2 = gcnew(EmptyDummy)();

			Assert::AreEqual((char*)ed1 + sizeof(EmptyDummy), (char*)ed2);

			GC::DestroyHeap();
		}

		//Test allocation of mixed types on heap
		TEST_METHOD(Managed_ObjectTest2)
		{
			GC::CreateHeap((5 * sizeof(EmptyDummy)) + (5 * sizeof(Dummy)));

			EmptyDummy* ed1 = gcnew(EmptyDummy)();

			Dummy* d1 = gcnew(Dummy)();

			EmptyDummy* ed2 = gcnew(EmptyDummy)();

			Assert::AreEqual((char*)ed1 + sizeof(EmptyDummy) + sizeof(Dummy), (char*)ed2);

			GC::DestroyHeap();
		}

		//Test allocation of large object proxy
		TEST_METHOD(Managed_ObjectTest3)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));
			GC::SetLargeObjThreshold(1024);

			EmptyDummy* ed1 = gcnew(EmptyDummy)();

			LargeDummy* ld1 = gcnew(LargeDummy)();

			TestSpy spy = TestSpy();

			void* offheap = &(*ld1);

			Assert::IsFalse(spy.IsOnHeap(offheap));

			GC::DestroyHeap();
		}

		//Test allocation of pinned object proxy
		TEST_METHOD(Managed_ObjectTest4)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			EmptyDummy* ed1 = gcnew_Pinned(EmptyDummy)();

			TestSpy spy = TestSpy();

			void* offHeap = &(*ed1);

			Assert::IsFalse(spy.IsOnHeap(offHeap));

			GC::DestroyHeap();
		}

		//Test simple access
		TEST_METHOD(SmartHandleTest1)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();

			SmartHandle<EmptyDummy> h2 = gcnew(EmptyDummy)();

			EmptyDummy* ed1 = &(*h1);
			EmptyDummy* ed2 = &(*h2);

			Assert::AreEqual((char*)ed1 + sizeof(EmptyDummy), (char*)ed2);

			GC::DestroyHeap();
		}

		//Test access after copy
		TEST_METHOD(SmartHandleTest2)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			SmartHandle<EmptyDummy> h1 = gcnew_Pinned(EmptyDummy)();
			TestSpy spy = TestSpy();

			EmptyDummy* ed1 = &(*h1);

			Assert::IsFalse(spy.IsOnHeap(ed1));

			GC::DestroyHeap();
		}

		//Test registration with object
		TEST_METHOD(SmartHandleTest3)
		{
			GC::CreateHeap(10 * sizeof(Dummy));
			TestSpy spy = TestSpy();

			Dummy* d1 = gcnew(Dummy)();

			d1->h1 = gcnew(Dummy)();

			spy.MarkChildrenGrey(d1);

			Dummy* d2 = &(*(d1->h1));

			bool result = spy.isGrey(d2);

			Assert::IsTrue(result);

			GC::DestroyHeap();
		}

		//Test registration with proxied object
		TEST_METHOD(SmartHandleTest4)
		{
			GC::CreateHeap(10 * sizeof(Dummy));
			TestSpy spy = TestSpy();

			Dummy* d1 = gcnew_Pinned(Dummy)();

			d1->h1 = gcnew(Dummy)();

			spy.MarkChildrenGrey(d1);

			Dummy* d2 = &(*(d1->h1));

			bool result = spy.isGrey(d2);

			Assert::IsTrue(result);

			GC::DestroyHeap();
		}

		//Test mutliple handles referencing same object
		TEST_METHOD(SmartHandleTest5)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();

			SmartHandle<EmptyDummy> h2 = h1;

			EmptyDummy* ed1 = &(*h1);
			EmptyDummy* ed2 = &(*h2);

			Assert::AreEqual((char*)ed1, (char*)ed2);

			GC::DestroyHeap();
		}

		//Test registration as nursery root
		TEST_METHOD(SmartHandleTest6)
		{
			GC::CreateHeap(10 * sizeof(Dummy));
			TestSpy spy = TestSpy();

			SmartHandle<Dummy> h1 = gcnew(Dummy)();

			spy.TraceNurseryRoots();

			Dummy* d1 = &(*h1);

			bool result = spy.isGrey(d1);

			Assert::IsTrue(result);

			GC::DestroyHeap();
		}

		//Test access to proxied object
		TEST_METHOD(SmartHandleTest7)
		{
			GC::CreateHeap(10 * sizeof(Dummy));
			TestSpy spy = TestSpy();

			SmartHandle<Dummy> h1 = gcnew_Pinned(Dummy)();

			h1->setResult(42);

			Dummy* d1 = &(*h1);

			bool result = spy.IsOnHeap(d1);

			Assert::IsFalse(result);

			int result2 = d1->getResult();

			Assert::AreEqual(42, result2);

			GC::DestroyHeap();
		}

		//Test initial colour is green
		TEST_METHOD(TracerTest1)
		{
			GC::CreateHeap(10 * sizeof(Dummy));

			TestSpy spy = TestSpy();

			SmartHandle<Dummy> h1 = gcnew(Dummy)();

			bool result = spy.isGreen(&(*h1));

			Assert::IsTrue(result);

			GC::DestroyHeap();
		}

		//Test root is registered as Nursery root
		TEST_METHOD(TracerTest2)
		{
			GC::CreateHeap(10 * sizeof(Dummy));

			TestSpy spy = TestSpy();

			SmartHandle<Dummy> h1 = gcnew(Dummy)();

			spy.TraceNurseryRoots();

			bool result = spy.isGrey(&(*h1));

			Assert::IsTrue(result);

			GC::DestroyHeap();
		}

		//Test references internal to Nursery not registered as Nursery roots
		TEST_METHOD(TracerTest3)
		{
			GC::CreateHeap(10 * sizeof(Dummy));

			TestSpy spy = TestSpy();

			SmartHandle<Dummy> h1 = gcnew(Dummy)();

			h1->h1 = gcnew(Dummy)();

			spy.TraceNurseryRoots();

			bool result = spy.isGrey(&(*h1));

			Assert::IsTrue(result);

			result = spy.isGreen(&(*(h1->h1)));

			Assert::IsTrue(result);

			GC::DestroyHeap();
		}

		//Test Tracing of Nursery
		TEST_METHOD(TracerTest4)
		{
			GC::CreateHeap(10 * sizeof(Dummy));

			TestSpy spy = TestSpy();

			SmartHandle<Dummy> h1 = gcnew(Dummy)();

			h1->h1 = gcnew(Dummy)();

			spy.TraceNursery();

			bool result = spy.isBlack(&(*h1));

			Assert::IsTrue(result);

			result = spy.isBlack(&(*(h1->h1)));

			Assert::IsTrue(result);

			GC::DestroyHeap();
		}

		//Test ciruclar reference is properly traced where reachable
		TEST_METHOD(TracerTest5)
		{
			GC::CreateHeap(10 * sizeof(Dummy));

			TestSpy spy = TestSpy();

			SmartHandle<Dummy> h1 = gcnew(Dummy)();

			h1->h1 = gcnew(Dummy)();

			h1->h1->h1 = gcnew(Dummy)();

			h1->h1->h1->h1 = h1->h1->h1;

			spy.TraceNursery();

			bool result = spy.isBlack(&(*h1));

			Assert::IsTrue(result);

			result = spy.isBlack(&(*(h1->h1)));

			Assert::IsTrue(result);

			result = spy.isBlack(&(*(h1->h1->h1)));

			Assert::IsTrue(result);

			GC::DestroyHeap();
		}

		//Test circular reference is not traced where unreachable
		TEST_METHOD(TracerTest6)
		{
			GC::CreateHeap(10 * sizeof(Dummy));

			TestSpy spy = TestSpy();

			Dummy* d1 = gcnew(Dummy)();

			Dummy* d2 = gcnew(Dummy)();

			d1->h1 = d2;

			d2->h1 = d1;

			spy.TraceNursery();

			bool result = spy.isGreen(d1);

			Assert::IsTrue(result);

			result = spy.isGreen(d2);

			Assert::IsTrue(result);

			GC::DestroyHeap();
		}

		//Test multiple handles referencing same object are traced correctly
		TEST_METHOD(TracerTest7)
		{
			GC::CreateHeap(10 * sizeof(Dummy));

			TestSpy spy = TestSpy();

			SmartHandle<Dummy> h1 = gcnew(Dummy)();

			SmartHandle<Dummy> h2 = gcnew(Dummy)();

			h1->h1 = gcnew(Dummy)();

			h2->h1 = h1->h1;

			h1->h2 = gcnew(Dummy)();

			h2->h2 = h1->h2;

			spy.TraceNursery();

			bool result = spy.isBlack(&(*h1));

			Assert::IsTrue(result);

			result = spy.isBlack(&(*h2));

			Assert::IsTrue(result);

			result = spy.isBlack(&(*(h1->h1)));

			Assert::IsTrue(result);

			result = spy.isBlack(&(*(h1->h2)));

			Assert::IsTrue(result);

			GC::DestroyHeap();
		}

		//Test multiple handles with circular references traced properly
		TEST_METHOD(TracerTest8)
		{
			GC::CreateHeap(10 * sizeof(Dummy));

			TestSpy spy = TestSpy();

			SmartHandle<Dummy> h1 = gcnew(Dummy)();

			SmartHandle<Dummy> h2 = gcnew(Dummy)();

			h1->h1 = gcnew(Dummy)();

			h2->h1 = h1->h1;

			h1->h2 = gcnew(Dummy)();

			h2->h2 = h1->h2;

			h1->h1->h1 = h1->h2;

			h1->h2->h1 = h1->h1;

			spy.TraceNursery();

			bool result = spy.isBlack(&(*h1));

			Assert::IsTrue(result);

			result = spy.isBlack(&(*h2));

			Assert::IsTrue(result);

			result = spy.isBlack(&(*(h1->h1)));

			Assert::IsTrue(result);

			result = spy.isBlack(&(*(h1->h2)));

			Assert::IsTrue(result);

			GC::DestroyHeap();
		}

		//Test nursery compaction call with no objects on the heap does not throw an errors
		TEST_METHOD(CompactorTest1)
		{
			GC::CreateHeap(10 * sizeof(Dummy));

			TestSpy spy = TestSpy();

			spy.NurseryCompact();

			GC::DestroyHeap();
		}

		//Test single live object is correctly compacted into Promotion Buffer
		TEST_METHOD(CompactorTest2)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));
			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();

			spy.TraceNursery();

			spy.NurseryCompact();

			bool result = spy.isBlack(&(*h1));

			Assert::IsTrue(result);

			GC::DestroyHeap();
		}

		//Test that single dead nursery object is discarded correctly
		TEST_METHOD(CompactorTest3)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			void* heapTop = spy.GetHeapTopPtr();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();
			EmptyDummy* ed1 = &(*h1);

			h1 = NULL;

			spy.TraceNursery();

			void* newHeapTop = spy.NurseryCompact();

			Assert::AreEqual((char*)heapTop, (char*)newHeapTop);

			GC::DestroyHeap();
		}

		//Test live object followed by dead object handled correctly
		TEST_METHOD(CompactorTest4)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();

			void* heapTop = spy.GetHeapTopPtr();

			SmartHandle<EmptyDummy> h2 = gcnew(EmptyDummy)();

			h2 = NULL;

			spy.TraceNursery();

			void* newHeapTop = spy.NurseryCompact();

			Assert::AreEqual((char*)heapTop, (char*)newHeapTop);

			GC::DestroyHeap();
		}

		//Test dead object followed by live object handled correctly
		TEST_METHOD(CompactorTest5)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h2 = gcnew(EmptyDummy)();

			EmptyDummy* oldAddh1 = &(*h1);

			h1 = NULL;

			spy.TraceNursery();

			spy.NurseryCompact();

			EmptyDummy* newAddh2 = &(*h2);

			Assert::AreEqual((char*) oldAddh1, (char*)newAddh2);

			GC::DestroyHeap();
		}

		//Test live object, dead object, live object handled correctly
		TEST_METHOD(CompactorTest6)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h2 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h3 = gcnew(EmptyDummy)();

			EmptyDummy* oldAddh2 = &(*h2);

			h2 = NULL;

			spy.TraceNursery();

			spy.NurseryCompact();

			EmptyDummy* newAddh3 = &(*h3);

			Assert::AreEqual((char*) oldAddh2, (char*)newAddh3);

			GC::DestroyHeap();
		}

		//Test dead object, dead object, live object handled correctly
		TEST_METHOD(CompactorTest7)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h2 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h3 = gcnew(EmptyDummy)();

			EmptyDummy* oldAddh1 = &(*h1);

			h1 = NULL;
			h2 = NULL;

			spy.TraceNursery();

			spy.NurseryCompact();

			EmptyDummy* newAddh3 = &(*h3);

			Assert::AreEqual((char*) oldAddh1, (char*)newAddh3);

			GC::DestroyHeap();
		}

		//Test dead object, live object, live object handled correctly
		TEST_METHOD(CompactorTest8)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h2 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h3 = gcnew(EmptyDummy)();

			EmptyDummy* oldAddh1 = &(*h1);
			EmptyDummy* oldAddh2 = &(*h2);

			h1 = NULL;

			spy.TraceNursery();

			spy.NurseryCompact();

			EmptyDummy* newAddh2 = &(*h2);
			EmptyDummy* newAddh3 = &(*h3);

			Assert::AreEqual((char*)oldAddh1, (char*)newAddh2);
			Assert::AreEqual((char*)oldAddh2, (char*)newAddh3);

			GC::DestroyHeap();
		}

		//Test that nursery collection works correctly with zero objects
		TEST_METHOD(NurseryCollectTest1)
		{
			GC::CreateHeap(10 * sizeof(Dummy));

			TestSpy spy = TestSpy();

			spy.NurseryCollection();

			GC::DestroyHeap();
		}

		//Test that nursery collection works correctly with one live object
		TEST_METHOD(NurseryCollectTest2)
		{
			GC::CreateHeap(10 * sizeof(Dummy));

			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();

			spy.NurseryCollection();

			bool result = spy.isBlack(&(*h1));

			Assert::IsTrue(result);

			GC::DestroyHeap();
		}

		//Test that single dead nursery object is discarded correctly
		TEST_METHOD(NurseryCollectTest3)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			void* heapTop = spy.GetHeapTopPtr();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();
			EmptyDummy* ed1 = &(*h1);

			h1 = NULL;

			spy.NurseryCollection();

			void* newHeapTop = spy.GetHeapTopPtr();

			Assert::AreEqual((char*)heapTop, (char*)newHeapTop);

			GC::DestroyHeap();
		}

		//Test live object followed by dead object handled correctly
		TEST_METHOD(NurseryCollectTest4)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();

			void* heapTop = spy.GetHeapTopPtr();

			SmartHandle<EmptyDummy> h2 = gcnew(EmptyDummy)();

			h2 = NULL;

			spy.NurseryCollection();

			void* newHeapTop = spy.GetHeapTopPtr();

			Assert::AreEqual((char*)heapTop, (char*)newHeapTop);

			GC::DestroyHeap();
		}

		//Test dead object followed by live object handled correctly
		TEST_METHOD(NurseryCollectTest5)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h2 = gcnew(EmptyDummy)();

			EmptyDummy* oldAddh1 = &(*h1);

			h1 = NULL;

			spy.NurseryCollection();

			EmptyDummy* newAddh2 = &(*h2);

			Assert::AreEqual((char*) oldAddh1, (char*)newAddh2);

			GC::DestroyHeap();
		}

		//Test live object, dead object, live object handled correctly
		TEST_METHOD(NurseryCollectTest6)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h2 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h3 = gcnew(EmptyDummy)();

			EmptyDummy* oldAddh2 = &(*h2);

			h2 = NULL;

			spy.NurseryCollection();

			EmptyDummy* newAddh3 = &(*h3);

			Assert::AreEqual((char*) oldAddh2, (char*)newAddh3);

			GC::DestroyHeap();
		}

		//Test dead object, dead object, live object handled correctly
		TEST_METHOD(NurseryCollectTest7)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h2 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h3 = gcnew(EmptyDummy)();

			EmptyDummy* oldAddh1 = &(*h1);

			h1 = NULL;
			h2 = NULL;

			spy.NurseryCollection();

			EmptyDummy* newAddh3 = &(*h3);

			Assert::AreEqual((char*) oldAddh1, (char*)newAddh3);

			GC::DestroyHeap();
		}

		//Test dead object, live object, live object handled correctly
		TEST_METHOD(NurseryCollectTest8)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h2 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h3 = gcnew(EmptyDummy)();

			EmptyDummy* oldAddh1 = &(*h1);
			EmptyDummy* oldAddh2 = &(*h2);

			h1 = NULL;

			spy.NurseryCollection();

			EmptyDummy* newAddh2 = &(*h2);
			EmptyDummy* newAddh3 = &(*h3);

			Assert::AreEqual((char*)oldAddh1, (char*)newAddh2);
			Assert::AreEqual((char*)oldAddh2, (char*)newAddh3);

			GC::DestroyHeap();
		}

		//Test that nursery collection works correctly with zero objects
		TEST_METHOD(FullCollectTest1)
		{
			GC::CreateHeap(10 * sizeof(Dummy));

			TestSpy spy = TestSpy();

			spy.NurseryCollection();
			spy.FullCollection();

			GC::DestroyHeap();
		}

		//Test that nursery collection works correctly with one live object
		TEST_METHOD(FullCollectTest2)
		{
			GC::CreateHeap(10 * sizeof(Dummy));

			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();

			spy.NurseryCollection();

			bool result = spy.isBlack(&(*h1));

			spy.FullCollection();

			result = result && spy.isWhite(&(*h1));

			Assert::IsTrue(result);

			GC::DestroyHeap();
		}

		//Test that single dead nursery object is discarded correctly
		TEST_METHOD(FullCollectTest3)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			void* heapTop = spy.GetHeapTopPtr();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();
			EmptyDummy* ed1 = &(*h1);

			spy.NurseryCollection();

			h1 = NULL;

			spy.FullCollection();

			void* newHeapTop = spy.GetHeapTopPtr();

			Assert::AreEqual((char*)heapTop, (char*)newHeapTop);

			GC::DestroyHeap();
		}

		//Test live object followed by dead object handled correctly
		TEST_METHOD(FullCollectTest4)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();

			void* heapTop = spy.GetHeapTopPtr();

			SmartHandle<EmptyDummy> h2 = gcnew(EmptyDummy)();

			spy.NurseryCollection();

			h2 = NULL;

			spy.FullCollection();

			void* newHeapTop = spy.GetHeapTopPtr();

			Assert::AreEqual((char*)heapTop, (char*)newHeapTop);

			GC::DestroyHeap();
		}

		//Test dead object followed by live object handled correctly
		TEST_METHOD(FullCollectTest5)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h2 = gcnew(EmptyDummy)();

			EmptyDummy* oldAddh1 = &(*h1);

			spy.NurseryCollection();

			h1 = NULL;

			spy.FullCollection();

			EmptyDummy* newAddh2 = &(*h2);

			Assert::AreEqual((char*) oldAddh1, (char*)newAddh2);

			GC::DestroyHeap();
		}

		//Test live object, dead object, live object handled correctly
		TEST_METHOD(FullCollectTest6)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h2 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h3 = gcnew(EmptyDummy)();

			EmptyDummy* oldAddh2 = &(*h2);

			spy.NurseryCollection();

			h2 = NULL;

			spy.FullCollection();

			EmptyDummy* newAddh3 = &(*h3);

			Assert::AreEqual((char*) oldAddh2, (char*)newAddh3);

			GC::DestroyHeap();
		}

		//Test dead object, dead object, live object handled correctly
		TEST_METHOD(FullCollectTest7)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h2 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h3 = gcnew(EmptyDummy)();

			EmptyDummy* oldAddh1 = &(*h1);

			spy.NurseryCollection();

			h1 = NULL;
			h2 = NULL;

			spy.FullCollection();

			EmptyDummy* newAddh3 = &(*h3);

			Assert::AreEqual((char*) oldAddh1, (char*)newAddh3);

			GC::DestroyHeap();
		}

		//Test dead object, live object, live object handled correctly
		TEST_METHOD(FullCollectTest8)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			SmartHandle<EmptyDummy> h1 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h2 = gcnew(EmptyDummy)();
			SmartHandle<EmptyDummy> h3 = gcnew(EmptyDummy)();

			EmptyDummy* oldAddh1 = &(*h1);
			EmptyDummy* oldAddh2 = &(*h2);

			spy.NurseryCollection();

			h1 = NULL;

			spy.FullCollection();

			EmptyDummy* newAddh2 = &(*h2);
			EmptyDummy* newAddh3 = &(*h3);

			Assert::AreEqual((char*)oldAddh1, (char*)newAddh2);
			Assert::AreEqual((char*)oldAddh2, (char*)newAddh3);

			GC::DestroyHeap();
		}

		//Test IncCollect where 8 objects are created, 4 made unreachable, and then IncCollect is run several times
		TEST_METHOD(IncCollectTest1)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			std::vector<SmartHandle<EmptyDummy> > hndlVec(8);

			for (int i = 0 ; i < 8; ++i)
			{
				hndlVec[i] = gcnew(EmptyDummy)();
			}

			EmptyDummy* origh0 = &(*(hndlVec[0]));
			EmptyDummy* origh1 = &(*(hndlVec[1]));
			EmptyDummy* origh2 = &(*(hndlVec[2]));
			EmptyDummy* origh3 = &(*(hndlVec[3]));

			spy.FullCollection();

			hndlVec[0] = NULL;
			hndlVec[1] = NULL;
			hndlVec[5] = NULL;
			hndlVec[7] = NULL;

			GC::SetCollectorBudget(0.1);

			for (int i = 0; i < 1000; ++i)
			{
				GC::IncCollect();
			}

			EmptyDummy* newh0 = &(*(hndlVec[2]));
			EmptyDummy* newh1 = &(*(hndlVec[3]));
			EmptyDummy* newh2 = &(*(hndlVec[4]));
			EmptyDummy* newh3 = &(*(hndlVec[6]));

			
			Assert::AreEqual((char*)origh0, (char*)newh0);
			Assert::AreEqual((char*)origh1, (char*)newh1);
			Assert::AreEqual((char*)origh2, (char*)newh2);
			Assert::AreEqual((char*)origh3, (char*)newh3);

			GC::DestroyHeap();
		}

		//Test IncCollect where a Managed_Array of 8 objects are created, 
		//4 made unreachable, and then IncCollect is run several times
		TEST_METHOD(ManagedArrayTest1)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			std::vector<SmartHandle<EmptyDummy> > hndlVec(8);
			SmartHandle<EmptyDummy> h1 = gcnew(Managed_Array<EmptyDummy>)(8);

			for (int i = 0 ; i < 8; ++i)
			{
				hndlVec[i] = &(h1[i]);
			}

			EmptyDummy* origh0 = &(*(hndlVec[0]));
			EmptyDummy* origh1 = &(*(hndlVec[1]));
			EmptyDummy* origh2 = &(*(hndlVec[2]));
			EmptyDummy* origh3 = &(*(hndlVec[3]));
			EmptyDummy* origh4 = &(*(hndlVec[4]));
			EmptyDummy* origh5 = &(*(hndlVec[5]));
			EmptyDummy* origh6 = &(*(hndlVec[6]));
			EmptyDummy* origh7 = &(*(hndlVec[7]));

			spy.FullCollection();

			hndlVec[0] = NULL;
			hndlVec[1] = NULL;
			hndlVec[5] = NULL;
			hndlVec[7] = NULL;

			GC::SetCollectorBudget(0.1);

			for (int i = 0; i < 1000; ++i)
			{
				GC::IncCollect();
			}

			EmptyDummy* newh2 = &(*(hndlVec[2]));
			EmptyDummy* newh3 = &(*(hndlVec[3]));
			EmptyDummy* newh4 = &(*(hndlVec[4]));
			EmptyDummy* newh6 = &(*(hndlVec[6]));


			Assert::AreEqual((char*)origh2, (char*)newh2);
			Assert::AreEqual((char*)origh3, (char*)newh3);
			Assert::AreEqual((char*)origh4, (char*)newh4);
			Assert::AreEqual((char*)origh6, (char*)newh6);


			GC::DestroyHeap();
		}

		//Test IncCollect where a Managed_Array of 8 objects are created, 
		//4 made unreachable, and then IncCollect is run several times
		//This time with the Managed_Array object also made unreachable
		TEST_METHOD(ManagedArrayTest2)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy));

			TestSpy spy = TestSpy();

			std::vector<SmartHandle<EmptyDummy> > hndlVec(8);
			SmartHandle<EmptyDummy> h1 = gcnew(Managed_Array<EmptyDummy>)(8);

			for (int i = 0 ; i < 8; ++i)
			{
				hndlVec[i] = &(h1[i]);
			}

			EmptyDummy* origh0 = &(*(hndlVec[0]));
			EmptyDummy* origh1 = &(*(hndlVec[1]));
			EmptyDummy* origh2 = &(*(hndlVec[2]));
			EmptyDummy* origh3 = &(*(hndlVec[3]));

			spy.FullCollection();

			hndlVec[0] = NULL;
			hndlVec[1] = NULL;
			hndlVec[5] = NULL;
			hndlVec[7] = NULL;
			h1 = NULL;

			GC::SetCollectorBudget(0.1);

			for (int i = 0; i < 1000; ++i)
			{
				GC::IncCollect();
			}

			EmptyDummy* newh0 = (EmptyDummy*)((char*)&(*(hndlVec[2])) + sizeof(Managed_Array<EmptyDummy>));
			EmptyDummy* newh1 = (EmptyDummy*)((char*)&(*(hndlVec[3])) + sizeof(Managed_Array<EmptyDummy>));
			EmptyDummy* newh2 = (EmptyDummy*)((char*)&(*(hndlVec[4])) + sizeof(Managed_Array<EmptyDummy>));
			EmptyDummy* newh3 = (EmptyDummy*)((char*)&(*(hndlVec[6])) + sizeof(Managed_Array<EmptyDummy>));


			Assert::AreEqual((char*)origh0, (char*)newh0);
			Assert::AreEqual((char*)origh1, (char*)newh1);
			Assert::AreEqual((char*)origh2, (char*)newh2);
			Assert::AreEqual((char*)origh3, (char*)newh3);

			GC::DestroyHeap();
		}

		//Test IncCollect where 8 objects are created referenced by handles in a handle array, 
		//4 made unreachable, and then IncCollect is run several times
		TEST_METHOD(HandleArrayTest1)
		{
			GC::CreateHeap(10 * sizeof(EmptyDummy) + 10 * sizeof(SmartHandle<EmptyDummy>) + sizeof(Handle_Array<EmptyDummy>));

			TestSpy spy = TestSpy();

			HndlArrHandle<EmptyDummy> hndlVec = gcnew(Handle_Array<EmptyDummy>)(8);

			for (int i = 0 ; i < 8; ++i)
			{
				hndlVec[i] = gcnew(EmptyDummy)();
			}

			EmptyDummy* origh0 = &(*(hndlVec[0]));
			EmptyDummy* origh1 = &(*(hndlVec[1]));
			EmptyDummy* origh2 = &(*(hndlVec[2]));
			EmptyDummy* origh3 = &(*(hndlVec[3]));

			spy.FullCollection();

			hndlVec[0] = NULL;
			hndlVec[1] = NULL;
			hndlVec[5] = NULL;
			hndlVec[7] = NULL;

			GC::SetCollectorBudget(0.1);

			for (int i = 0; i < 1000; ++i)
			{
				GC::IncCollect();
			}

			EmptyDummy* newh0 = &(*(hndlVec[2]));
			EmptyDummy* newh1 = &(*(hndlVec[3]));
			EmptyDummy* newh2 = &(*(hndlVec[4]));
			EmptyDummy* newh3 = &(*(hndlVec[6]));


			Assert::AreEqual((char*)origh0, (char*)newh0);
			Assert::AreEqual((char*)origh1, (char*)newh1);
			Assert::AreEqual((char*)origh2, (char*)newh2);
			Assert::AreEqual((char*)origh3, (char*)newh3);

			GC::DestroyHeap();
		}
	};
}