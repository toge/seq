/**
 * MIT License
 *
 * Copyright (c) 2022 Victor Moncada <vtr.moncada@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */



#include <seq/flat_map.hpp>
#include <seq/radix_map.hpp>
#include <seq/format.hpp>
#include <seq/testing.hpp>
#include <seq/any.hpp>
#include <seq/ordered_map.hpp>


#include "phmap/btree.h"
#include "boost/container/flat_set.hpp"

#include <iostream>
#include <unordered_set>
#include <set>

using namespace seq;




template<class T>
inline size_t convert_to_size_t(const T& v)
{
	return static_cast<size_t>(v);
}
template<class Char, class Traits, size_t S, class Al>
inline size_t convert_to_size_t(const tiny_string<Char,Traits, Al,S>& v)
{
	return static_cast<size_t>(v.size());
}
inline size_t convert_to_size_t(const std::string& v)
{
	return static_cast<size_t>(v.size());
}
inline size_t convert_to_size_t(const std::wstring& v)
{
	return static_cast<size_t>(v.size());
}
template<class Interface, size_t S, size_t A, bool R>
inline size_t convert_to_size_t(const seq::hold_any<Interface, S, A, R>& t) {
	return reinterpret_cast<size_t>(t.data());
}

template<class C>
struct is_boost_map : std::false_type {};
template<class C>
struct is_boost_map<boost::container::flat_set<C> > : std::true_type {};

template<class C, class K>
struct find_val
{
	static SEQ_ALWAYS_INLINE bool find(const C& s,const K & key, size_t& out)
	{
		auto it = s.find(key);
		if (it != s.end()) {
			out += convert_to_size_t(*it);
			return true;
		}
		
		return false;
	}
};
template<class C, class K>
struct find_val<flat_set<C>,K>
{
	static SEQ_ALWAYS_INLINE bool find(const flat_set<C>& s, const K& key, size_t& out)
	{
		auto it = s.find_pos(key);
		if (it != s.size()) {
			out += convert_to_size_t(s.pos(it));
			return true;
		}
		
		return false;
	}
};


template<class C, class K>
struct insert_val
{
	static bool insert( C& s, const K& key)
	{
		return s.insert(key).second;
	}
};
template<class C, class K>
struct insert_val<flat_set<C>, K>
{
	static bool insert( flat_set<C>& s, const K& key)
	{
		return s.insert_pos(key).second;
	}
};
template<class C, class K>
bool insert_value(C& s, const K& key)
{
	return insert_val<C, K>::insert(s, key);
}


template<class C>
void check_sorted(C& set)
{
	size_t dist = std::distance(set.begin(), set.end());
	SEQ_TEST(dist == set.size());
	SEQ_TEST(std::is_sorted(set.begin(), set.end()));
	SEQ_TEST(std::is_sorted(set.rbegin(), set.rend(), std::greater<>{}));
	for (auto it = set.begin(); it != set.end(); ++it)
		SEQ_TEST(set.find(*it) != set.end());
}

template<class T, class hash, class equal, class allocator>
void check_sorted(std::unordered_set<T,hash,equal,allocator>& )
{
}

template<class T, class hash, class equal, class allocator>
void check_sorted(seq::ordered_set<T, hash, equal, allocator>&)
{
}


template<class C>
struct is_radix_set: std::false_type{};

template<class T, class Extract, class Al>
struct is_radix_set<radix_set<T,Extract,Al> >: std::true_type{};


