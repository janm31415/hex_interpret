#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include <sstream>

bool is_little_endian()
{
  short int number = 0x1;
  char *num_ptr = (char*)&number;
  return (num_ptr[0] == 1);
}

std::string int_to_hex(uint8_t i)
{
  std::string hex;
  int h1 = (i >> 4) & 0x0f;
  if (h1 < 10)
    hex += '0' + h1;
  else
    hex += 'A' + h1 - 10;
  int h2 = (i) & 0x0f;
  if (h2 < 10)
    hex += '0' + h2;
  else
    hex += 'A' + h2 - 10;
  return hex;
}

std::string int_to_hex(uint16_t i)
{
  std::string hex;
  uint8_t h1 = (i >> 8) & 0x00ff;
  uint8_t h2 = i & 0x00ff;
  return int_to_hex(h1) + int_to_hex(h2);
}

std::string int_to_hex(uint32_t i)
{
  std::string hex;
  uint16_t h1 = (i >> 16) & 0x0000ffff;
  uint16_t h2 = i & 0x0000ffff;
  return int_to_hex(h1) + int_to_hex(h2);
}

std::string int_to_hex(char ch)
{
  uint8_t* c = reinterpret_cast<uint8_t*>(&ch);
  return int_to_hex(*c);
}

char to_str(char c)
{
  if (c >= 32 && c < 127)
  {
    return c;
  }
  else
  {
    if (c < 0)
    {
      return c;
    }
    else
      return '.';
  }
}

uint8_t char_to_int(char c)
{
  if (c >= '0' && c <= '9')
  {
    return (uint8_t)(c-'0');
  }
  else if (c >= 'a' && c <= 'f')
  {
    return (uint8_t)(c-'a'+10);
  }
  else if (c >= 'A' && c <= 'F')
  {
    return (uint8_t)(c-'A'+10);
  }
  return 0;
}

void treat_buffer(std::vector<uint8_t>& arr, std::vector<char>& buffer)
{
  if (buffer.size() == 2)
  {
    uint8_t a = char_to_int(buffer[0]);
    uint8_t b = char_to_int(buffer[1]);
    arr.push_back(a*16+b);
    buffer.clear();
  }
}

std::vector<uint8_t> hex_to_byte_array(const std::string& hex)
{
  std::vector<uint8_t> arr;
  auto it = hex.begin();
  const auto it_end = hex.end();
  char previous_c = (char)0;
  std::vector<char> buffer;
  for (; it != it_end; ++it)
  {
    char c = *it;
    if (((c >= '0' && c <= '9')) ||
        ((c >= 'a') && (c <= 'f')) ||
        ((c >= 'A') && (c <= 'F')))
    {
      buffer.push_back(c);
      treat_buffer(arr, buffer);
    }
    else if (((c == 'x') || (c == 'X')) && ((previous_c == '0') || (previous_c == '#')))
    {
      if (previous_c == '0')
        buffer.pop_back();
      treat_buffer(arr, buffer);
      buffer.clear();
    }
    else if (c != ' ' && c != '\n' && c != '#')
    {
      std::cout << "Error: invalid character " << c << " at position " << std::distance(hex.begin(), it) << std::endl;
      treat_buffer(arr, buffer);
      buffer.clear();
    }
    previous_c = c;
  }
  return arr;
}

class SimpleInterpreter
{
public:
  void operator()(const std::vector<uint8_t>& characters, std::ostream& str)
  {
    for (auto c : characters)
    {
      str << to_str((char)c);
    }
  }
};

template <class T>
void output(std::ostream& str, T value)
{
  str << value << " ";
}

template <>
void output(std::ostream& str, uint8_t value)
{
  str << to_str((char)value);
}

template <>
void output(std::ostream& str, int8_t value)
{
  str << to_str((char)value);
}

template <>
void output(std::ostream& str, char value)
{
  str << to_str((char)value);
}


template <class T>
class TypeInterpreter
{
public:
  TypeInterpreter(bool little_endiann = true) : _little_endiann(little_endiann) {}
  
