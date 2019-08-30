long long pow(long long b, long long e) {
    long long x = 1;
    for(; e; e >>= 1) {
        if(e & 1) x *= b;
        b *= b;
    }
    return x;
}
