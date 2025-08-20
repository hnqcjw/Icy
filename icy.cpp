#include <iostream> // Standard I/O (pretty obvious)
#include <fstream> // Open files
#include <string> // Strings (also pretty obvious)
#include <unordered_map> // Hashmap
#include <vector> // For vectors
#include <cstdlib> // For system()
#include <unistd.h> // sleep

struct Function { // Structure to define functions
    std::string name;
    std::vector<std::string> body;
};

std::unordered_map<std::string, Function> functions; // unordered map for all the functions
std::unordered_map<std::string, std::string> variables; // same with functions but for variables ( IMPLEMENTING SOON )
std::vector<std::string> globalLines;

std::string trim(const std::string& str) { // Function with std::string return type, trims spaces, tabs and newlines
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    return (first == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

void executePrintLn(const std::string& argument) {
    if (argument.size() >= 1 && argument.front() == '"' && argument.back() == '"') { // Gets the string in between the two double quotes
        std::cout << argument.substr(1, argument.size() - 2) << std::endl;
    } else {
        auto it = variables.find(argument);
        if (it != variables.end()) { // If no double quotes, tries to find a variable ( will also implement that soon )
            std::cout << it->second << std::endl;
        } else {
            std::cout << "undefined variable: " << argument << std::endl;
        }
    }
}

void executePrintF(const std::string& argument) {
    if (argument.size() >= 1 && argument.front() == '"' && argument.back() == '"') { // It's the same thing as printLn but doesn't print a newline, basically
        std::cout << argument.substr(1, argument.size() - 2);
    } else {
        auto it = variables.find(argument);
        if (it != variables.end()) {
            std::cout << it->second;
        } else {
            std::cout << "undefined variable: " << argument << std::endl;
        }
    }
}

void executeAsk(const std::string& argument) {
    std::string someString;
    if (argument.size() >= 1 && argument.front() == '"' && argument.back() == '"') {
        std::cout << argument.substr(1, argument.size() - 2);
        std::cin >> someString;
    } else {
        auto it = variables.find(argument);
        if (it != variables.end()) {
            std::cout << it->second;
            std::cin >> someString;
        } else {
            std::cout << "undefined variable: " << argument << std::endl;
        }
    }
}

void executeSys(const std::string& argument) {
    if (argument.size() >= 1 && argument.front() == '"' && argument.back() == '"') {
        std::string command = argument.substr(1, argument.size() - 2);
        int result = system(command.c_str());
        if (result == -1) {
            std::cerr << "Error: Command execution failed with code " << result << std::endl;
        }
    } else {
        std::cerr << "Error: Invalid argument format for sys(). Expected format: \"command\"." << std::endl;
    }
}

void executePause(const std::string& argument) {
    try {
        int command = std::stoi(argument);
        sleep(command);
    } catch (const std::invalid_argument&) {
        std::cerr << "Error: Invalid argument format for pause(). Expected format: pause(integer)" << std::endl;
    } catch (const std::out_of_range&) {
        std::cerr << "Error: Argument out of range for pause()." << std::endl;
    }
}

void executeLine(const std::string& line, size_t& currentLine);

void executeFunction(const std::string& name) {
    auto it = functions.find(name);
    if (it == functions.end()) {
        std::cerr << "Error: function " << name << " not found." << std::endl;
        return;
    }
    
    size_t currentLine = 0; // Initialize currentLine for the function execution
    for (const auto& line : it->second.body) {
        executeLine(line, currentLine);
    }
}

void executeGoToLn(const std::string& argument, size_t& currentLine) { 
        try {
            int targetLine = std::stoi(argument);
            if (targetLine >= 0 && static_cast<size_t>(targetLine) < globalLines.size()) {
                currentLine = targetLine - 1; // Adjust for zero index
            } else {
                std::cerr << "Error: goToLn target line out of range." << std::endl;
            }
        } catch (...) {
            std::cerr << "Error: invalid goToLn argument." << std::endl;
        }
}

void executeLine(const std::string& line, size_t& currentLine) {
    std::string trimmed = trim(line);
    size_t commentPos = trimmed.find("//");
    if (commentPos != std::string::npos) {
        trimmed = trimmed.substr(0, commentPos);
    }
    trimmed = trim(trimmed);
    if (trimmed.empty()) return;

    if (trimmed.find("printLn(") != std::string::npos) {
        size_t start = trimmed.find('(');
        size_t end = trimmed.rfind(')');
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string arg = trimmed.substr(start + 1, end - start - 1);
            executePrintLn(arg);
        }
        return;
    }

    if (trimmed.find("printF(") != std::string::npos) {
        size_t start = trimmed.find('(');
        size_t end = trimmed.rfind(')');
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string arg = trimmed.substr(start + 1, end - start - 1);
            executePrintF(arg);
        }
        return;
    }


    if (trimmed.find("ask(") != std::string::npos) {
        size_t start = trimmed.find('(');
        size_t end = trimmed.rfind(')');
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string arg = trimmed.substr(start + 1, end - start - 1);
            executeAsk(arg);
        }
        return;
    }

    if (trimmed.find("sys(") != std::string::npos) {
        size_t start = trimmed.find('(');
        size_t end = trimmed.rfind(')');
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string arg = trimmed.substr(start + 1, end - start - 1);
            executeSys(arg);
        }
        return;
    }
    if (trimmed.find("pause(") != std::string::npos) {
        size_t start = trimmed.find('(');
        size_t end = trimmed.rfind(')');
        if (start != std::string::npos && end != std::string::npos && end > start) {
            std::string arg = trimmed.substr(start + 1, end - start - 1);
            executePause(arg);
        }
        return;
    }


    if (trimmed.find("goToLn(") != std::string::npos) {
        size_t start = trimmed.find('(');
        size_t end = trimmed.rfind(')');
        std::string arg = trimmed.substr(start + 1, end - start - 1);
        if (start != std::string::npos && end != std::string::npos && end > start) {
        executeGoToLn(arg, currentLine);
        }
        return;
    }

    if (trimmed.back() == ';' && trimmed.find('(') != std::string::npos) {
        size_t parenPos = trimmed.find('(');
        std::string fname = trimmed.substr(0, parenPos);
        executeFunction(fname); // Execute function with fname
        return;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./icy <filename>" << std::endl;
        return 1; // Return 1 for error
    }

    std::ifstream sourceFile(argv[1]); // Use argv[1] for the input file
    if (!sourceFile.is_open()) {
        std::cerr << "Error: Could not open file: " << argv[1] << std::endl;
        return 1; // Return 1 for error
    }

    bool inFunction = false;
    Function currentFunction;
    std::string line;

    while (std::getline(sourceFile, line)) {
        std::string trimmed = trim(line);
        if (trimmed.empty()) continue; // Skip empty lines

        if (!inFunction) {
            if (trimmed.find("func ") == 0) { // Check if it starts with "func "
                size_t nameStart = 5; // Start after "func "
                size_t nameEnd = trimmed.find('(', nameStart);
                if (nameEnd != std::string::npos) {
                    std::string fname = trimmed.substr(nameStart, nameEnd - nameStart);
                    currentFunction = Function{fname, {}}; // Initialize function
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

    size_t currentLine = 0;
    while (currentLine < globalLines.size()) {
        size_t ogLine = currentLine;
        executeLine(globalLines[currentLine],currentLine);

        if (currentLine == ogLine) {
            currentLine++;
        }
    }

    return 0; // Return 0 for successful execution
}
