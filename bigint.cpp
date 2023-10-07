#include "bigint.h"
#include <iostream>
#include <sstream>
#include <chrono>

unsigned int create_bitmask(unsigned char bits)
{
    unsigned int bitmask = 0;
    for (int i = 0; i < bits; i++) {
        bitmask |= (1 << i);
    }
    return bitmask;
}

bigint::bigint() : is_negative(false)
{
}

bigint::bigint(bool negative, const unsigned int* digits_arr, size_t count) : is_negative(negative)
{
    for(int i = count - 1; i>= 0; i--){
        unsigned int digit = digits_arr[i];
        digits.push_back(digit);
    }
}

bigint::bigint(long long num) : is_negative(false)
{
    if (num < 0){
        is_negative = true;
        num = -num;
    }
    else if (num == 0){
        digits.push_back(0);
    }
    while(num > 0){
        digits.push_back(num % base);
        num /= base;
    }
}

bigint::bigint(RNG &rng, unsigned int bits) : is_negative(false)
{
    unsigned int major_bits = bits % bit_base;
    unsigned int whole_digits_count = bits / bit_base;
    unsigned int digits_count = whole_digits_count + (major_bits > 0 ? 1 : 0);
    digits.resize(digits_count);
    for (int i = 0; i < whole_digits_count; i++) {
        digits[i] = rng.get_random(0, base - 1);
    }
    digits[whole_digits_count] = rng.get_random(0, (1 << major_bits) - 1);
}

bigint::bigint(const bigint& b) {
    digits = b.digits;
    is_negative = b.is_negative;
}

std::string bigint::to_string() const{
    std::stringstream result;
    if (digits.size() == 0){
        result << 0;
        return result.str();
    }
    if (is_negative) result << '-';
    for(int i = digits.size() - 1; i >= 0; i--){
        result << std::hex << digits[i];
    }
    return result.str();
}

bool bigint::is_prime()
{
    return false;
}

unsigned long long bigint::log2()
{
    return _bits_count() - 1;
}

void bigint::operator+=(bigint b){
    bigint tmp = *this + b;
    *this = tmp;
}

bigint bigint::operator+ (bigint b){
    if (b.is_negative) {
        bigint pos_b = b;
        pos_b.is_negative = false;
        return *this - pos_b;
    }
    bigint c;
    int len = 0;
    bigint *bigger;
    if (digits.size() < b.digits.size()){
         len = digits.size();
         bigger = &b;
    }
    else {
        len = b.digits.size();
        bigger = this;
    }
    int carry = 0;
    for(int i = 0; i < len; i++){
        unsigned long long digit = digits[i];
        digit = digit + b.digits[i] + carry;
        carry = digit / base;
        digit = digit % base;
        c.digits.push_back(digit);
    }
    for(int i = len; i < bigger->digits.size(); i++){
        unsigned long long digit = bigger->digits[i] + carry;
        carry = digit / base;
        digit = digit % base;
        c.digits.push_back(digit);
    }
    if (carry) c.digits.push_back(carry);
    return c;
}

void bigint::operator-= (bigint b){
    bigint tmp = *this - b;
    *this = tmp;
}

void bigint::operator*=(bigint b)
{
    bigint tmp = *this * b;
    *this = tmp;
}

void bigint::operator/=(bigint b)
{
    bigint tmp = *this / b;
    *this = tmp;
}

void bigint::operator%=(bigint b)
{
    bigint tmp = *this % b;
    *this = tmp;
}

void bigint::operator++(int d)
{
    bigint one(1);
    *this += one;
}

void bigint::operator--(int d)
{
    bigint one(1);
    *this -= one;
}

bigint bigint::operator- (bigint b){
    bigint c;
    if (*this < b){
        c = b - *this;
        c.is_negative = true;
        return c;
    }
    int len = b.digits.size();
    int carry = 0;
    for(int i = 0; i < len; i++){
        long long digit = digits[i];
        digit = digit - b.digits[i] - carry;
        carry = 0;
        if (digit < 0){
            digit += base;
            carry = 1;
        }
        c.digits.push_back(digit);
    }
    for(int i = len; i < digits.size(); i++){
        long long digit = digits[i] - carry;
        carry = 0;
        if (digit < 0){
            digit += base;
            carry = 1;
        }
        c.digits.push_back(digit);
    }
    if (carry) c.digits.push_back(carry);
    c.trim();
    return c;
}

