#!/bin/bash

# Activate the virtual environment
source ~/.bashrc
pyenv activate vara

# Set Python path
export PYTHONPATH=~/vara-feature/bindings/python

# Execute the experiment
python batch_executor.py --index $1  # Use the passed Slurm array ID