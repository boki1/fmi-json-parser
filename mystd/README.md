# mystd

#### What is that?

Since this is a project implemented for my OOP course @fmi, I was forced to
adhere only to the following standard headers and not use any other. However, I
couldn't manage both this requirement and the task of satisfying my personal
desire for a beautiful implementation at the same time without implementing
some of these highly value by me structures. Therefore this is "mystd" -
implementation of (probably [1]) `variant`, `optional`, `unordered_map`,
`expected` and probably some more.

#### How is this used?

I have used feature toggles for each of "mystd" header with the pattern
"USE\_MYSTD\_\*" where  at the end stands the data structure name. By enabling
it through `cmake` my implementation is going to be used. Of course, these
implementations are standard complined (at least as far as user API is
considered :smile:).

-----

Remark [1]: At the time of writing this I am only guessing what I am going to
use since the implementation is definately not final yet :smile:.
