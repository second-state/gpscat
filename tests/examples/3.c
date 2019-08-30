int bar(int x) {
    return 3 * x + 1;
}
int foo(int x) {
    int ret = 1;
    for(int i = 0; i < x; ++i)
        ret *= i;
    return bar(ret);
}
int h(int x) {
    return foo(x*2+1);
}
int g(int x) {
    int ret = 0;
    for(int i = 0; i < x; i += 2)
        ret += h(i);
    return ret;
}
int f(int x) {
    int sum = 0;
    for(int i = 1; i <= x; ++i)
        for(int j = 1; j <= x; j += i)
            sum += g(j);
    return sum;
}
int main(int x) {
    return f(x);
}
