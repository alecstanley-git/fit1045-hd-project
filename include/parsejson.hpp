#ifndef PARSEJSON_H
#define PARSEJSON_H

#include <map>
#include <string>
#include <utility>

/*
I felt the need to build in a way to store files that contain information about different collision parameters. This way, all I'd need to do is upload a single file to my application that initialises the parameters automatically.
This does three things:
1. Saves time
2. Allows for repeatability
  - If someone is tinkering with the app and finds an interesting configuration, they can repeat it easily
3. Will help me demonstrate my program to others
  - It is much more interesting to a viewer if I skip the process of initialising all the objects
*/

/*
I roughly followed this guide's structure:
https://dev.to/uponthesky/c-making-a-simple-json-parser-from-scratch-250g
*/

// Tags which member of the union below is currently active, so a consumer
// can read the value back safely.
enum class JsonType
{
  Null, // no valid value (e.g. file missing/unreadable/malformed)
  Int,
  Double,
  Object,
};

struct JsonValue
{
  JsonType type; // Tag saying which union member below is the live one

  // A union lets several members share the exact same chunk of memory, so the
  // struct is only ever as big as its largest member rather than the sum of all
  // of them. Only one member is valid at a time. I've used it here because any
  // given JSON value is exactly one of these things: an int, a double, or a
  // nested object, never more than one at once. The 'type' tag above records
  // which member is currently live so callers know which one is safe to read
  // (reading the wrong member is undefined behaviour).
  union
  {
    int i;                                  // Live when type == Int
    double d;                               // Live when type == Double
    std::map<std::string, JsonValue> *json; // Live when type == Object (heap-owned, see FreeJson)
  };
};

// Reads the whole file at 'path' into 'output', preserving newlines.
// @param const std::string &path - filesystem path of the file to read
// @param std::string &output - output parameter that receives the file contents
// @return bool - false if the file could not be opened, true otherwise
bool ReadFile(const std::string &path, std::string &output);

// Parses the text between [start, end) as a numeric primitive (int or double).
// @param const std::string &text - the full source text being parsed
// @param std::string::iterator start - iterator to the first character of the value
// @param std::string::iterator end - iterator one past the last character of the value
// @return JsonValue - an Int or Double value depending on the text
JsonValue ParsePrimitive(const std::string &text, std::string::iterator start, std::string::iterator end);

// Parses a JSON object ({...}) starting at 'it', advancing 'it' past it.
// Throws std::runtime_error if the braces are missing.
// @param const std::string &text - the full source text being parsed
// @param std::string::iterator &it - cursor into the text, advanced past the object in place
// @return JsonValue - an Object value owning the parsed key/value map
JsonValue ParseJsonHelper(const std::string &text, std::string::iterator &it);

// Parses a single "key": value entry, advancing 'it' past it (and any comma).
// Throws std::runtime_error if the key quoting or colon separator is missing.
// @param const std::string &text - the full source text being parsed
// @param std::string::iterator &it - cursor into the text, advanced past the pair in place
// @return std::pair<std::string, JsonValue> - the parsed key and its value
std::pair<std::string, JsonValue> RetrieveKeyValuePair(const std::string &text, std::string::iterator &it);

// Top-level entry point: reads the file at 'path' and parses it into a JsonValue.
// Returns a Null value (not an exception) if the file is missing or malformed,
// so callers can simply fall back to their defaults.
// @param const std::string &path - filesystem path of the JSON file to parse
// @return JsonValue - the parsed Object, or a Null value on any failure
JsonValue ParseJson(const std::string &path);

// Recursively frees any nested objects owned by 'value'.
// @param JsonValue &value - the value whose heap-owned object map is freed in place
void FreeJson(JsonValue &value);

// Safe lookup: returns a pointer to the value stored under 'key', or nullptr if
// 'object' is not an Object or the key is absent. Unlike map::operator[], this
// never inserts a garbage entry.
// @param const JsonValue &object - the Object value to search within
// @param const std::string &key - the key to look up
// @return const JsonValue * - pointer to the stored value, or nullptr if absent
const JsonValue *Lookup(const JsonValue &object, const std::string &key);

// Typed getters. They read a number regardless of whether it was stored as an
// int or a double, and return 'fallback' if the key is missing or non-numeric.

// @param const JsonValue &object - the Object value to read from
// @param const std::string &key - the key to look up
// @param int fallback - value returned when the key is missing or non-numeric
// @return int - the stored number as an int, or 'fallback'
int GetInt(const JsonValue &object, const std::string &key, int fallback = 0);

// @param const JsonValue &object - the Object value to read from
// @param const std::string &key - the key to look up
// @param double fallback - value returned when the key is missing or non-numeric
// @return double - the stored number as a double, or 'fallback'
double GetDouble(const JsonValue &object, const std::string &key, double fallback = 0.0);

#endif
