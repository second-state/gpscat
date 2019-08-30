## Test Results
The test results of the examples in gpscat/tests/examples.

| example                     | None | O0 | O1 | O2 | O3 | Os | Oz |
| --------------------------- | ---- | -- | -- | -- | -- | -- | -- |
| 1.bc                        |    O |  O |  O |  O |  O |  O |  O |  
| 10.bc (infinite loop)       |    I |  I |  I |  I |  I |  I |  I |
| 11.bc                       |    O |  O |  O |  O |  O |  O |  O |
| 2.bc                        |    O |  O |  O |  O |  O |  O |  O |
| 3.bc                        |    I |  I |  I |  I |  I |  I |  I |
| 4.bc                        |    O |  O |  O |  O |  O |  O |  O |
| 5.bc                        |    T |  T |  T |  T |  T |  T |  T |
| 6.bc                        |    T |  T |  T |  T |  T |  T |  T |
| 7.bc                        |    O |  O |  O |  O |  O |  O |  O |
| 8.bc                        |    O |  O |  O |  O |  O |  O |  O |
| 9.bc                        |    O |  O |  O |  O |  O |  O |  O |
| base64decode.bc             |    O |  O |  I |  I |  I |  I |  I |
| base64encode.bc             |    I |  I |  I |  I |  I |  I |  I |
| bellmanford.bc              |    O |  O |  O |  O |  O |  O |  O |
| binarygcd.bc                |    T |  T |  T |  T |  T |  T |  T |
| binarysearch.bc             |    I |  I |  O |  O |  O |  O |  O |
| gcd.bc                      |    I |  I |  O |  O |  O |  O |  O |
| matmul.bc                   |    O |  O |  O |  O |  O |  O |  O |
| mergesort.bc                |    I |  I |  I |  I |  I |  I |  I |
| pow.bc                      |    O |  O |  O |  O |  O |  O |  O |
| quicksort.bc                |    T |  T |  T |  T |  T |  T |  I |
| sha256.bc                   |    X |  X |  X |  X |  X |  X |  X |
| trivial.bc                  |    O |  O |  O |  O |  O |  O |  O |
| zvalue.bc                   |    I |  I |  I |  I |  I |  I |  I |

Status codes:
- O : Okay. No errors occurred when running gpscat-cost and gpscat-score.
- I : Infinity. The gpscat-cost outputs an infinity uppber bound.
- X : Fail. Some errors occurred when testing. Can be caused by timeout, too.
- T : Timeout.

Optimization Levels:
- None : No optimization options were passed to opt.
- O0, O1, O2, O3, Os, Oz : Optimization level passed to opt.

## Known Issues

### Known Issues of gpscat

### Known Issues of External Tools

#### CoFloCo cannot analyze harmonic series

Loops of this form cannot be handled correctly by CoFloCo.

```cpp
for(int i = 1; i <= n; ++i)
    for(int j = 0; j < n; j += i)
        //do something
```

The bounds of them are closely related to harmonic series, which attain
complexity of O(nlogn).

Though CoFloCo cannot analyze logarithmic bounds, it should be able
to obtain O(n^2) bounds for this kinds of loops. However, it produces
infinity bounds.

#### CoFloCo may not be able to analyze all kinds of amortized bounds

The test example zvalue.bc, which has a complexity of O(n) by amortized
analysis, is not correctly analyzed by CoFloCo, which gives a infinity bound.
