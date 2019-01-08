// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <time.h>
#include <omp.h>
#include <vector>

// Implement the problem of numerical integration with OpenMP.Do speedup test on different sizes of problem.Write a report.
template<typename Func>
double numerical_integration( uint64_t thread_num, uint64_t n, double a, double b, Func && func)
{
	if ( a > b) {
		std::cerr << "a > b" << std::endl;
	}

	double h = ( b - a) / n;
	double r_sum = 0;

	#pragma omp parallel for num_threads( thread_num) reduction(+:r_sum)
	for ( int i = 0; i < n; ++i) {
		double x = a + h * i;
		r_sum += func( x);
	}
	return r_sum * h;
}

void test1(uint64_t thread_num)
{
	std::vector<time_t> results( thread_num);

	for ( int i = 0; i < thread_num; ++i) {
		time_t t1 = time( nullptr);
		double volatile r = numerical_integration( i + 1, 100000000, 0, 10, []( double x) { return 5 * pow( x, 3); /*useless function*/ });
		time_t t2 = time( nullptr);
		results[i] = t2 - t1;
	}

	for ( int i = 0; i < thread_num; ++i) {
		std::cout << "t" << i + 1 << ": " << results[i] << "s" << std::endl;
	}
}

// The Cauchy problem of the differential equation (2 any equations)
template<typename Func>
std::pair<double, double> cauchy(uint64_t thread_num, double x0, double y0, double h, uint64_t n, Func && func)
{
	double y = y0;
	#pragma omp parallel for ordered num_threads( thread_num)
	for ( int i = 1; i < n + 1; i++) {
		double x = x0 + ( i - 1) * h;
		double old_y;
		
		#pragma omp ordered
		old_y = y;
		
		y = old_y + h * func( x, old_y);
	}
	return { x0 + n * h, y };
}

template<typename Func>
void test2(uint64_t thread_num, Func && func)
{
	std::vector<time_t> results( thread_num);

	double x0 = 0;
	double y0 = 2;
	double h = 0.1;
	uint64_t n = 100000000;

	for ( int i = 0; i < thread_num; ++i) {
		time_t t1 = time( nullptr);
		auto volatile r = cauchy( i + 1, x0, y0, h, n, func);
		time_t t2 = time( nullptr);
		results[i] = t2 - t1;
	}

	for ( int i = 0; i < thread_num; ++i) {
		std::cout << "t" << i + 1 << ": " << results[i] << "s" << std::endl;
	}
}

int main()
{
	test1( 4);

	// t1: 11s
	// t2: 5s
	// t3: 4s
	// t4: 3s

	test2( 4, []( double x, double y) { return 3 * sin( 2 * y) + x; /*useless function*/ });
	test2( 4, []( double x, double y) { return 3 * sin( 2 * x) + x; /*useless function*/ });
	test2( 4, []( double x, double y) { return x; /*useless function*/ });

	// t1: 11s
	// t2: 11s
	// t3: 11s
	// t4: 10s
	
	// t1: 11s
	// t2: 11s
	// t3: 10s
	// t4: 10s

	return 0;
}
