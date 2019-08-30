#!/usr/bin/env python3

import os
import sys
import subprocess

options = [None, 'O0', 'O1', 'O2', 'O3', 'Os', 'Oz']

filenames = sorted(os.listdir())
bcFilenames = list(filter(lambda x : x[-3:] == '.bc', filenames))

for bcFilename in bcFilenames:
    print('-' * 30 + '\n')
    print(bcFilename + '\n')
    for option in options:
        optArg = ['-' + option] if option else []
        optTool = subprocess.Popen(['opt',
                                    bcFilename,
                                    *optArg], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        costAnalyzer = subprocess.Popen(['gpscat-cost',
                                         'costModel.csv',
                                         '-arch=wasm32',
                                         '-replace-nat',
                                         #'-eager-inline',
                                         '-inline=1000',
                                         ], stdin=optTool.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        optTool.stdout.close()
        costFunction, err = costAnalyzer.communicate()

        evaluator = subprocess.Popen(['gpscat-score',
                                      '-bounds-file=bounds',
                                      ], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        score, err = evaluator.communicate(input=costFunction)

        print(option, costFunction.decode('utf-8').strip(), score.decode('utf-8').strip(), sep='\n', end='\n\n')
