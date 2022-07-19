#pragma once
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "Ws2_32.lib")

#include <SDKDDKVER.h>

#define NOATOM
#define NOGDI
#define NOGDICAPMASKS
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NORASTEROPS
#define NOSCROLL
#define NOSOUND
#define NOSYSMETRICS
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOCRYPT
#define NOMCX
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WS2tcpip.h>
#include <MSWSock.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
using namespace DirectX;
using namespace DirectX::PackedVector;

#include <iostream>
#include <syncstream>
#include <fstream>
#include <memory>
#include <exception>
#include <filesystem>
#include <concepts>
#include <execution>
#include <random>
#include <chrono>
#include <atomic>
#include <thread>
#include <future>
#include <stop_token>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <array>
#include <concurrent_priority_queue.h>
#include <concurrent_vector.h>
#include <stack>
#include <tuple>
#include <span>
#include <ranges>
#include <algorithm>

using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::make_shared;
using std::make_unique;
using std::const_pointer_cast;
using std::static_pointer_cast;
using std::reinterpret_pointer_cast;
using Filepath = std::filesystem::path;
using std::derived_from;

using RandomDevice = std::random_device;
using RandomEngine = std::default_random_engine;
using RandomDistributionInteger = std::uniform_int_distribution<int>;
using RandomDistributionLong = std::uniform_int_distribution<long>;
using RandomDistributionLLong = std::uniform_int_distribution<long long>;
using RandomDistributionFloat = std::uniform_real_distribution<float>;

template<typename ...Ty>
using Tuple = std::tuple<Ty...>;
template<typename ...Ty>
using Pair = std::pair<Ty...>;
using std::make_tuple;
using std::make_pair;

using int_pair = std::pair<int, int>;
using float_pair = std::pair<float, float>;
using Thread = std::jthread;
using String = std::string;
using Sentence = std::string_view;

using Clock = std::chrono::system_clock::time_point;
using Duration = std::chrono::system_clock::duration;

using std::atomic;
using std::atomic_flag;

template <class _Ty>
concept Arithmetic = std::is_arithmetic_v<_Ty>;

template<Arithmetic Ty1, Arithmetic Ty2>
inline constexpr auto operator+(std::pair<Ty1, Ty2> &&lhs, std::pair<Ty1, Ty2> &&rhs)
-> std::pair<std::remove_cvref_t<Ty1>, std::remove_cvref_t<Ty2>>
{
	return std::make_pair<Ty1, Ty2>(
		std::forward<Ty1>(lhs.first) + std::forward<Ty1>(rhs.first)
		, std::forward<Ty2>(lhs.second) + std::forward<Ty2>(rhs.second));
}

template<Arithmetic Ty1, Arithmetic Ty2>
constexpr auto operator+(const std::pair<Ty1, Ty2> &lhs, const std::pair<Ty1, Ty2> &rhs)
-> std::pair<std::remove_cvref_t<Ty1>, std::remove_cvref_t<Ty2>>
{
	return std::make_pair<Ty1, Ty2>(lhs.first + rhs.first, lhs.second + rhs.second);
}

template<Arithmetic Ty>
constexpr auto operator+(std::pair<Ty, Ty> &&lhs, std::pair<Ty, Ty> &&rhs)
-> std::pair<std::remove_cvref_t<Ty>, std::remove_cvref_t<Ty>>
{
	return std::make_pair<Ty, Ty>(
		std::forward<Ty>(lhs.first) + std::forward<Ty>(rhs.first)
		, std::forward<Ty>(lhs.second) + std::forward<Ty>(rhs.second));
}

template <>
struct std::default_delete<WSABUF>
{
	constexpr default_delete() noexcept = default;

	template <class _Ty2, enable_if_t<is_convertible_v<_Ty2 *, WSABUF *>, int> = 0>
	default_delete(const default_delete<_Ty2> &) noexcept {}

	void operator()(WSABUF *_Ptr) const noexcept /* strengthened */
	{ // delete a pointer
		static_assert(0 < sizeof(WSABUF), "can't delete an incomplete type");

		delete _Ptr->buf;
		delete _Ptr;
	}
};

namespace std
{
	//template<typename Ty, typename Dx>
	//unique_ptr(Ty, Dx)->unique_ptr<std::remove_cvref_t<Ty>, Dx>;

	//template<typename Ty, typename Dx>
	//unique_ptr(Ty[], Dx)->unique_ptr<std::remove_all_extents_t<std::remove_cvref_t<Ty>>[], Dx>;
}

template<typename Ty, std::forward_iterator Iterator>
class index_view_iterator
{
public
	constexpr index_view_iterator() = default;

