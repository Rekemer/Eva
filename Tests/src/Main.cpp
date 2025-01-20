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

bool IsStringTrue(const std::string_view str)
{
    return str == "true" && str.size() == 4;
}

bool IsStringFalse(const std::string_view str)
{
    return str == "false" && str.size() == 5;
}

bool IsBool(const std::string_view str)
{
    return IsStringTrue(str) || IsStringFalse(str);
}

bool StringToBool(const std::string_view str)
{
    if (str == "true") return true;
    if (str == "false") return false;
}

bool IsFloat(const std::string_view number)
{
    return (number.find('.') != std::string::npos);
}

// stops iterator on value and , if there is no whitespace
// a = 2,
void FillTest(TestCase& test,std::string_view variableName, std::string_view line)
{
    
    auto name = std::string{ variableName };
    auto index = 0l;
    while (line[index] != '=' || line[index] == ' ')
    {
        index++;
    }
    index++;
    while (line[index] == ' ')
    {
        index++;
    }

    if (line[index] == '"')
    {
        index++;
        auto startIndex = index;
        // parse string
        bool stringParsed = false;
        while (line[index] != '"')
        {
            index++;
        }
        //index++;
        auto valueString = std::string{ line.substr(startIndex,index - startIndex ) };
        test.expected[name] = ValueContainer{ valueString };
    }
    else
    {   
        ValueContainer v;
        auto valueString = line.substr(index, line.size());
        auto isTrue = IsStringTrue(valueString.substr(0, 4));
        auto isFalse= IsStringFalse(valueString.substr(0, 5));
        if (isFalse)
        {
            v = ValueContainer{ false };
        }
        else if (isTrue)
        {
            v = ValueContainer{ true};
        }
        else  v = IsFloat(valueString)
            ? ValueContainer{ std::stof(std::string{valueString}) }
        : ValueContainer{ std::stoi(std::string{valueString}) };

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
                std::string lineValue;
                std::getline(scriptFile,lineValue);
                FillTest(test, *it, lineValue);
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
    TestCase t;
    t.filePath = "./tests/expressions/expression_2.eva";
    cases = { t };
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