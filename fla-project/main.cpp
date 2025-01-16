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

const string BARRIER = "---------------------------------------------";

class Logger
{
public:
  Logger() : logToStderr(false) {}

  bool isLogToStderr() { return logToStderr; }
  void setLogToStderr(bool value) { logToStderr = value; }

  template <typename T>
  Logger &operator<<(const T &message)
  {
    if (logToStderr) {
      cout << message;
    } else {
      // No output here
    }
    return *this;
  }

  Logger &operator<<(std::ostream &(*manip)(std::ostream &))
  {
    if (logToStderr) {
      manip(cout);
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

namespace fla {
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
  args.verbose     = false;
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
};  // namespace fla
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
        auto hash1 = std::hash<T1>()(p.first);
        auto hash2 = std::hash<T2>()(p.second);
        // 合并两个哈希值
        return hash1 ^ (hash2 << 1);
      }
    };
    string                                                        name;
    bool                                                          is_accept_;
    unordered_map<pair<char, char>, pair<int, string>, pair_hash> transitions;

  public:
    PDA_STATE(string name, bool is_accept) : name(name), is_accept_(is_accept) {};
    PDA_STATE()                                  = default;
    PDA_STATE(const PDA_STATE &other)            = default;
    PDA_STATE &operator=(const PDA_STATE &other) = default;
    PDA_STATE(PDA_STATE &&other)                 = default;
    void   set_accept(bool value) { is_accept_ = value; }
    bool   is_accept() { return is_accept_; }
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
    pair<bool, pair<int, string> > get_transition(char input, char stack_top)
    {
      pair<bool, pair<int, string> > ret(false, pair<int, string>(-1, ""));
      pair<char, char>               key(input, stack_top);
      auto                           it = transitions.find(key);
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
      ret += is_accept_ ? "accept" : "normal";
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
  void load_stack()
  {
    for (int idx = initial_stack_symbol.length() - 1; idx >= 0; idx--) {
      pda_stack.push(initial_stack_symbol[idx]);
    }
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
        // print_states(logger);
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
        // logger << "Input Alphabet paresed: ";
        // for (char ch : input_alphabet) {
        //   logger << ch << " ";
        // }
        // logger << endl;
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
        // logger << "Stack Alphabet paresed: ";
        // for (char ch : stack_alphabet) {
        //   logger << ch << " ";
        // }
        // logger << endl;
        break;
      }
      case INITIAL_STATE: {
        initial_state = state_map[statement.content];
        // logger << "parsed Initial state: " << state_list[initial_state].to_string() << endl;
        break;
      }
      case STACK_INITIAL_SYMBOL: {
        initial_stack_symbol = statement.content;
        // logger << "parsed Stack Initial Symbol: ";
        // logger << initial_stack_symbol << endl;

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
        // print_states(logger);
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
        // pda::PDA_Wrapper::print_pda_statement(statement, logger);
        parse_pda_statement(statement, logger);
      }
    }
    // for (auto &state : state_list) {
    //   state.print_transitions(logger, state_list);
    // }
  }

  bool input_alphabet_check(char ch)
  {
    for (char input : input_alphabet) {
      if (input == ch) {
        return true;
      }
    }
    return false;
  }

private:
  Logger                    &logger;
  vector<char>               input_alphabet;
  vector<char>               stack_alphabet;
  vector<PDA_STATE>          state_list;
  unordered_map<string, int> state_map;
  string                     initial_stack_symbol;
  int                        initial_state;
  int                        current_state;
  stack<char>                pda_stack;

