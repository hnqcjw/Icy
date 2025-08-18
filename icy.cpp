#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map> // For hashmap
#include <vector>
#include <cstdlib> // for std::stoi
#include <unistd.h> // For sleep()

struct Function { // Struct for storing functions
    std::string name; // string for name of function
    std::vector<std::string> body; // stores actual code
};

std::unordered_map<std::string, Function> functions; // Creates a hashmap for function names using struct Function format
std::unordered_map<std::string, std::string> variables; // Create a hashmap for variable names

// declare globalLines here so executeLine can use it
std::vector<std::string> globalLines;

void executePrintLn(const std::string& argument) { // gets the argument from two ()s showed in line 61
    if (argument.size() >= 2 && argument.front() == '"' && argument.back() == '"') { // gets 2 double-quotes
        std::cout << argument.substr(1, argument.size() - 2) << std::endl;
    } else {
        auto it = variables.find(argument); // Finds variabels
        if (it != variables.end()) {
            std::cout << it->second << std::endl;
        } else {
            std::cout << "undefined variable: " << argument << std::endl;
        }
    }
}

void executeSys(const std::string& argument) {
    if (argument.size() >= 2 && argument.front() == '"' && argument.back() == '"') {
        std::string command = argument.substr(1, argument.size() - 2);
        int result = system(command.c_str()); // Use c_str() to convert to const char* array
        if (result != 0) {
            std::cerr << "Error: Command execution failed with code " << result << std::endl;
        }
    } else {
        std::cerr << "Error: Invalid argument format for sys(). Expected format: \"command\"." << std::endl;
    }
}

void executePause(const std::string& argument) {
    if (argument.size() >= 2 && argument.front() == '"' && argument.back() == '"') {
        std::string command = argument.substr(1, argument.size() - 2);
        int result = sleep(std::stoi(command));
        if (result != 0) {
            std::cerr << "Error: Command execution failed with code " << result << std::endl;
        }
    } else {
        std::cerr << "Error: Invalid argument format for sys(). Expected format: \"command\"." << std::endl;
    }
}



// Forward declaration of executeLine with currentLine param
void executeLine(const std::string& line, size_t& currentLine);

void executeFunction(const std::string& name) {
    auto it = functions.find(name);
    if (it == functions.end()) {
        std::cerr << "Error: function " << name << " not found." << std::endl;
        return;
    }
    
    size_t currentLine = 0; // Initialize currentLine for the function execution
    for (const auto& line : it->second.body) {
        executeLine(line, currentLine); // Pass currentLine as an argument
    }
}



void executeLine(const std::string& line, size_t& currentLine) { // From line 47
    std::string trimmed = line;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r")); // Trim off tabs, newlines, spaces and carriage returns ( usually for newlines )
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

    size_t commentPos = trimmed.find("//"); // Gets rid of anything after a // in a line
    if (commentPos != std::string::npos) {
        trimmed = trimmed.substr(0, commentPos);
        trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);
    }

    if (trimmed.empty()) return;

    if (trimmed.find("printLn(") == 0) { // Finds string "printLn", and finds ()s. similar process to sys abd goToLn
        size_t start = trimmed.find('(');
        size_t end = trimmed.rfind(')');
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string arg = trimmed.substr(start + 1, end - start - 1);
            executePrintLn(arg);
        }
        return;
    }

    if (trimmed.find("sys(") == 0) {
        size_t start = trimmed.find('(');
        size_t end = trimmed.rfind(')');
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string arg = trimmed.substr(start + 1, end - start - 1);
            executeSys(arg);
        }
        return;
    }


    if (trimmed.find("pause(") == 0) {
        size_t start = trimmed.find('(');
        size_t end = trimmed.rfind(')');
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string arg = trimmed.substr(start + 1, end - start - 1);
            executePause(arg);
        }
        return;
    }


    if (trimmed.find("goToLn(") == 0) {
        size_t start = trimmed.find('(');
        size_t end = trimmed.rfind(')');
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string arg = trimmed.substr(start + 1, end - start - 1);
            try {
                int targetLine = std::stoi(arg);
                if (targetLine >= 1 && (size_t)targetLine <= globalLines.size()) {
                    currentLine = targetLine - 2; // Adjust for zero index and for loop increment
                } else {
                    std::cerr << "Error: goToLn target line out of range." << std::endl;
                }
            } catch (...) {
                std::cerr << "Error: invalid goToLn argument." << std::endl;
            }
        }
        return;
    }

    if (trimmed.back() == ';' && trimmed.find('(') != std::string::npos) {
        size_t parenPos = trimmed.find('(');
        std::string fname = trimmed.substr(0, parenPos); // Declare "fname" ( function name )
        executeFunction(fname); // Execute function w/ fname
        return;
    }
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: No Input Files!" << std::endl;
        return 1;
    }

    std::ifstream sourceFile(argv[1]);
    if (!sourceFile.is_open()) {
        std::cerr << "Error: Could not open file!" << argv[1] << std::endl;
        return 1;
    }

    bool inFunction = false;
    Function currentFunction;
    std::string line;

    while (std::getline(sourceFile, line)) { // Continues program until no more lines
        std::string trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r")); // trim off spaces, tabs, newlines and carriage returns ( mostly part of newlines )
        trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

        if (!inFunction) {
            if (trimmed.find("func ") == 0) { // If starts with func, uses a struct declared earlier in the code
                size_t nameStart = 5;
                size_t nameEnd = trimmed.find('(', nameStart);
                if (nameEnd != std::string::npos) {
                    std::string fname = trimmed.substr(nameStart, nameEnd - nameStart);
                    currentFunction = Function{}; // Use struct
                    currentFunction.name = fname; // Uses fname from earlier as the name of function
                    inFunction = true;
                }
            } else {
                globalLines.push_back(trimmed);
            }
        } else {
            if (trimmed == "}") {
                functions[currentFunction.name] = currentFunction;
                inFunction = false;
            } else {
                currentFunction.body.push_back(trimmed);
            }
        }
    }
    sourceFile.close();

    for (size_t i = 0; i < globalLines.size(); ++i) {
        executeLine(globalLines[i], i);
    }

    return 0;
}
