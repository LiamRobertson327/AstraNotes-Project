#include "IPlugin.h"

// Even if the class is an interface, the Q_OBJECT macro 
// requires a translation unit (.cpp) to hook into the Meta-Object System.
// No extra code is needed here since your methods are pure virtual (= 0).