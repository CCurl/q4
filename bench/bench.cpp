// temp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <time.h>

int isPrimeNumber(int n) {
    for (int i = 3; i < n; i += 2) {
        int q = n / i;
        int r = n % i;
        if (r == 0) { return 0; }
        if (q < i) { return 1; }
    }
    return 1;
}

int NumPrimes(int n) {
    int num = 4;
    for (int i = 11; i <= n; i += 2) {
        if (isPrimeNumber(i)) { ++num; }
    }
    return num;
}

void pbench(int n) {
    long s = clock();
    int np = NumPrimes(n);
    long e = clock();
    printf("\n#primes to %d ... %ld (%ld ms)", n, np, e - s);
}

void bench() {
    int s = clock();
    int a = 0, b = 0, c;
    for (int i = 0; i < 400000000; i++) { c = (++a) + (++b); }
    int e = clock();
    printf("%d %d %d\n", c, e-s, (e-s)/1000);
}

void benches(int num) {
    for (int i = 0; i < num; i++) { bench(); }
}

void jBench() {
  long pp = 2124680;
  int s = clock();
  // 710 ms (android clang -O3)
  for(long a=2; a<=100; a++) {
    long b = pp-1;
    for(long c=2; c<=b; c++) {
      long r= pp % c;
      if (r==0) {
	      printf("%ld is not prime says %ld\n", pp, c);
      }
    }
  }
  int e = clock();
  printf("time: %ld %ld\n", e-s, (e-s)/1000);
}

int main()
{
    //benches(3);
    // jBench();
    //int n = 256;
    //for (int i = 1; i <= 16; i++) {
    //    n += n;
    pbench(10*1000*1000);
    //}
}
