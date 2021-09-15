#ifndef GSK_ASSERTIONS_H
#define GSK_ASSERTIONS_H

#if defined ASSERT_ENABLED

#include <iostream>
#include <cassert>

#define ASSERT(cond, msg) do { if(!(cond)) { std::cerr << msg << std::endl; assert(false); } } while(0)
#else
#define ASSERT(...)
#endif

#endif //GSK_ASSERTIONS_H
