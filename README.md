# C++ Template Constraints Exploration

This code demonstrates various approaches to constraining templates in C++, from traditional SFINAE techniques to modern C++20 concepts.

## Table of Contents
1. [Traditional SFINAE Approaches](#traditional-sfinae-approaches)
2. [C++20 Concepts](#c20-concepts)
3. [Requires Expressions](#requires-expressions)
4. [Mocking and Testing](#mocking-and-testing)

## Traditional SFINAE Approaches

### `foo1` - Type Alias SFINAE
```cpp
template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
void foo1(T value);
```
- Uses a default template parameter with `std::enable_if_t`
- The function is only available when `T` is integral
- The second template parameter becomes `void` when the condition is true

### `foo2` - Non-type Template Parameter SFINAE
```cpp
template <typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
void foo2(T value);
```
- Uses a non-type template parameter with default value `true`
- More readable than the type alias approach

### `foo3` - Return Type SFINAE
```cpp
template <typename T>
std::enable_if_t<std::is_floating_point<T>::value> foo3(T value);
```
- Uses `std::enable_if_t` in the return type
- The return type becomes `void` when the condition is met

## C++20 Concepts

### Basic Concept Definition
```cpp
template<typename T>
concept IntegralConcept = std::is_integral_v<T>;

template<typename T>
concept MyIntegralConcept = MyIntegral<T>::value;
```
- Concepts provide a more readable and expressive way to specify constraints
- Can be built from type traits or custom traits

### Concept Usage in Functions
```cpp
void fooC(MyIntegralConcept auto value);
template<MyIntegralConcept I> I add2(I x, I y);
template<typename T> requires std::is_integral_v<T> T add3(T a, T b);
```
- Three ways to apply concepts:
  1. `ConceptName auto` parameter
  2. Template parameter with concept
  3. `requires` clause

### Operator Overloading with Concepts
```cpp
Vec3 operator+(const Vec3 v, std::integral auto s);
```
- Concepts can be used to constrain operator overloads
- Here, the scalar must be an integral type

## Requires Expressions

### Simple Requirement
```cpp
template<typename T>
concept C = requires(T p[2]) {
    (decltype(p)) nullptr;
    (int*) nullptr;
};
```
- Checks if expressions are valid
- In this case, verifies pointer conversions

### Compound Requirements
```cpp
template<typename T>
concept UserTypeConcept = 
    requires(T t) {
        { t.username } -> std::convertible_to<std::string>;
    } && requires {
        { std::declval<T>().email } -> std::convertible_to<std::string>;
    };
```
- `{ expression } -> concept` syntax checks both:
  - The expression is valid
  - The result satisfies the concept
- `std::declval<T>()` allows checking without constructing an object

## Mocking and Testing

### Digital Input Concept
```cpp
template <typename T>
concept DigitalInputConcept = requires {
    { std::declval<T>().init() } -> std::same_as<void>;
    { std::declval<T>().read() } -> std::same_as<int>;
};
```
- Defines interface requirements for digital input devices
- Both return types are checked precisely

### Concept-Based Button Class
```cpp
template<DigitalInputConcept DIn>
class ButtonWithConcept {
    // Uses constrained template parameter
};
```
- Clean, readable constraint syntax
- Clear error messages when constraints are violated

### SFINAE-Based Alternative
```cpp
template<typename T>
class is_digital_input {
    // Complex SFINAE detection using decltype and overload resolution
};

template <typename DIn, typename = std::enable_if_t<is_digital_input<DIn>::value>>
class ButtonWithSfinae;
```
- More verbose and harder to read
- Achieves the same goal as concepts but with more boilerplate

## Key Benefits of Concepts Over SFINAE

1. **Readability**: Concepts provide clear, self-documenting constraints
2. **Better Error Messages**: Compiler errors directly reference concept violations
3. **Expressiveness**: More natural syntax for expressing requirements
4. **Maintainability**: Easier to understand and modify constraints

## Testing Strategy

The code demonstrates testing with mock objects:
- `MockedDigitalInput` and `MockedDigitalInput2` simulate hardware
- Both concept and SFINAE approaches can use the same mock
- Invalid types like `MalformedInput` are correctly rejected at compile time

This exploration shows the evolution of template constraints in C++, with concepts representing a significant improvement in usability and clarity over traditional SFINAE techniques.