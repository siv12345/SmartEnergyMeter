#!/bin/bash
source /opt/conda/etc/profile.d/conda.sh
# conda activate base
conda activate

cd /home/jupyter/smartenergymeter

papermill /home/jupyter/smartenergymeter/Geyser_Function_Influx.ipynb /home/jupyter/smartenergymeter/runs/geyser.ipynb

conda deactivate
