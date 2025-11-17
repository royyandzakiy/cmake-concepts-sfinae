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
class DigitalIn {
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

// --------- SFINAE --------- 
template<typename T>
class is_digital_input {
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

template <typename DIn, typename = std::enable_if_t<is_digital_input<DIn>::value>>
class ButtonWithSfinae {
    static_assert(is_digital_input<DIn>::value, 
                  "DIn must have init() and read() methods");
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

void test_malformed_button() {
    MalformedDigitalInput malformedInput;
    // ButtonWithSfinae<MalformedDigitalInput> buttonSfin(&malformedInput);  // SFINAE in action! Compile error
    // ButtonWithConcept<MalformedDigitalInput> buttonCon(&malformedInput);  // SFINAE in action! Compile error
}

int main()
{
	foo1(1);
	foo2(2.5f);
	foo3(3.7f);
	std::println("{}", add3(1, 2));
	test_button_concept();
	test_button_sfinae();
	test_malformed_button();

	return 0;
}