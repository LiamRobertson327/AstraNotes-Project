#pragma once

// Qt 6.5.x can require stdext-related declarations on newer MSVC toolchains
// when compiling in latest standard mode. Pulling in <iterator> early restores
// those declarations for Qt headers.
#include <iterator>
