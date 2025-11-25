#include <concepts>
#include <iostream>
#include <type_traits>
#include <print>
#include <cassert>

//================================
// 			FOO CHECK
//================================
// --------- PRE-CONCEPT --------- 
template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
void foo1(T value) {
	std::cout << "Value is " << value << '\n';
};

template <typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
void foo2(T value) {
	std::cout << "Value is " << value << '\n';
};

template <typename T>
std::enable_if_t<std::is_floating_point<T>::value> foo3(T value) {
	std::cout << "Value is " << value << '\n';
};

// --------- CONCEPT --------- 

template<typename T>
concept IntegralConcept = std::is_integral_v<T>;

template <typename T> struct MyIntegral {
	static constexpr bool value = false;
};

template <> struct MyIntegral<int> {
	static constexpr bool value = true;
};

template<typename T>
concept MyIntegralConcept = MyIntegral<T>::value;

static_assert(IntegralConcept<int>);
// static_assert(IntegralConcept<float>);
static_assert(MyIntegralConcept<int>);
// static_assert(IntegralConcept<float>);

void fooC(MyIntegralConcept auto value) {
	std::cout << "Value is " << value << '\n';
}

//================================
// 			ADD CHECK
//================================
template<typename T>
T add(T t, T u) {
	return t + u;
}

template<MyIntegralConcept I>
I add2(I x, I y) {
	return x + y;
}

template<typename T>
requires std::is_integral_v<T>
T add3(T a, T b) {
	return a + b;
}

struct Vec3 {
	float e0;
	float e1;
	float e2;
};

// add each vector member with a scalar
Vec3 operator+(const Vec3 v, std::integral auto s) {
	return Vec3 { .e0 = v.e0 + s, .e1 = v.e1 + s, .e2 = v.e2 + s };
}

//================================
// 			REQUIRES
//================================
template<typename T>
concept C = requires(T p[2])
{
	(decltype(p)) nullptr;
	(int*) nullptr;
};

static_assert(C<int*>); // checking if int* satisfies the expression

struct User {
	std::string username;
	std::string email;
};


template<typename T>
concept UserTypeConcept = 
	requires(T t) {
		{ t.username } -> std::convertible_to<std::string>;
		/*
			- requires is the definition / identity of the constraint that concept will use
			- bagian kiri bisa dibaca: { t.username } results in std::string&, which is a reference to User::username
			- bagian kanan bisa dibaca: std::convertible_to<std::string&, std::string>;
		*/
	} && requires {
		{ std::declval<T>().email } -> std::convertible_to<std::string>;
	};

template<typename T>
concept UserTypeDeclValConcept = requires {
	{ std::declval<T>().username } -> std::convertible_to<std::string>;
};

struct MalformedUser {
	int username;
};

static_assert(UserTypeConcept<User>);
static_assert(UserTypeDeclValConcept<User>);
// static_assert(UserType<MalformedUser>); // will fail, because username type is int
static_assert(std::same_as<int, int>);

//================================
// 			MOCKING
//================================
// --------- GENERAL --------- 
class DigitalIn { // This is NOT used, it acts as reference to be "followed" by Mocks without doing polimorphism, enforced by Concept & SFINAE based trait detection (by creating custom type traits)
public:
	void init();
	int read();
};

class MockedDigitalInput {
public:
	void init() {}
	int read() { return value_; }

	void set_value(int v) { value_ = v; }
private:
	int value_{};
};

class MalformedDigitalInput { /* no init(), no read() */ };

// --------- SFINAE --------- 
// Old Sfinae (C++11)
template<typename T>
class is_digital_input_old_sfinae_type_trait {
private:
	template<typename U>
	static auto test_init(int) -> decltype(std::declval<U>().init(), std::true_type{});
	
	template<typename U>
	static auto test_init(...) -> std::false_type;

	template<typename U>
	static auto test_read(int) -> decltype(std::declval<U>().read(), std::true_type{});
	
	template<typename U>
	static auto test_read(...) -> std::false_type;

public:
	static constexpr bool value =
		decltype(test_init<T>(0))::value &&
		decltype(test_read<T>(0))::value;
};

// Newer Sfinae (C++17)
template<typename T, typename = void>
struct is_digital_input_new_sfinae_type_trait : std::false_type {};

template<typename T>
struct is_digital_input_new_sfinae_type_trait<T, std::void_t<
    decltype(std::declval<T>().init()),
    decltype(std::declval<T>().read())
>> : std::true_type {};

template<typename T>
constexpr bool is_digital_input_new_sfinae_type_trait_v = is_digital_input_new_sfinae_type_trait<T>::value;

