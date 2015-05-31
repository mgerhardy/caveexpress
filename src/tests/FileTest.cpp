#include "tests/FileTest.h"
#include "common/FileSystem.h"
#include "common/Pointers.h"

class FileTest: public MapSuite {
};

TEST_F(FileTest, testListDirectory) {
	const std::string& campaignsPath = FS.getCampaignsDir();
	const DirectoryEntries& campaigns = FS.listDirectory(campaignsPath);
	ASSERT_TRUE(!campaigns.empty());
}

TEST_F(FileTest, testFileLoad) {
	const std::string& campaignsPath = FS.getCampaignsDir();
	const DirectoryEntries& campaigns = FS.listDirectory(campaignsPath);
	ASSERT_TRUE(!campaigns.empty());
	FilePtr f = FS.getFile(FS.getCampaignsDir() + *campaigns.begin());
	ASSERT_TRUE(f);
	char *buffer;
	int fileLen = f->read((void **) &buffer);
	ScopedArrayPtr<char> p(buffer);
	ASSERT_TRUE(buffer) << "could not read " + *campaigns.begin();
	ASSERT_TRUE(fileLen > 0) << "could not read " + *campaigns.begin();
}

TEST_F(FileTest, testFileWrite) {
	std::string filename = "temp.test";
	const std::string testStr = "testFileWrite";
	const unsigned char *buf =
			reinterpret_cast<const unsigned char *>(testStr.c_str());
	const size_t length = testStr.size();
	ASSERT_TRUE(FS.writeFile(filename, buf, length, true) != -1L);
	ASSERT_TRUE(FS.writeFile(filename, buf, length, false) == -1L);
	ASSERT_TRUE(FS.writeFile(filename, buf, length, true) != -1L);
}

TEST_F(FileTest, testFileSysWrite) {
	std::string filename = "temp.test";
	const std::string testStr = "testFileSysWrite";
	const unsigned char *buf =
			reinterpret_cast<const unsigned char *>(testStr.c_str());
	const size_t length = testStr.size();
	ASSERT_TRUE(FS.writeSysFile(filename, buf, length, true) != -1L);
	ASSERT_TRUE(FS.writeSysFile(filename, buf, length, false) == -1L);
	ASSERT_TRUE(FS.writeSysFile(filename, buf, length, true) != -1L);
}

TEST_F(FileTest, testDelete) {
	std::string filename = "delete.test";
	const std::string testStr = "testDelete";
	const unsigned char *buf =
			reinterpret_cast<const unsigned char *>(testStr.c_str());
	const size_t length = testStr.size();
	ASSERT_TRUE(FS.writeFile(filename, buf, length, true) != -1L);
	ASSERT_TRUE(FS.deleteFile(filename));
}

TEST_F(FileTest, testCopy) {
	std::string filename = "test/gamestate2.sqlite";
	std::string targetFilename = "test/testcopy.test.temp";
	const std::string testStr = "testCopy";
	const unsigned char *buf =
			reinterpret_cast<const unsigned char *>(testStr.c_str());
	const size_t length = testStr.size();
	ASSERT_TRUE(FS.writeFile(filename, buf, length, true) != -1L);
	ASSERT_TRUE(FS.copy(filename, targetFilename));
}

TEST_F(FileTest, testName) {
	std::string filename = "testdir/testname";
	FilePtr p = FS.getFile(filename);
	ASSERT_EQ(FS.getDataDir() + "testdir", p->getPath());
	ASSERT_EQ("testname", p->getFileName());
	ASSERT_EQ(FS.getDataDir() + filename, p->getName());
}
