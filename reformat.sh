#!/bin/bash


awk 'NR <= 10000 {val=strtonum($2); printf "0x%x\n", val}' addr.gen > parseaddr.gen
python3 test.py

md5sum parseaddr.gen test.out