public:
  PDA_Wrapper(Logger &logger_) : logger(logger_) {};
  bool compile(string &pda_content)
  {
    parse_pda(pda_content, logger);
    return true;
  }
  void print()
  {
    logger << "All parsed PDA information:" << endl;
    logger << "Input Alphabet: ";
    for (char ch : input_alphabet) {
      logger << ch << " ";
    }
    logger << endl;
    logger << "Stack Alphabet: ";
    for (char ch : stack_alphabet) {
      logger << ch << " ";
    }
    logger << endl;
    logger << "Initial State: " << state_list[initial_state].get_name() << endl;
    logger << "Stack Initial Symbol: ";
    logger << initial_stack_symbol << endl;
    logger << "Final States: ";
    for (auto &state : state_list) {
      if (state.is_accept()) {
        logger << state.get_name() << " ";
      }
    }
    logger << endl;
    for (auto &state : state_list) {
      state.print_transitions(logger, state_list);
    }
    logger << "PDA information end" << endl;
  }
  void runtime_print()
  {
    logger << "Runtime print start" << endl;
    stack<char> temp_stack = pda_stack;
    logger << "Current State: " << state_list[current_state].get_name() << endl;
    logger << "Current Stack: ";
    while (!temp_stack.empty()) {
      logger << temp_stack.top();
      temp_stack.pop();
    }
    logger << endl;
    logger << "Runtime print end" << endl;
  }
  bool run(string &input)
  {
    load_stack();
    current_state = initial_state;
    for (char ch : input) {
      if (!input_alphabet_check(ch)) {
        cout << "Illegal Input" << endl;
        exit(EXIT_FAILURE);  // Illegal input character
      }
      pair<bool, pair<int, string> > transition = state_list[current_state].get_transition(ch, pda_stack.top());
      if (!transition.first) {
        return false;
      }
      current_state = transition.second.first;
      if (!pda_stack.empty()) {
        pda_stack.pop();
      } else {
        return false;
      }
      if (transition.second.second != "_") {
        for (int idx = transition.second.second.length() - 1; idx >= 0; idx--) {
          pda_stack.push(transition.second.second[idx]);
        }
      }
    }
    // Now try to use the empty string to finish the stack
    char ch = '_';
    while (true) {
      // Check if the current state is the final state
      if (state_list[current_state].is_accept()) {
        return true;
      }
      pair<bool, pair<int, string> > transition = state_list[current_state].get_transition(ch, pda_stack.top());
      if (!transition.first) {
        return false;
      }
      current_state = transition.second.first;
      if (!pda_stack.empty()) {
        pda_stack.pop();
      } else {
        return false;
      }
      if (transition.second.second != "_") {
        for (int idx = transition.second.second.length() - 1; idx >= 0; idx--) {
          pda_stack.push(transition.second.second[idx]);
        }
      }
    }
    return false;
  }
};

};  // namespace pda
// End PDA parsing

namespace tm_space {
class TM_Wrapper
{
private:
  Logger &debug_logger;
  Logger &verbose_logger;
  enum TM_STATEMENT_TYPE
  {
    STATES,
    INPUT_ALPHABET,
    TAPE_SYMBOLS,
    INITIAL_STATE,
    BRANKE_SYMBOL,
    ACCEPT_STATES,
    TAPE_COUNT,
    TRANSITION
  };
  struct TM_STATEMENT
  {
    TM_STATEMENT_TYPE type;
    string            content;
    string            to_string()
    {
      switch (this->type) {
        case TRANSITION: return "Transition: " + content;
        case STATES: return "States: " + content;
        case INPUT_ALPHABET: return "Input Alphabet: " + content;
        case TAPE_SYMBOLS: return "Tape Symbols: " + content;
        case INITIAL_STATE: return "Initial State: " + content;
        case BRANKE_SYMBOL: return "Brank Symbol: " + content;
        case ACCEPT_STATES: return "Accept States: " + content;
        case TAPE_COUNT: return "Tape Count: " + content;
      };
    };
  };
  vector<TM_STATEMENT> lexer(string &content);
  void                 parse_statement_(TM_STATEMENT &statement);
  void                 parse_statement(vector<TM_STATEMENT> &statements)
  {
    for (auto &statement : statements) {
      parse_statement_(statement);
    }
    for (auto &state : state_list) {
      debug_logger << BARRIER << endl;
      debug_logger << state.to_string(state_list) << endl;
      debug_logger << BARRIER << endl;
    }
  }

private:
  struct vector_hash
  {
    template <typename T>
    std::size_t operator()(const std::vector<T> &v) const
    {
      std::size_t seed = 0;
      for (const T &elem : v) {
        seed ^= std::hash<T>()(elem) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
      return seed;
    }
  };

private:
  class Tape
  {
  private:
    list<char>           tape_;
    list<char>::iterator head_;
    list<char>::iterator zero_;
    char                 blank_symbol;

