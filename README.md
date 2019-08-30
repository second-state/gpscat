# gpscat

gpscat is a general-purpose static cost analysis toolchain for LLVM IR.

# Dependencies

## For Building gpscat

- g++ >= 7.0 (with c++17 support)
- CMake >= 3.8
- LLVM >= 8.0.0
- SymEngine >= 0.4.0 (should be built with thread safety)
- Boost >= 1.67.0

## For Executing gpscat Tools

- CoFloCo
- llvm2kittel (with modification for supporting LLVM 8.0.0)
- llc (LLVM static compiler)

# Building

```bash
cd gpscat
mkdir build && cd build
cmake ..
make
# Now, you will see two tools "gpscat-cost" and "gpscat-score" in the build directory
# You can optionally run the unit tests
ctest
```

Once the building process is finished, you can try out gpscat with this command.

```bash
./gpscat-cost -arch=wasm32 ../tests/examples/costModel.csv ../tests/examples/1.bc -eager-inline | ./gpscat-score -bounds-file ../tests/examples/bounds
```

For more info about how to use these tools, pass `-help` to them.

```bash
gpscat-cost -help
gpscat-score -help
```
