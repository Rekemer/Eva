#include <filesystem>
#include <string>
#include <fstream>
#include <unordered_map>
#include "Value.h"
#include "Serialize.h"
#include "Log.h"

struct TestCase {
    std::string name;
    std::string filePath;
    std::unordered_map<std::string, ValueContainer> expected;
};

/*
    \\ is \ in " " for cpp

    cmd doesn't recognise / slashes at all, need to use \ 
*/ 
const std::string_view COMPILER_PATH = ".\\..\\bin\\Eva-Compiler\\Debug-windows-x86_64\\Eva-Compiler.exe";
const std::string_view VM_PATH = ".\\..\\bin\\Eva-VM\\Debug-windows-x86_64\\Eva-VM.exe";
const std::string_view TEST_CASES_DIR = ".\\tests\\";
// Function to discover all test cases
std::vector<TestCase> DiscoverTestCases(const std::string_view testDirectory) {
    std::vector<TestCase> testCases;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(testDirectory)) {
        if (!entry.is_directory()) {
            std::string testName = entry.path().filename().string();
            std::string filePath = entry.path().string();
            testCases.push_back(TestCase{ testName, filePath });
        }
    }
    return testCases;
}

bool IsFloat(const std::string_view number)
{
    return (number.find('.') != std::string::npos);
}

// stops iterator on value and , if there is no whitespace
// a = 2,
void FillTest(TestCase& test, std::istream_iterator<std::string>& it)
{
    
    std::string name{ *it };
    // skip name
    it++;
    // skip =
    it++;
    std::string_view vString = (*it);
    if ((*it)[it->size() - 1] == ',')
    {
        vString  = vString.substr(0, it->size() - 1);
    }

    if (vString[0] == '\"')
    {
        // parse string

        std::string_view str = vString.substr(1, vString.size() - 2);
        test.expected[name] = ValueContainer{ std::string{str}};
    }
    else
    {
        ValueContainer v = IsFloat(vString)
            ? ValueContainer{ std::stof(std::string{vString}) }
        : ValueContainer{ std::stoi(std::string{vString}) };

        test.expected[name] = v;
    }
    

}

int GetExpectedData(TestCase& test)
{
    std::ifstream scriptFile(test.filePath);

    if (scriptFile.is_open())
    {
        std::istream_iterator<std::string> start(scriptFile), end;
        auto it = start;
        while (*it != "/*")
        {
            it++;
        }
        auto parseTest = false;
        for (; *it != "*/"; ) {
            if ("#test" == *it)
            {
                parseTest = true;
                it++;
            }
            if (parseTest)
            {
                FillTest(test, it);
                //if (*it == "," || (*it)[it->size() - 1] == ',')
                //{
                //    it++;
                //}
                //else
                //{
                //}
              //      it++;
            }
            //else
            //{
                it++;
                // we can write  a = 2, and  a = 2 ,
                if ((*it) == ",") it++;
            //}
        }



        //auto ret = compiler.Compile(contents.data());
        //return ret;
    }
    return 0;
}

int main()
{
    auto cases = DiscoverTestCases(TEST_CASES_DIR);
    auto str = std::filesystem::current_path();
    for (auto& caseTest : cases)
    {
        // preproccess test data
        GetExpectedData(caseTest);

        // compile
        {
            auto currentPath = std::filesystem::current_path();
            std::string command = std::format("{} -spath=\"{}\" -bpath=\"{}\" > error_log_cmp.txt 2>&1", COMPILER_PATH, caseTest.filePath, "./test.evc");
            int result = system(command.c_str());
            assert(result == 0);
        }
        // run 
        {
            std::string command = std::format("{} -test {} > error_log_vm.txt 2>&1", VM_PATH, "./test.evc");
            int result = system(command.c_str());
            assert(result == 0);
        }
        // check
        std::ifstream dumpFile(".\\dumpGlobal.json");
        if (!dumpFile.is_open())
        {
            Log::GetLog()->error("VM didn't produce dump file\n");
            return -1;
        }
        HashTable real;
        {
            cereal::JSONInputArchive iarchive{ dumpFile };
            iarchive(real);
        }
        //for (auto e : real)
        //{
        //    std::cout << e.key;
        //}

        auto& expected = caseTest.expected;
        bool isFailed = false;
        std::vector<std::string> msgs;
        for (auto e : expected)
        {
            auto& eKey = e.first;
            auto& eValue = e.second;
            if (!real.IsExist(eKey))
            {
                Log::GetLog()->error("the test {} is not specifed correctly", caseTest.filePath);
                break;
            }
            auto realEntry = real.Get(eKey);
            if (realEntry->value != eValue)
            {
                isFailed = true;
                msgs.push_back(std::format("{} ({}) is not equal to {}", eKey, realEntry->value.ToString(), eValue.ToString()));
            }
        }
        if (isFailed)
        {
            Log::GetLog()->error("the test {} failed ", caseTest.filePath);
            for (auto& msg : msgs)
            {
                Log::GetLog()->error(msg);
            }
        }
        else
        {
            Log::GetLog()->info("the test {} succeeded!", caseTest.filePath);
        }

    }

	return 0;
}