  public:
    Tape(char blank_symbol) : blank_symbol(blank_symbol), head_(tape_.begin()), zero_(tape_.begin()) {};
    void reset(const string &str)
    {
      tape_.clear();
      for (char ch : str) {
        tape_.push_back(ch);
      }
      head_ = tape_.begin();
      zero_ = tape_.begin();
    }
    void move_left()
    {
      if (head_ == tape_.begin()) {
        tape_.push_front(blank_symbol);
      }
      head_--;
    }
    void move_right()
    {
      head_++;
      if (head_ == tape_.end()) {
        tape_.push_back(blank_symbol);
        auto it = tape_.end();
        it--;
        head_ = it;
      }
    }
    char read() { return *head_; }
    void write(char ch) { *head_ = ch; }
    int  get_length() { return tape_.size(); }
    int  get_head_pos()
    {
      int ret = 0;
      for (auto it = tape_.begin(); it != head_; it++) {
        ret++;
      }
      return ret;
    }
    int get_zero_position()
    {
      int ret = 0;
      for (auto it = tape_.begin(); it != zero_; it++) {
        ret++;
      }
      return ret;
    }
    string to_string()
    {
      string ret;
      for (auto it = tape_.begin(); it != tape_.end(); it++) {
        ret += *it;
      }
      return ret;
    }
  };

  class State;
  struct Transition
  {
    int          to_state;
    vector<char> write_symbols;
    vector<char> move_dirctions;
    string       to_string(vector<State> &statelist)
    {
      string ret;
      ret += "To state: ";
      ret += statelist[to_state].get_name();
      ret += "\n";
      ret += "Write symbols: ";
      for (char ch : write_symbols) {
        ret += ch;
        ret += " ";
      }
      ret += "\n";
      ret += "Move directions: ";
      for (char ch : move_dirctions) {
        ret += ch;
        ret += " ";
      }
      ret += "\n";
      return ret;
    }
  };

  class State
  {
  private:
    string                                               name;
    bool                                                 is_accept_;
    unordered_map<vector<char>, Transition, vector_hash> transition_map;

  public:
    State(string name) : name(name), is_accept_(false) {};
    void   set_accept(bool value) { is_accept_ = value; };
    bool   is_accept() { return is_accept_; };
    string get_name() { return name; };
    void   add_transition(
          vector<char> tap_sympols, int to_state, vector<char> write_symbols, vector<char> move_directions)
    {
      Transition transition;
      transition.to_state         = to_state;
      transition.write_symbols    = write_symbols;
      transition.move_dirctions   = move_directions;
      transition_map[tap_sympols] = transition;
    };
    struct Transition get_transition(vector<char> tap_sympols)
    {
      for (unordered_map<vector<char>, Transition, vector_hash>::iterator it = transition_map.begin();
           it != transition_map.end();
           it++) {
        Transition    trans(it->second);
        vector<char> &write_symbols = trans.write_symbols;
        for (int i = 0; i < it->first.size(); i++) {
          if (it->first[i] != '*' && it->first[i] != tap_sympols[i]) {
            break;
          } else if (it->first[i] == '*') {
            if (tap_sympols[i] == '_') {
              break;
            }
            if (write_symbols[i] == '*') {
              write_symbols[i] = tap_sympols[i];
            }
          }
          if (i == it->first.size() - 1) {
            return trans;
          }
        }
      }
      Transition trans;
      trans.to_state = -1;
      return trans;
    };
    string to_string(vector<State> &state_list)
    {
      string ret;
      ret += "Name:";
      ret += name;
      ret += " ";
      ret += "Type: ";
      ret += is_accept_ ? "accept" : "normal";
      ret += "\n";
      ret += "Transitions:\n";
      for (auto &transition : transition_map) {
        ret += "Tape symbols: ";
        for (char ch : transition.first) {
          ret += ch;
          ret += " ";
        }
        ret += "\n";
        ret += transition.second.to_string(state_list);
      }
      return ret;
    }
  };

