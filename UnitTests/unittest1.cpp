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
		}

		TEST_METHOD(TestMethod2)
		{
			Assert::IsTrue(convert_C_pF(0) == 29);
			Assert::IsTrue(convert_C_pF(1) == 44+29);
			Assert::IsTrue(convert_C_pF(0x0800) == 1538 + 29);
			Assert::IsTrue(convert_C_pF(0x0801) == 1538 + 44 + 29);
			Assert::IsTrue(convert_C_pF(0x0F00) == 3280);
			Assert::IsTrue(convert_C_pF(0x0FFF) == 3954);
			

			Assert::IsTrue(convert_L_nH(0) == 280);
			Assert::IsTrue(convert_L_nH(1) == 650 + 280);
			Assert::IsTrue(convert_L_nH(0x0800) == 23150 + 280);
			Assert::IsTrue(convert_L_nH(0x0801) == 23150 + 650 + 280);
			Assert::IsTrue(convert_L_nH(0x0F00) == 46540);
			Assert::IsTrue(convert_L_nH(0x0FFF) == 55220);
		}
	};
}