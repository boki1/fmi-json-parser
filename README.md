# fmi-json-parser

**Description**

Implement a JSON parser command line application. However I will implement a
JSON parser library and a demo application which uses this library as an
example as this seems way more natural to me :). In addition there are some tests
in the corresponding directory.

_Note:_ By the task description I am very limited to the part of C++ that is used
throughout the course - only things contained in <iostream>, <fstream>, <new>,
<cstring>, <cmath>, <exception>, <stdexcept>, <string> and <vector>. Therefore on
occassion, the implementation may be somewhat bad :/. I may fix it after that;
we'll see.

**Building**

```bash
$ mkdir build
$ cd build
$ ccmake -S .. -B . -G Ninja
```

**License**

MIT, Kristiyan Stoimenov 2023