template<class C, class U, bool Launch = !(is_hold_any<U>::value && is_radix_set<C>::value)>
struct LaunchTest
{
	template<class Format>
	static void test(const char * name,  const std::vector<U> & vec, Format f, bool write = true)
	{
		using T = typename C::value_type;
		C set;
		
		std::vector<U> success(vec.begin(), vec.begin()+vec.size()/2);
		std::vector<U> fail(vec.begin() + vec.size() / 2, vec.end());
		//seq::random_shuffle(success.begin(), success.end(),1);
		//seq::random_shuffle(fail.begin(), fail.end(),1);

		reset_memory_usage();
		size_t start_mem = get_memory_usage();
		size_t insert_range, insert_range_mem;
		size_t insert, insert_mem;

		{
			C s;
			reset_memory_usage();
			start_mem = get_memory_usage();

			//insert range
			tick();
			s.insert(success.begin(), success.end());
			insert_range = tock_ms();
			insert_range_mem = (get_memory_usage() - start_mem) / (1024 * 1024);

			check_sorted(s);

		}

		
			//insert
	#ifndef TEST_BOOST_INSERT_ERASE
			if (std::is_same< boost::container::flat_set<T>, C>::value) {
				insert = 1000000;
				insert_mem = 0;
				set.insert(success.begin(), success.end());
			}
			else
	#endif
			{
				reset_memory_usage();
				start_mem = get_memory_usage();

				tick();
				for (size_t i = 0; i < success.size(); ++i)
				{
					SEQ_TEST(insert_value(set, success[i]));
				}
					
				insert = tock_ms();
				insert_mem = (get_memory_usage() - start_mem) / (1024 * 1024);

				check_sorted(set);

			}
		
		
		//insert fail
		tick();
		for (size_t i = 0; i < success.size() ; ++i)
			SEQ_TEST(!insert_value(set, success[i]));
		size_t insert_fail = tock_ms();
		//size_t insert_fail_mem = (get_memory_usage() - start_mem) / (1024 * 1024);

		check_sorted(set);


		//find success
		tick();
		size_t sum = 0;
		for (size_t i = 0; i < success.size(); ++i)
			SEQ_TEST(find_val<C, U>::find(set, success[i], sum));
		size_t find = tock_ms();
		print_null(sum);

		//find success
		tick();
		sum = 0;
		for (size_t i = 0; i < success.size(); ++i)
			SEQ_TEST(set.lower_bound( success[i]) != set.end());
		size_t lower_bound = tock_ms();
		print_null(sum);

		//find fail
		tick();
		sum = 0;
		for (size_t i = 0; i < fail.size(); ++i)
			SEQ_TEST(!find_val<C, U>::find(set, fail[i], sum));
			//SEQ_TEST(set.lower_bound(fail[i]) != typename C::const_iterator());
		size_t find_fail = tock_ms();
		print_null(sum);

		//walk
		tick();
		sum = 0;
		for (auto it = set.begin(); it != set.end(); ++it)
			sum += convert_to_size_t(*it);
		size_t iterate = tock_ms();
		print_null(sum);

		size_t erase = 0;
	#ifndef TEST_BOOST_INSERT_ERASE
		if (std::is_same< boost::container::flat_set<T>, C>::value) {
			erase = 1000000;
		}
		else
	#endif
		{
			tick();
			for (size_t i = 0; i < success.size()/2; ++i) {
				typename C::const_iterator it = set.find(success[i]);
				if (it != set.end())
					set.erase(it);
			}
			
			erase = tock_ms();
			print_null(set.size());
			SEQ_TEST(set.size() == (success.size()/2 + success.size() % 2));
			check_sorted(set);

			for (size_t i = 0; i < success.size()/2; ++i)
				SEQ_TEST(set.find(success[i]) == set.end());
			for (size_t i = success.size() / 2; i < success.size(); ++i)
				SEQ_TEST(set.find(success[i]) != set.end());

			//reinsert
			for (size_t i = 0; i < success.size() / 2; ++i)
				SEQ_TEST(set.insert(success[i]).second);
			check_sorted(set);
			for (size_t i = 0; i < success.size() ; ++i)
				SEQ_TEST(set.find(success[i]) != set.end());
		}
		if(write)
			std::cout << f(name, fmt(insert_range, insert_range_mem), fmt(insert, insert_mem), insert_fail, find, lower_bound, find_fail, iterate, erase) << std::endl;
	}

};

template<class C, class T>
struct LaunchTest<C, T, false>
{
	template<class Format>
	static void test(const char* name, const std::vector<T>& vec, Format f, bool write) {}
};


template<class C, class U, class Format>
void test_set(const char * name,  const std::vector<U> & vec, Format f, bool write = true)
{
	LaunchTest<C,U>::test(name,vec,f,write);
}




