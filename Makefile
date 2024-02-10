GENERATOR_SRC = ./tasksetgen
GENERATOR_EXP = $(GENERATOR_SRC)/experiment.py

EXPLORER_SRC = ./mcsexplorer
EXPLORER_BUILD = $(EXPLORER_SRC)/build-$(shell uname -m)
EXPLORER_MAKEFILE = $(EXPLORER_BUILD)/Makefile

VENV = $(GENERATOR_SRC)/.venv-$(shell uname -m)
PYTHON = python3
VENV_PYTHON = $(VENV)/bin/$(PYTHON)


OUTPUT_DIR = ./outputs
DT := $(shell date +%Y%m%d_%H%M%S)

default: all

$(VENV):
	$(PYTHON) -m venv $(VENV)
	$(VENV)/bin/pip install --upgrade pip
	$(VENV)/bin/pip install -r $(GENERATOR_SRC)/requirements.txt

$(EXPLORER_BUILD):
	mkdir -p $(EXPLORER_BUILD)

$(EXPLORER_MAKEFILE): $(EXPLORER_BUILD)
	cmake -DCMAKE_BUILD_TYPE=Release -S $(EXPLORER_SRC) -B $(EXPLORER_BUILD)

install-explorer: $(EXPLORER_BUILD) $(EXPLORER_MAKEFILE)
	$(MAKE) -C $(EXPLORER_BUILD) -j$(nproc) VERBOSE=1

install-all: $(VENV) install-explorer

all: install-all xp-statespace-ntasks-small

generate-set-ntasks-small: $(VENV)
	$(VENV_PYTHON) $(GENERATOR_EXP) \
	-t n_tasks \
	-o $(OUTPUT_DIR)/$(DT)_ntasks_small_def.txt \
	-c $(OUTPUT_DIR)/$(DT)_ntasks_small_header.csv \
	-phi 0.5 \
	-rhi 2.5 \
	-tas 2 3 \
	-s 50 \
	-min_t 5 \
	-max_t 20 \
	-max_c_lo 10

xp-statespace-ntasks-small: generate-set-ntasks-small
	$(EXPLORER_BUILD)/evaluation_mcs antichain $(OUTPUT_DIR)/$(DT)_ntasks_small_def.txt $(OUTPUT_DIR)/$(DT)_ntasks_small_statespace_explo.csv
	export MCS_HEADER_PATH=$(OUTPUT_DIR)/$(DT)_ntasks_small_header.csv; \
	export MCS_SIMULATION_PATH=$(OUTPUT_DIR)/$(DT)_ntasks_small_statespace_explo.csv; \
	$(VENV)/bin/jupyter notebook --port 8888 --ip 0.0.0.0 --no-browser --allow-root --notebook-dir=. --NotebookApp.token='' --NotebookApp.password=''

generate-set-utilisation: $(VENV)
	$(VENV_PYTHON) $(GENERATOR_EXP) \
	-t utilisation \
	-o $(OUTPUT_DIR)/$(DT)_utilisation_def.txt \
	-c $(OUTPUT_DIR)/$(DT)_utilisation_header.csv \
	-phi 0.5 \
	-ta 3 \
	-u 0.6 \
	-U 1 \
	-us 0.01 \
	-ss 100 \
	-min_t 5 \
	-max_t 50

xp-statespace-utilisation: generate-set-utilisation
	$(EXPLORER_BUILD)/evaluation_mcs antichain $(OUTPUT_DIR)/$(DT)_utilisation_def.txt $(OUTPUT_DIR)/$(DT)_utilisation_statespace_explo.csv

generate-set-ntasks: $(VENV)
	$(VENV_PYTHON) $(GENERATOR_EXP) \
	-t n_tasks \
	-o $(OUTPUT_DIR)/$(DT)_ntasks_def.txt \
	-c $(OUTPUT_DIR)/$(DT)_ntasks_small.csv \
	-phi 0.5 \
	-rhi 3 \
	-tas 2 3 4 5 \
	-s 1000 \
	-min_t 6 \
	-max_t 12 \
	-max_c_lo 3

xp-statespace-ntasks: generate-set-ntasks
	$(EXPLORER_BUILD)/evaluation_mcs antichain $(OUTPUT_DIR)/$(DT)_ntasks_def.txt $(OUTPUT_DIR)/$(DT)_ntasks_small_statespace_explo.csv

notebook: $(VENV)
	$(VENV)/bin/jupyter notebook --port 8888 --ip 0.0.0.0 --no-browser --allow-root --notebook-dir=. --NotebookApp.token='' --NotebookApp.password=''
