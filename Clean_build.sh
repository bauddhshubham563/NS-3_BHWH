#!/bin/bash
# run_ai_project.sh
# Full experiment: Baseline + Blackhole + Wormhole + AI Mitigation
# Author: You ✨


cd ../cmake-cache
make clean
cmake .. -DCMAKE_BUILD_TYPE=Release -DNS3_EXAMPLES=ON -DNS3_TESTS=OFF
make -j$(nproc)
