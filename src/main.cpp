#include <concepts>
#include <iostream>
#include <type_traits>
#include <print>

//================================
// 			FOO CHECK
//================================
// --------- PRE-CONCEPT --------- 
template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
void foo(T value) {
	std::cout << "Value is " << value << '\n';
};

template <typename T, std::enable_if_t<std::is_floating_point<T>::value>>
void foo2(T value) {
	std::cout << "Value is " << value << '\n';
}

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

void foo3(MyIntegralConcept auto value) {
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

int main()
{
	// foo2(1);
	std::printf("%d", add3(1, 2));

	return 0;
}