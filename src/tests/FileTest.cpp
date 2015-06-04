#include "TestShared.h"
#include "common/FileSystem.h"
#include "common/Pointers.h"

class FileTest: public AbstractTest {
};

TEST_F(FileTest, testListDirectory) {
	const std::string path = "dirlisting/";
	const DirectoryEntries& campaigns = FS.listDirectory(path);
	ASSERT_TRUE(!campaigns.empty()) << FS.getDataDir() << path << " is empty or not readable";
}

TEST_F(FileTest, testFileLoad) {
	const std::string path = "dirlisting/";
	const DirectoryEntries& files = FS.listDirectory(path);
	ASSERT_TRUE(!files.empty()) << FS.getDataDir() << path << " is empty or not readable";
	for (const std::string file : files) {
		const std::string filePath = path + file;
		FilePtr f = FS.getFile(filePath);
		ASSERT_NE(-1, f->length()) << "Could not load " << FS.getDataDir() << filePath;
		char *buffer;
		int fileLen = f->read((void **) &buffer);
		ASSERT_NE(-1, fileLen) << "Could not read " << FS.getDataDir() << filePath;
		ScopedArrayPtr<char> p(buffer);
		ASSERT_TRUE(fileLen == 0 || buffer) << "could not read " << FS.getDataDir() << filePath;
		ASSERT_TRUE(fileLen >= 0) << "could not read " << FS.getDataDir() << filePath;
	}
}

TEST_F(FileTest, testFileWrite) {
	std::string filename = "temp.test";
	const std::string testStr = "testFileWrite";
	const unsigned char *buf =
			reinterpret_cast<const unsigned char *>(testStr.c_str());
	const size_t length = testStr.size();
	ASSERT_TRUE(FS.writeFile(filename, buf, length, true) != -1L) << "Failed to write file " << filename;
	ASSERT_TRUE(FS.writeFile(filename, buf, length, false) == -1L) << "Failed to not write file " << filename;
	ASSERT_TRUE(FS.writeFile(filename, buf, length, true) != -1L) << "Failed to write file " << filename;
}

TEST_F(FileTest, testFileSysWrite) {
	std::string filename = "temp.test";
	const std::string testStr = "testFileSysWrite";
	const unsigned char *buf =
			reinterpret_cast<const unsigned char *>(testStr.c_str());
	const size_t length = testStr.size();
	ASSERT_TRUE(FS.writeSysFile(filename, buf, length, true) != -1L) << "Failed to write file " << filename;
	ASSERT_TRUE(FS.writeSysFile(filename, buf, length, false) == -1L) << "Failed to not write file " << filename;
	ASSERT_TRUE(FS.writeSysFile(filename, buf, length, true) != -1L) << "Failed to write file " << filename;
}

TEST_F(FileTest, testDelete) {
	std::string filename = "delete.test";
	const std::string testStr = "testDelete";
	const unsigned char *buf =
			reinterpret_cast<const unsigned char *>(testStr.c_str());
	const size_t length = testStr.size();
	ASSERT_TRUE(FS.writeFile(filename, buf, length, true) != -1L) << "Failed to write file " << filename;
	ASSERT_TRUE(FS.deleteFile(filename)) << "Failed to delete file " << filename;
}

TEST_F(FileTest, testCopy) {
	std::string filename = "gamestate2.sqlite";
	std::string targetFilename = "testcopy.test.temp";
	const std::string testStr = "testCopy";
	const unsigned char *buf =
			reinterpret_cast<const unsigned char *>(testStr.c_str());
	const size_t length = testStr.size();
	ASSERT_TRUE(FS.writeFile(filename, buf, length, true) != -1L) << "Failed to write file " << filename;
	ASSERT_TRUE(FS.copy(filename, targetFilename)) << "Failed to copy file " << filename << " to " << targetFilename;
}

TEST_F(FileTest, testName) {
	std::string filename = "testdir/testname";
	FilePtr p = FS.getFile(filename);
	ASSERT_TRUE(p) << "Could not load " << filename;
	ASSERT_EQ(FS.getDataDir() + "testdir", p->getPath());
	ASSERT_EQ("testname", p->getFileName());
	ASSERT_EQ(FS.getDataDir() + filename, p->getName());
}
