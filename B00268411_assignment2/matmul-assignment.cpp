/*
Games Console Development COMP10037 - Assignment 2
Matrix Multiplication
B00268411
https://software.intel.com/sites/landingpage/IntrinsicsGuide/
*/

#include <iostream>
#include <cstring>
#include <iomanip>
#include <chrono>
#include <vector>
#include <tuple>
#include <string>
#include <immintrin.h>


// $CXX -03 -mavx matmul_assignment.cpp


#ifdef __ORBIS__
#include <stdlib.h>
size_t sceLibcHeapSize = SCE_LIBC_HEAP_SIZE_EXTENDED_ALLOC_NO_LIMIT;
unsigned int sceLibcHeapExtendedAlloc = 1;
#endif

#if (!defined(_MSC_VER))
#pragma clang diagnostic ignored "-Wc++17-extensions"
#endif
#define SZ (1 << 2)// (1 << 10) == 1024
struct mat {
	float* data;
	const size_t sz;
	bool operator==(const mat& rhs) const {
		return !std::memcmp(data, rhs.data, sz * sz * sizeof(data[0]));
	}
};



void matmul(mat& mres, const mat& m1, const mat& m2)
{
	for (int i = 0; i < mres.sz; i++) {
		for (int j = 0; j < mres.sz; j++) {
			mres.data[i * mres.sz + j] = 0;
			for (int k = 0; k < mres.sz; k++) {
				mres.data[i * mres.sz + j] += m1.data[i * mres.sz + k] * m2.data[k * mres.sz + j];
			}
		}
	}
}

void print_mat(const mat& m) {
	for (int i = 0; i < m.sz; i++) {
		for (int j = 0; j < m.sz; j++) {
			std::cout << std::setw(3) << m.data[i * m.sz + j] << ' ';
		}
		std::cout << '\n';
	}
	std::cout << '\n';
}

// A simply initialisation pattern. For a 4x4 matrix:

// 1   2  3  4
// 5   6  7  8
// 9  10 11 12
// 13 14 15 16

void init_mat(mat& m) {
	int count = 1;
	for (int i = 0; i < m.sz; i++) {
		for (int j = 0; j < m.sz; j++) {
			m.data[i * m.sz + j] = count++;
		}
	}
}

// Creates an identity matrix. For a 4x4 matrix:

// 1 0 0 0
// 0 1 0 0
// 0 0 1 0
// 0 0 0 1

void identity_mat(mat& m) {
	int count = 0;
	for (int i = 0; i < m.sz; i++) {
		for (int j = 0; j < m.sz; j++) {
			m.data[i * m.sz + j] = (count++ % (m.sz + 1)) ? 0 : 1;
		}
	}
}
// struct for SIMD double-precision
struct d_mat {
	double* data;
	const size_t sz;
	bool operator==(const d_mat& rhs) const {
		return !std::memcmp(data, rhs.data, sz * sz * sizeof(data[0]));
	}
};

void print_d_mat(d_mat& m) {
	for (int i = 0; i < m.sz; i++) {
		for (int j = 0; j < m.sz; j++) {
			std::cout << std::setw(3) << m.data[i * m.sz + j] << ' ';
		}
		std::cout << '\n';
	}
	std::cout << '\n';
}

void init_d_mat(d_mat& m) {
	int count = 1;
	for (int i = 0; i < m.sz; i++) {
		for (int j = 0; j < m.sz; j++) {
			m.data[i * m.sz + j] = count++;
		}
	}
}
 
void identity_d_mat(d_mat& m) {
	int count = 0;
	for (int i = 0; i < m.sz; i++) {
		for (int j = 0; j < m.sz; j++) {
			m.data[i * m.sz + j] = (count++ % (m.sz + 1)) ? 0 : 1;
		}
	}
}

//single precision
void matmul_simd_singlePrecision(mat& mres, const mat& m1, const mat& m2) 
	{
	int row = 0, datachunk = 0, column = 0;
	 	alignas (sizeof(__m128)) float columnAligned[SZ]{}; 

	for (int row = 0; row < mres.sz; row++) {
		for (int column = 0; column < mres.sz; column++) {
			mres.data[row * mres.sz + column] = 0;

			__m128 vectorSum = _mm_set_ps1(0); //set the sum of the vector to 0
			
			for (int i = 0; i < SZ; i++)
			{
				columnAligned[i] = m2.data[(i * SZ) + column]; //turn the column into a row for ease of multiplication
			}

			for (datachunk = 0; datachunk < SZ; datachunk += 4) 
			{
				__m128 columnchunk = _mm_load_ps(&columnAligned[(datachunk)]); //add the aligned column to __m128
				__m128 rowchunk = _mm_load_ps(&m1.data[(row * mres.sz) + datachunk]); //add the row to __m128

				__m128 vectorProducts = _mm_mul_ps(rowchunk, columnchunk); // calculate the datachunk products
				vectorSum = _mm_add_ps(vectorProducts, vectorSum);
			}

			vectorSum = _mm_hadd_ps(vectorSum, vectorSum); // add the sum of products
 
			_mm_store_ss(&mres.data[(row * mres.sz) + column], vectorSum); // new matrix to store the results 
		}
	}
}


