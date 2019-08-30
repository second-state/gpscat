#!/usr/bin/env python3

'''
The idea of this experiment is from this paper:

Georgiou, Kyriakos, et al.
"Less is more: Exploiting the standard compiler optimization levels for better performance and energy consumption."
Proceedings of the 21st International Workshop on Software and Compilers for Embedded Systems. ACM, 2018.

'''
import sys
import os
import subprocess
import pandas as pd

# parse command line arguments

def getCommandlineArg(i):
    return sys.argv[i] if i < len(sys.argv) else None

assert(len(sys.argv) >= 2)

# where the benchmark bitcodes are
benchmarkDirectory = getCommandlineArg(1)
# the base sequence for FDO experiment
optSequenceBaseArg = getCommandlineArg(2)

# constants

O0params = """-ee-instrument -forceattrs -always-inline""".split()
O1params = """-ee-instrument -simplifycfg -sroa -early-cse -lower-expect
              -forceattrs -inferattrs
              -ipsccp -called-value-propagation -globalopt -mem2reg
              -deadargelim -instcombine -simplifycfg -globals-aa -prune-eh
              -always-inline -functionattrs
              -sroa -early-cse-memssa -speculative-execution -jump-threading
              -correlated-propagation -simplifycfg
              -instcombine -libcalls-shrinkwrap -pgo-memop-opt -tailcallelim
              -simplifycfg -reassociate -loop-simplify -loop-rotate -licm
              -loop-unswitch -simplifycfg -instcombine -indvars
              -loop-idiom -loop-deletion -loop-unroll
              -memcpyopt -sccp -bdce -instcombine -jump-threading
              -correlated-propagation -dse -licm -adce
              -simplifycfg -instcombine -barrier
              -rpo-functionattrs -globalopt -globaldce -globals-aa -float2int
              -loop-rotate -loop-distribute -loop-vectorize -loop-load-elim
              -instcombine -simplifycfg
              -loop-unroll -instcombine -licm -alignment-from-assumptions
              -strip-dead-prototypes
              -loop-sink -instsimplify -div-rem-pairs -simplifycfg""".split()
O2params = """-ee-instrument -simplifycfg -sroa -early-cse -lower-expect
              -forceattrs -inferattrs
              -ipsccp -called-value-propagation -globalopt -mem2reg
              -deadargelim -instcombine -simplifycfg -globals-aa -prune-eh
              -inline -functionattrs
              -sroa -early-cse-memssa -speculative-execution -jump-threading
              -correlated-propagation -simplifycfg
              -instcombine -libcalls-shrinkwrap -pgo-memop-opt -tailcallelim
              -simplifycfg -reassociate -loop-simplify -loop-rotate -licm
              -loop-unswitch -simplifycfg -instcombine -indvars
              -loop-idiom -loop-deletion -loop-unroll -mldst-motion -gvn
              -memcpyopt -sccp -bdce -instcombine -jump-threading
              -correlated-propagation -dse -licm -adce
              -simplifycfg -instcombine -barrier -elim-avail-extern
              -rpo-functionattrs -globalopt -globaldce -globals-aa -float2int
              -loop-rotate -loop-distribute -loop-vectorize -loop-load-elim
              -instcombine -simplifycfg -slp-vectorizer -instcombine
              -loop-unroll -instcombine -licm -alignment-from-assumptions
              -strip-dead-prototypes -globaldce -constmerge
              -loop-sink -instsimplify -div-rem-pairs -simplifycfg""".split()
O3params = """-ee-instrument -simplifycfg -sroa -early-cse -lower-expect
              -forceattrs -inferattrs -callsite-splitting
              -ipsccp -called-value-propagation -globalopt -mem2reg
              -deadargelim -instcombine -simplifycfg -globals-aa -prune-eh
              -inline -functionattrs -argpromotion
              -sroa -early-cse-memssa -speculative-execution -jump-threading
              -correlated-propagation -simplifycfg -aggressive-instcombine
              -instcombine -libcalls-shrinkwrap -pgo-memop-opt -tailcallelim
              -simplifycfg -reassociate -loop-simplify -loop-rotate -licm
              -loop-unswitch -simplifycfg -instcombine -indvars
              -loop-idiom -loop-deletion -loop-unroll -mldst-motion -gvn
              -memcpyopt -sccp -bdce -instcombine -jump-threading
              -correlated-propagation -dse -licm -adce
              -simplifycfg -instcombine -barrier -elim-avail-extern
              -rpo-functionattrs -globalopt -globaldce -globals-aa -float2int
              -loop-rotate -loop-distribute -loop-vectorize -loop-load-elim
              -instcombine -simplifycfg -slp-vectorizer -instcombine
              -loop-unroll -instcombine -licm -alignment-from-assumptions
              -strip-dead-prototypes -globaldce -constmerge
              -loop-sink -instsimplify -div-rem-pairs -simplifycfg""".split()

# run experiment

optSequenceBase = {'O0': O0params, 'O1': O1params, 'O2': O2params, 'O3': O3params}.get(optSequenceBaseArg, O3params)
baselineOptLevels = [None, 'O0', 'O1', 'O2', 'O3', 'Os', 'Oz']

inputBitcodes = list(filter(lambda x : x[-3:] == '.bc', sorted(os.listdir(benchmarkDirectory))))

def isNumber(x):
    try:
        float(x)
        return True
    except ValueError:
        return False

def getScore(optArgList):
    optTool = subprocess.Popen(['opt',
                                os.path.join(benchmarkDirectory, inputBitcode),
                                *optArgList,
                                ], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    costAnalyzer = subprocess.Popen(['gpscat-cost',
                                     'costModel.csv',
                                     '-arch=wasm32',
                                     '-replace-nat',
                                     '-inline=1000',
                                     ], stdin=optTool.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    evaluator = subprocess.Popen(['gpscat-score',
                                  '-bounds-file=bounds',
                                  ], stdin=costAnalyzer.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    optTool.stdout.close()
    costAnalyzer.stdout.close()
    output, err = evaluator.communicate()

    score = output.decode('utf-8').strip()
    return score if isNumber(score) else None

experimentResults = {}

for inputBitcode in inputBitcodes:
    print(inputBitcode)
    experimentResults[inputBitcode] = []
    # get baseline score
    for baselineOptLevel in baselineOptLevels:
        score = getScore(['-' + baselineOptLevel] if baselineOptLevel else [])
        print("Baseline {} score:".format(baselineOptLevel), score)
        experimentResults[inputBitcode].append(score)

    # get experiment scores
    for i in range(len(optSequenceBase)+1):
        optSequence = optSequenceBase[:i]
        score = getScore(optSequence)
        print("Prefix {} score:".format(i), score)
        experimentResults[inputBitcode].append(score)

resultDataFrame = pd.DataFrame.from_dict(experimentResults, orient='index', columns=['Baseline {}'.format(i) for i in baselineOptLevels] + ['Prefix {}'.format(i) for i in range(len(optSequenceBase)+1)])

resultDataFrame.to_csv('result.csv', index_label='Filename')
