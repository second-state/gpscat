int h(int x) {
    int ret = 1;
    for(int i = 0; i < x; ++i)
        ret *= i;
    return ret;
}
int g(int x) {
    int ret = 0;
    for(int i = 0; i < x; ++i)
        ret += h(i);
    return ret;
}
int f(int x) {
    int sum = 0;
    for(int i = 0; i < x; ++i)
        sum += g(i);
    return sum;
}
int main() {
    return f(42);
}
