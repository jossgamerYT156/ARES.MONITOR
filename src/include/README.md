# INCLUDES

Yes, i know these are mostly .cpp files.

i also know i can monolit this, but i won't.

i also know these use a lot of `#pragma once` statements, but that's to ensure stuff doesn't get re-defined on each f*cking file.

there's some issues, I KNOW. i am gonna fix them, someday.

a lot of stuff(individual functionality) is split into individual files, that's stuff like [\@HALT](hlt.cpp), [\@WRITE](write.cpp), [\@EXEC](runexternals.cpp) and so on so forth, that's because, they might break eventually, and i prefer this to be easily traceable than a monolith hell.