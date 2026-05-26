#ifndef LENTUS_VARIABLE_HPP
#define LENTUS_VARIABLE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

enum class variable_type : int
{
    NONE,
    BOOLEAN,
    NUMBER,
    STRING,
    LIST,
    TABLE,
    FUNCTION,
};

class expression;
class variable;
class statement;
class function;

struct variable_hasher
{
    std::size_t operator()(const variable &var) const;
};

struct variable_equal
{
    bool operator()(const variable &lhs, const variable &rhs) const;
};

class variable_block
{
public:
    virtual std::string to_string() const = 0;
    virtual ~variable_block() = default;
};

class lentus_list : public variable_block
{
private:
    std::vector<variable> elements;

public:
    lentus_list() : elements() {}
    lentus_list(const std::vector<variable> &elements) : elements(elements) {}
    lentus_list(std::vector<variable> &&elements) : elements(std::move(elements)) {}

    variable &operator[](size_t index) { return elements[index]; }
    const variable &operator[](size_t index) const { return elements[index]; }

    variable &at(size_t index) { return elements.at(index); }
    const variable &at(size_t index) const { return elements.at(index); }

    void push_back(const variable &var) { elements.push_back(var); }
    void push_back(variable &&var) { elements.push_back(std::move(var)); }

    void pop_back() { elements.pop_back(); }
    void remove(size_t index) { elements.erase(elements.begin() + index); }

    size_t size() const { return elements.size(); }
    std::vector<variable> &get_elements() { return elements; }
    const std::vector<variable> &get_elements() const { return elements; }

    std::vector<variable>::iterator begin() { return elements.begin(); }
    std::vector<variable>::iterator end() { return elements.end(); }
    std::vector<variable>::const_iterator begin() const { return elements.begin(); }
    std::vector<variable>::const_iterator end() const { return elements.end(); }

    lentus_list &operator=(const std::vector<variable> &new_elements)
    {
        elements = new_elements;
        return *this;
    }
    lentus_list &operator=(std::vector<variable> &&new_elements)
    {
        elements = std::move(new_elements);
        return *this;
    }

    std::string to_string() const override { return "<list @" + std::to_string((size_t)this) + ">"; }
};

class lentus_table : public variable_block
{
private:
    std::unordered_map<variable, variable, variable_hasher, variable_equal> entries;

public:
    lentus_table() : entries(0, variable_hasher(), variable_equal()) {}
    lentus_table(const std::unordered_map<variable, variable, variable_hasher, variable_equal> &entries) : entries(entries) {}
    lentus_table(std::unordered_map<variable, variable, variable_hasher, variable_equal> &&entries) : entries(std::move(entries)) {}

    variable &operator[](const variable &key) { return entries[key]; }
    const variable &operator[](const variable &key) const { return entries.at(key); }

    variable &at(const variable &key) { return entries.at(key); }
    const variable &at(const variable &key) const { return entries.at(key); }

    void remove(const variable &key) { entries.erase(key); }
    bool contains(const variable &key) const { return entries.contains(key); }
    size_t size() const { return entries.size(); }

    std::unordered_map<variable, variable, variable_hasher, variable_equal> &get_entries() { return entries; }
    const std::unordered_map<variable, variable, variable_hasher, variable_equal> &get_entries() const { return entries; }

    std::unordered_map<variable, variable, variable_hasher, variable_equal>::iterator begin() { return entries.begin(); }
    std::unordered_map<variable, variable, variable_hasher, variable_equal>::iterator end() { return entries.end(); }
    std::unordered_map<variable, variable, variable_hasher, variable_equal>::const_iterator begin() const { return entries.begin(); }
    std::unordered_map<variable, variable, variable_hasher, variable_equal>::const_iterator end() const { return entries.end(); }

    lentus_table &operator=(const std::unordered_map<variable, variable, variable_hasher, variable_equal> &new_entries)
    {
        entries = new_entries;
        return *this;
    }
    lentus_table &operator=(std::unordered_map<variable, variable, variable_hasher, variable_equal> &&new_entries)
    {
        entries = std::move(new_entries);
        return *this;
    }

    std::string to_string() const override { return "<table @" + std::to_string((size_t)this) + ">"; }
};

class lentus_string : public variable_block
{
private:
    std::string value;

public:
    lentus_string() : value("") {}
    lentus_string(const std::string &value) : value(value) {}
    lentus_string(std::string &&value) : value(std::move(value)) {}

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
 * Represents a variable in the Lentus language.
 * Copying a variable (by constructor or operator=) creates a shallow copy of the variable.
 * This means that for the string, list and table types, the pointer to the underlying data structure is copied, but the data structure itself is not copied.
 * The variable class also doesn't perform memory management for the pointers.
 */
class variable
{
private:
    variable_type type;
    union
    {
        bool boolean;
        double number;
        lentus_string *string;
        lentus_list *list;
        lentus_table *table;
        function *fn;
    };

public:
    /**
     * Constructor for the none variable type.
     */
    variable() : type(variable_type::NONE) {}
    variable(variable_type type);
    variable(bool value) : type(variable_type::BOOLEAN), boolean(value) {}
    variable(double value) : type(variable_type::NUMBER), number(value) {}
    variable(int value) : type(variable_type::NUMBER), number(value) {}

