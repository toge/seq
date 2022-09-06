


#include <deque>
#include <list>
#include <vector>


#include "tiered_vector.hpp"
#include "testing.hpp"


using namespace seq;



template<class Deq1, class Deq2>
bool equal_deq(const Deq1& d1, const Deq2& d2)
{
	if (d1.size() != d2.size())
		return false;
	if (d1.size() == 0)
		return true;
	if (d1.front() != d2.front())
		return false;
	if (d1.back() != d2.back()) {
		return false;
	}
	auto it1 = d1.begin();
	auto it2 = d2.begin();
	int i = 0;
	while (it1 != d1.end())
	{
		if (*it1 != *it2) {
			return false;
		}
		++it1;
		++it2;
		++i;
	}
	return true;
}


void test_deque_algorithms(size_t count = 5000000)
{
	// Test algorithms on tiered_vector, some of them requiring random access iterators
	
	typedef size_t type;
	typedef tiered_vector<type, std::allocator<type> > deque_type;

	// Build with non unique random values
	deque_type  tvec;
	std::deque<type> deq;
	srand(0);// time(NULL));
	for (size_t i = 0; i < count; ++i) {
		int r = rand();
		deq.push_back((size_t)r);
		tvec.push_back((size_t)r);
	}

	// Test sort
	std::sort(deq.begin(), deq.end());
	std::sort(tvec.begin(), tvec.end());
		
	SEQ_TEST_ASSERT(equal_deq(deq, tvec));

	// Test unique after sorting
	auto it1 = std::unique(deq.begin(), deq.end());
	auto it2 = std::unique(tvec.begin(), tvec.end());
	deq.resize(it1 - deq.begin());
	tvec.resize(it2 - tvec.begin());
		
	SEQ_TEST_ASSERT(equal_deq(deq, tvec));

	// Reset values
	deq.resize(count);
	tvec.resize(count);

	for (size_t i = 0; i < count; ++i)
		tvec[i] = deq[i] = (size_t) rand();

	// Test rotate
	std::rotate(deq.begin(), deq.begin() + deq.size() / 2, deq.end());
	std::rotate(tvec.begin(), tvec.begin() + tvec.size() / 2, tvec.end());
		
	SEQ_TEST_ASSERT(equal_deq(deq, tvec));

	// Test reversing
	std::reverse(deq.begin(), deq.end());
	std::reverse(tvec.begin(), tvec.end());

	SEQ_TEST_ASSERT(equal_deq(deq, tvec));

	// Reset values
	for (size_t i = 0; i < count; ++i)
		tvec[i] = deq[i] = (size_t) rand();

	// Test partial sort
	std::partial_sort(deq.begin(), deq.begin() + deq.size() / 2, deq.end());
	std::partial_sort(tvec.begin(), tvec.begin() + tvec.size() / 2, tvec.end());
	SEQ_TEST_ASSERT(equal_deq(deq, tvec));


	
	for (size_t i = 0; i < count; ++i)
		tvec[i] =deq[i] = (size_t)rand();

	// Strangely, msvc implementation of std::nth_element produce a warning as it tries to modify the value of const iterator
	// Test nth_element
	std::nth_element(deq.begin(), deq.begin() + deq.size() / 2, deq.end());
	std::nth_element(tvec.begin(), tvec.begin() + tvec.size() / 2, tvec.end());
	SEQ_TEST_ASSERT(equal_deq(deq, tvec));
	
}





