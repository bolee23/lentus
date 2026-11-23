#ifndef LENTUS_DATA_HPP
#define LENTUS_DATA_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace lentus
{

/**
 * Enumeration of all value types in the Lentus language.
 */
enum class value_type : int
{
    NONE,
    BOOLEAN,
    NUMBER,
    STRING,
    LIST,
    TABLE,
    FUNCTION, // Encompasses both native and user-defined functions
};

class expression;
class value;
class statement;

struct value_hasher
{
    std::size_t operator()(const value &var) const;
};

struct value_equal
{
    bool operator()(const value &lhs, const value &rhs) const;
};

/**
 * Base class for a dynamically allocated block of memory to store a complex value type
 * (string, list, table or function). This is used to manage memory for these types of variables.
 */
class value_block
{
public:
    virtual std::string to_string() const = 0;
    virtual ~value_block() = default;
};

class table;
class string;
class list;
class function;

/**
 * Represents a value in the Lentus language.
 * This class does not perform memory management for complex types.
 * If copied using operator=, shallow copies are made for performance.
 * Memory management is handled by the program_class.
 */
class value
{
private:
    value_type type;
    union
    {
        // Simple data
        bool boolean;
        double number;
        // Complex data
        string *string_value;
        list *list_value;
        table *table_value;
        function *fn;
    };

public:
    /**
     * Constructor for the none value type.
     */
    value() : type(value_type::NONE) {}
    value(value_type type);
    value(bool value) : type(value_type::BOOLEAN), boolean(value) {}
    value(double value) : type(value_type::NUMBER), number(value) {}
    value(int value) : type(value_type::NUMBER), number(value) {}

    /**
     * Creates a string value that holds a pointer to the given string. 
     * No memory management is performed.
     */
    value(string *val) : type(value_type::STRING), string_value(val) {}

    /**
     * Creates a list value that holds a pointer to the given list.
     * No memory management is performed.
     */
    value(list *val) : type(value_type::LIST), list_value(val) {}

    /**
     * Creates a table value that holds a pointer to the given table.
     * No memory management is performed.
     */
    value(table *val) : type(value_type::TABLE), table_value(val) {}

    /**
     * Creates a function value that holds a pointer to the given function.
     * No memory management is performed.
     */
    value(function *value) : type(value_type::FUNCTION), fn(value) {}

    /**
     * Shallow copy constructor for the value class. 
     * Complex types are not deep copied.
     * @param var The value to copy
     */
    value(const value &var);

    /**
     * Move constructor for the value class.
     * @param var The value to move
     */
    value(value &&var);

    /**
     * Performs a shallow copy of the value.
     * No deallocations are performed for overwriting complex types.
     * @param var The value to copy
     * @return A reference to this value after assignment
     */
    value &operator=(const value &var);

    /**
     * Performs a shallow move of the value.
     * No deallocations are performed for overwriting complex types.
     * @param var The value to move
     * @return A reference to this value after assignment
     */
    value &operator=(value &&var);

    value_type get_type() const { return type; }
    bool is(value_type type) const { return this->type == type; }
    std::string get_type_name() const;

    void write_to_string_recursive(std::string &str) const;
    std::string to_string_recursive() const;
    std::string to_string() const;

    /**
     * Destructor does not perform any deallocations.
     */
    ~value();

    bool operator==(const value &var) const;
    bool operator!=(const value &var) const { return !(*this == var); }

    bool operator<=(const value &var) const;
    bool operator<(const value &var) const;

    bool operator>=(const value &var) const { return !(*this < var); }
    bool operator>(const value &var) const { return !(*this <= var); }

    bool get_boolean() const;
    double get_number() const;

    string *get_string();
    list *get_list();
    table *get_table();
    function *get_function();

    const string *get_string() const;
    const list *get_list() const;
    const table *get_table() const;
    const function *get_function() const;
};

/**
 * Represents a table (map, associative array, dictionary) value in the Lentus language.
 */
class table : public value_block
{
private:
    std::unordered_map<value, value, value_hasher, value_equal> entries;

public:
    table() : entries(0, value_hasher(), value_equal()) {}
    table(const std::unordered_map<value, value, value_hasher, value_equal> &entries) : entries(entries) {}
    table(std::unordered_map<value, value, value_hasher, value_equal> &&entries) : entries(std::move(entries)) {}

    /**
     * Inserts a key-value pair into the table. If the key already exists, the value is overwritten.
     */
    void add(value key, value val) { entries.insert_or_assign(key, val); }

    /**
     * Accesses the value associated with the key. Throws std::out_of_range if the key does not exist in the table.
     */
    value &at(const value &key) { return entries.at(key); }

    /**
     * Accesses the value associated with the key. Throws std::out_of_range if the key does not exist in the table.
     */
    const value &at(const value &key) const { return entries.at(key); }

    /**
     * Accesses a copy of the value associated with the key. Returns std::nullopt if the key does not exist.
     */
    std::optional<value> get(const value &key) const
    {
        auto it = entries.find(key);
        if (it != entries.end())
        {
            return it->second;
        }
        else
        {
            return std::nullopt;
        }
    }

    value &operator[](const value &key) { return entries[key]; }
    const value &operator[](const value &key) const { return entries.at(key); }

    /**
     * Removes the key-value pair associated with the key. Returns true if the key was found and removed, false otherwise.
     */
    bool remove(const value &key) { return entries.erase(key) > 0; }

    /**
     * Checks if the table contains the key.
     */
    bool contains(const value &key) const { return entries.contains(key); }
    size_t size() const { return entries.size(); }

    std::unordered_map<value, value, value_hasher, value_equal> &get_entries() { return entries; }
    const std::unordered_map<value, value, value_hasher, value_equal> &get_entries() const { return entries; }

    std::unordered_map<value, value, value_hasher, value_equal>::iterator begin() { return entries.begin(); }
    std::unordered_map<value, value, value_hasher, value_equal>::iterator end() { return entries.end(); }
    std::unordered_map<value, value, value_hasher, value_equal>::const_iterator begin() const { return entries.begin(); }
    std::unordered_map<value, value, value_hasher, value_equal>::const_iterator end() const { return entries.end(); }

    table &operator=(const std::unordered_map<value, value, value_hasher, value_equal> &new_entries)
    {
        entries = new_entries;
        return *this;
    }
    table &operator=(std::unordered_map<value, value, value_hasher, value_equal> &&new_entries)
    {
        entries = std::move(new_entries);
        return *this;
    }

    table &operator=(const table &other)
    {
        entries = other.entries;
        return *this;
    }

    table &operator=(table &&other)
    {
        entries = std::move(other.entries);
        return *this;
    }

    std::string to_string() const override { return "<table @" + std::to_string((size_t)this) + ">"; }
};

/**
 * Represents a list (array) value in the Lentus language.
 * Internally uses a vector to store elements.
 */
class list : public value_block
{
private:
    std::vector<value> elements;

public:
    list() : elements() {}
    list(const std::vector<value> &elements) : elements(elements) {}
    list(std::vector<value> &&elements) : elements(std::move(elements)) {}

    value &operator[](size_t index) { return elements[index]; }
    const value &operator[](size_t index) const { return elements[index]; }

    value &at(size_t index) { return elements.at(index); }
    const value &at(size_t index) const { return elements.at(index); }

    void push_back(const value &var) { elements.push_back(var); }
    void push_back(value &&var) { elements.push_back(std::move(var)); }

    void pop_back() { elements.pop_back(); }
    void remove(size_t index) { elements.erase(elements.begin() + index); }

    size_t size() const { return elements.size(); }
    std::vector<value> &get_elements() { return elements; }
    const std::vector<value> &get_elements() const { return elements; }

    std::vector<value>::iterator begin() { return elements.begin(); }
    std::vector<value>::iterator end() { return elements.end(); }
    std::vector<value>::const_iterator begin() const { return elements.begin(); }
    std::vector<value>::const_iterator end() const { return elements.end(); }

    list &operator=(const std::vector<value> &new_elements)
    {
        elements = new_elements;
        return *this;
    }
    list &operator=(std::vector<value> &&new_elements)
    {
        elements = std::move(new_elements);
        return *this;
    }

    std::string to_string() const override { return "<list @" + std::to_string((size_t)this) + ">"; }
};

/**
 * Represents a string value in the Lentus language.
 */
class string : public value_block
{
private:
    std::string value;

public:
    string() : value("") {}
    string(const std::string &value) : value(value) {}
    string(std::string &&value) : value(std::move(value)) {}

    const std::string &get_value() const { return value; }
    std::string &get_value() { return value; }

    const std::string &operator*() const { return value; }
    std::string &operator*() { return value; }

    const std::string *operator->() const { return &value; }
    std::string *operator->() { return &value; }

    void operator=(const std::string &new_value) { value = new_value; }
    void operator=(std::string &&new_value) { value = std::move(new_value); }

    std::string to_string() const override { return value; }
};

/**
 * A reference to a value.
 * If the reference is a modifiable reference, the reference holds a pointer to the value
 * If the reference is a direct reference, the reference holds the value directly to avoid heap allocation
 *
 * Modifiable references to an list or table should not be held after the list or table could potentially be reallocated.
 * This class does not perform any checks or memory management.
 */
class reference
{
private:
    bool is_modifiable;
    union
    {
        value *modifiable_value;
        value direct_value;
    };

public:
    /**
     * Constructs a direct reference with the value "none".
     */
    reference() : is_modifiable(false), direct_value() {}

    /**
     * Constructs a modifiable reference from a pointer.
     */
    reference(value *var) : is_modifiable(true), modifiable_value(var) {}

    /**
     * Constructs a direct reference. The value is moved into the reference.
     */
    reference(value &&var) : is_modifiable(false), direct_value(std::move(var)) {}

    /**
     * Constructs a direct reference. The value is copied into the reference.
     */
    reference(const value &var) : is_modifiable(false), direct_value(var) {}

    reference(const reference &other)
    {
        if (other.is_modifiable)
        {
            modifiable_value = other.modifiable_value;
            is_modifiable = true;
        }
        else
        {
            new (&direct_value) value(other.direct_value);
            is_modifiable = false;
        }
    }

    reference(reference &&other)
    {
        if (other.is_modifiable)
        {
            modifiable_value = other.modifiable_value;
            is_modifiable = true;
        }
        else
        {
            new (&direct_value) value(std::move(other.direct_value));
            is_modifiable = false;
        }
    }

    const value get() const { return is_modifiable ? *modifiable_value : direct_value; }
    const value operator*() const { return is_modifiable ? *modifiable_value : direct_value; }
    const value *operator->() const { return is_modifiable ? modifiable_value : &direct_value; }

    /**
     * If the reference is a direct value, care should be taken to not modify the value. This is not checked.
     */
    value &operator*() { return is_modifiable ? *modifiable_value : direct_value; }

    /**
     * If the reference is a direct value, care should be taken to not modify the value. This is not checked.
     */
    value &get() { return is_modifiable ? *modifiable_value : direct_value; }

    /**
     * If the reference is a direct value, care should be taken to not modify the value. This is not checked.
     */
    value *operator->() { return is_modifiable ? modifiable_value : &direct_value; }

    /**
     * Assigns a value to the reference (modifiable)
     */
    reference &operator=(value *var)
    {
        if (!is_modifiable)
        {
            direct_value.~value();
        }
        is_modifiable = true;
        modifiable_value = var;
        return *this;
    }
    /**
     * Assigns a value to the reference (direct)
     */
    reference &operator=(value &&var)
    {
        if (!is_modifiable)
        {
            direct_value.~value();
        }
        is_modifiable = false;
        new (&direct_value) value(std::move(var));
        return *this;
    }

    /**
     * Assigns a value to the reference (direct)
     */
    reference &operator=(const value &var)
    {
        if (!is_modifiable)
        {
            direct_value.~value();
        }
        is_modifiable = false;
        new (&direct_value) value(var);
        return *this;
    }

    reference &operator=(const reference &other)
    {
        if (this != &other)
        {
            if (!is_modifiable)
            {
                direct_value.~value();
            }

            if (other.is_modifiable)
            {
                modifiable_value = other.modifiable_value;
                is_modifiable = true;
            }
            else
            {
                new (&direct_value) value(other.direct_value);
                is_modifiable = false;
            }
        }
        return *this;
    }

    reference &operator=(reference &&other)
    {
        if (this != &other)
        {
            if (!is_modifiable)
            {
                direct_value.~value();
            }

            if (other.is_modifiable)
            {
                modifiable_value = other.modifiable_value;
                is_modifiable = true;
            }
            else
            {
                new (&direct_value) value(std::move(other.direct_value));
                is_modifiable = false;
            }
        }
        return *this;
    }

    bool is_modifiable_reference() const { return is_modifiable; }

    ~reference()
    {
        if (!is_modifiable)
        {
            direct_value.~value();
        }
    }
};

} // namespace lentus

#endif
