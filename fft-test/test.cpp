// --------------------------------------------------
// The slowest "fast fourier" transform in the world.
// --------------------------------------------------

// Recursion based fft radix 2, 
// accepts any length as long as it's radix 2.
// i.e. x = pow(2, some_number);

// Requirements: The setup of W is made before
// the call to fft(). 

// Setting up W is easy:
// we want W[i..N] in steps of 1. 
// i.e. for (int i = 0; i < N; i++) W[i] = Exp[-j*2*PI*i/N]
// (where j represents the imaginary number i; since i is
// used as an index).

// Assumes T is a type of complex number.
// e.g. complex float or complex double.
// or that you change the code to handle both the real
// and the imaginary.

// Challenge: rewrite this code as a template metaprogram
// which automatically expands and unrolls the recursion.

#include <vector>
#include <complex>
#include <cmath>

#include <iostream>

using namespace std;

template <class T>
void setup_w(vector <T> & W, unsigned int N)
{
	for (unsigned int i = 0; i < N; i++)
	{
		T z = 2.0 * M_PI * i / N;
		z = z * T(0.0, -1.0);
		W.push_back(exp(z));
	}
}

// NB: The algorithm here is similar to merge sort, and is typical
// of divide and conquer class of algorithms resulting in O(nlogn).

template <class T>
void fft(vector <T> & W, unsigned int N, vector <T> & x)
{
	if (N > 1)
	{
		vector <T> x_even;
		vector <T> x_odd;

		for (unsigned int i = 0; i < N; i += 2)
		{
			x_even.push_back(x[i]);
			x_odd.push_back(x[i+1]);
		}

		fft(W, N/2, x_even);
		fft(W, N/2, x_odd);

		// merge results

		for (unsigned int i = 0; i < N; i++)
		{
			x[i] = x_even[i % (N/2)] + 
				x_odd[i % (N/2)] * W[i * W.size() / N];
		}
	}
	else if (N == 1) {}
	else abort();
	// else if (N == 1) do nothing, just return x vector as is.
	// else (i.e. N = 0) should never occur, 
	// unless incorrect value of N is used (i.e. non-radix-2 value).
}

template <class T>
void dft(vector <T> & W, unsigned int N, vector <T> & x)
{
	vector <T> result;
	for (unsigned int j = 0; j < N; j++) {
		T sum(0.0, 0.0);
		for (unsigned int i = 0; i < N; i++) {
			sum += x[i] * exp(std::complex<double> (0.0, -1.0) * 2.0 * M_PI * (double) i * (double) j / (double) N);
		}

		result.push_back(sum);
	}

	x = result;
}

int main(int argc, char **argv)
{
	vector < complex < double > > W;
	vector < complex < double > > X, Y, Z;


	for (int i = 0; i < 1024; i++) {
		X.push_back(std::complex<double>(sin(i * 2.0 * M_PI / 1024), 0.0));
	}

	setup_w(W, X.size());

	Y = X;
	Z = X;

	fft(W, Y.size(), Y);
	dft(W, Z.size(), Z);

// NB: FFT seems to work, but DFT seems to be broken. Not sure why. 

	for (int i = 0; i < 1024; i++) {
		std::cout << "i: " << i << "\t\t" << X[i] << "\t\t" << abs(Y[i]) << "\t\t" << abs(Z[i]) << std::endl;
	}

	return 0;
}