    variable(lentus_string *value) : type(variable_type::STRING), string(value) {}

    variable(lentus_list *value) : type(variable_type::LIST), list(value) {}

    variable(lentus_table *value) : type(variable_type::TABLE), table(value) {}

    variable(function *value) : type(variable_type::FUNCTION), fn(value) {}

    variable(const variable &var);
    variable(variable &&var);

    variable &operator=(const variable &var);
    variable &operator=(variable &&var);

    variable_type get_type() const { return type; }
    bool is(variable_type type) const { return this->type == type; }
    std::string get_type_name() const;

    void write_to_string_recursive(std::string &str) const;
    std::string to_string_recursive() const;
    std::string to_string() const;

    ~variable();

    bool operator==(const variable &var) const;
    bool operator!=(const variable &var) const { return !(*this == var); }

    bool operator<=(const variable &var) const;
    bool operator<(const variable &var) const;

    bool operator>=(const variable &var) const { return !(*this < var); }
    bool operator>(const variable &var) const { return !(*this <= var); }

    bool get_boolean() const;
    double get_number() const;

    lentus_string *get_string();
    lentus_list *get_list();
    lentus_table *get_table();
    function *get_function();

    const lentus_string *get_string() const;
    const lentus_list *get_list() const;
    const lentus_table *get_table() const;
    const function *get_function() const;
};

/**
 * A reference to a variable.
 * If the reference is a modifiable reference, the reference holds a pointer to the variable
 * If the reference is a direct reference, the reference holds the variable directly to avoid heap allocation
 *
 * Modifiable references to an list or table should not be held after the list or table could potentially be reallocated.
 */
class reference
{
private:
    bool is_modifiable;
    union
    {
        variable *modifiable_value;
        variable direct_value;
    };

public:
    /**
     * Constructs a direct reference with the value "none".
     */
    reference() : is_modifiable(false), direct_value() {}

    /**
     * Constructs a modifiable reference from a pointer.
     */
    reference(variable *var) : is_modifiable(true), modifiable_value(var) {}

    /**
     * Constructs a direct reference. The variable is moved into the reference.
     */
    reference(variable &&var) : is_modifiable(false), direct_value(std::move(var)) {}

    /**
     * Constructs a direct reference. The variable is copied into the reference.
     */
    reference(const variable &var) : is_modifiable(false), direct_value(var) {}

    reference(const reference &other)
    {
        if (other.is_modifiable)
        {
            modifiable_value = other.modifiable_value;
            is_modifiable = true;
        }
        else
        {
            direct_value = other.direct_value;
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
            direct_value = std::move(other.direct_value);
            is_modifiable = false;
        }
    }

    const variable get() const { return is_modifiable ? *modifiable_value : direct_value; }
    const variable operator*() const { return is_modifiable ? *modifiable_value : direct_value; }
    const variable *operator->() const { return is_modifiable ? modifiable_value : &direct_value; }

    /**
     * If the reference is a direct value, care should be taken to not modify the variable. This is not checked.
     */
    variable &operator*() { return is_modifiable ? *modifiable_value : direct_value; }

    /**
     * If the reference is a direct value, care should be taken to not modify the variable. This is not checked.
     */
    variable &get() { return is_modifiable ? *modifiable_value : direct_value; }

    /**
     * If the reference is a direct value, care should be taken to not modify the variable. This is not checked.
     */
    variable *operator->() { return is_modifiable ? modifiable_value : &direct_value; }

    /**
     * Assigns a variable to the reference (modifiable)
     */
    reference &operator=(variable *var)
    {
        modifiable_value = var;
        is_modifiable = true;
        return *this;
    }
    /**
     * Assigns a variable to the reference (direct)
     */
    reference &operator=(variable &&var)
    {
        direct_value = std::move(var);
        is_modifiable = false;
        return *this;
    }

    /**
     * Assigns a variable to the reference (direct)
     */
    reference &operator=(const variable &var)
    {
        direct_value = var;
        is_modifiable = false;
        return *this;
    }

    reference &operator=(const reference &other)
    {
        if (this != &other)
        {
            if (other.is_modifiable)
            {
                modifiable_value = other.modifiable_value;
                is_modifiable = true;
            }
            else
            {
                direct_value = other.direct_value;
                is_modifiable = false;
            }
        }
        return *this;
    }

    reference &operator=(reference &&other)
    {
        if (this != &other)
        {
            if (other.is_modifiable)
            {
                modifiable_value = other.modifiable_value;
                is_modifiable = true;
            }
            else
            {
                direct_value = std::move(other.direct_value);
                is_modifiable = false;
            }
        }
        return *this;
    }

    bool is_modifiable_reference() const { return is_modifiable; }

    ~reference() {};
};

#endif