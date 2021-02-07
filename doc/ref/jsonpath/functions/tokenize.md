### tokenize

```
array[string] tokenize(string input, string pattern)
```

Returns an array of strings formed by splitting the input string into an array of strings, separated by substrings that match the provided regular expression pattern.

Returns `null` if either argument is not a string.

### Example

```c++
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

using json = jsoncons::json;
namespace jsonpath = jsoncons::jsonpath;

int main() 
{
    std::string data = R"(
{
    "books":
    [
        {
            "title" : "A Wild Sheep Chase",
            "author" : "Haruki Murakami"
        },
        {
            "title" : "Almost Transparent Blue",
            "author" : "Ryu Murakami"
        },
        {
            "title" : "The Quiet American",
            "author" : "Graham Greene"
        }
    ]
}
    )";

    json j = json::parse(data);

    // All titles whose author's last name is 'Murakami'
    std::string expr = R"($.books[?(tokenize(@.author,'\\s+')[-1] == 'Murakami')].title)";

    json result = jsonpath::json_query(j, expr);
    std::cout << pretty_print(result) << "\n\n";
}
```

Output:

```json
[
    "A Wild Sheep Chase",
    "Almost Transparent Blue"
]
```