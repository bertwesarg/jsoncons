### jsoncons::jsonpath::result_options

```c++
enum class result_options 
{
   value = 1,
   path = 2,
   nodups = 4 | path
};
```

A [BitmaskType](https://en.cppreference.com/w/cpp/named_req/BitmaskType) 
used to specify result options for `json_query` 
