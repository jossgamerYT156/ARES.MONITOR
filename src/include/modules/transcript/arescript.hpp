#pragma once
#ifndef ARESCRIPT_HPP
#define ARESCRIPT_HPP
// These values are EXPLICIT to Arescript.
// I do this so i don't accidentlly use this somewhere else and cause a fucking mess.
// Code cleanup and all that shit.
namespace ARES::MODULES::ARESCRIPT {
  std::string arescript_version = "0.0.323-alpha";
  std::string arescript_rev = "2026-01-01";

  // Error code definitions for Arescript pre-processing.
  enum AresErrorCode {
    ARES_OK = 0,
  
    // Script errors (100–199)
    ARES_FILE_NOT_FOUND = 100,
    ARES_INVALID_DIRECTIVE = 101,
    ARES_INVALID_GROUP_VALUE = 102,
    ARES_WRAP_MISSING_ARGS = 103,
    ARES_UNKNOWN_RULE = 104,
    ARES_INVALID_SECTION = 106,
    ARES_INVALID_ASSIGN = 107,
  
    // Structural errors (200–299)
    ARES_STRICT_GROUP_MISMATCH = 200,
    ARES_EMPTY_INPUT = 201,
  
    // Transformation errors (300–399)
    ARES_DELIM_NO_SPACE = 300,
    ARES_VARIABLE_UNDEFINED = 301,

    // Internal failures (900+)
    ARES_INTERNAL_ERROR = 900
  };

  enum class WDelimMode {
    NONE,
    WRAP,
    QUOTE_SINGLE,
    QUOTE_DOUBLE,
    QPERCENT,
    KEY_BARE_VALUE_QUOTED
  };
}
#endif