/// @brief Compare various operations on seq::flat_set, phmap::btree_set, boost::container::flat_set, std::set and std::unordered_set<T>
/// @tparam T 
/// @tparam Gen 
/// @param count 
/// @param gen 
template<class T, class Gen, class Extract = default_key<T> >
void test_map(size_t count, Gen gen, Extract e = Extract())
{
	std::cout << std::endl;
	std::cout << "Test sorted containers with type = " << typeid(T).name() << " and count = " << count / 2 << std::endl;
	std::cout << std::endl;

	std::vector<T> vec(count);
	for (int i = 0; i < vec.size(); ++i) {
		vec[i] = (T)gen(i);
	}


	std::sort(vec.begin(), vec.end(), std::less<>{});
	auto it = std::unique(vec.begin(), vec.end());
	vec.resize(it - vec.begin());
	seq::random_shuffle(vec.begin(), vec.end(),1);

	std::cout << "vector size: " << vec.size() << std::endl;
	
	/*std::ofstream out("points.bin", std::ios::binary);
	for (size_t i = 0; i < vec.size(); ++i)
	{
		write(out, vec[i]);
	}
	out.close();*/
	/*std::ifstream fin("points.bin", std::ios::binary);
	vec.clear();
	while (true) {
		T p = read<T>(fin);
		if (!fin)
			break; 
		vec.push_back(p);
	}
	*/

	auto f = join("|",
		_s().l(30),  //name
		_fmt(_u(), " ms/", _u(), " MO").c(20),  //insert(range)
		_fmt(_u(), " ms/", _u(), " MO").c(20),  //insert
		_fmt(_u(), " ms").c(15),  //insert(fail)
		_fmt(_u(), " ms").c(15), //find
		_fmt(_u(), " ms").c(15), //lower_bound
		_fmt(_u(), " ms").c(15),  //find(fail)
		_fmt(_u(), " ms").c(15),   //iterate
		_fmt(_u(), " ms").c(15), ""); //erase 
	
	auto header = join("|", _s().l(30), _s().c(20), _s().c(20), _s().c(15), _s().c(15), _s().c(15), _s().c(15), _s().c(15), _s().c(15), "");
	std::cout << header("Set name", "Insert(range)", "Insert", "Insert(failed)", "Find (success)", "LB (success)", "Find (failed)", "Iterate", "Erase") << std::endl;
	std::cout << header(fill('-'), fill('-'), fill('-'), fill('-'), fill('-'), fill('-'), fill('-'), fill('-'), fill('-')) << std::endl;

	// Warmup
	//test_set<flat_set<T>>("seq::flat_set", vec, f, false);
	
	test_set<flat_set<T>>("seq::flat_set", vec, f);
	test_set<phmap::btree_set<T> >("phmap::btree_set", vec, f);
	test_set<boost::container::flat_set<T> >("boost::flat_set<T>", vec,f);
	test_set<radix_set<T, Extract>>("seq::radix_set", vec, f);
	test_set<std::set<T> >("std::set", vec,f);
	//test_set<std::unordered_set<T> >("std::unordered_set", vec,f);	
}



int bench_map(int , char ** const)
{
	test_map<size_t>(4000000, [](size_t i) { return (i); });
		 
	random_float_genertor<double> rng;
	test_map<double>(4000000, [&rng](size_t i) { return rng(); });
	
	test_map<tstring>(4000000, [](size_t i) { return generate_random_string<tstring>(13, true); });
	test_map<tstring>(2000000, [](size_t i) { return generate_random_string<tstring>(63, false); });
	 // Test flat set with seq::r_any
	test_map<seq::r_any>(1000000, [](size_t i)
		 {
			 size_t idx = i & 3U; 
			 switch (idx) {
			 case 0:return seq::r_any(i * UINT64_C(0xc4ceb9fe1a85ec53));
			 case 1:return seq::r_any((double)(i * UINT64_C(0xc4ceb9fe1a85ec53)));
			 default:return seq::r_any(generate_random_string<tstring>(14, true));
			 }
		 }
	 );
	return 0;
}



/*template<class Map, class Format>
void test_small_map_repeat(const char * name, int count, int repeat, Format f)
{
	using value_type = typename Map::value_type;
	std::vector< value_type> vec;
	for (int i = 0; i < count; ++i)
		vec.push_back(static_cast<value_type>(i));
	seq::random_shuffle(vec.begin(), vec.end());

	tick();
	for (int i = 0; i < repeat; ++i)
	{
		Map m ;
		print_null(m.size());
		for (size_t j = 0; j < vec.size() / 2; ++j)
			m.insert(vec[j]);
		m.insert(vec.begin() + vec.size() / 2, vec.end());

		size_t sum = 0;
		for (size_t j = 0; j < vec.size(); ++j)
			//sum += *(size_t)m.find(vec[j]);
			find_val<Map,value_type>::find(m,vec[j],sum);

		print_null(sum);

		for (size_t j = 0; j < vec.size(); ++j)
			m.erase(vec[j]);

		print_null(m.size());
	}
	size_t el = tock_ms();
	std::cout << f(name, el) << std::endl;
}

template<class T>
void test_small_map(int count, int repeat)
{
	std::cout << std::endl;
	std::cout << "Test small sorted containers with type = " << typeid(T).name() << " and size = " << count  << std::endl;
	std::cout << std::endl;

	auto f = fmt(pos<0,2>(), 
		fmt("").l(30), "|",  //name
		fmt<size_t>().c(20), "|"); //time

	std::cout << fmt(fmt("Set name").l(30), "|", fmt("Tims (ms)").c(20), "|") << std::endl;
	std::cout << fmt(str().c(30).f('-'), "|", str().c(20).f('-'), "|") << std::endl;

	
	test_small_map_repeat<phmap::btree_set<T > >("phmap::btree_set", count, repeat, f);
	test_small_map_repeat < flat_set < T, std::less<T>, std::allocator<T> > >("seq::flat_set", count, repeat, f);
	test_small_map_repeat<boost::container::flat_set<T> >("boost::flat_set<T>", count, repeat, f);
	test_small_map_repeat<std::set<T> >("std::set", count, repeat, f);
}*/