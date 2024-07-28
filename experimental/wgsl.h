#ifndef GPU_CPP_WGSL_H
#define GPU_CPP_WGSL_H

#include <string>
#include <regex>
#include "utils/logging.h"        // LOG

namespace gpu {

// Loop-unrolling optimization with regex
//
// Note: Be cautious, as it does not correctly recognize comments or lexical tokens.
std::string loopUnrolling(const std::string& code, int threshold = 32) {
  // This regex pattern matches a for loop with the following structure:
  // for (var <varName>: u32 = <start>; <varName> < <end>; <varName>++) { <loopBody> }
  std::regex forLoopPattern(R"(for\s*\(\s*var\s+(\w+):\s*u32\s*=\s*(\d+)\s*;\s*\1\s*<\s*(\d+)\s*;\s*\1\+\+\s*\)\s*\{\s*([^{}]*)\})");
  // Explanation of the regex:
  // for\s*\(                        : Matches 'for (' with optional whitespace
  // \s*var\s+                       : Matches 'var ' with optional whitespace
  // (\w+)                           : Captures the variable name (alphanumeric characters and underscores)
  // :\s*u32\s*=\s*                  : Matches ': u32 = ' with optional whitespace
  // (\d+)                           : Captures the start value (one or more digits)
  // \s*;\s*                         : Matches ';' with optional whitespace
  // \1\s*<\s*                       : Matches the captured variable name followed by '<' with optional whitespace
  // (\d+)                           : Captures the end value (one or more digits)
  // \s*;\s*                         : Matches ';' with optional whitespace
  // \1\+\+\s*                       : Matches the captured variable name followed by '++' with optional whitespace
  // \)\s*\{\s*                      : Matches ')' followed by '{' with optional whitespace
  // ([^{}]*)                        : Captures the loop body (anything except '{' or '}')
  // \}                              : Matches the closing '}'

  // Example:
  //
  // Input code:
  // for (var i: u32 = 0; i < 3; i++) { std::cout << i << std::endl; }
  //
  // Matches:
  //   varName = "i"
  //   start = "0"
  //   end = "3"
  //   loopBody = "std::cout << i << std::endl;"
  //
  // Unrolled:
  // std::cout << 0 << std::endl;
  // std::cout << 1 << std::endl;
  // std::cout << 2 << std::endl;  
  //
  std::smatch match;
  std::string unrolledCode = code;
  while (std::regex_search(unrolledCode, match, forLoopPattern)) {
    std::string varName = match[1];
    int start = std::stoi(match[2]);
    int end = std::stoi(match[3]);
    std::string loopBody = match[4];

    if (end - start > threshold ) {
      std::string skippedLoop =
	"for (var " +
	std::string(match[1]) + ": u32 = " + std::string(match[2]) + ";"+
	std::string(match[1]) + " < " + std::string(match[3]) + ";"+
	std::string(match[1]) + "++) /* Skipped */ {"+
	std::string(match[4]) +
	"}";
      // LOG(kDefLog, kInfo, "Roll loop:%s", skippedLoop.c_str());
      unrolledCode = unrolledCode.substr(0, match.position()) + skippedLoop + unrolledCode.substr(match.position() + match.length());
    } else {
      // LOG(kDefLog, kInfo, "Unroll loop(var: %s, start:%d, end:%d, body:%s)", varName.c_str(), start, end, loopBody.c_str());
      std::string unrolledLoop;
      for (int i = start; i < end; ++i) {
        std::string unrolledIteration = loopBody;
        std::regex varPattern(varName);
        unrolledIteration = std::regex_replace(unrolledIteration, varPattern, std::to_string(i));
        unrolledLoop += unrolledIteration;
      }
      unrolledCode = unrolledCode.substr(0, match.position()) + unrolledLoop + unrolledCode.substr(match.position() + match.length());
    }
  }

  return unrolledCode;
}


std::string removeUnnecessaryIfStatements(const std::string& code) {
  // Pattern to match if(true) {...} else {...}
  std::regex ifTrueElsePattern(R"(if\s*\(\s*true\s*\)\s*\{([^{}]*)\}\s*else\s*\{([^{}]*)\})");
  // Pattern to match if(false) {...} else {...}
  std::regex ifFalseElsePattern(R"(if\s*\(\s*false\s*\)\s*\{([^{}]*)\}\s*else\s*\{([^{}]*)\})");
  // Pattern to match if(true) {...}
  std::regex ifTruePattern(R"(if\s*\(\s*true\s*\)\s*\{([^{}]*)\})");
  // Pattern to match if(false) {...}
  std::regex ifFalsePattern(R"(if\s*\(\s*false\s*\)\s*\{([^{}]*)\})");

  std::string optimizedCode = code;
  std::smatch match;

  // Handle if(true) {...} else {...}
  while (std::regex_search(optimizedCode, match, ifTrueElsePattern)) {
    std::string trueBlock = match[1].str();
    optimizedCode = optimizedCode.substr(0, match.position()) + trueBlock + optimizedCode.substr(match.position() + match.length());
  }

  // Handle if(false) {...} else {...}
  while (std::regex_search(optimizedCode, match, ifFalseElsePattern)) {
    std::string elseBlock = match[2].str();
    optimizedCode = optimizedCode.substr(0, match.position()) + elseBlock + optimizedCode.substr(match.position() + match.length());
  }

  // Handle if(true) {...}
  while (std::regex_search(optimizedCode, match, ifTruePattern)) {
    std::string trueBlock = match[1].str();
    optimizedCode = optimizedCode.substr(0, match.position()) + trueBlock + optimizedCode.substr(match.position() + match.length());
  }

  // Handle if(false) {...}
  while (std::regex_search(optimizedCode, match, ifFalsePattern)) {
    optimizedCode = optimizedCode.substr(0, match.position()) + optimizedCode.substr(match.position() + match.length());
  }

  return optimizedCode;
}

} // namespace gpu

#endif
