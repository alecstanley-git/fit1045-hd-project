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
  Int,
  Double,
  Object,
};

struct JsonValue
{
  JsonType type;
  union
  {
    int i;
    double d;
    std::map<std::string, JsonValue> *json;
  };
};

void ReadFile(const std::string &path, std::string &output);

JsonValue ParsePrimitive(const std::string &text, std::string::iterator start, std::string::iterator end);

JsonValue ParseJsonHelper(const std::string &text, std::string::iterator &it);

std::pair<std::string, JsonValue> RetrieveKeyValuePair(const std::string &text, std::string::iterator &it);

JsonValue ParseJson(const std::string &path);

// Recursively frees any nested objects owned by `value`.
void FreeJson(JsonValue &value);

// Safe lookup: returns a pointer to the value stored under `key`, or nullptr if
// `object` is not an Object or the key is absent. Unlike map::operator[], this
// never inserts a garbage entry.
const JsonValue *Lookup(const JsonValue &object, const std::string &key);

// Typed getters. They read a number regardless of whether it was stored as an
// int or a double, and return `fallback` if the key is missing or non-numeric.
int GetInt(const JsonValue &object, const std::string &key, int fallback = 0);
double GetDouble(const JsonValue &object, const std::string &key, double fallback = 0.0);

#endif