	constexpr index_view_iterator &operator++()	// 레퍼런스를 리턴해야 한다.
	{
		// weakly_incrementable 컨셉
		++index;
		return *this;
	}

	constexpr index_view_iterator operator++(int)
	{
		auto temp = index_view_iterator{ handle, index };
		++index;

		return index_view_iterator{ temp };
	}

	// indirectly_readable 및 forward_iterator 컨셉을 만족하려면
	// 아래 두 함수의 리턴타입은 같아야 함
	constexpr const std::pair<Ty, std::size_t> &operator *() const
	{

	}

	constexpr const std::pair<Ty, std::size_t> &operator *()
	{

	}

	constexpr bool operator==(const index_view_iterator &_other) const
	{
		return this->pos == _other.pos;
	}

	Ty *handle;
	std::size_t index;
};

template <std::ranges::view View>
class index_view : public std::ranges::view_interface<index_view<View>>
{
public:
	constexpr index_view() requires std::default_initializable<View> = default;

	constexpr index_view(View ranged)
		noexcept(std::is_nothrow_move_constructible_v<View>)
		requires std::ranges::range<const View>
	: myBase(std::move(ranged))
	{}

	constexpr index_view(View &&ranged)
		noexcept(std::is_nothrow_move_constructible_v<View>)
		requires std::ranges::range<const View>
	: myBase(std::forward(ranged))
	{}

	[[nodiscard]] constexpr View base() const &
		noexcept(std::is_nothrow_copy_constructible_v<View>)
		requires std::copy_constructible<View>
	{
		return myBase;
	}

	[[nodiscard]] constexpr View base() &&
		noexcept(std::is_nothrow_move_constructible_v<View>)
	{
		return std::move(myBase);
	}

	constexpr index_view_iterator begin() const
		requires std::ranges::range<const View>
	{
		return index_view_iterator{ std::ranges::begin(myBase), 0 };
	}

	constexpr index_view_iterator end() const
		requires std::ranges::sized_range<const View>
	{
		return index_view_iterator{ std::ranges::begin(myBase), std::ranges::size(myBase) };
	}

	constexpr const index_view_iterator cbegin() const
		requires std::ranges::range<const View>
	{
		return index_view_iterator{ std::ranges::cbegin(myBase), 0 };
	}

	constexpr const index_view_iterator cend() const
		requires std::ranges::sized_range<const View>
	{
		return index_view_iterator{ std::ranges::cbegin(myBase), std::ranges::size(myBase) };
	}

	View myBase;
};

constexpr double PI = 3.141592653589793;

template<typename T>
inline constexpr T dcos(T value)
{
	return std::cos(value * PI / 180);
}

template<typename T>
inline constexpr T dsin(T value)
{
	return std::sin(value * PI / 180);
}

namespace srv
{
	[[nodiscard]]
	inline SOCKET CreateSocket()
	{
		return WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	}

	[[noreturn]]
	inline void RaisePlainError(const std::string_view &description) noexcept(false)
	{
		throw std::exception(description.data());
	}

	[[noreturn]]
	inline void RaiseRuntimeError(const std::string_view &description) noexcept(false)
	{
		throw std::runtime_error(description.data());
	}

	[[noreturn]]
	inline void RaiseSystemError(std::errc code) noexcept(false)
	{
		throw std::system_error(std::make_error_code(code));
	}
}

class XYZWrapper
{
public:
	constexpr XYZWrapper(float &x, float &y, float &z)
		: x(x), y(y), z(z)
	{}

	constexpr XYZWrapper(XMFLOAT3 &position)
		: XYZWrapper(position.x, position.y, position.z)
	{}

	XYZWrapper(XMFLOAT3 &&position) = delete;

	inline constexpr XYZWrapper &operator=(float(&list)[3])
	{
		x = list[0];
		y = list[1];
		z = list[2];
		return *this;
	}

	inline constexpr XYZWrapper &operator=(std::span<float, 3> list)
	{
		x = list[0];
		y = list[1];
		z = list[2];
		return *this;
	}

	inline constexpr XYZWrapper &operator=(const XMFLOAT3 &vector)
	{
		x = vector.x;
		y = vector.y;
		z = vector.z;
		return *this;
	}

	inline constexpr XYZWrapper &operator=(XMFLOAT3 &&vector)
	{
		x = std::forward<float>(vector.x);
		y = std::forward<float>(vector.y);
		z = std::forward<float>(vector.z);
		return *this;
	}

	inline constexpr explicit operator XMFLOAT3() const noexcept
	{
		return XMFLOAT3(x, y, z);
	}

	float &x;
	float &y;
	float &z;
};