bigint bigint::operator* (bigint b){
    bigint c;
    c.digits.resize(digits.size() + b.digits.size(), 0);
    for(int i = 0; i < b.digits.size(); i++){
        unsigned long long mult_carry = 0;
        unsigned long long add_carry = 0;
        for(int j = 0; j < digits.size(); j++){
            unsigned long long digit = digits[j];
            digit = digit * b.digits[i] + mult_carry;
            mult_carry = digit / base;
            digit %= base;
            unsigned long long num = c.digits[i + j];
            num += digit + add_carry;
            add_carry = num / base;
            num %= base;
            c.digits[i + j] = num;
        }
        if (mult_carry) c.digits[i + digits.size()] += mult_carry;
        if (add_carry) {
            for(int j = i + digits.size(); j < c.digits.size(); j++){
                unsigned long long digit = c.digits[j];
                digit += add_carry;
                add_carry = digit / base;
                digit %= base;
                c.digits[j] = digit;
            }
        }
    }
    c.trim();
    c.is_negative = is_negative ^ b.is_negative;
    return c;
}

int bigint::_bits_count(){
    int bits_count = (digits.size() - 1) * bit_base;
    int last_bits = 0;
    unsigned int last_digit = digits.back();
    while (last_digit > 0)
    {
        last_bits++;
        last_digit >>= 1;
    }
    return bits_count + last_bits;
}

void bigint::set_bit(int pos, bool val){
    if (pos < digits.size() * bit_base){
        int index = pos / bit_base;
        int bit_index = pos % bit_base;
        if (val){
            digits[index] |= (1 << bit_index);
        }
        else{
            digits[index] &= (~(1 << bit_index));
        }
    }
}
bool bigint::get_bit(int pos){
    if (pos < _bits_count()){
        int index = pos / bit_base;
        int bit_index = pos % bit_base;
        return (digits[index] & (1 << bit_index)) >> bit_index;
    }
    return false;
}

void bigint::trim()
{
    while (digits.size() > 1 && digits.back() == 0) {
        digits.pop_back();
    }
}

void bigint::_div(bigint &b, bigint *res, bigint *mod){
    *res = 0LL;
    *mod = 0LL;
    res->digits.resize(digits.size(), 0);
    for(int i = _bits_count() - 1; i >= 0; i--){
        *mod <<= 1;
        unsigned int bit = get_bit(i);
        mod->set_bit(0, bit);
        if (*mod >= b){
            *mod -= b;
            res->set_bit(i, 1);
        }
    }
    res->trim();
}

bigint bigint::operator/ (bigint b){
    if (b == 0){
        throw "Divide by 0";
    }
    bigint res, mod;
    _div(b, &res, &mod);
    res.is_negative = is_negative ^ b.is_negative;
    return res;
}

bigint bigint::operator% (bigint b){
    if (b == 0){
        throw "Divide by 0";
    }
    if (is_negative || b.is_negative) {
        throw "Only positive numbers mod supported";
    }
    bigint res, mod;
    _div(b, &res, &mod);
    return mod;
}

bigint bigint::operator+(long num)
{
    bigint b(num);
    return *this + b;
}

bigint bigint::operator-(long num)
{
    bigint b(num);
    return *this - b;
}

bigint bigint::operator*(long num)
{
    bigint b(num);
    return *this * b;
}

bigint bigint::operator/(long num)
{
    bigint b(num);
    return *this / b;
}

bigint bigint::operator%(long num)
{
    bigint b(num);
    return *this % b;
}

bool bigint::operator == (bigint b){
    if (is_negative != b.is_negative) return false;
    if (digits.size() != b.digits.size()) return false;
    for(int i = 0; i < digits.size(); i++){
        if (digits[i] != b.digits[i])
            return false;
    }
    return true;
}

bool bigint::operator == (long num){
    bigint rhs = bigint(num);
    return *this == rhs;
}

bool bigint::operator != (bigint b){
    return !(*this == b);
}

bool bigint::operator!=(long num)
{
    bigint rhs = bigint(num);
    return *this == rhs;
}

