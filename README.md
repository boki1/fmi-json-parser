# fmi-json-parser

![JSON logo](http://www.logicum.co/wp-content/uploads/2013/10/JSON.png)

**Description**

Implement a JSON parser command line application. However I will implement a
JSON parser library and a demo application which uses this library as an
example as this seems way more natural to me :). These are contained in `lib/`
and `app/` respectively. In addition there are some tests in the corresponding
directory. In another addition, I am required to use only a certain part of the
standard library as that is what is the OOP course @fmi is based upon which
means that from time to time I will have to implement a certain type myself.
Check the `mystd/` directory for them.

[_Problem description in bulgarian._](https://docs.google.com/document/d/1yGwTjf8gskWtwMzavdfM4g3cZAkf-ZNiJQaht1i083o/edit#heading=h.vros15jkdqoa)

[![build-and-tests](https://github.com/boki1/fmi-json-parser/actions/workflows/ci.yml/badge.svg)](https://github.com/boki1/fmi-json-parser/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

**Building the source code**

```bash
$ ./build.sh --output=cmake-build-debug
```

**Building the documentation**

```bash
$ ./build.sh --docs open
```

**License**

MIT, Kristiyan Stoimenov 2023
