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

// Start file reading
#include <fstream>

enum fla_type
{
  TM,
  PDA
};

struct fla_content
{
  fla_type type;
  string   content;
};

struct fla_content load_fla_file(const string &file_path)
{
  struct fla_content ret;
  if (file_path.substr(file_path.find_last_of(".") + 1) == "tm") {
    ret.type = TM;
  } else if (file_path.substr(file_path.find_last_of(".") + 1) == "pda") {
    ret.type = PDA;
  } else {
    throw runtime_error("Invalid file type");
  }
  ifstream file(file_path);
  if (!file.is_open()) {
    string error = "Unable to open file: " + file_path;
    throw runtime_error(error);
  }
  stringstream buffer;
  buffer << file.rdbuf();
  ret.content = buffer.str();
  return ret;
}
// End file reading
int main(int argc, char *argv[])
{
  Logger debug_logger;
  debug_logger.setLogToStderr(true);
  struct Args args;

  try {
    args = parse_args(argc, argv, debug_logger);
    print_args(args, debug_logger);
  } catch (const exception &e) {
    cerr << "Error: " << e.what() << endl;
    exit(EXIT_FAILURE);
  }

  try {
    struct fla_content content = load_fla_file(args.file_path);
    debug_logger << "File type: " << content.type << "\nFile content: " << content.content << endl;

  } catch (const exception &e) {
    cerr << "Error: " << e.what() << endl;
    exit(EXIT_FAILURE);
  }
  return 0;
}