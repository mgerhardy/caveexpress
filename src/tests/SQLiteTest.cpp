#include "TestShared.h"
#include "common/SQLite.h"

class SQLiteTest: public AbstractTest {
};

TEST_F(SQLiteTest, testOpen)
{
	SQLite s;
	ASSERT_TRUE(s.open()) << "could not open the sqlite db";
}

TEST_F(SQLiteTest, testCreateInsertAndSelect)
{
	SQLite s;
	ASSERT_TRUE(s.open()) << "could not open the sqlite db";
	ASSERT_TRUE(s.exec("CREATE TABLE Foobar	(ID INT PRIMARY KEY NOT NULL)")) << "failed to create table: " << s.getError();
	ASSERT_TRUE(s.exec("INSERT INTO Foobar (ID) VALUES ('1')")) << "failed to insert into table: " << s.getError();

	SQLiteStatement statement;
	ASSERT_TRUE(s.prepare(statement, "SELECT * FROM Foobar")) << "could not select from table: " << s.getError();
	ASSERT_EQ(SQLITE_ROW, statement.step()) << "no row selected: " << statement.getError();
	ASSERT_EQ(1, statement.getInt(0)) << "invalid value in the table: " << statement.getError();
	ASSERT_EQ(SQLITE_DONE, statement.step()) << "more than one entry in the database: " << statement.getError();
}

TEST_F(SQLiteTest, testError)
{
	SQLite s;
	ASSERT_TRUE(s.open()) << "could not open the sqlite db";
	ASSERT_FALSE(s.exec("CREATE SyntaxError")) << "expected syntax error";
	ASSERT_NE("", s.getError());
}