// Concepts (C++20)
template<typename T>
concept digital_input_concept = requires(T t) { t.init(); t.read(); };

// ---- C++11 ---- 
// template <typename DIn, typename = std::enable_if_t<is_digital_input_old_sfinae_type_trait<DIn>::value>>
// class ButtonWithSfinae {
//     static_assert(is_digital_input_old_sfinae_type_trait<DIn>::value, 
//                   "DIn must have init() and read() methods");

// ---- C++17 (only static_assert) ---- 
// template <typename DIn>
// class ButtonWithSfinae {
//     static_assert(std::is_same_v<decltype(std::declval<DIn>().init()), void> &&
//                   std::is_same_v<decltype(std::declval<DIn>().read()), int>, 
//                   "DIn must have init() and read() methods");

// ---- C++17 (Sfinae) ---- 
template <typename DIn, typename = std::enable_if_t<is_digital_input_new_sfinae_type_trait_v<DIn>>>
class ButtonWithSfinae {
    static_assert(is_digital_input_new_sfinae_type_trait_v<DIn>, 
                  "DIn must have init() and read() methods");

// ---- C++20 ---- 
// template <digital_input_concept DIn>
// class ButtonWithSfinae {
public:
	ButtonWithSfinae(DIn *input) : digitalInput_(input) {}

	void init() {
		digitalInput_->init();
		// rest of logic...
	}

	int read() {
		return digitalInput_->read();
	}

private:	
	DIn *digitalInput_;
};

void test_button_sfinae() {
	MockedDigitalInput input;
	ButtonWithSfinae<MockedDigitalInput> button(&input);

	input.set_value(42);
	std::cout << button.read() << std::endl;
	assert(button.read() == 42);
}

// --------- CONCEPT --------- 
template <typename T>
concept DigitalInputConcept = requires {
	{ std::declval<T>().init() } -> std::same_as<void>;
	{ std::declval<T>().read() } -> std::same_as<int>;
};

template<DigitalInputConcept DIn>
class ButtonWithConcept {
public:
	ButtonWithConcept(DIn* input): digitalInput_(input) {}
	void init() {
		digitalInput_->init();
		// rest of logic...
	}
	int read() {
		return digitalInput_->read();
	}
private:
	DIn *digitalInput_;
};

void test_button_concept() {
	MockedDigitalInput input;
	ButtonWithConcept<MockedDigitalInput> button(&input);

	input.set_value(100);
	std::println("{}", button.read());
	assert(button.read() == 100);
}

void test_malformed_button() {
    MalformedDigitalInput malformedInput;
    // ButtonWithSfinae<MalformedDigitalInput> buttonSfin(&malformedInput);  // SFINAE in action! Compile error
    // ButtonWithConcept<MalformedDigitalInput> buttonCon(&malformedInput);  // SFINAE in action! Compile error
}

//================================
// 			MOCKING 2
//================================
class DigitalSensor {
public:
    void init() { std::cout << "Sensor initialized\n"; }
    bool read() { return true; }
};

class AnalogSensor {
public:
    void setup() {}  // wrong method names
    int getValue() { return 42; }
};

template<typename T>
constexpr bool is_digital_input_v = 
    requires(T t) { 
        t.init(); 
        t.read(); 
    };

// Template constraints
template<typename T>
void processSensor(T& sensor) {
	if constexpr (is_digital_input_v<T>) {
		sensor.init();
		bool data = sensor.read();
		std::cout << "Digital data: " << data << "\n";
	} else {
		std::cout << "Not a digital input sensor\n";
	}
}

// Usage
void test_digital_sensor() {
    std::cout << "DigitalSensor is digital input: " 
              << (is_digital_input_v<DigitalSensor> ? "true" : "false") << "\n";  // true
    
    std::cout << "AnalogSensor is digital input: " 
              << (is_digital_input_v<AnalogSensor> ? "true" : "false") << "\n";   // false
    
    DigitalSensor ds;
    AnalogSensor as;
    processSensor(ds);  // Uses digital path
    processSensor(as);  // Uses fallback path
}

//================================
// 			MAIN
//================================
int main()
{
	std::println("-------- FOO CHECK --------");
	foo1(1);
	foo2(2.5f);
	foo3(3.7f);
	
	std::println("-------- ADD CHECK --------");
	std::println("{}", add3(1, 2));
	
	std::println("-------- MOCKING 1 --------");
	test_button_concept();
	test_button_sfinae();
	test_malformed_button();
	
	std::println("-------- MOCKING 2 --------");
	test_digital_sensor();

	return 0;
}