template<class T>
void test_tiered_vector(size_t count = 5000000)
{
	
	// First, test some stl algorithms
	test_deque_algorithms(count);
	
	typedef T type;
	std::deque<type > deq;
	typedef tiered_vector<type, std::allocator<T>, OptimizeForMemory > deque_type;
	deque_type tvec;
	std::vector<type> vec;
		
	SEQ_TEST_ASSERT(tvec.begin() == tvec.end());
	SEQ_TEST_ASSERT(tvec.size() == 0);
	SEQ_TEST_ASSERT(tvec.lower_bound(0) == 0);
	SEQ_TEST_ASSERT(tvec.upper_bound(0) == 0);
	SEQ_TEST_ASSERT(tvec.binary_search(0) == 0);

	tvec.resize(10);
	SEQ_TEST_ASSERT(tvec.size() == 10);
	tvec.clear();
	SEQ_TEST_ASSERT(tvec.size() == 0 && !tvec.manager());


	// Fill containers
	for (size_t i = 0; i < count; ++i)
		deq.push_back((T)i);
	for (size_t i = 0; i < count; ++i)
		tvec.push_back((T)i);
	for (size_t i = 0; i < count; ++i)
		vec.push_back((T)i);
		
	SEQ_TEST_ASSERT(equal_deq(deq, tvec));

		
	// Test resiz lower
	deq.resize(deq.size() / 10);
	tvec.resize(tvec.size() / 10);
	SEQ_TEST_ASSERT(equal_deq(deq, tvec));

	// Test resize upper
	deq.resize(count,0);
	tvec.resize(count,0);
	SEQ_TEST_ASSERT(equal_deq(deq, tvec));

	{
		// Test copy contruct
		std::deque<type> d2 = deq;
		deque_type dd2 = tvec;
		SEQ_TEST_ASSERT(equal_deq(d2, dd2));
	}

	{
		// Test insert range based on random access itertor, left side
		deq.insert(deq.begin() + (deq.size() * 2) / 5, vec.begin(), vec.end());
		tvec.insert(tvec.begin() + (tvec.size() * 2) / 5, vec.begin(), vec.end());
		SEQ_TEST_ASSERT(equal_deq(deq, tvec));
			
		deq.resize(count);
		tvec.resize(count);
		SEQ_TEST_ASSERT(equal_deq(deq, tvec));

		// Test insert range based on random access itertor, right side
		deq.insert(deq.begin() + (deq.size() * 3) / 5, vec.begin(), vec.end());
		tvec.insert(tvec.begin() + (tvec.size() * 3) / 5, vec.begin(), vec.end());
			
		SEQ_TEST_ASSERT(equal_deq(deq, tvec));

		deq.resize(count);
		tvec.resize(count);

		SEQ_TEST_ASSERT(equal_deq(deq, tvec));
	}


	{
		// Rest values
		for (size_t i = 0; i < deq.size(); ++i) {
			deq[i] = tvec[i] = (T)i;
		}
		SEQ_TEST_ASSERT(equal_deq(deq, tvec));

		// Test erase range, left side
		deq.erase(deq.begin() + deq.size() / 4, deq.begin() + deq.size() / 2);
		tvec.erase(tvec.size() / 4, tvec.size() / 2);
		SEQ_TEST_ASSERT(equal_deq(deq, tvec));

		deq.resize(count,0);
		tvec.resize(count,0);

		// Test erase range, right side
		deq.erase(deq.begin() + deq.size() / 2, deq.begin() + deq.size() * 3 / 4);
		tvec.erase(tvec.size() / 2, tvec.size() * 3 / 4);
		SEQ_TEST_ASSERT(equal_deq(deq, tvec));
	}

	{
		deq.resize(vec.size() / 2,0);
		tvec.resize(vec.size() / 2,0);

		// Test assign from lower size
		deq.assign(vec.begin(), vec.end());
		tvec.assign(vec.begin(), vec.end());
		SEQ_TEST_ASSERT(equal_deq(deq, tvec));

		deq.resize(vec.size() * 2,0);
		tvec.resize(vec.size() * 2,0);

		// Test assign from greater size
		deq.assign(vec.begin(), vec.end());
		tvec.assign(vec.begin(), vec.end());
		SEQ_TEST_ASSERT(equal_deq(deq, tvec));
	}
	{
		std::list<type> lst;
		for (size_t i = 0; i < count; ++i)
			lst.push_back((T)i);

		deq.resize(lst.size() / 2,0);
		tvec.resize(lst.size() / 2,0);

		// Test assign from forward iterators
		deq.assign(lst.begin(), lst.end());
		tvec.assign(lst.begin(), lst.end());
		SEQ_TEST_ASSERT(equal_deq(deq, tvec));

		deq.resize(lst.size() * 2,0);
		tvec.resize(lst.size() * 2,0);

		// Test assign from forward iterators
		deq.assign(lst.begin(), lst.end());
		tvec.assign(lst.begin(), lst.end());
		SEQ_TEST_ASSERT(equal_deq(deq, tvec));
	}

	deq.resize(count,0);
	tvec.resize(count,0);
	SEQ_TEST_ASSERT(equal_deq(deq, tvec));

	//fill again, backward
	for (size_t i = 0; i < deq.size(); ++i) {
		deq[i] = (T)(deq.size() - i - 1);
		tvec[i] = (T)(tvec.size() - i - 1);
	}

	// Test pop_back
	while (deq.size() > 25)
		deq.pop_back();
	while (tvec.size() > 25)
		tvec.pop_back();
	SEQ_TEST_ASSERT(equal_deq(deq, tvec));

	deq.resize(count,0);
	tvec.resize(count,0);

	SEQ_TEST_ASSERT(equal_deq(deq, tvec));

	//fill again, backward
	for (size_t i = 0; i < deq.size(); ++i) {
		deq[i] = (T)(deq.size() - i - 1);
		tvec[i] = (T)(tvec.size() - i - 1);
	}


	SEQ_TEST_ASSERT(equal_deq(deq, tvec));

	// Test pop_front
	while (deq.size() > 25) {
		deq.pop_front();
	}
	while (tvec.size() > 25)
		tvec.pop_front();
	SEQ_TEST_ASSERT(equal_deq(deq, tvec));


	{
		// Test insert/erase single element
		deque_type d;
		std::deque<type> dd;
		d.resize(128 * 3,0);
		dd.resize(128 * 3,0);
		for (size_t i = 0; i < d.size(); ++i) {
			d[i] = dd[i] = (T)i;
		}
		SEQ_TEST_ASSERT(equal_deq(d, dd));
		d.insert(10, -1);
		dd.insert(dd.begin() + 10, -1);
		SEQ_TEST_ASSERT(equal_deq(d, dd));
		for (size_t i = 0; i < 128; ++i) {
			d.erase(0);
			dd.erase(dd.begin());
			SEQ_TEST_ASSERT(equal_deq(d, dd));
		}
		SEQ_TEST_ASSERT(equal_deq(d, dd));
		d.erase(0);
		dd.erase(dd.begin());
		SEQ_TEST_ASSERT(equal_deq(d, dd));
	}


	int insert_count = (int) std::max((size_t)50, count / 100);
	std::vector<size_t> in_pos;
	int ss= (int)deq.size();
	srand(0);
	for(int i=0; i< insert_count; ++i)
		in_pos.push_back(rand() % ss++);

	// Test insert single value at random position
	for (int i = 0; i < insert_count; ++i) {
		deq.insert(deq.begin() + in_pos[i], (T)i);
	}
	for (int i = 0; i < insert_count; ++i) {
		tvec.insert(in_pos[i], (T)i);
	}
	SEQ_TEST_ASSERT(equal_deq(deq, tvec));


	
	{
		// Test erase single value at random position
		deque_type d;
		std::deque<type> dd;
		d.resize(100,0);
		dd.resize(100,0);
		for (size_t i = 0; i < d.size(); ++i) {
			d[i] = dd[i] = (T)i;
		}

		for (int i = 0; i < 50; ++i) {
			int pos = i % 5;
			pos = (int)(d.size() * pos / 4);
			if (pos == (int)d.size()) --pos;
			dd.erase(dd.begin() + pos);
			d.erase(pos);
			SEQ_TEST_ASSERT(equal_deq(d, dd));
		}
	}



	deq.resize(count,0);
	tvec.resize(count,0);

	// Test shrink_to_fit
	deq.shrink_to_fit();
	SEQ_TEST_ASSERT(equal_deq(deq, tvec));

	//fill again, backward
	for (size_t i = 0; i < deq.size(); ++i) {
		deq[i] = (T)(deq.size() - i - 1);
		tvec[i] = (T)(tvec.size() - i - 1);
	}

	// Test erase single values at random position
	int erase_count = (int) deq.size()/2;
	std::vector<size_t> er_pos;
	int sss= (int)count;
	srand(0);
	for(int i=0; i< erase_count; ++i)
		er_pos.push_back(rand() % sss--);


	for (int i = 0; i < erase_count; ++i) {
		deq.erase(deq.begin() + er_pos[i]);
	}
	for (int i = 0; i < erase_count; ++i) {
		tvec.erase( er_pos[i]);
	}
	SEQ_TEST_ASSERT(equal_deq(deq, tvec));

	


	tvec.resize(count);
	deq.resize(count);
	for (size_t i = 0; i < deq.size(); ++i) {
		deq[i] = tvec[i] = (T)i;
	}

	// Test move assign and move copy

	std::deque<type> deq2 = std::move(deq);
	deque_type tvec2 = std::move(tvec);
	SEQ_TEST_ASSERT(equal_deq(deq2, tvec2) && tvec2.size() > 0 && deq.size() == 0 && tvec.size() == 0);

	deq = std::move(deq2);
	tvec = std::move(tvec2);
	SEQ_TEST_ASSERT(equal_deq(deq, tvec) && tvec.size() > 0 && tvec2.size() == 0 && deq2.size() == 0);
}