  bool is_valid_input(char ch)
  {
    for (char input : input_alphabet) {
      if (input == ch) {
        return true;
      }
    }
    return false;
  };
  // 成员变量
private:
  char                       blank_symbol;
  vector<Tape>               tapes;
  vector<State>              state_list;
  vector<char>               input_alphabet;
  vector<char>               tape_alphabet;
  unordered_map<string, int> state_map;
  int                        initial_state;
  int                        current_state;

public:
  TM_Wrapper(Logger &debug_logger_, Logger &verbose_logger_)
      : debug_logger(debug_logger_), verbose_logger(verbose_logger_) {};
  void               compile(string &tm_content);
  void               print();
  void               verbose(int step);
  pair<bool, string> run(string &input);

  // implement the TM_Wrapper class here
};
vector<TM_Wrapper::TM_STATEMENT> TM_Wrapper::lexer(string &content)
{
  stringstream         ss(content);
  string               line;
  vector<TM_STATEMENT> vec;
  while (getline(ss, line, '\n')) {
    size_t pos = line.find(';');
    if (pos != string::npos) {
      line = line.substr(0, pos);
    }
    line.erase(line.find_last_not_of(" \n\r\t") + 1);
    line.erase(0, line.find_first_not_of(" \n\r\t"));
    if (!line.empty()) {
      struct TM_STATEMENT ret;
      if (line[0] == '#') {
        // Here is the first six cases
        switch (line[1]) {
          case 'Q':
            ret.type    = tm_space::TM_Wrapper::STATES;
            ret.content = line.substr(5);
            break;
          case 'S':
            ret.type    = tm_space::TM_Wrapper::INPUT_ALPHABET;
            ret.content = line.substr(5);
            break;
          case 'G':
            ret.type    = tm_space::TM_Wrapper::TAPE_SYMBOLS;
            ret.content = line.substr(5);
            break;
          case 'q':
            if (line[2] != '0') {
              throw runtime_error("Invalid statement");
            }
            ret.type    = tm_space::TM_Wrapper::INITIAL_STATE;
            ret.content = line.substr(6);
            break;
          case 'B':
            ret.type    = tm_space::TM_Wrapper::BRANKE_SYMBOL;
            ret.content = line.substr(5);
            break;
          case 'F':
            ret.type    = tm_space::TM_Wrapper::ACCEPT_STATES;
            ret.content = line.substr(5);
            break;
          case 'N':
            ret.type    = tm_space::TM_Wrapper::TAPE_COUNT;
            ret.content = line.substr(5);
            break;
          default: throw runtime_error("Invalid statement");
        }
      } else {
        ret.type    = TRANSITION;
        ret.content = line;
      }
      // Statement created
      vec.push_back(ret);
    }
  }
  return vec;
}
void TM_Wrapper::parse_statement_(TM_STATEMENT &statement)
{
  switch (statement.type) {
    case STATES: {
      debug_logger << "States: " << statement.content << endl;
      string       true_content = statement.content.substr(1, statement.content.size() - 2);
      stringstream ss(true_content);
      string       state;
      while (getline(ss, state, ',')) {
        if (state_map.find(state) != state_map.end()) {
          throw runtime_error("State already exists: " + state);
        }
        State st = State(state);
        state_list.push_back(st);
        state_map[state] = state_list.size() - 1;
      }
      for (auto &state : state_list) {
        debug_logger << state.to_string(state_list) << endl;
      }
      break;
    }
    case INPUT_ALPHABET: {
      debug_logger << "Input Alphabet: " << statement.content << endl;
      string       true_content = statement.content.substr(1, statement.content.size() - 2);
      stringstream ss(true_content);
      string       character;
      while (getline(ss, character, ',')) {
        if (character.size() != 1) {
          throw runtime_error("Invalid character: " + character);
        }
        input_alphabet.push_back(character[0]);
      }
      debug_logger << "Input Alphabet paresed: ";
      for (char ch : input_alphabet) {
        debug_logger << ch << " ";
      }
      debug_logger << endl;
      break;
    }
    case TAPE_SYMBOLS: {
      debug_logger << "Tape Symbols: " << statement.content << endl;
      string       true_content = statement.content.substr(1, statement.content.size() - 2);
      stringstream ss(true_content);
      string       character;
      while (getline(ss, character, ',')) {
        if (character.size() != 1) {
          throw runtime_error("Invalid character: " + character);
        }
        tape_alphabet.push_back(character[0]);
      }
      debug_logger << "Tape Alphabet paresed: ";
      for (char ch : tape_alphabet) {
        debug_logger << ch << " ";
      }
      debug_logger << endl;
      break;
    }
    case INITIAL_STATE: {
      debug_logger << "Initial State: " << statement.content << endl;
      initial_state = state_map[statement.content];
      debug_logger << "parsed Initial state: " << state_list[initial_state].to_string(state_list) << endl;
      break;
    }
    case BRANKE_SYMBOL: {
      debug_logger << "Brank Symbol: " << statement.content << endl;
      blank_symbol = statement.content[0];
      debug_logger << "parsed Stack Initial Symbol: ";
      debug_logger << blank_symbol << endl;
      break;
    }
    case ACCEPT_STATES: {
      debug_logger << "Accept States: " << statement.content << endl;
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
      for (auto &state : state_list) {
        debug_logger << state.to_string(state_list) << endl;
      }
      break;
    }
    case TAPE_COUNT: {
      debug_logger << "Tape Count: " << statement.content << endl;
      int tape_cnt = stoi(statement.content);
      for (int i = 0; i < tape_cnt; i++) {
        tapes.emplace_back(blank_symbol);
      }
      debug_logger << "Tape count: " << tape_cnt << endl;
      int tmp = 0;
      for (auto tape : tapes) {
        debug_logger << tape.to_string() << endl;
        debug_logger << "Tape " << tmp << " end" << endl;
        tmp++;
      }
      break;
    }
    case TRANSITION: {
      debug_logger << "Transition: " << statement.content << endl;
      stringstream   ss(statement.content);
      string         transition_part;
      vector<string> transition_parts;
      int            cnt = 1;
      string         old_state;
      vector<char>   old_tape_symbols;
      string         new_state;
      vector<char>   new_tape_symbols;
      vector<char>   move_directions;
      while (getline(ss, transition_part, ' ')) {
        switch (cnt) {
          case 1:
            // Old state
            old_state = transition_part;
            break;
          case 2:
            // Old tape symbol
            for (char ch : transition_part) {
              old_tape_symbols.push_back(ch);
            }
            break;
          case 3:
            // New tape symbols
            for (char ch : transition_part) {
              new_tape_symbols.push_back(ch);
            }
            break;
          case 4:
            // direction
            for (char ch : transition_part) {
              move_directions.push_back(ch);
            }
            break;
          case 5:
            // New state
            new_state = transition_part;
            break;
          default: throw runtime_error("Invalid transition statement");
        }
        cnt++;
      }
      if (cnt != 6) {
        string error = "Invalid transition statement: " + statement.content;
        throw runtime_error(error);
      }
      debug_logger << "Old state: " << old_state << endl;
      debug_logger << "Old tape symbols: ";
      for (char ch : old_tape_symbols) {
        debug_logger << ch << " ";
      }
      debug_logger << endl;
      debug_logger << "New state: " << new_state << endl;
      debug_logger << "New tape symbols: ";
      for (char ch : new_tape_symbols) {
        debug_logger << ch << " ";
      }
      debug_logger << endl;
      debug_logger << "Move directions: ";
      for (char ch : move_directions) {
        debug_logger << ch << " ";
      }
      debug_logger << endl;
      auto it = state_map.find(old_state);
      if (it == state_map.end()) {
        string error = "State not found: " + old_state;
        throw runtime_error(error);
      }
      state_list[it->second].add_transition(old_tape_symbols, state_map[new_state], new_tape_symbols, move_directions);
      break;
    }
    default: debug_logger << "Unknown statement type" << endl; break;
  }
}
void TM_Wrapper::compile(string &tm_content)
{
  debug_logger << "TM content: " << tm_content << endl;
  vector<tm_space::TM_Wrapper::TM_STATEMENT> vec = lexer(tm_content);
  debug_logger << "Start parsing" << endl;
  parse_statement(vec);
}
void TM_Wrapper::print() {}
void TM_Wrapper::verbose(int step)
{
  // Use the verbose logger to print the runtime information
  verbose_logger << "Step   : " << step << endl;
  verbose_logger << "State  : " << state_list[current_state].get_name() << endl;
  for (int i = 0; i < tapes.size(); i++) {
    string tape_content          = tapes[i].to_string();
    int    head_pos              = tapes[i].get_head_pos();
    int    zero_pos              = tapes[i].get_zero_position();
    int    print_start_pos       = 0;
    bool   find_first_none_blank = false;
    for (int i = 0; i < tape_content.length(); i++) {
      if (tape_content[i] != blank_symbol) {
        find_first_none_blank = true;
        break;
      } else {
        print_start_pos++;
      }
    }
    if (head_pos < print_start_pos) {
      // Now the head is at the left of the none blank space
      print_start_pos = head_pos;
    }

    // get the last none blank space
    int print_end_pos = tape_content.length() - 1;
    for (int i = tape_content.length() - 1; i >= 0; i--) {
      if (tape_content[i] != blank_symbol) {
        break;
      } else {
        print_end_pos--;
      }
    }
    if (head_pos > print_end_pos) {
      // This symbolizes that the whole tape is blank
      print_end_pos = head_pos;
    }
    int print_length = print_end_pos - print_start_pos + 1;
    int base_idx     = print_start_pos - zero_pos;
    if (!find_first_none_blank) {
      // Here the whole tape is blank. We should print the head only
      int idx = base_idx;
      idx     = idx < 0 ? -idx : idx;
      verbose_logger << "Index" << i << " : " << idx << endl;
      verbose_logger << "Tape" << i << "  : " << blank_symbol << endl;
      verbose_logger << "Head" << i << "  : "
                     << "^" << endl;
    } else {
      // prepare the true content
      string true_content = tape_content.substr(print_start_pos);
      // print index
      verbose_logger << "Index" << i << " : ";
      for (int j = 0; j < print_length; j++) {
        int idx = j + base_idx;
        if (idx < 0) {
          idx = -idx;
        }
        verbose_logger << idx << " ";
      }
      verbose_logger << endl;
      // print tape content
      verbose_logger << "Tape" << i << "  : ";
      for (int j = 0; j < print_length; j++) {
        int idx = j + base_idx;
        if (idx < 0) {
          idx = -idx;
        }
        int blank_size = std::to_string(idx).size();
        verbose_logger << true_content[j];
        for (int k = 0; k < blank_size; k++) {
          verbose_logger << " ";
        }
      }
      verbose_logger << endl;
      // print head
      verbose_logger << "Head" << i << "  : ";
      for (int j = 0; j < print_length; j++) {
        if (j == head_pos - print_start_pos) {
          verbose_logger << "^";
        } else {
          verbose_logger << " ";
        }
        int idx = j + base_idx;
        if (idx < 0) {
          idx = -idx;
        }
        int blank_size = std::to_string(idx).size();
        for (int k = 0; k < blank_size; k++) {
          verbose_logger << " ";
        }
      }
      verbose_logger << endl;
    }
  }
  verbose_logger << BARRIER << endl;
}
pair<bool, string> TM_Wrapper::run(string &input)
{
  verbose_logger << "Input: ";
  verbose_logger << input << endl;
  for (size_t i = 0; i < input.size(); i++) {
    if (!is_valid_input(input[i])) {
      if (verbose_logger.isLogToStderr()) {
        verbose_logger << "==================== ERR ====================" << endl;
        char error_msg[100];
        snprintf(error_msg, sizeof(error_msg), "error: '%c' was not declared in the set of input symbols", input[i]);
        verbose_logger << "Input: " << input << endl;
        for (size_t pos = 0; pos < 7 + i; pos++) {
          verbose_logger << ' ';
        }
        verbose_logger << "^\n";
        verbose_logger << "==================== END ====================" << endl;
      } else {
        cerr << "Illegal input" << endl;
      }
      exit(EXIT_FAILURE);
    }
  }

  verbose_logger << "==================== RUN ====================" << endl;
  bool success = false;
  tapes[0].reset(input);
  for (int i = 1; i < tapes.size(); i++) {
    tapes[i].reset("_");
  }
  current_state = initial_state;
  int step_cnt  = 0;
  while (true) {
    verbose(step_cnt);
    State &state = state_list[current_state];
    success      = state.is_accept();
    vector<char> current_tape_symbols;
    for (int i = 0; i < tapes.size(); i++) {
      current_tape_symbols.push_back(tapes[i].read());
    }
    debug_logger << "Enter Get Transition" << endl;
    Transition transition = state.get_transition(current_tape_symbols);
    if (transition.to_state == -1) {
      debug_logger << "Halt" << endl;
    } else {
      debug_logger << "Transition: " << transition.to_string(state_list) << endl;
    }
    if (transition.to_state == -1) {
      string           str   = tapes[0].to_string();
      string::iterator start = str.begin();
      string::iterator end   = str.end();
      while (start != str.end() && *start == blank_symbol) {
        start++;
      }
      while (end != start && *(end - 1) == blank_symbol) {
        end--;
      }
      str = string(start, end);
      return make_pair(success, str);
    }
    for (size_t i = 0; i < tapes.size(); i++) {
      tapes[i].write(transition.write_symbols[i]);
      if (transition.move_dirctions[i] == 'l') {
        tapes[i].move_left();
      } else if (transition.move_dirctions[i] == 'r') {
        tapes[i].move_right();
      } else if (transition.move_dirctions[i] == '*') {
        // Do nothing
      } else {
        throw runtime_error("Invalid move direction");
      }
    }
    current_state = transition.to_state;
    step_cnt++;
  }
  verbose(step_cnt);
  string ret_str = tapes[0].to_string();
  return make_pair(success, tapes[0].to_string());
}
};  // namespace tm_space

int main(int argc, char *argv[])
{
  Logger debug_logger;
  debug_logger.setLogToStderr(true);
  struct fla::Args args;

  try {
    args = fla::parse_args(argc, argv, debug_logger);
    // print_args(args, debug_logger);
  } catch (const exception &e) {
    cerr << "Error: " << e.what() << endl;
    exit(EXIT_FAILURE);
  }

  struct fla::fla_content content;
  try {
    content = fla::load_fla_file(args.file_path);
    // debug_logger << "File type: " << content.type << "\nFile content: " << content.content << endl;

  } catch (const exception &e) {
    cerr << "Error: " << e.what() << endl;
    exit(EXIT_FAILURE);
  }
  if (content.type == fla::TM) {
    // debug_logger.setLogToStderr(false);
    Logger verbose_logger;
    verbose_logger.setLogToStderr(false);
    if (args.verbose)
      verbose_logger.setLogToStderr(true);
    debug_logger.setLogToStderr(false);
    tm_space::TM_Wrapper wrapper(debug_logger, verbose_logger);
    wrapper.compile(content.content);
    pair<bool, string> ret = wrapper.run(args.input);
    if (verbose_logger.isLogToStderr()) {
      cout << "Result: " << ret.second << endl;
      cout << "==================== END ====================" << endl;
    } else
      cout << ret.second << endl;
  } else if (content.type == fla::PDA) {
    pda::PDA_Wrapper wrapper(debug_logger);
    wrapper.compile(content.content);
    // wrapper.print();
    bool success = wrapper.run(args.input);
    if (success) {
      cout << "true" << endl;
    } else {
      cout << "false" << endl;
    }
  } else {
    cerr << "Invalid file type" << endl;
    exit(EXIT_FAILURE);
  }

  return 0;
}