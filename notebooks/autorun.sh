#!/bin/bash
source /opt/conda/etc/profile.d/conda.sh
# conda activate base
conda activate

cd /home/jupyter/smartenergymeter

papermill /home/jupyter/smartenergymeter/testnotebooks/FireStore.ipynb /home/jupyter/smartenergymeter/testrun.ipynb

conda deactivate
