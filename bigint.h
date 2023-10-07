#pragma once
#include <vector>
#include <string>
#include <string.h>
#include <random>

// Number of bits in 1 digit. Must be <= 32
const int bit_base = 32;
// Base of the counting system. Numbers are stored in binary format, so base = 2**bit_base
const unsigned long long base = (unsigned long long)pow(2, bit_base);

class RNG;
unsigned int create_bitmask(unsigned char bits);

class bigint
{
private:
    std::vector<unsigned int> digits;
    bool is_negative;
    void _div(bigint &b, bigint *res, bigint *mod);
    int _bits_count();
    void set_bit(int pos, bool val);
    bool get_bit(int pos);
public:
    bigint();
    bigint(bool negative, const unsigned int* digits_arr, size_t count);
    bigint(long long num);
    bigint(RNG &rng, unsigned int bits);
    bigint(const bigint& b);

    std::string to_string() const;

    void operator+=(bigint &b);
    void operator-= (bigint &b);

    bigint operator+ (bigint &b);
    bigint operator- (bigint &b);
    bigint operator* (bigint &b);
    bigint operator/ (bigint &b);
    bigint operator% (bigint &b);

    bool operator == (bigint &b);
    bool operator == (long num);
    bool operator != (bigint &b);
    bool operator < (bigint &b);
    bool operator < (long num);
    bool operator > (bigint &b);
    bool operator > (long num);
    bool operator <= (bigint &b);
    bool operator <= (long num);
    bool operator >= (bigint &b);
    bool operator >= (long num);

    bigint operator >> (int bits);
    bigint operator << (int bits);
    void operator <<= (int bits);
    void operator >>= (int bits);

    bigint friend operator- (bigint& big);

    ~bigint();
};

bigint operator- (bigint& big);
std::ostream& operator << (std::ostream &os, const bigint &big);

class RNG {
    std::mt19937 engine;
public:
    RNG();
    unsigned get_random(unsigned min, unsigned max);
};