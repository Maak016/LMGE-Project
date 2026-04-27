#include "engineGovernor.h"
#include "inputSystem.h"

//namespace engine
bool terminationStatus = false;
void engine::terminate() { terminationStatus = true; }
bool engine::engineTerminated() { return terminationStatus; }