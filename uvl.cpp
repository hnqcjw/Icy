#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map> // For hashmap
#include <vector>
#include <cstdlib> // for std::stoi

struct Function { // Struct for
    std::string name; // string for name of function
    std::vector<std::string> body; // stores actual code
};

std::unordered_map<std::string, Function> functions; // Creates a hashmap for function names using struct Function format
std::unordered_map<std::string, std::string> variables; // Create a hashmap for variable names

// Declare globalLines here, so executeLine can use it
std::vector<std::string> globalLines;

void executePrintLn(const std::string& argument) { // gets the argument from two ()s showed in line 61
    if (argument.size() >= 2 && argument.front() == '"' && argument.back() == '"') { // gets 2 quotes
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

// Forward declaration of executeLine with currentLine param
void executeLine(const std::string& line, size_t& currentLine);

void executeFunction(const std::string& name) {
    auto it = functions.find(name);
    if (it == functions.end()) {
        std::cerr << "Error: function " << name << " not found." << std::endl; // ErrorMsg to
        return;
    }
    for (const auto& line : it->second.body) {
        executeLine(line);
    }
}

void executeLine(const std::string& line, size_t& currentLine) {
    std::string trimmed = line;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

	size_t commentPos = trimmed.find("//");
		if (commentPos != std::string::npos) {
    	trimmed = trimmed.substr(0, commentPos);
    	// Trim again to clean trailing spaces after removing comment
    	trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);
	}

    if (trimmed.empty()) return;

    if (trimmed.find("printLn(") == 0) { // Finds printLn(s, declare start at ( and declares end at )
        size_t start = trimmed.find('(');
        size_t end = trimmed.rfind(')');
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string arg = trimmed.substr(start + 1, end - start - 1);
            executePrintLn(arg); // Execute executePrintLn function from earlier
        }
        return; // Terminate function
    }

    if (trimmed.find("goToLn(") == 0) { // goToLn() implementation, similar to printLn
        size_t start = trimmed.find('(');
        size_t end = trimmed.rfind(')');
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string arg = trimmed.substr(start + 1, end - start - 1);
            try {
                int targetLine = std::stoi(arg);
                if (targetLine >= 1 && (size_t)targetLine <= globalLines.size()) {
                    currentLine = targetLine - 2; // adjust for zero index + for loop increment
                } else {
                    std::cerr << "Error: goToLn target line out of range." << std::endl;
                }
            } catch (...) {
                std::cerr << "Error: invalid goToLn argument." << std::endl;
            }
        }
        return; // Terminate function
    }

    if (trimmed.back() == ';' && trimmed.find('(') != std::string::npos) {
        size_t parenPos = trimmed.find('(');
        std::string fname = trimmed.substr(0, parenPos);
        executeFunction(fname);
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

    while (std::getline(sourceFile, line)) {
        std::string trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
        trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

        if (!inFunction) {
            if (trimmed.find("func ") == 0) {
                size_t nameStart = 5;
                size_t nameEnd = trimmed.find('(', nameStart);
                if (nameEnd != std::string::npos) {
                    std::string fname = trimmed.substr(nameStart, nameEnd - nameStart);
                    currentFunction = Function{};
                    currentFunction.name = fname;
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
