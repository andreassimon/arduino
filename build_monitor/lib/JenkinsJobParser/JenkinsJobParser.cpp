#include "JenkinsJobParser.h"

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

JenkinsJobParser::JenkinsJobParser() {
  currentKeyIndex = 0;

  openingDelimiters = (char*)malloc(MAX_DELIMITERS);
  lastDelimiterIndex = -1;

  expectedLiteralValue = (char*)malloc(6);
  nextExpectedLiteralValueIndex = 0;

  color = String("");
}

String JenkinsJobParser::getColor() {
  return color;
}

void JenkinsJobParser::processHeaderChar(const char c) {
  switch(c) {
    case VERTICAL_TAB:
      return;
    case '\n':
      if(parserState == PARSER_IN_HEADER__CURRENT_ROW_IS_EMPTY) {
        parserState = PARSER_IN_BODY;
        return;
      }
      parserState = PARSER_IN_HEADER__CURRENT_ROW_IS_EMPTY;
      return;
    default:
      parserState = PARSER_IN_HEADER;
  }
}

void JenkinsJobParser::processBodyChar(const char c) {
  switch(c) {
    case '"':
      parserState = PARSER_IN_KEY;
  }
}

void JenkinsJobParser::processKeyChar(const unsigned char c) {
  switch(c) {
    case '"':
      currentKey[currentKeyIndex] = '\0';
      if(strncmp("color", currentKey, strlen("color")) == 0) {
        parserState = PARSER_AFTER_COLOR_KEY;
      } else {
        parserState = PARSER_AFTER_KEY;
      }
      return;
  }
  if(currentKeyIndex < MAX_KEY_LEN) {
    currentKey[currentKeyIndex] = c;
    currentKeyIndex++;
  }
}

void JenkinsJobParser::pushDelimiter(const char c) {
  lastDelimiterIndex++;
  if(lastDelimiterIndex >= MAX_DELIMITERS) {
    Serial.print("ERROR: The maximum number of delimiters (");
    Serial.print(MAX_DELIMITERS);
    Serial.print(") is exceeded, you try to push '");
    Serial.print(c);
    Serial.println("'");
    parserState = PARSER_ERROR;
    return;
  }
  openingDelimiters[lastDelimiterIndex] = c;
}

void JenkinsJobParser::processDelimitedValueChar(const char c) {
  char lastDelimiter;
  switch(c) {
    case '[':
    case '{':
      pushDelimiter(c);
      break;
    case '"':
      if('"' == peekDelimiter()) {
        popDelimiter();
      } else {
        pushDelimiter(c);
      }
      break;
    case ']':
      lastDelimiter = peekDelimiter();
      if('[' == lastDelimiter) {
        popDelimiter();
      } else {
        Serial.print("'");
        Serial.print(openingDelimiters);
        Serial.println("'");
        Serial.print("ERROR: Parser expected last opening delimiter to be '[', but was '");
        Serial.print(lastDelimiter);
        Serial.println("'");
        parserState = PARSER_ERROR;
      }
      break;
    case '}':
      lastDelimiter = peekDelimiter();
      if('{' == lastDelimiter) {
        popDelimiter();
      } else {
        Serial.print("ERROR: Parser expected last opening delimiter to be '{', but was '");
        Serial.print(lastDelimiter);
        Serial.println("'");
        parserState = PARSER_ERROR;
      }
      break;
  }
  if(noDelimitersOpen()) {
    parserState = PARSER_VALUE_CLOSED;
  }
}

void JenkinsJobParser::processColorChar(const char c) {
  if('"' == c) {
    parserState = PARSER_VALUE_CLOSED;
    return;
  }
  color += c;
}

