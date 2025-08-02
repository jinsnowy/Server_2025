#include "pch.h"
#include "CppUnitTest.h"
#include "Core/System/Delegate.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace System {

}
namespace System
{
	TEST_CLASS(DelegateTest)
	{
	public:
		
		TEST_METHOD(BindFunction_OneParam)
		{
			Delegate<int> delegate;
			int value = 0;
			delegate.BindFunction([&value](int v) {
				value = v;
			});
			delegate.Execute(42);
			Assert::AreEqual(42, value);
			delegate.ExecuteIfBound(100);
			Assert::AreEqual(100, value);
			Assert::IsTrue(delegate.IsBound());
		}

		TEST_METHOD(BindFunction_MemberFunc)
		{
			class A {
			public:
				A(int& ref_value)
					:
					value(ref_value) {
				}
				void Method(int v) {
					value = v;
				}
				int& value;
			};

			int value = 0;
			Delegate<int> delegate;

			auto a = std::make_shared<A>(value);

			delegate.BindWeak(a, &A::Method);
			delegate.Execute(42);
			Assert::AreEqual(42, value);
			std::weak_ptr<A> weak_a = a;
			a.reset();
			delegate.Execute(100);
			Assert::IsTrue(weak_a.expired());
			Assert::AreEqual(42, value); // Should not change since weak pointer expired
		}

		TEST_METHOD(BindFunction_MemberFunc_Strong)
		{
			class A {
			public:
				A(int& ref_value)
					:
					value(ref_value) {
				}
				void Method(int v) {
					value = v;
				}
				int& value;
			};
			int value = 0;
			Delegate<int> delegate;
			auto a = std::make_shared<A>(value);
			delegate.BindSP(a, &A::Method);
			delegate.Execute(42);
			Assert::AreEqual(42, value);
			a.reset();
			delegate.Execute(100);
			Assert::AreEqual(100, value); // Should change since strong pointer was used
		}

		TEST_METHOD(Unbind)
		{
			Delegate<int> delegate;
			int value = 0;
			delegate.BindFunction([&value](int v) {
				value = v;
			});
			delegate.Execute(42);
			Assert::AreEqual(42, value);
			delegate.Unbind();
			delegate.ExecuteIfBound(100);
			Assert::AreEqual(42, value); // Should not change since unbound
			Assert::IsFalse(delegate.IsBound());
		}
	};

	TEST_CLASS(MulticastDelegateTest)
	{
		TEST_METHOD(BindFunction_OneParam)
		{
			MulticastDelegate<int> delegate;
			int value1 = 0, value2 = 0;
			delegate.BindFunction([&value1](int v) {
				value1 = v;
			});
			delegate.BindFunction([&value2](int v) {
				value2 = v;
			});
			delegate.Broadcast(42);
			Assert::AreEqual(42, value1);
			Assert::AreEqual(42, value2);
		}

		TEST_METHOD(BindFunction_MemberFunc)
		{
			class A {
			public:
				A(int& ref_value)
					:
					value(ref_value) {
				}
				void Method(int v) {
					value = v;
				}
				int& value;
			};
			int value1 = 0, value2 = 0;
			MulticastDelegate<int> delegate;
			auto a1 = std::make_shared<A>(value1);
			auto a2 = std::make_shared<A>(value2);
			delegate.BindWeak(a1, &A::Method);
			delegate.BindWeak(a2, &A::Method);
			delegate.Broadcast(42);
			Assert::AreEqual(42, value1);
			Assert::AreEqual(42, value2);
			std::weak_ptr<A> weak_a1 = a1;
			std::weak_ptr<A> weak_a2 = a2;
			a1.reset();
			a2.reset();
			delegate.Broadcast(100);
			Assert::IsTrue(weak_a1.expired());
			Assert::IsTrue(weak_a2.expired());
			Assert::AreEqual(42, value1); // Should not change since weak pointer expired
			Assert::AreEqual(42, value2); // Should not change since weak pointer expired
		}

		TEST_METHOD(BindFunction_MemberFunc_Strong)
		{
			class A {
			public:
				A(int& ref_value)
					:
					value(ref_value) {
				}
				void Method(int v) {
					value = v;
				}
				int& value;
			};
			int value1 = 0, value2 = 0;
			MulticastDelegate<int> delegate;
			auto a1 = std::make_shared<A>(value1);
			auto a2 = std::make_shared<A>(value2);
			delegate.BindSP(a1, &A::Method);
			delegate.BindSP(a2, &A::Method);
			delegate.Broadcast(42);
			Assert::AreEqual(42, value1);
			Assert::AreEqual(42, value2);
			a1.reset();
			a2.reset();
			delegate.Broadcast(100);
			Assert::AreEqual(100, value1); // Should change since strong pointer was used
			Assert::AreEqual(100, value2); // Should change since strong pointer was used
		}

		TEST_METHOD(Remove)
		{
			MulticastDelegate<int> delegate;
			int value1 = 0, value2 = 0;
			auto callback1 = delegate.BindFunction([&value1](int v) {
				value1 = v;
				});
			auto callback2 = delegate.BindFunction([&value2](int v) {
				value2 = v;
				});
			delegate.Broadcast(42);
			Assert::AreEqual(42, value1);
			Assert::AreEqual(42, value2);

			delegate.Remove(callback1);
			delegate.Broadcast(100);
			Assert::AreEqual(42, value1); // Should not change since callback1 was removed
			Assert::AreEqual(100, value2); // Should change since callback2 is still bound

			Assert::IsTrue(delegate.Size() == 1);
			Assert::IsFalse(delegate.IsEmpty());

			delegate.Remove(callback2);
			delegate.Broadcast(200);

			Assert::AreEqual(42, value1); // Should not change since callback2 was removed
			Assert::AreEqual(100, value2); // Should not change since callback2 was removed
			Assert::IsTrue(delegate.Size() == 0);
			Assert::IsTrue(delegate.IsEmpty());
		}

		TEST_METHOD(Clear)
		{
			MulticastDelegate<int> delegate;
			int value1 = 0, value2 = 0;
			auto callback1 = delegate.BindFunction([&value1](int v) {
				value1 = v;
			});
			auto callback2 = delegate.BindFunction([&value2](int v) {
				value2 = v;
			});
			delegate.Broadcast(42);
			Assert::AreEqual(42, value1);
			Assert::AreEqual(42, value2);
			
			delegate.Clear();
			Assert::IsTrue(delegate.Size() == 0);
			Assert::IsTrue(delegate.IsEmpty());
		}

	};
}
