// temp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <stdio.h>

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
    long s = GetTickCount();
    int np = NumPrimes(n);
    long e = GetTickCount();
    printf("\n#primes to %d ... %d (%d ms)", n, np, e - s);
}

void bench() {
    int s = GetTickCount();
    int a = 0, b = 0, c;
    for (int i = 0; i < 400000000; i++) { c = (++a) + (++b); }
    int e = GetTickCount();
    printf("%d %d %d\n", c, e-s, (e-s)/1000);
}

void benches(int num) {
    for (int i = 0; i < num; i++) { bench(); }
}

int main()
{
    benches(3);
    //int n = 256;
    //for (int i = 1; i <= 16; i++) {
    //    n += n;
    //    pbench(n);
    //}
}
