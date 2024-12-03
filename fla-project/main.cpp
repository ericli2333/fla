#include <iostream>
#include <sstream>
using namespace std;

class Logger {
public:
    Logger() : logToStderr(false) {}

    void setLogToStderr(bool value) {
        logToStderr = value;
    }

    template<typename T>
    Logger& operator<<(const T& message) {
        if (logToStderr) {
            cerr << message;
        } else {
            // No output here
        }
        return *this;
    }

    Logger& operator<<(std::ostream& (*manip)(std::ostream&)) {
        if (logToStderr) {
            manip(cerr);
        } else {
            // No output here
        }
        return *this;
    }

private:
    bool logToStderr;
};

int parse_args(int argc, char* argv[], Logger& logger) {
    if (argc == 1) {
        logger << "No arguments provided" << endl;
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[]){
    Logger logger;
    logger.setLogToStderr(false); // 设置是否输出到stderr

    if(argc == 1){
        return 1;
    }
    logger << "This is for testing" << endl;
    return 0;
}