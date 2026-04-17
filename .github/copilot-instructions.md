# Code Review Instructions

Strictly follow Google's C++ style guide. It's okay to use the one you know
during training time, no need to look it up on the internet unless you don't
remember certain details. Exceptions are listsed below.

## Key things to remember

- Headers are `.h`, implementations are `.cc`.
- Use CamelCase for function and method names.
- We're using C++23, although the style guide does not recommend C++23 yet.
- Do not use `auto` type. Use `using ...` in an anonymous namespace (`.cc` files
  only), add aliases for commonly used custom types if the type is too long.
- Throwing exceptions is strictly forbidden. Use `std::expect` to communicate
  possible error states. Try / catch blocks are only permitted if a third-party
  library may throw, but the return should be a result type. If the type name
  becomes too long with `std::expected`, create aliases for status and result
  types similar to Abseil's `Status` and `StatusOr<T>`.
- Use `std::string_view` for immutable string parameters in instead of
  `const std::string&`, use `std::span` for immutable array parameters instead
  of `const std::vector<T>&`.
