#include <iostream>
#include <list>
#include <stack>
#include <string>
#include <stdexcept>
#include <fstream>
#include <map>
#include <unordered_map>
#include <sstream>
#include <cstring>
#include <vector>
#include <utility>
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

class PDA_Wrapper
{
private:
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
  struct PDA_STATEMENT;

  struct PDA_STATEMENT
  {
    PDA_STATEMENT_TYPE type;
    string             content;
  };
  class PDA_STATE;

  void print_pda_statement(struct PDA_STATEMENT &statement, Logger &logger)
  {
    switch (statement.type) {
      case STATES: logger << "States: " << statement.content << endl; break;
      case INPUT_ALPHABET: logger << "Input Alphabet: " << statement.content << endl; break;
      case STACK_SYMBOLS: logger << "Stack Symbols: " << statement.content << endl; break;
      case INITIAL_STATE: logger << "Initial State: " << statement.content << endl; break;
      case STACK_INITIAL_SYMBOL: logger << "Stack Initial Symbol: " << statement.content << endl; break;
      case FINAL_STATES: logger << "Final States: " << statement.content << endl; break;
      case TRANSITION: logger << "Transition: " << statement.content << endl; break;
      default: logger << "Unknown statement type" << endl; break;
    }
  }

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

private:
  class PDA_STATE
  {
  private:
    struct pair_hash
    {
      template <class T1, class T2>
      std::size_t operator()(const std::pair<T1, T2> &p) const
      {
        auto hash1 = std::hash<T1>{}(p.first);
        auto hash2 = std::hash<T2>{}(p.second);
        // 合并两个哈希值
        return hash1 ^ (hash2 << 1);
      }
    };
    string                                                        name;
    bool                                                          is_accept;
    unordered_map<pair<char, char>, pair<int, string>, pair_hash> transitions;

