#include <iostream>
#include <cassert>
#include "../src/app/Clock.hpp"

// Simple test macros without Catch2
#define TEST_ASSERT(condition, message) \
  do { \
    if (!(condition)) { \
      std::cerr << "FAILED: " << message << std::endl; \
      std::cerr << "  at line " << __LINE__ << std::endl; \
      return false; \
    } else { \
      std::cout << "PASSED: " << message << std::endl; \
    } \
  } while(0)

bool test_clock_60_per_second() {
  std::cout << "\n=== Testing Clock ~60 ticks per second ===" << std::endl;
  
  Clock c; 
  c.dt = 1.0/60.0;
  
  for (int i = 0; i < 1000; i++) {
    c.step(0.001); // 1.000s total
  }
  
  std::cout << "Expected: 59-61 ticks, Actual: " << c.ticks << " ticks" << std::endl;
  TEST_ASSERT((c.ticks >= 59 && c.ticks <= 61), "Clock ticks should be ~60 per second");
  
  return true;
}

bool test_clock_90_for_1_5_seconds() {
  std::cout << "\n=== Testing Clock ~90 ticks for 1.5 seconds ===" << std::endl;
  
  Clock c; 
  c.dt = 1.0/60.0;
  
  for (int i = 0; i < 1500; i++) {
    c.step(0.001); // 1.500s total
  }
  
  std::cout << "Expected: 89-91 ticks, Actual: " << c.ticks << " ticks" << std::endl;
  TEST_ASSERT((c.ticks >= 89 && c.ticks <= 91), "Clock ticks should be ~90 for 1.5 seconds");
  
  return true;
}

bool test_clock_accumulator_precision() {
  std::cout << "\n=== Testing Clock accumulator precision ===" << std::endl;
  
  Clock c;
  c.dt = 1.0/60.0;
  
  // Test sub-dt steps
  c.step(0.01); // 0.6 of a 60Hz step
  TEST_ASSERT(c.ticks == 0, "Should have 0 ticks for sub-dt step");
  TEST_ASSERT(c.accumulator > 0.009 && c.accumulator < 0.011, "Accumulator should preserve sub-dt time");
  
  c.step(0.01); // Now we should have enough for 1 tick
  TEST_ASSERT(c.ticks == 1, "Should have 1 tick after accumulating enough time");
  
  return true;
}

int main() {
  std::cout << "Arena Engine - Clock Tests (Simplified)" << std::endl;
  std::cout << "========================================" << std::endl;
  
  int passed = 0;
  int total = 0;
  
  // Run all tests
  ++total; if (test_clock_60_per_second()) ++passed;
  ++total; if (test_clock_90_for_1_5_seconds()) ++passed;
  ++total; if (test_clock_accumulator_precision()) ++passed;
  
  std::cout << "\n========================================" << std::endl;
  std::cout << "Test Results: " << passed << "/" << total << " tests passed" << std::endl;
  
  if (passed == total) {
    std::cout << "All tests PASSED! 🎉" << std::endl;
    return 0;
  } else {
    std::cout << "Some tests FAILED! ❌" << std::endl;
    return 1;
  }
}