  void operator()(const std::vector<uint8_t>& characters, std::ostream& str)
  {
    assert(sizeof(T) <= 8);
    auto it = characters.begin();
    const auto it_end = characters.end();
    while (it != it_end)
    {
      std::vector<uint64_t> values;
      for (int i = 0; i < sizeof(T); ++i)
      {
        if (it != it_end)
          values.push_back(*it++);
      }
      uint64_t number = 0;
      if (_little_endiann)
      {
        for (auto rit = values.rbegin(); rit != values.rend(); ++rit)
        {
          number = (number << 8) | *rit;
        }
      }
      else
      {
        for (auto rit = values.begin(); rit != values.end(); ++rit)
        {
          number = (number << 8) | *rit;
        }
      }
      const T val = *reinterpret_cast<T*>(&number);
      output<T>(str, val);
    }
  }
  
  bool _little_endiann;
};

template <class TIter, class TInterpreter>
void print_byte_array(uint32_t address, TIter first, TIter last, TInterpreter interpreter, uint32_t elements_per_row, std::ostream& str)
{
  size_t size = std::distance(first, last);
  std::vector<uint8_t> characters;
  str << int_to_hex((uint32_t)address) << ": ";
  for (uint32_t i = 0; i < size; ++i, ++first)
  {
    str << int_to_hex(*first) << " ";
    characters.push_back(*first);
    if ((i + 1) % elements_per_row == 0)
    {
      str << "| ";
      interpreter(characters, str);
      characters.clear();
      str << std::endl;
      if (i != size - 1)
      {
        str << int_to_hex((uint32_t)(i+1+address)) << ": ";
      }
    }
  }
  if (size % elements_per_row)
  {
    for (int i = 0; i < (elements_per_row - (size % elements_per_row)); ++i)
      str << "   ";
    str << "| ";
    interpreter(characters, str);
    str << std::endl;
  }
}

std::vector<uint8_t> read_input(const std::string& input)
{
  std::fstream f(input, std::fstream::in | std::fstream::binary);
  if (f.is_open())
  {
    std::cout << "Interpreting command line argument as a binary file.\n";
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(f), {});
    return buffer;
  }
  else
  {
    std::cout << "Interpreting command line argument as a hex text.\n";
    return hex_to_byte_array(input);
  }
}

enum class dumptype
{
  dumptype_uint8,
  dumptype_int8,
  dumptype_uint16,
  dumptype_int16,
  dumptype_uint32,
  dumptype_int32,
  dumptype_uint64,
  dumptype_int64,
  dumptype_float,
  dumptype_double
};

uint32_t size_of(dumptype dt)
{
  switch (dt)
  {
    case dumptype::dumptype_uint8:
      return sizeof(uint8_t);
    case dumptype::dumptype_int8:
      return sizeof(int8_t);
    case dumptype::dumptype_uint16:
      return sizeof(uint16_t);
    case dumptype::dumptype_int16:
      return sizeof(int16_t);
    case dumptype::dumptype_uint32:
      return sizeof(uint32_t);
    case dumptype::dumptype_int32:
      return sizeof(int32_t);
    case dumptype::dumptype_uint64:
      return sizeof(uint64_t);
    case dumptype::dumptype_int64:
      return sizeof(int64_t);
    case dumptype::dumptype_float:
      return sizeof(float);
    case dumptype::dumptype_double:
      return sizeof(double);
  }
}

std::string dump_type_to_str(dumptype dt)
{
  switch (dt)
  {
    case dumptype::dumptype_uint8:
      return std::string("uint8_t");
    case dumptype::dumptype_int8:
      return std::string("int8_t");
    case dumptype::dumptype_uint16:
      return std::string("uint16_t");
    case dumptype::dumptype_int16:
      return std::string("int16_t");
    case dumptype::dumptype_uint32:
      return std::string("uint32_t");
    case dumptype::dumptype_int32:
      return std::string("int32_t");
    case dumptype::dumptype_uint64:
      return std::string("uint64_t");
    case dumptype::dumptype_int64:
      return std::string("int64_t");
    case dumptype::dumptype_float:
      return std::string("float");
    case dumptype::dumptype_double:
      return std::string("double");
  }
}

void print_help()
{
  std::cout << "Available commands:\n";
  std::cout << "  d, dump         : dump the interpreted hex data\n";
  std::cout << "  offset <nr>     : change the dump offset to nr\n";
  std::cout << "  length <nr>     : change the dump length to nr\n";
  std::cout << "  row <nr>        : change the row length to nr\n";
  std::cout << "  + <nr>          : add nr to the offset\n";
  std::cout << "  - <nr>          : subtract nr from the offset\n";
  std::cout << "  type b|B|h|H|i|I|q|Q|f|d\n";
  std::cout << "                  : change the interpreted type\n";
  std::cout << "  find <str>      : find next occurrence of str\n";
  std::cout << "  find# <hex str> : find next occurrence of hex ?str\n";
  std::cout << "  little          : interpret as little endianness\n";
  std::cout << "  big             : interpret as big endianness\n";
  std::cout << "  endianness      : shows this PCs endianness\n";
  std::cout << "  state           : print the current dump state\n";
  std::cout << "  >> <file>       : stream output to a file\n";
  std::cout << "  q, quit, exit   : quit the application\n";
}