  public:
    PDA_STATE(string name, bool is_accept) : name(name), is_accept(is_accept) {};
    PDA_STATE()                                  = default;
    PDA_STATE(const PDA_STATE &other)            = default;
    PDA_STATE &operator=(const PDA_STATE &other) = default;
    PDA_STATE(PDA_STATE &&other)                 = default;
    void   set_accept(bool value) { is_accept = value; }
    string get_name() { return name; }
    void   print_transitions(Logger &logger, vector<PDA_STATE> &state_list)
    {
      logger << "Transitions for state: " << name << endl;
      for (auto &transition : transitions) {
        logger << transition.first.first << " " << transition.first.second << " -> "
               << state_list[transition.second.first].get_name() << " " << transition.second.second << endl;
      }
    }
    void add_transition(char input, char stack_top, const string &state_name, string out_stack_symbol,
        unordered_map<string, int> &state_map)
    {
      auto it = state_map.find(state_name);
      if (it == state_map.end()) {
        string error = "State not found: " + state_name;
        throw runtime_error(error);
      }
      pair<char, char>  key(input, stack_top);
      pair<int, string> value(it->second, out_stack_symbol);
      transitions[key] = value;
    }
    pair<bool, pair<int, string>> get_transition(char input, char stack_top, list<PDA_STATE> &state_list)
    {
      pair<bool, pair<int, string>> ret(false, pair<int, string>(-1, ""));
      pair<char, char>              key(input, stack_top);
      auto                          it = transitions.find(key);
      if (it == transitions.end()) {
        return ret;
      } else {
        ret.first  = true;
        ret.second = it->second;
        return ret;
      }
    }
    string to_string()
    {
      string ret;
      ret += name;
      ret += " ";
      ret += is_accept ? "accept" : "normal";
      return ret;
    }
  };

private:
  bool create_state(string &name, bool is_accept)
  {
    if (state_map.find(name) != state_map.end()) {
      return false;
    }
    state_list.emplace_back(name, is_accept);
    state_map[name] = state_list.size() - 1;
    return true;
  }

private:
  void print_states(Logger &logger)
  {
    logger << "ALL parsed States:" << endl;
    for (auto &state : state_list) {
      logger << state.to_string() << endl;
    }
    logger << endl;
  }

private:
  // parsing the content start
  void parse_pda_statement(struct pda::PDA_Wrapper::PDA_STATEMENT &statement, Logger &logger)
  {
    switch (statement.type) {
      case STATES: {
        string       true_content = statement.content.substr(1, statement.content.size() - 2);
        stringstream ss(true_content);
        string       state;
        while (getline(ss, state, ',')) {
          bool success = create_state(state, false);
          if (!success) {
            string error = "State already exists: " + state;
            throw runtime_error(error);
          }
        }
        print_states(logger);
        break;
      }
      case INPUT_ALPHABET: {
        string       true_content = statement.content.substr(1, statement.content.size() - 2);
        stringstream ss(true_content);
        string       character;
        while (getline(ss, character, ',')) {
          if (character.size() != 1) {
            string error = "Invalid character: " + character;
            throw runtime_error(error);
          }
          input_alphabet.push_back(character[0]);
        }
        logger << "Input Alphabet paresed: ";
        for (char ch : input_alphabet) {
          logger << ch << " ";
        }
        logger << endl;
        break;
      }
      case STACK_SYMBOLS: {
        string       true_content = statement.content.substr(1, statement.content.size() - 2);
        stringstream ss(true_content);
        string       character;
        while (getline(ss, character, ',')) {
          if (character.size() != 1) {
            string error = "Invalid character: " + character;
            throw runtime_error(error);
          }
          stack_alphabet.push_back(character[0]);
        }
        logger << "Stack Alphabet paresed: ";
        for (char ch : stack_alphabet) {
          logger << ch << " ";
        }
        logger << endl;
        break;
      }
      case INITIAL_STATE: {
        initial_state = state_map[statement.content];
        logger << "parsed Initial state: " << state_list[initial_state].to_string() << endl;
        break;
      }
      case STACK_INITIAL_SYMBOL: {
        for (int idx = statement.content.length() - 1; idx >= 0; idx--) {
          pda_stack.push(statement.content[idx]);
        }
        logger << "parsed Stack Initial Symbol: ";
        while (!pda_stack.empty()) {
          logger << pda_stack.top() << endl;
          pda_stack.pop();
        }

        break;
      }
      case FINAL_STATES: {
        string       true_content = statement.content.substr(1, statement.content.size() - 2);
        stringstream ss(true_content);
        string       str;
        while (getline(ss, str, ',')) {
          auto it = state_map.find(str);
          if (it == state_map.end()) {
            string error = "State not found: " + str;
            throw runtime_error(error);
          }
          state_list[it->second].set_accept(true);
        }
        print_states(logger);
        break;
      }
      case TRANSITION: {
        stringstream   ss(statement.content);
        string         transition_part;
        vector<string> transition_parts;
        while (getline(ss, transition_part, ' ')) {
          transition_parts.push_back(transition_part);
        }
        if (transition_parts.size() != 5) {
          string error = "Invalid transition statement: " + statement.content;
          throw runtime_error(error);
        }
        string input_state  = transition_parts[0];
        string input_char   = transition_parts[1];
        string stack_top    = transition_parts[2];
        string output_state = transition_parts[3];
        string output_stack = transition_parts[4];
        auto   it           = state_map.find(input_state);
        if (it == state_map.end()) {
          string error = "State not found: " + input_state;
          throw runtime_error(error);
        }
        state_list[it->second].add_transition(input_char[0], stack_top[0], output_state, output_stack, state_map);
        break;
      }

      default: break;
    }
  }

  // parsing the statement end

private:
  // input the content and creat a pda
  void parse_pda(string &input, Logger &logger)
  {
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
        struct pda::PDA_Wrapper::PDA_STATEMENT statement = pda::PDA_Wrapper::get_statement(line);
        pda::PDA_Wrapper::print_pda_statement(statement, logger);
        parse_pda_statement(statement, logger);
      }
    }
    for (auto &state : state_list) {
      state.print_transitions(logger, state_list);
    }
  }

private:
  Logger                     logger;
  vector<char>               input_alphabet;
  vector<char>               stack_alphabet;
  vector<PDA_STATE>          state_list;
  unordered_map<string, int> state_map;
  int                        initial_state;
  int                        current_state;
  stack<char>                pda_stack;

public:
  PDA_Wrapper(Logger logger_) : logger(logger_) {};
  bool create_pda(string &pda_content)
  {
    parse_pda(pda_content, logger);
    return true;
  }
};

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

  pda::PDA_Wrapper wrapper(debug_logger);
  wrapper.create_pda(content.content);
  return 0;
}