int gcd(int a, int b) {
    while((a%=b)&&(b%=a));
    return a + b;
}
