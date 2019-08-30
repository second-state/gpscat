int foo(int x) {
    int ret = 1002;
    for(int i = 1; i <= x/2; ++i)
        ret /= i;
    return ret;
}
int h(int x) {
    int ret = 1;
    for(int i = 1; i <= x*2; ++i)
        ret *= foo(i);
    return ret;
}
int g(int x) {
    int ret = 0;
    for(int i = 1; i <= x+2; ++i)
        ret -= h(i);
    return ret;
}
int f(int x) {
    int sum = 0;
    for(int i = 1; i <= x-2; ++i)
        sum += g(i);
    return sum;
}
int main(int x) {
    return f(x);
}
