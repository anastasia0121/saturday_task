#include "pch.h"
#include <iostream>
#include <time.h>
#include <omp.h>
#include <vector>

int calc_cell( uint64_t thread_num, const std::vector<std::vector<int> > &a, uint64_t i, uint64_t j, uint64_t n)
{
	int d = 0;
	#pragma omp parallel for num_threads( thread_num) reduction( +:d)
	for ( int k = 0; k < n; ++k) {
		d += ( a[i][k] - a[j][k]) * ( a[i][k] - a[j][k]);
	}
	return d;
}

void calc_matrix( uint64_t thread_num, const std::vector<std::vector<int> > &a, std::vector<std::vector<int> > &d)
{
	uint64_t r = a.size();
	uint64_t c = a[0].size();

	//#pragma omp parallel for num_threads( thread_num)
	for ( int i = 0; i < r; ++i) {
		d[i][i] = 0;
		for ( int j = i + 1; j < r; ++j) {
			d[i][j] = calc_cell( thread_num, a, i, j, c);
		}
	}
	//#pragma omp parallel for num_threads( thread_num)
	for (int i = 0; i < r; ++i) {
		for (int j = i + 1; j < r; ++j) {
			d[j][i] = d[i][j];
		}
	}
}

void test3( uint64_t thread_num)
{
	uint64_t r = 1000;
	uint64_t c = 40;
	std::vector<std::vector<int> > a( r, std::vector<int>( c));  //{{0, 1, 1}, {4, 0, 2}, {3, 1, 1}, {0, 0, 0}, {2, 1, 2}};
	for ( auto &row : a) {
		for ( auto &cell : row) {
			cell = rand();
		}
	}
	
	// a lot of useless memory, we need only half (but we create like in the example)
	std::vector<std::vector<int> > d( r, std::vector<int>( r)); 

	std::vector<time_t> results( thread_num);
	for (int i = 0; i < thread_num; ++i) {
		time_t t1 = time( nullptr);
		calc_matrix( i + 1, a, d);
		time_t t2 = time( nullptr);
		results[i] = t2 - t1;
	}

	for ( int i = 0; i < thread_num; ++i) {
		std::cout << "t" << i + 1 << ": " << results[i] << "s" << std::endl;
	}

	//for ( const auto &row : d) {
	//	for ( const auto cell : row) {
	//		std::cout << cell << " ";
	//	}
	//	std::cout << std::endl;
	//}
}


int main()
{
	test3( 4);
	// t1: 37s
	// t2: 27s
	// t3: 21s
	// t4: 16s

	return 0;
}
