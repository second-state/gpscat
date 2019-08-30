int binarysearch(int n, int* A, int val) {
    int l = 0, r = n - 1;
    while(l <= r) {
        int m = (l + r) / 2;
        if(A[m] < val)
            l = m + 1;
        else if(A[m] > val)
            r = m - 1;
        else
            return m;
    }
    return -1;
}
