#include <gtest/gtest.h>

#include <fstream>
#include <stdexcept>
#include <string>

#include "parser.hh"

TEST(TestParser, ReadConfigWithCommentsAndWhitespace)
{
    const std::string path = "/tmp/hipo_parser_valid.config";
    std::ofstream out(path, std::ofstream::out | std::ofstream::trunc);
    out << "# comment\n";
    out << "ProjectName = demo_project\n";
    out << "nThreads=4\n";
    out << "batch_mode = TRUE # inline comment\n";
    out.close();

    Parser parser;
    EXPECT_TRUE(parser.ReadConfig(path));
    EXPECT_TRUE(parser.HasKey("ProjectName"));
    EXPECT_EQ(parser.GetSValue("ProjectName"), "demo_project");
    EXPECT_EQ(parser.GetIValue("nThreads"), 4);
    EXPECT_TRUE(parser.GetBValue("batch_mode"));
}

TEST(TestParser, MissingKeyThrows)
{
    const std::string path = "/tmp/hipo_parser_missing.config";
    std::ofstream out(path, std::ofstream::out | std::ofstream::trunc);
    out << "ProjectName=demo\n";
    out.close();

    Parser parser;
    ASSERT_TRUE(parser.ReadConfig(path));
    EXPECT_THROW(parser.GetSValue("UnknownKey"), std::runtime_error);
}

TEST(TestParser, MalformedLineFails)
{
    const std::string path = "/tmp/hipo_parser_invalid.config";
    std::ofstream out(path, std::ofstream::out | std::ofstream::trunc);
    out << "InvalidLineWithoutEquals\n";
    out.close();

    Parser parser;
    EXPECT_FALSE(parser.ReadConfig(path));
}