int count_connectors(std::string temp)
{
  auto it = temp.find_first_of('"');
  int found = 0;
  while (it != std::string::npos)
  {
    ++found;
    temp.erase(temp.begin(), temp.begin() + it + 1);
    it = temp.find_first_of('"');
  }
  return found;
}

std::vector<std::string> get_arguments(const std::string& command)
{
  std::vector<std::string> output;
  std::stringstream sstr;
  sstr << command;
  bool connector_present = false;
  while (!sstr.eof())
  {
    std::string temp;
    sstr >> temp;
    int connectors_found = count_connectors(temp);
    bool connector_found = false;
    if (connectors_found % 2 == 1)
      connector_found = true;
    if (connector_present)
    {
      output.back() += " ";
      output.back() += temp;
      if (connector_found)
        connector_present = false;
    }
    else
    {
      output.push_back(temp);
      if (connector_found)
        connector_present = true;
    }
  }
  return output;
}

uint32_t interpret_number(const std::string& s)
  {
  if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) // hex number
    {
    std::stringstream sstr;
    sstr << std::hex << s;
    uint32_t x;
    sstr >> x;
    return x;
    }
  else
    {
    std::stringstream sstr;
    sstr << s;
    uint32_t x;
    sstr >> x;
    return x;
    }
  }
  
dumptype interpret_dumptype(const std::string& s)
  {
  if (s == "int8" || s == "int8_t" || s == "b")
    return dumptype::dumptype_int8;
  if (s == "uint8" || s == "uint8_t" || s == "B")
    return dumptype::dumptype_uint8;
  if (s == "int16" || s == "int16_t" || s == "h")
    return dumptype::dumptype_int16;
  if (s == "uint16" || s == "uint16_t" || s == "H")
    return dumptype::dumptype_uint16;
  if (s == "int32" || s == "int32_t" || s == "i")
    return dumptype::dumptype_int32;
  if (s == "uint32" || s == "uint32_t" || s == "I")
    return dumptype::dumptype_uint32;
  if (s == "int64" || s == "int64_t" || s == "q")
    return dumptype::dumptype_int64;
  if (s == "uint64" || s == "uint64_t" || s == "Q")
    return dumptype::dumptype_uint64;
  if (s == "float" || s == "f")
    return dumptype::dumptype_float;
  if (s == "double" || s == "d")
    return dumptype::dumptype_double;
  return dumptype::dumptype_uint8;
  }
  
void find_next_occurence(uint32_t& offset, const std::vector<uint8_t>& byte_arr, const std::string& s, bool string_is_hex)
  {
  std::vector<uint8_t> find_arr;
  if (string_is_hex)
    find_arr = hex_to_byte_array(s);
  else
    {
    find_arr.reserve(s.size());
    for (const auto ch : s)
      find_arr.push_back((uint8_t)ch);
    }
  if (find_arr.empty())
    {
    std::cout << "Nothing to find.\n";
    return;
    }
  uint32_t current_matched_index = 0;
  for (uint32_t i = offset+1; i < (uint32_t)byte_arr.size(); ++i)
    {
    if (byte_arr[i] == find_arr[current_matched_index])
      {
      ++current_matched_index;
      if (current_matched_index >= find_arr.size())
        {
        uint32_t pos = i + 1 - current_matched_index;
        std::cout << "Found next occurence at position 0x" << int_to_hex(pos) << ".\n";
        offset = pos;
        std::cout << "Setting offset to " << offset << "(0x" << int_to_hex(offset) << ").\n";
        return;
        }
      }
    else
      {
      current_matched_index = 0;
      }
    }
  uint32_t end_of_find = offset+(uint32_t)find_arr.size();
  if (end_of_find > (uint32_t)byte_arr.size())
    end_of_find = (uint32_t)byte_arr.size();
  for (uint32_t i = 0; i < end_of_find; ++i)
    {
    if (byte_arr[i] == find_arr[current_matched_index])
      {
      ++current_matched_index;
      if (current_matched_index >= find_arr.size())
        {
        uint32_t pos = i + 1 - current_matched_index;
        std::cout << "Found next occurence at position 0x" << int_to_hex(pos) << ".\n";
        offset = pos;
        std::cout << "Setting offset to " << offset << "(0x" << int_to_hex(offset) << ").\n";
        return;
        }
      }
    else
      {
      current_matched_index = 0;
      }
    }
  std::cout << "Found no occurrence.\n";
  }