//double precision
 void matmul_simd_doublePrecision(d_mat& mres, const d_mat& m1, const d_mat& m2)
{
	 int row = 0, datachunk = 0, column = 0;
	alignas (sizeof(__m256d)) double columnAligned[SZ]{}; 

	for (row = 0; row < mres.sz; row++) {
		for (column = 0; column < mres.sz; column++) {
			mres.data[row * mres.sz + column] = 0;

			__m256d vectorSum = _mm256_set_pd(0, 0, 0, 0); //set the sum of the vector to 0

			for (int i = 0; i < SZ; i++)
			{
				columnAligned[i] = m2.data[(i * SZ) + column]; //turn the column into a row for ease of multiplication
			}

			for (datachunk = 0; datachunk < SZ; datachunk += 4) 
			{
				__m256d columnchunk = _mm256_load_pd(&columnAligned[(datachunk)]); //add the aligned column to __m256
				__m256d rowchunk = _mm256_load_pd(&m1.data[(row * mres.sz) + datachunk]);//add the row to __m256

				__m256d vectorProduct = _mm256_mul_pd(rowchunk, columnchunk); // calculates products of the datachunks
				vectorSum = _mm256_add_pd(vectorProduct, vectorSum);
			}
			double total = 0; 
			double vecSumChunkArray[4]; 
			_mm256_store_pd(vecSumChunkArray, vectorSum); // store the vector sum
			for (int i = 0; i < 4; i++)
			{
				total += vecSumChunkArray[i]; // calculate the sum of the values in the array
			}

			mres.data[(row * mres.sz) + column] = total; // setting mres to totalsum

		}
	}
}



int main(int argc, char* argv[])
{
	mat mres{ new float[SZ * SZ],SZ }, m{ new float[SZ * SZ],SZ }, id{ new float[SZ * SZ],SZ };
	mat mres2{ new float[SZ * SZ],SZ };
	d_mat mres3{ new double[SZ * SZ],SZ }, d_m{ new double[SZ * SZ],SZ }, d_id{ new double[SZ * SZ],SZ };

	using namespace std::chrono;
	using tp_t = time_point<high_resolution_clock>;
	tp_t t1, t2;

	std::cout << "Each " << SZ << 'x' << SZ;
	std::cout << " matrix is " << sizeof(float) * SZ * SZ << " bytes.\n";

	init_mat(m);
	identity_mat(id);

	init_d_mat(d_m);
	identity_d_mat(d_id);

	//time for Serial code
	t1 = high_resolution_clock::now();
	matmul(mres, m, m);
	t2 = high_resolution_clock::now();

	auto serial = duration_cast<microseconds>(t2 - t1).count();
	std::cout << " Serial code time: " << serial << ' ' << "microseconds.\n";

	// time for SIMD single-precision 
	t1 = high_resolution_clock::now();
	matmul_simd_singlePrecision(mres2, m, m);
	t2 = high_resolution_clock::now();

    auto single_precision = duration_cast<microseconds>(t2 - t1).count();
	std::cout << " SIMD single-precision time: " << single_precision << ' ' << "microseconds.\n";

	//time for SIMD double-precision: 
	t1 = high_resolution_clock::now();
	matmul_simd_doublePrecision(mres3, d_m, d_m);
	t2 = high_resolution_clock::now();

    auto double_precision = duration_cast<microseconds>(t2 - t1).count();
	std::cout << " SIMD double-precision time: " << double_precision << ' ' << "microseconds.\n";

   // print_mat(m);
   // print_mat(mres);
   // print_mat(mres2);
   // print_d_mat(mres3);

	const bool correct = mres == mres2;


	delete[] mres.data;
	delete[] mres2.data;
	delete[] mres3.data;
	delete[] id.data;
	delete[] d_id.data;
	delete[] m.data;
	delete[] d_m.data;
	

	return correct ? 0 : -1;
}