bool bigint::operator < (bigint b){
    // this < 0 and b > 0 => this < b
    if (is_negative && !b.is_negative) return true;
    // this > 0 and b < 0 => this > b
    if (!is_negative && b.is_negative) return false;
    // both > 0
    if (!is_negative && !b.is_negative) {
        if (digits.size() != b.digits.size()) return digits.size() < b.digits.size();
        for (int i = digits.size() - 1; i >= 0; i--) {
            if (digits[i] > b.digits[i]) return false;
            if (digits[i] < b.digits[i]) return true;
        }
        // this == b
        return false;
    }
    // both < 0, invert comparisons
    if (is_negative && b.is_negative) {
        if (digits.size() != b.digits.size()) return digits.size() > b.digits.size();
        for (int i = digits.size() - 1; i >= 0; i--) {
            if (digits[i] < b.digits[i]) return false;
            if (digits[i] > b.digits[i]) return true;
        }
        // this == b
        return false;
    }
}

bool bigint::operator < (long num)
{
    bigint rhs = bigint(num);
    return *this < rhs;
}

bool bigint::operator > (bigint b){
    return b < *this;
}

bool bigint::operator > (long num)
{
    bigint rhs = bigint(num);
    return *this > rhs;
}

bool bigint::operator <= (bigint b){
    return !(*this > b);
}
bool bigint::operator<=(long num)
{
    return *this < num || *this == num;
}
bool bigint::operator >= (bigint b){
    return !(*this < b);
}

bool bigint::operator>=(long num)
{
    return *this > num || *this == num;
}

bigint bigint::operator>>(int bits)
{
    if (bits >= bit_base || bits >= 32) return *this;
    bigint res;
    res.digits.resize(digits.size());
    for (int i = 0; i < digits.size(); i++) {
        res.digits[i] = digits[i] >> bits;
        if(i + 1 < digits.size()){
            unsigned int bmask = create_bitmask(bits);
            unsigned int masked = digits[i + 1] & (bmask);
            unsigned int shifted = masked << (bit_base - bits);
            res.digits[i] |= shifted;
        }
    }
    res.trim();
    return res;
}

bigint bigint::operator << (int bits){
    if (bits >= bit_base || bits >= 32)
        throw "bits >= bit_base or size of uint32";
    bigint res;
    res.digits.resize(digits.size());
    if (res.digits.size() == 0) {
        res.digits.push_back(0);
        return res;
    }
    if (digits[digits.size() - 1] & (create_bitmask(bits) << (bit_base - bits))){
        unsigned int bmask = create_bitmask(bits) << (bit_base - bits);
        unsigned int masked = digits[digits.size() - 1] & (bmask);
        unsigned int shifted = masked >> (bit_base - bits);
        res.digits.push_back(shifted);
    }
    for (int i = digits.size() - 1; i >=0 ; i--) {
        res.digits[i] = digits[i] << bits;
        if(i - 1 >= 0){
            unsigned int bmask = create_bitmask(bits) << (bit_base - bits);
            unsigned int masked = digits[i - 1] & (bmask);
            unsigned int shifted = masked >> (bit_base - bits);
            res.digits[i] |= shifted;
        }
    }
    return res;
}

void bigint::operator <<= (int bits){
    bigint tmp = *this << bits;
    *this = tmp;
}
void bigint::operator >>= (int bits){
    bigint tmp = *this >> bits;
    *this = tmp;
}

unsigned int bigint::operator[](int index)
{
    if (index >= digits.size())
        throw "Index is too big";
    return digits[index];
}

bigint::~bigint()
{
}

bigint operator- (bigint& big) {
    bigint c = big;
    c.is_negative = !big.is_negative;
    return c;
}

std::ostream& operator << (std::ostream &os, const bigint &big)
{
    std::string str = big.to_string();
    return os << str;
}

RNG::RNG() {
    std::random_device device;
    engine.seed(std::chrono::steady_clock::now().time_since_epoch().count());
}
unsigned RNG::get_random(unsigned min, unsigned max) {
    std::uniform_int_distribution<unsigned> dist(min, max);
    return dist(engine);
}

bigint RNG::get_big_random(bigint min, bigint max)
{
    unsigned int bits = max.log2() + 1;
    bigint res(*this, bits);
    if (res + min > max) {
        bigint tmp = (tmp + min) % max;
        res = tmp;
    }
    return res + min;
}