void hex_interpret(const std::vector<uint8_t>& byte_arr)
{
  std::string command;
  bool little_endiann = is_little_endian();
  uint32_t offset = 0;
  uint32_t length = 0xffffffff;
  dumptype dump_type = dumptype::dumptype_uint8;
  uint32_t data_per_line = 16;
  while (command != "exit" && command != "quit" && command != "q")
  {
    std::cout << "> ";
    std::getline(std::cin, command);
    auto arguments = get_arguments(command);
    size_t argc = arguments.size();
    std::string outputfile;
    bool dump = false;
    for (int i = 0; i < argc; ++i)
    {
      if (arguments[i] == "help" || arguments[i] == "?" || arguments[i] == "-?")
        print_help();
      else if (arguments[i] == "little")
        little_endiann = true;
      else if (arguments[i] == "big")
        little_endiann = false;
      else if (arguments[i] == "offset" && (i < (argc - 1)))
      {
        ++i;
        offset = interpret_number(arguments[i]);
        std::cout << "Setting offset to " << offset << "(0x" << int_to_hex(offset) << ").\n";
      }
      else if (arguments[i] == "offset")
        {
        std::cout << "The offset equals " << offset << "(0x" << int_to_hex(offset) << ").\n";
        }
      else if (arguments[i] == "length" && (i < (argc - 1)))
      {
        ++i;
        length = interpret_number(arguments[i]);
        std::cout << "Setting length to " << length << "(0x" << int_to_hex(length) << ").\n";
      }
      else if (arguments[i] == "length")
        {
        std::cout << "The length equals " << length << "(0x" << int_to_hex(length) << ").\n";
        }
      else if (arguments[i] == "type" && (i < (argc - 1)))
      {
        ++i;
        dump_type = interpret_dumptype(arguments[i]);
        std::cout << "Interpreting the bytes as " << dump_type_to_str(dump_type) << ".\n";
      }
      else if (arguments[i] == "type")
        {
        std::cout << "The type equals " << dump_type_to_str(dump_type) << ".\n";
        }
      else if (arguments[i] == "row" && (i < (argc - 1)))
      {
        ++i;
        data_per_line = interpret_number(arguments[i]);
        std::cout << data_per_line << " interpreted values will be printed per row.\n";
      }
      else if (arguments[i] == "row")
        {
        std::cout << data_per_line << " interpreted values will be printed per row.\n";
        }
      else if (arguments[i] == "find" && (i < (argc - 1)))
      {
        ++i;
        find_next_occurence(offset, byte_arr, arguments[i], false);
      }
      else if (arguments[i] == "find#" && (i < (argc - 1)))
      {
        ++i;
        find_next_occurence(offset, byte_arr, arguments[i], true);
      }
      else if (arguments[i] == "+" && (i < (argc - 1)))
      {
        ++i;
        offset += interpret_number(arguments[i]);
        std::cout << "Setting offset to " << offset << "(0x" << int_to_hex(offset) << ").\n";
      }
      else if (arguments[i].find("+") == 0)
      {
        arguments[i].erase(arguments[i].begin(), arguments[i].begin() + 1);
        offset += interpret_number(arguments[i]);
        std::cout << "Setting offset to " << offset << "(0x" << int_to_hex(offset) << ").\n";
      }
      else if (arguments[i] == "-" && (i < (argc - 1)))
      {
        ++i;
        uint32_t subtract = interpret_number(arguments[i]);
        offset = subtract > offset ? 0 : offset-subtract;
        std::cout << "Setting offset to " << offset << "(0x" << int_to_hex(offset) << ").\n";
      }
      else if (arguments[i].find("-") == 0)
      {
        arguments[i].erase(arguments[i].begin(), arguments[i].begin() + 1);
        uint32_t subtract = interpret_number(arguments[i]);
        offset = subtract > offset ? 0 : offset-subtract;
        std::cout << "Setting offset to " << offset << "(0x" << int_to_hex(offset) << ").\n";
      }
      else if (arguments[i] == ">>" && (i < (argc - 1)))
      {
        ++i;
        outputfile = arguments[i];
      }
      else if (arguments[i].find(">>") == 0)
      {
        arguments[i].erase(arguments[i].begin(), arguments[i].begin() + 2);
        outputfile = arguments[i];
      }
      else if (arguments[i] == "endianness")
      {
        if (is_little_endian())
          std::cout << "I detected little-endian" << std::endl;
        else
          std::cout << "I detected big-endian" << std::endl;
      }
      else if (arguments[i] == "state")
      {
        if (little_endiann)
          std::cout << "I interpret data as little-endian.\n";
        else
          std::cout << "I interpret data as big-endian.\n";
        std::cout << "A dump will start at offset " << offset << "(0x" << int_to_hex(offset) << ").\n";
        if (length == 0xffffffff)
          std::cout << "A dump will print untill the end of the given data.\n";
        else
          std::cout << "A dump will print " << length << "(0x" << int_to_hex(length) << ") bytes.\n";
        std::cout << "Interpreting the bytes as " << dump_type_to_str(dump_type) << ".\n";
        std::cout << data_per_line << " interpreted values will be printed per row.\n";
        std::cout << "The input data is " << byte_arr.size() << " bytes long.\n";
      }
      else if (arguments[i] == "dump" || arguments[i] == "d")
      {
        dump = true;
      }
    }
    if (dump) {
      auto it = byte_arr.begin() + offset;
      auto it_end = byte_arr.end();
      if (length < byte_arr.size())
      {
        if ((uint64_t)length+(uint64_t)offset < byte_arr.size())
          it_end = it + length;
      }
      std::ofstream f;
      std::ostream* str = &std::cout;
      if (!outputfile.empty())
      {
        f.open(outputfile);
        if (f.is_open())
          str = &f;
      }
      uint32_t elements_per_row = data_per_line*size_of(dump_type);
      switch (dump_type)
      {
        case dumptype::dumptype_uint8:
          print_byte_array(offset, it, it_end, TypeInterpreter<uint8_t>(little_endiann), elements_per_row, *str);
          break;
        case dumptype::dumptype_int8:
          print_byte_array(offset, it, it_end, TypeInterpreter<int8_t>(little_endiann), elements_per_row, *str);
          break;
        case dumptype::dumptype_uint16:
          print_byte_array(offset, it, it_end, TypeInterpreter<uint16_t>(little_endiann), elements_per_row, *str);
          break;
        case dumptype::dumptype_int16:
          print_byte_array(offset, it, it_end, TypeInterpreter<int16_t>(little_endiann), elements_per_row, *str);
          break;
        case dumptype::dumptype_uint32:
          print_byte_array(offset, it, it_end, TypeInterpreter<uint32_t>(little_endiann), elements_per_row, *str);
          break;
        case dumptype::dumptype_int32:
          print_byte_array(offset, it, it_end, TypeInterpreter<int32_t>(little_endiann), elements_per_row, *str);
          break;
        case dumptype::dumptype_uint64:
          print_byte_array(offset, it, it_end, TypeInterpreter<uint64_t>(little_endiann), elements_per_row, *str);
          break;
        case dumptype::dumptype_int64:
          print_byte_array(offset, it, it_end, TypeInterpreter<int64_t>(little_endiann), elements_per_row, *str);
          break;
        case dumptype::dumptype_float:
          print_byte_array(offset, it, it_end, TypeInterpreter<float>(little_endiann), elements_per_row, *str);
          break;
        case dumptype::dumptype_double:
          print_byte_array(offset, it, it_end, TypeInterpreter<double>(little_endiann), elements_per_row, *str);
          break;
      }
      if (f.is_open())
        f.close();
    }
    
  }
}

int main(int argc, char** argv)
{
  if (argc > 1)
  {
    std::string input = std::string(argv[1]);
    std::vector<uint8_t> byte_arr = read_input(input);
    hex_interpret(byte_arr);
  }
  else
  {
    std::cout << "Usage:    hex_interpret \"<hex text dump>\"|<binfile>" << std::endl;
    std::cout << "Example:  hex_interpret \"01 01 AA FF CA 3F 27 28\"" << std::endl;
    std::cout << "          hex_interpret d3d12.dll" << std::endl;
  }
  return 0;
}
