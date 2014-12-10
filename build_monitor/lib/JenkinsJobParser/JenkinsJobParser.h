#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

const int VERTICAL_TAB = 13;

const byte PARSER_IN_HEADER = 0;
const byte PARSER_IN_HEADER__CURRENT_ROW_IS_EMPTY = 1;
const byte PARSER_IN_BODY = 2;
const byte PARSER_IN_KEY = 3;
const byte PARSER_AFTER_KEY = 4;
const byte PARSER_AFTER_COLOR_KEY = 5;
const byte PARSER_BEFORE_VALUE = 6;
const byte PARSER_BEFORE_COLOR_VALUE = 7;
const byte PARSER_IN_DELIMITED_VALUE = 8;
const byte PARSER_IN_COLOR_VALUE = 9;
const byte PARSER_IN_LITERAL_VALUE = 10;
const byte PARSER_IN_NUMERIC_VALUE = 11;
const byte PARSER_VALUE_CLOSED = 12;
const byte PARSER_FINISHED = 13;
const byte PARSER_ERROR = 255;

const unsigned int MAX_KEY_LEN = 5;
const signed int MAX_DELIMITERS = 20;

class JenkinsJobParser {

  byte parserState;
  unsigned int parsedCharacters;
  char currentKey[MAX_KEY_LEN];
  unsigned int currentKeyIndex;
  char* openingDelimiters;
  int lastDelimiterIndex;
  char* expectedLiteralValue;
  unsigned int nextExpectedLiteralValueIndex;
  String color;

  public:
  JenkinsJobParser();

  String getColor();
  void processHeaderChar(const char c);
  void processBodyChar(const char c);
  void processKeyChar(const unsigned char c);
  void pushDelimiter(const char c);
  void processDelimitedValueChar(const char c);
  void processColorChar(const char c);
  void processResponseChar(const char c);
  void reset();
  void resetDelimiters();
  boolean noDelimitersOpen();
  void popDelimiter();
  char peekDelimiter();

};
