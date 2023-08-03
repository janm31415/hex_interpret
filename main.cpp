#include <iostream>
#include <string>
#include <vector>

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
  
class UInt32InterpreterBigEndianness
  {
  public:
    void operator()(const std::vector<uint8_t>& characters, std::ostream& str)
      {
      auto it = characters.begin();
      const auto it_end = characters.end();
      while (it != it_end)
        {
        uint32_t number = 0;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        str << number << " ";
        }
      }
  };

class UInt64InterpreterBigEndianness
  {
  public:
    void operator()(const std::vector<uint8_t>& characters, std::ostream& str)
      {
      auto it = characters.begin();
      const auto it_end = characters.end();
      while (it != it_end)
        {
        uint64_t number = 0;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        str << number << " ";
        }
      }
  };
  
class UInt32InterpreterLittleEndianness
  {
  public:
    void operator()(const std::vector<uint8_t>& characters, std::ostream& str)
      {
      auto it = characters.begin();
      const auto it_end = characters.end();
      while (it != it_end)
        {
        uint32_t number = 0;
        std::vector<uint32_t> values;
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        for (auto rit = values.rbegin(); rit != values.rend(); ++rit)
          {
          number = (number << 8) | *rit;
          }
        str << number << " ";
        }
      }
  };
  
class UInt64InterpreterLittleEndianness
  {
  public:
    void operator()(const std::vector<uint8_t>& characters, std::ostream& str)
      {
      auto it = characters.begin();
      const auto it_end = characters.end();
      while (it != it_end)
        {
        uint64_t number = 0;
        std::vector<uint64_t> values;
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        for (auto rit = values.rbegin(); rit != values.rend(); ++rit)
          {
          number = (number << 8) | *rit;
          }
        str << number << " ";
        }
      }
  };
  
class FloatInterpreterBigEndianness
  {
  public:
    void operator()(const std::vector<uint8_t>& characters, std::ostream& str)
      {
      auto it = characters.begin();
      const auto it_end = characters.end();
      while (it != it_end)
        {
        uint32_t number = 0;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        float f = *reinterpret_cast<float*>(&number);
        str << f << " ";
        }
      }
  };
  
class DoubleInterpreterBigEndianness
  {
  public:
    void operator()(const std::vector<uint8_t>& characters, std::ostream& str)
      {
      auto it = characters.begin();
      const auto it_end = characters.end();
      while (it != it_end)
        {
        uint64_t number = 0;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        if (it != it_end)
          number = (number << 8) | *it++;
        double f = *reinterpret_cast<double*>(&number);
        str << f << " ";
        }
      }
  };
    
  
class FloatInterpreterLittleEndianness
  {
  public:
    void operator()(const std::vector<uint8_t>& characters, std::ostream& str)
      {
      auto it = characters.begin();
      const auto it_end = characters.end();
      while (it != it_end)
        {
        uint32_t number = 0;
        std::vector<uint32_t> values;
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        for (auto rit = values.rbegin(); rit != values.rend(); ++rit)
          {
          number = (number << 8) | *rit;
          }
        float f = *reinterpret_cast<float*>(&number);
        str << f << " ";
        }
      }
  };
  
class DoubleInterpreterLittleEndianness
  {
  public:
    void operator()(const std::vector<uint8_t>& characters, std::ostream& str)
      {
      auto it = characters.begin();
      const auto it_end = characters.end();
      while (it != it_end)
        {
        uint64_t number = 0;
        std::vector<uint64_t> values;
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        if (it != it_end)
          values.push_back(*it++);
        for (auto rit = values.rbegin(); rit != values.rend(); ++rit)
          {
          number = (number << 8) | *rit;
          }
        double f = *reinterpret_cast<double*>(&number);
        str << f << " ";
        }
      }
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
 
void hex_interpret(const std::string& hex)
  {
  auto ba = hex_to_byte_array(hex);
  //print_byte_array(0, ba.begin(), ba.end(), SimpleInterpreter(), 16, std::cout);
  print_byte_array(0, ba.begin()+0, ba.end(), UInt32InterpreterLittleEndianness(), 12, std::cout);
  //print_byte_array(4, ba.begin()+4, ba.end(), FloatInterpreterLittleEndianness(), 12, std::cout);
  }

int main(int argc, char** argv)
  {
  std::string hex_text;
  if (argc > 1)
    {
    hex_text = std::string(argv[1]);
    }
  else
    {
    std::cout << "Enter your hex text: ";
    std::cin >> hex_text;
    }
  hex_interpret(hex_text);
  return 0;
  }
