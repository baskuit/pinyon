#pragma once

class Rational {

    int p;
    int q;

    void reduce () {
        // eulcid's algorithm
        int a = p;
        int b = q;
        int c;
        while (b) {
            a %= b;
            c = a;
            a = b;
            b = c;
        }
        p /= a;
        q /= a;
    }

public : 

    Rational () :
        p(1), q(1) {}

    Rational (int p, int q) :
        p(p), q(q) {}

    Rational operator+ (Rational y) {
        Rational z = {p*y.q + y.p*q, p*q};
        z.reduce();
        return z;
    }

    Rational operator* (Rational y) {
        Rational z = {p*y.p, q*y.q};
        z.reduce();
        return z;
    }

    bool operator< (Rational y) {
        return p*y.q < y.p*q;
    }

    bool operator<= (Rational y) {
        return p*y.q <= y.p*q;
    }

    bool operator> (Rational y) {
        return p*y.q > y.p*q;
    }

    bool operator>= (Rational y) {
        return p*y.q >= y.p*q;
    }

    operator float() {
        return p/(float)q;
    }

    operator double() {
        return p/(double)q;
    }
    
};