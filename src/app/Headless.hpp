#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Function to run headless loop for specified milliseconds and return tick count
int RunHeadlessForMs(int ms, int tickHz);

#ifdef __cplusplus
}
#endif
