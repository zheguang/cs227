/*
 * Copyright (c) 2015 Brown University.
 */
#ifndef FATAL_ERROR_HPP
#define FATAL_ERROR_HPP

#define ERROR_MESSAGE_LEN 2048

#define throwFatalError(...) { char errorMessage[ERROR_MESSAGE_LEN]; snprintf(errorMessage, ERROR_MESSAGE_LEN, __VA_ARGS__); throw hmindex::FatalError(errorMessage, __FILE__, __LINE__); }

#include <stdexcept>

namespace hmindex {

  class FatalError : public std::runtime_error {
    public:
      FatalError(std::string reason, const char *filename, unsigned long lineno):
        runtime_error(reason), _reason(reason), _filename(filename), _lineno(lineno)
        {}

      virtual const char* what() const throw() {
        char errorMessage[ERROR_MESSAGE_LEN];
        std::snprintf(errorMessage, ERROR_MESSAGE_LEN, "Fatal error at %s:%ld: %s", _filename, _lineno, _reason.c_str());
        return std::string(errorMessage).c_str();
      }

    private:
      const std::string _reason;
      const char *_filename;
      const unsigned long _lineno;
  };
}
#endif
