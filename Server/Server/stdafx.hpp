#pragma once
#include <SDKDDKVER.h>

#include <iostream>
#include <fstream>
#include <memory>
#include <filesystem>
#include <concepts>
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

using Thread = std::jthread;
using String = std::string;
using Sentence = std::string_view;
