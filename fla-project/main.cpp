#include <iostream>
#include <sstream>
using namespace std;

class Logger
{
public:
  Logger() : logToStderr(false) {}

  void setLogToStderr(bool value) { logToStderr = value; }

  template <typename T>
  Logger &operator<<(const T &message)
  {
    if (logToStderr) {
      cerr << message;
    } else {
      // No output here
    }
    return *this;
  }

  Logger &operator<<(std::ostream &(*manip)(std::ostream &))
  {
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

void print_help()
{
  printf("usage: fla [-h|--help] <pda> <input> \n\
            fla [-v|--verbose] [-h|--help] <tm> <input>");
}

struct Args
{
  bool   verbose;
  string file_path;
  string input;
};

void print_args(struct Args &args, Logger &logger)
{
  logger << "verbose: " << args.verbose << endl;
  logger << "file_path: " << args.file_path << endl;
  logger << "input: " << args.input << endl;
}

struct Args parse_args(int argc, char *argv[], Logger &logger)
{
  if (argc == 1) {
    logger << "No arguments provided" << endl;
    exit(1);
  }
  Args args;
  bool accept_file = false;
  bool paresd_all  = false;
  for (size_t i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_help();
      exit(0);
    } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
      args.verbose = true;
      logger.setLogToStderr(true);
    } else {
      if (paresd_all) {
        logger << "unexpected argument: " << argv[i] << endl;
        throw runtime_error("Unexpected argument");
      }
      if (!accept_file) {
        args.file_path = argv[i];
        accept_file    = true;
      } else {
        args.input = argv[i];
        paresd_all = true;
      }
    }
  }
  if (!paresd_all) {
    logger << "too less argument" << endl;
    throw runtime_error("too less argument");
  }
  return args;
}

int main(int argc, char *argv[])
{
  Logger logger;
  logger.setLogToStderr(true);

  struct Args args = parse_args(argc, argv, logger);
  print_args(args, logger);
  logger << "This is for testing" << endl;
  return 0;
}