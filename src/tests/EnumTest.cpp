#include "TestShared.h"
#include "common/Enum.h"

class FooEnum: public Enum<FooEnum> {
public:
	FooEnum (const std::string& test = "test") :
			Enum<FooEnum>(test)
	{
	}
};
class BarEnum: public Enum<BarEnum> {
public:
	BarEnum (const std::string& test = "test2") :
			Enum<BarEnum>(test)
	{
	}
};

TEST(EnumTest, testNew) {
	FooEnum test_1;
	FooEnum test_2;
	ASSERT_EQ(1, test_1.id);
	ASSERT_EQ(2, test_2.id);
	ASSERT_EQ(test_1, test_1);
	ASSERT_NE(test_2, test_1);
	ASSERT_EQ(FooEnum::get(1), test_1);

	BarEnum test2_1;
	BarEnum test2_2;
	ASSERT_EQ(1, test2_1.id);
	ASSERT_EQ(2, test2_2.id);
	ASSERT_EQ(test2_1, test2_1);
	ASSERT_NE(test2_2, test2_1);
	ASSERT_EQ(BarEnum::get(1), test2_1);

	//ASSERT_NE(BarEnum::get(1), FooEnum::get(1));
}
