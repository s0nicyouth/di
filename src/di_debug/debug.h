#include <string>

#ifndef DEBUG_H_
#define DEBUG_H_

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

#ifndef RELEASE_BUILD
    #define DCHECK(cond, msg) cond ? void() : debug::printDebug("[DCHECK " + std::string(__FILE__) + ":" + std::string(LINE_STRING) + "]: " + std::string(msg))
#else
    #define DCHECK(cond, msg)
#endif

namespace debug {
void printDebug(const std::string& message);
}

#endif  // DEBUG_H_
