#include <filesystem>
#include <string>
#include <fstream>
#include <unordered_map>
#include "Value.h"

struct TestCase {
    std::string name;
    std::string filePath;
    std::unordered_map<std::string, ValueContainer> expected;
};

const std::string TEST_CASES_DIR = "./tests";
// Function to discover all test cases
std::vector<TestCase> DiscoverTestCases(const std::string testDirectory) {
    std::vector<TestCase> testCases;
    for (const auto& entry : std::filesystem::directory_iterator(testDirectory)) {
        if (entry.is_directory()) {
            std::string testName = entry.path().filename().string();
            std::string filePath = entry.path().string() + "/test.eva";
            testCases.push_back(TestCase{ testName, filePath });
        }
    }
    return testCases;
}


int GetExpectedData(const TestCase& test)
{
    std::ifstream scriptFile(test.filePath);

    if (scriptFile.is_open())
    {
        std::stringstream sstream;
        sstream << scriptFile.rdbuf();
        std::string contents = sstream.str();

        auto ret = compiler.Compile(contents.data());
        return ret;
    }
}

int main()
{
    auto cases = DiscoverTestCases(TEST_CASES_DIR);
    for (auto& caseTest : cases)
    {
        // preproccess test data

        
        // compile

        // run 

        // check
    }

	return 0;
}