//
// Created by zia on 4/29/21.
//

#ifndef P3_H
#define P3_H

#include <ostream>

using namespace std;

class p3
{
public:
    enum search_type
    {
        UVW_SEARCHABLE, WEIGHT_SEARCHABLE
    };

private:
    static int n;

    unsigned long long to_hash() const
    {
        unsigned long long _n = n;
        unsigned long long _u = u;
        unsigned long long _v = v;
        unsigned long long _w = w;
        return
                _n * _n * (_u < _w ? _u : _w) +
                _n * _v +
                (_u > _w ? _u : _w);
    }

    search_type search_t;

public:

    float weight;
    int u, v, w;        // u is always assumed to be smaller than w in comparing calculations
    p3(int u, int v, int w, float weight, search_type s_t) : u(u), v(v), w(w), weight(weight), search_t(
            s_t) {}       // u and w are always assumed to be connected to v

    friend bool operator<(const p3 &p3_1, const p3 &p3_2)
    {
        if (p3_1.search_t == UVW_SEARCHABLE && p3_2.search_t == UVW_SEARCHABLE)
            return p3_1.to_hash() < p3_2.to_hash();
        else if (p3_1.search_t == WEIGHT_SEARCHABLE && p3_2.search_t == WEIGHT_SEARCHABLE)
            return p3_1.weight > p3_2.weight;
        exit(-1);       // should never reach this line
    }

    friend ostream &operator<<(ostream &os, const p3 &_p3)
    {
        os << "[p3: " << (_p3.u < _p3.w ? _p3.u : _p3.w) << ", " << _p3.v << ", "
           << (_p3.u < _p3.w ? _p3.w : _p3.u) << "]";
        return os;
    }

    static void set_n(int _n)
    {
        n = _n;
    }
};

int p3::n = 0;

#endif
