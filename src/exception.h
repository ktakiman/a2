#pragma once

enum EParseErrorCode {
  gRegexError,
  gIndentCount
};

class ParseException : std::exception {
  public:
    ParseException(EParseErrorCode code) : Code(code) {}
    EParseErrorCode Code;
};

