#ifndef RIN_DEBUG_SYSTEM_HPP
#define RIN_DEBUG_SYSTEM_HPP

#define BE_UNREACHABLE()                                                \
{ throw std::runtime_error("DEBUG: Received unreachable signal"); }

#define BE_ASSERT(EXPR)                                                 \
{ if (!(EXPR)) throw std::runtime_error("DEBUG: Assertion Failure"); }

#endif // RIN_DEBUG_SYSTEM_HPP
