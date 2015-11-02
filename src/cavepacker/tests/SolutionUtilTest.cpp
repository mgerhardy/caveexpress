#include "tests/TestShared.h"
#include "cavepacker/shared/SolutionUtil.h"

namespace cavepacker {

TEST(SolutionUtilTest, testRLESimple)
{
	ASSERT_EQ("lulu", SolutionUtil::decompress("2(lu)"));
	ASSERT_EQ("lulurrrurururur", SolutionUtil::decompress("2(lu)3r4(ur)"));
	ASSERT_EQ("uu", SolutionUtil::decompress("2u"));
	ASSERT_EQ("uurrddlll", SolutionUtil::decompress("2u2r2d3l"));
	ASSERT_EQ("uuddrrruuddrrr", SolutionUtil::decompress("2(2u2d3r)"));
	ASSERT_EQ("uuddrrrluluuuddrrrlulu", SolutionUtil::decompress("2(2u2d3r2(lu))"));
	// nobody would... hopefully - but anyway
	ASSERT_EQ("uuuu", SolutionUtil::decompress("2(2u)"));
}

TEST(SolutionUtilTest, testRLE)
{
	ASSERT_NE("", SolutionUtil::decompress("6(lu)u16r7(dru3l)drdlu5(u3rddlrul)u3rddlruurrddldr3ulldlddrluruu3l3drlurluru3l"
			"3drlurluru3l3drlurluull3drl3ull3drdl4urrddrddlrul3urrdr3dlrulruluu3r4d3(lru)urr4dlrulr3urr4dlr4urr4dldr5u"
			"ll3dlddrlur4ullddl3drlurlur3ulldl4drlurl4ull5drl5ull5drdl6urr4drddlrul5urr3dr3dlrulruluuruurr6dlrulr5urr6"
			"dlr6urr6dldr7ull5dlddrlur6ull4dl3drlurlur5u3l7d3(rlu)r4u2(3l7d6(rlu)ru)3ldru14r14d10(3lurd)3luruld8(d3ruu"
			"lrdl)d3ruulrddrruulur3dlluluurldrdd3l3urldrldrd3l3urldrldrd3l3urldrldrd3l3urldrldrd3l3urldrldrd3l3urldrld"
			"rd3l3urldrlddll3url3dll3urul4drruurulrul4drrur3ulrdlrdld4(d3r4u3(lrd)l)d3r4u3(lrd)drr4ulrdlr3drr4ulr4drr4"
			"ulur5dll3uluurlddr3dlluul3urlddr3drruulrdd4lul4urlddrl3dll5u2(rldd)rd3l6u2(drld)drd3l5u2(rldd)rd3l2(urlu)"
			"urur3u3(rlu)ruu3l4drdru4dldl5dll2(urlu)u3(ru)dldldl5dll2(urlu)u4(ru)rlud4(dl)5dll2(urlu)u5(ru)d4(ld)l5dll"
			"3(urlluurd)rrururldld3l3urdrdull3u3(rd)ulull3urdrduluurrdlu3rdldlddrduluuruu8r4d4ldllululrdrdrrul5r4u5ldl"
			"r3dululrruuldu3rdlrddlr3u3rddllrrdd3luurdurrdllrr3ulldu4rddlrdllrrdlrddluur4dluur6u4(r4(rddl)r8u)r4(rddl)"
			"r4dlrddr6(ruul)"));
}

TEST(SolutionUtilTest, testCompressUncompress) {
	const std::string solution = "lllrrrddduuududlrldlrlud";
	const std::string rle = SolutionUtil::compress(solution);
	ASSERT_EQ("3l3r3d3ududlrldlrlud", rle);
	ASSERT_EQ(solution, SolutionUtil::decompress(rle));
}

}