void JenkinsJobParser::processResponseChar(const char c) {
  parsedCharacters++;
  switch(parserState) {
    case PARSER_IN_HEADER:
    case PARSER_IN_HEADER__CURRENT_ROW_IS_EMPTY:
      processHeaderChar(c);
      return;
    case PARSER_IN_BODY:
      processBodyChar(c);
      return;
    case PARSER_IN_KEY:
      processKeyChar(c);
      return;
    case PARSER_AFTER_KEY:
      if(':' != c) {
        Serial.print("ERROR @ char ");
        Serial.print(parsedCharacters);
        Serial.print(": Parser expected '");
        Serial.print(':');
        Serial.print("' (");
        Serial.print((int)':', HEX);
        Serial.print("), but found '");
        Serial.print(c);
        Serial.print("' (");
        Serial.print((int)c, HEX);
        Serial.println(")");
        parserState = PARSER_ERROR;
        return;
      }
      parserState = PARSER_BEFORE_VALUE;
      return;
    case PARSER_AFTER_COLOR_KEY:
      if(':' != c) {
        Serial.print("ERROR @ char ");
        Serial.print(parsedCharacters);
        Serial.print(": Parser expected '");
        Serial.print(':');
        Serial.print("' (");
        Serial.print((int)':', HEX);
        Serial.print("), but found '");
        Serial.print(c);
        Serial.print("' (");
        Serial.print((int)c, HEX);
        Serial.println(")");
        parserState = PARSER_ERROR;
        return;
      }
      parserState = PARSER_BEFORE_COLOR_VALUE;
      return;
    case PARSER_BEFORE_COLOR_VALUE:
      if('"' == c) {
        parserState = PARSER_IN_COLOR_VALUE;
        color = String("");
      } else {
        Serial.print("ERROR: Parser expected '\"', but found '");
        Serial.print(c);
        Serial.println("'");
        parserState = PARSER_ERROR;
      }
      return;
    case PARSER_IN_COLOR_VALUE:
      processColorChar(c);
      return;
    case PARSER_BEFORE_VALUE:
      switch(c) {
        case '{':
        case '[':
        case '}':
        case ']':
        case '"':
          parserState = PARSER_IN_DELIMITED_VALUE;
          resetDelimiters();
          processDelimitedValueChar(c);
          break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          parserState = PARSER_IN_NUMERIC_VALUE;
          break;
        case 'f':
          expectedLiteralValue = (char*)"false";
          nextExpectedLiteralValueIndex = 1;
          parserState = PARSER_IN_LITERAL_VALUE;
          break;
        case 'n':
          expectedLiteralValue = (char*)"null";
          nextExpectedLiteralValueIndex = 1;
          parserState = PARSER_IN_LITERAL_VALUE;
          break;
        case 't':
          expectedLiteralValue = (char*)"true";
          nextExpectedLiteralValueIndex = 1;
          parserState = PARSER_IN_LITERAL_VALUE;
          break;
        default:
          Serial.print("ERROR: Parser can not detect JSON value starting with '");
          Serial.print(c);
          Serial.println("'");
          parserState = PARSER_ERROR;
      }
      return;
    case PARSER_IN_LITERAL_VALUE:
      if(expectedLiteralValue[nextExpectedLiteralValueIndex] == c) {
        nextExpectedLiteralValueIndex++;

        if(nextExpectedLiteralValueIndex >= strlen(expectedLiteralValue)) {
          parserState = PARSER_VALUE_CLOSED;
        }
      } else {
        Serial.print("ERROR: Parser expected '");
        Serial.print(expectedLiteralValue[nextExpectedLiteralValueIndex]);
        Serial.print("' as part of literal '");
        Serial.print(expectedLiteralValue);
        Serial.print("', but found '");
        Serial.print(c);
        Serial.println("'");
        parserState = PARSER_ERROR;
      }
      return;
    case PARSER_IN_DELIMITED_VALUE:
      processDelimitedValueChar(c);
      return;
    case PARSER_IN_NUMERIC_VALUE:
      if('0' <= c && c <= '9') {
        return;
      } else if(',' == c) {
        parserState = PARSER_IN_BODY;
      }
      return;
    case PARSER_VALUE_CLOSED:
      switch(c) {
        case ',':
          parserState = PARSER_IN_BODY;
          break;
        case '}':
          parserState = PARSER_FINISHED;
          break;
        default:
          Serial.print("ERROR: Parser expected ',' or '}', but found '");
          Serial.print(c);
          Serial.println("'");
          parserState = PARSER_ERROR;
      }
      return;
    case PARSER_FINISHED:
      Serial.print("ERROR: Parser expected no more characters, but found '");
      Serial.print(c);
      Serial.println("'");
      parserState = PARSER_ERROR;
  }
}

void JenkinsJobParser::reset() {
  parserState = PARSER_IN_HEADER;
  parsedCharacters = 0;
}

void JenkinsJobParser::resetDelimiters() {
  lastDelimiterIndex = -1;
}

boolean JenkinsJobParser::noDelimitersOpen() {
  return lastDelimiterIndex < 0;
}

void JenkinsJobParser::popDelimiter() {
  lastDelimiterIndex--;
}

char JenkinsJobParser::peekDelimiter() {
  return openingDelimiters[lastDelimiterIndex];
}
