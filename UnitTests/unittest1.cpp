#include "CppUnitTest.h"
#include "atu.h"
#include "myutils.h"

#ifdef _MSC_VER
#pragma warning (disable: 4127)             // condition expression is constant
#pragma warning (disable: 4996)             // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			Assert::IsTrue(sizeof(U8)  == 1);
			Assert::IsTrue(sizeof(U16) == 2);
			Assert::IsTrue(sizeof(U32) == 4);
			Assert::IsTrue(sizeof(U64) == 8);

			Assert::IsTrue(sizeof(S8) == 1);
			Assert::IsTrue(sizeof(S16) == 2);
			Assert::IsTrue(sizeof(S32) == 4);
			Assert::IsTrue(sizeof(S64) == 8);

			char data[100];
			strcpy(data, "hello test");
			Assert::IsTrue(true);
		}

	};
}