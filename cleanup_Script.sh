#!/bin/bash
# cleanup.sh - Clean up all scattered files
cd /home/sirf-1/ns-allinone-3.39/ns-3.39

echo "Cleaning up all scattered files..."
rm -f pdr_results.txt nodes_stats.csv
rm -f *.xml *.png
rm -rf ~/ns-allinone-3.39/ns-3.39/ai_out
rm -rf ~/ns-allinone-3.39/ns-3.39/scratch/ai_out
rm -rf ~/ns-allinone-3.39/ns-3.39/scratch/scratch/

# Keep only the main scratch/ai_out but clean its contents
rm -rf scratch/ai_out
mkdir -p scratch/ai_out

echo "✅ Cleanup complete!"
echo "✅ All files will now be created in: scratch/ai_out/"
