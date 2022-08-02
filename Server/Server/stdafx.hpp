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

namespace srv
{
	inline constexpr bool CheckError(const int socket_result) noexcept
	{
		return SOCKET_ERROR == socket_result;
	}
}

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
#include <numeric>
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

template<typename Ty, typename Allocator = std::allocator<Ty>>
using ConcurrentVector = concurrency::concurrent_vector<Ty, Allocator>;

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

//template<typename Container>
//class index_view_enumerator
//{};

template<std::ranges::bidirectional_range Container>
class index_enumerator
{
public:
	using iterator_type = std::ranges::iterator_t<Container>;
	using iterator_concept = iterator_type::iterator_concept;
	using iterator_category = iterator_type::iterator_category;
	using value_type = iterator_type::value_type;
	using difference_type = iterator_type::difference_type;
	using pointer = iterator_type::pointer;
	using reference = iterator_type::reference;

	constexpr index_enumerator()
		requires std::default_initializable<Container> && std::default_initializable<iterator_type> = default;

	template<std::integral Integral>
	constexpr index_enumerator(Container& container, Integral npos = 0)
		: range(std::addressof(container)), handle(std::begin(container))
		, index(static_cast<std::size_t>(npos))
	{}

	template<std::integral Integral>
	constexpr index_enumerator(iterator_type iter, Integral npos = 0)
		: range(nullptr), handle(iter)
		, index(static_cast<std::size_t>(npos))
	{}

	inline constexpr index_enumerator &operator++()
	{
		// weakly_incrementable да╪а
		++handle;
		++index;
		return *this;
	}

	inline constexpr index_enumerator operator++(int)
	{
		auto temp = index_enumerator{ handle, index };
		++handle;
		++index;

		return temp;
	}

	inline constexpr index_enumerator &operator--()
	{
		--handle;
		--index;
		return *this;
	}

	inline constexpr index_enumerator operator--(int)
	{
		auto temp = index_enumerator{ handle, index };
		--handle;
		--index;

		return temp;
	}

	inline constexpr const auto operator *() const
	{
		return make_pair(handle, index);
	}

	inline constexpr const auto operator *()
	{
		return make_pair(handle, index);
	}

	inline constexpr auto operator->() const noexcept
	{
		return this->handle;
	}

	inline constexpr bool operator==(const index_enumerator &_other) const
	{
		return this->handle == _other.handle;
	}

	inline constexpr bool operator==(const iterator_type&_other) const
	{
		return this->handle == _other;
	}

	inline constexpr bool operator==(iterator_type&&_other) const
	{
		return this->handle == std::forward<iterator_type>(_other);
	}

	Container* range;
	iterator_type handle;
	std::size_t index;
};

template<std::ranges::bidirectional_range Container, std::integral numeric>
index_enumerator(std::ranges::iterator_t<Container>, numeric)->index_enumerator<Container>;

template<std::ranges::bidirectional_range Container>
index_enumerator(Container)->index_enumerator<Container>;

#ifdef __DDD__
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

	constexpr auto begin() const
		requires std::ranges::range<const View>
	{
		return index_view_iterator{ std::ranges::begin(myBase), 0 };
	}

	constexpr auto end() const
		requires std::ranges::sized_range<const View>
	{
		return index_view_iterator{ std::ranges::end(myBase), std::ranges::size(myBase) };
	}

	constexpr const auto cbegin() const
		requires std::ranges::range<const View>
	{
		return index_view_iterator{ std::ranges::cbegin(myBase), 0 };
	}

	constexpr const auto cend() const
		requires std::ranges::sized_range<const View>
	{
		return index_view_iterator{ std::ranges::cend(myBase), std::ranges::size(myBase) };
	}

	View myBase;
};
#endif

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

extern "C" static LPFN_DISCONNECTEX DisconnectEx = nullptr;

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
