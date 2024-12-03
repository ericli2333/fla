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

// Start PDA parsing
namespace pda {
enum PDA_STATEMENT_TYPE
{
  STATES,
  INPUT_ALPHABET,
  STACK_SYMBOLS,
  INITIAL_STATE,
  STACK_INITIAL_SYMBOL,
  FINAL_STATES,
  TRANSITION
};
struct PDA_STATEMENT
{
  PDA_STATEMENT_TYPE type;
  string             content;
};

struct PDA_STATEMENT get_statement(string &input)
{
  struct PDA_STATEMENT ret;
  if (input[0] == '#') {
    // Here is the first six cases
    switch (input[1]) {
      case 'Q':
        ret.type    = STATES;
        ret.content = input.substr(5);
        break;
      case 'S':
        ret.type    = INPUT_ALPHABET;
        ret.content = input.substr(5);
        break;
      case 'G':
        ret.type    = STACK_SYMBOLS;
        ret.content = input.substr(5);
        break;
      case 'q':
        if (input[2] != '0') {
          throw runtime_error("Invalid statement");
        }
        ret.type    = INITIAL_STATE;
        ret.content = input.substr(6);
        break;
      case 'z':
        if (input[2] != '0') {
          throw runtime_error("Invalid statement");
        }
        ret.type    = STACK_INITIAL_SYMBOL;
        ret.content = input.substr(6);
        break;
      case 'F':
        ret.type    = FINAL_STATES;
        ret.content = input.substr(5);
        break;
      default: throw runtime_error("Invalid statement");
    }
  } else {
    ret.type    = TRANSITION;
    ret.content = input;
  }
  return ret;
}

void parse_pda(string &input, Logger &logger)
{
  logger << "Start parsing pdas" << endl;
  stringstream ss(input);
  string       line;
  while (getline(ss, line, '\n')) {
    size_t pos = line.find(';');
    if (pos != string::npos) {
      line = line.substr(0, pos);
    }
    line.erase(line.find_last_not_of(" \n\r\t") + 1);
    line.erase(0, line.find_first_not_of(" \n\r\t"));
    if (!line.empty()) {
      logger << line << endl;
    }
  }
}
};  // namespace pda
// End PDA parsing

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

  struct fla_content content;
  try {
    content = load_fla_file(args.file_path);
    debug_logger << "File type: " << content.type << "\nFile content: " << content.content << endl;

  } catch (const exception &e) {
    cerr << "Error: " << e.what() << endl;
    exit(EXIT_FAILURE);
  }

  pda::parse_pda(content.content, debug_logger);
  return 0;
}