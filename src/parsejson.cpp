#include <cctype>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <stdexcept>
#include "parsejson.hpp"

// A "no value" result, returned whenever parsing can't proceed (file missing,
// unreadable, or malformed). Because its type is not Object, Lookup() returns
// nullptr for any key, so consumers fall back to their defaults and skip.
static JsonValue NullValue()
{
    JsonValue v;
    v.type = JsonType::Null;
    v.json = nullptr;
    return v;
}

// Reads the whole file into a string (preserving newlines so the parser sees
// the original text exactly).
bool ReadFile(const std::string &path, std::string &output)
{
    std::ifstream file(path);
    if (!file.is_open())
        return false; // file not found / unreadable -> caller skips silently

    std::stringstream ss;
    ss << file.rdbuf();
    output = ss.str();
    return true;
}

// Advances `it` past any insignificant whitespace, stopping at end of text.
static void SkipWhitespace(const std::string &text, std::string::iterator &it)
{
    while (it != text.end() && (*it == ' ' || *it == '\n' || *it == '\t' || *it == '\r'))
    {
        it++;
    }
}

// Parse the primitive (the actual value of each entry) as either double or int
JsonValue ParsePrimitive(const std::string &text, std::string::iterator start, std::string::iterator end)
{
    JsonValue value;

    std::string substr = text.substr(start - text.begin(), end - start);

    // Anything with a decimal point or an exponent is a double; otherwise int.
    bool is_double = substr.find('.') != std::string::npos ||
                     substr.find('e') != std::string::npos ||
                     substr.find('E') != std::string::npos;

    if (is_double)
    {
        value.type = JsonType::Double;
        value.d = std::stod(substr);
    }
    else
    {
        value.type = JsonType::Int;
        value.i = std::stoi(substr);
    }

    return value;
}

JsonValue ParseJsonHelper(const std::string &text, std::string::iterator &it)
{
    SkipWhitespace(text, it);
    if (it == text.end() || *it != '{') // must start with a left curly brace
        throw std::runtime_error("expected '{'");
    it++;

    std::map<std::string, JsonValue> *json_map = new std::map<std::string, JsonValue>;

    SkipWhitespace(text, it);
    while (it != text.end() && *it != '}')
    {
        const auto [key, value] = RetrieveKeyValuePair(text, it);
        (*json_map)[key] = value;

        SkipWhitespace(text, it);
    }

    if (it == text.end() || *it != '}') // must close with a right curly brace
        throw std::runtime_error("expected '}'");
    it++;

    JsonValue result;
    result.type = JsonType::Object;
    result.json = json_map;
    return result;
}

std::pair<std::string, JsonValue> RetrieveKeyValuePair(const std::string &text, std::string::iterator &it)
{
    SkipWhitespace(text, it);

    // a double quote opens the key
    if (it == text.end() || *it != '\"')
        throw std::runtime_error("expected '\"' opening key");
    std::string::iterator key_start = ++it;
    while (it != text.end() && *it != '\"')
    {
        it++;
    }

    std::string key = text.substr(key_start - text.begin(), it - key_start);

    if (it == text.end() || *it != '\"') // closing quote of the key
        throw std::runtime_error("expected '\"' closing key");
    it++;

    SkipWhitespace(text, it);
    if (it == text.end() || *it != ':')
        throw std::runtime_error("expected ':'");
    it++;

    // Get the corresponding value
    SkipWhitespace(text, it);

    JsonValue value;
    if (it != text.end() && *it == '{')
    {
        value = ParseJsonHelper(text, it);
    }
    else
    {
        std::string::iterator val_start = it;
        while (it != text.end() &&
               (std::isdigit(static_cast<unsigned char>(*it)) ||
                *it == '.' || *it == '-' || *it == '+' || *it == 'e' || *it == 'E'))
        {
            it++;
        }
        value = ParsePrimitive(text, val_start, it);
    }

    // move past a comma if there is another element
    SkipWhitespace(text, it);
    if (it != text.end() && *it == ',')
    {
        it++;
    }

    return std::make_pair(key, value);
}

JsonValue ParseJson(const std::string &path)
{
    std::string text;
    if (!ReadFile(path, text) || text.empty())
        return NullValue(); // missing/unreadable/empty -> caller skips silently

    try
    {
        std::string::iterator start = text.begin();
        return ParseJsonHelper(text, start);
    }
    catch (const std::exception &)
    {
        return NullValue(); // malformed JSON -> skip silently instead of crashing
    }
}

const JsonValue *Lookup(const JsonValue &object, const std::string &key)
{
    if (object.type != JsonType::Object || object.json == nullptr)
    {
        return nullptr;
    }

    auto found = object.json->find(key);
    if (found == object.json->end())
    {
        return nullptr;
    }

    return &found->second;
}

int GetInt(const JsonValue &object, const std::string &key, int fallback)
{
    const JsonValue *value = Lookup(object, key);
    if (value == nullptr)
    {
        return fallback;
    }

    if (value->type == JsonType::Int)
    {
        return value->i;
    }
    if (value->type == JsonType::Double)
    {
        return static_cast<int>(value->d);
    }

    return fallback;
}

double GetDouble(const JsonValue &object, const std::string &key, double fallback)
{
    const JsonValue *value = Lookup(object, key);
    if (value == nullptr)
    {
        return fallback;
    }

    if (value->type == JsonType::Double)
    {
        return value->d;
    }
    if (value->type == JsonType::Int)
    {
        return static_cast<double>(value->i);
    }

    return fallback;
}

void FreeJson(JsonValue &value)
{
    if (value.type == JsonType::Object && value.json != nullptr)
    {
        for (auto &[key, child] : *value.json)
        {
            FreeJson(child);
        }
        delete value.json;
        value.json = nullptr;
    }
}
