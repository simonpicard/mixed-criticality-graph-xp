THIS_MAKEFILE := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
ROOT_DIR := $(patsubst %/,%,$(dir $(abspath $(THIS_MAKEFILE))))

GENERATOR_SRC = ./tasksetgen
GENERATOR_EXP = $(GENERATOR_SRC)/experiment.py

EXPLORER_SRC = ./mcsexplorer
EXPLORER_BUILD = $(EXPLORER_SRC)/build-$(shell uname -m)
EXPLORER_MAKEFILE = $(EXPLORER_BUILD)/Makefile

VENV = $(ROOT_DIR)/.venv-$(shell uname -m)
PYTHON = python3
VENV_PYTHON = $(VENV)/bin/$(PYTHON)
JUPYTER = $(VENV)/bin/jupyter
NB_INTERACTIVE = $(JUPYTER) notebook --port 8888 --ip 0.0.0.0 --no-browser --allow-root --notebook-dir=. --NotebookApp.token='' --NotebookApp.password=''
NB_GENERATE_HTML = $(JUPYTER) nbconvert --execute --to html notebooks/*.ipynb
NB_CLEAR = $(JUPYTER) nbconvert --ClearOutputPreprocessor.enabled=True --inplace notebooks/*.ipynb

OUTPUT_DIR = ./outputs
DT := $(shell date +%Y%m%d_%H%M%S)

default: all

$(VENV):
	$(ROOT_DIR)/install-venv.sh $(VENV)

explicit_venv: $(VENV)

$(EXPLORER_BUILD):
	mkdir -p $(EXPLORER_BUILD)

$(EXPLORER_MAKEFILE): $(EXPLORER_BUILD)
	cmake -DCMAKE_BUILD_TYPE=Release -S $(EXPLORER_SRC) -B $(EXPLORER_BUILD)

install-explorer: $(EXPLORER_BUILD) $(EXPLORER_MAKEFILE)
	$(MAKE) -C $(EXPLORER_BUILD) -j$(nproc)

install-py: $(VENV)
install-py-macos:
	$(ROOT_DIR)/install-venv-macos.sh $(VENV)


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

xp-statespace-ntasks-small: generate-set-ntasks-small install-explorer
	$(EXPLORER_BUILD)/evaluation_mcs antichain $(OUTPUT_DIR)/$(DT)_ntasks_small_def.txt $(OUTPUT_DIR)/$(DT)_ntasks_small_statespace_explo.csv
	export MCS_HEADER_PATH=../$(OUTPUT_DIR)/$(DT)_ntasks_small_header.csv; \
	export MCS_SIMULATION_PATH=../$(OUTPUT_DIR)/$(DT)_ntasks_small_statespace_explo.csv; \
	$(NB_GENERATE_HTML); \
	$(NB_INTERACTIVE)

generate-set-statespace-tasks-period-utilisation: $(VENV)
	$(VENV_PYTHON) $(GENERATOR_EXP) \
	-t utilisation \
	-o $(OUTPUT_DIR)/$(DT)_statespace_utilisation_def.txt \
	-c $(OUTPUT_DIR)/$(DT)_statespace_utilisation_header.csv \
	-phi 0.5 \
	-ta 3 \
	-u 25 \
	-U 100 \
	-us 5 \
	-ss 100 \
	-min_t 5 \
	-max_t 50

generate-set-statespace-tasks-period-utilisation5: $(VENV)
	$(VENV_PYTHON) $(GENERATOR_EXP) \
	--type utilisation \
	--task_sets_output $(OUTPUT_DIR)/$(DT)_statespace_utilisation_def.txt \
	--header_output $(OUTPUT_DIR)/$(DT)_statespace_utilisation_header.csv \
	--probability_of_HI 0.5 \
	--task_amount 5 \
	--from_utilisation 50 \
	--to_utilisation 100 \
	--utilisation_step 5 \
	--sets_per_step 1000 \
	--minimum_period 5 \
	--maximum_period 20

xp-statespace-utilisation: generate-set-statespace-tasks-period-utilisation install-explorer
	$(EXPLORER_BUILD)/evaluation_mcs antichain $(OUTPUT_DIR)/$(DT)_statespace_utilisation_def.txt $(OUTPUT_DIR)/$(DT)_utilisation_statespace_explo.csv

xp-statespace-utilisation5: generate-set-statespace-tasks-period-utilisation5 install-explorer
	$(VENV_PYTHON) $(ROOT_DIR)/parallelruns/parallel_simulator.py \
		--xp-type=antichain \
		--build-dir=$(EXPLORER_BUILD) \
		--input-file=$(OUTPUT_DIR)/$(DT)_statespace_utilisation_def.txt \
		--output-prefix=$(OUTPUT_DIR)/$(DT)_utilisation_statespace_explo

generate-set-ntasks: $(VENV)
	$(VENV_PYTHON) $(GENERATOR_EXP) \
	-t n_tasks \
	-o $(OUTPUT_DIR)/$(DT)_ntasks_def.txt \
	-c $(OUTPUT_DIR)/$(DT)_ntasks_header.csv \
	-phi 0.5 \
	-rhi 3 \
	-tas 2 3 4 5 \
	-s 1000 \
	-min_t 6 \
	-max_t 12 \
	-max_c_lo 3

xp-statespace-ntasks: generate-set-ntasks install-explorer
	$(EXPLORER_BUILD)/evaluation_mcs antichain $(OUTPUT_DIR)/$(DT)_ntasks_def.txt $(OUTPUT_DIR)/$(DT)_ntasks_statespace_explo.csv

notebook: $(VENV)
	$(NB_INTERACTIVE)

clear-notebook: $(VENV)
	$(NB_CLEAR)

generate-set-oracles: $(VENV)
	$(VENV_PYTHON) $(GENERATOR_EXP) \
	--type utilisation \
	--task_sets_output $(OUTPUT_DIR)/$(DT)_oracles_def.txt \
	--header_output $(OUTPUT_DIR)/$(DT)_oracles_header.csv \
	--probability_of_HI 0.5 \
	--task_amount 5 \
	--from_utilisation 80 \
	--to_utilisation 100 \
	--utilisation_step 1 \
	--sets_per_step 100 \
	--minimum_period 5 \
	--maximum_period 20

generate-set-oracles-hta: $(VENV)
	$(VENV_PYTHON) $(GENERATOR_EXP) \
	--type utilisation \
	--task_sets_output $(OUTPUT_DIR)/$(DT)_oracles_def.txt \
	--header_output $(OUTPUT_DIR)/$(DT)_oracles_header.csv \
	--probability_of_HI 0.5 \
	--task_amount 7 \
	--from_utilisation 80 \
	--to_utilisation 80 \
	--utilisation_step 1 \
	--sets_per_step 1 \
	--minimum_period 5 \
	--maximum_period 20

xp-oracles: install-all generate-set-oracles install-explorer
	$(EXPLORER_BUILD)/evaluation_mcs oracle $(OUTPUT_DIR)/$(DT)_oracles_def.txt $(OUTPUT_DIR)/$(DT)_oracles_explo.csv

generate-set-scheduling1: $(VENV)
	$(VENV_PYTHON) $(GENERATOR_EXP) \
	-t utilisation \
	-o $(OUTPUT_DIR)/$(DT)_scheduling1_def.txt \
	-c $(OUTPUT_DIR)/$(DT)_scheduling1_header.csv \
	-phi 0.5 \
	-ta 5 \
	-u 50 \
	-U 100 \
	-us 5 \
	-ss 1000 \
	-min_t 5 \
	-max_t 20

xp-scheduling1: install-all generate-set-scheduling1
	$(VENV_PYTHON) $(ROOT_DIR)/parallelruns/parallel_simulator.py \
		--xp-type=scheduling \
		--build-dir=$(EXPLORER_BUILD) \
		--input-file=$(OUTPUT_DIR)/$(DT)_scheduling1_def.txt \
		--output-prefix=$(OUTPUT_DIR)/$(DT)_scheduling1_explo

generate-set-scheduling2: $(VENV)
	$(VENV_PYTHON) $(GENERATOR_EXP) \
	-t utilisation \
	-o $(OUTPUT_DIR)/$(DT)_scheduling2_def.txt \
	-c $(OUTPUT_DIR)/$(DT)_scheduling2_header.csv \
	-phi 0.5 \
	-ta 5 \
	-u 80 \
	-U 100 \
	-us 2 \
	-ss 1000 \
	-min_t 5 \
	-max_t 20

xp-scheduling2: install-all generate-set-scheduling2
	$(VENV_PYTHON) $(ROOT_DIR)/parallelruns/parallel_simulator.py \
		--xp-type=scheduling \
		--build-dir=$(EXPLORER_BUILD) \
		--input-file=$(OUTPUT_DIR)/$(DT)_scheduling2_def.txt \
		--output-prefix=$(OUTPUT_DIR)/$(DT)_scheduling2_explo

xp-scheduling: xp-scheduling1 xp-scheduling2 install-explorer

xp-oracles-hta: install-all generate-set-oracles-hta
	$(EXPLORER_BUILD)/evaluation_mcs oracle $(OUTPUT_DIR)/$(DT)_oracles_def.txt $(OUTPUT_DIR)/$(DT)_oracles_explo.csv

xp-oracles-split: install-all generate-set-oracles
	$(VENV_PYTHON) $(ROOT_DIR)/parallelruns/parallel_simulator.py \
		--xp-type=oracle \
		--build-dir=$(EXPLORER_BUILD) \
		--input-file=$(OUTPUT_DIR)/$(DT)_oracles_def.txt \
		--output-prefix=$(OUTPUT_DIR)/$(DT)_oracles_explo

xp-statespace-antichain-oracle: generate-set-oracles install-explorer
	mv $(OUTPUT_DIR)/$(DT)_oracles_def.txt $(OUTPUT_DIR)/$(DT)_ac_hi_od_hi_idle_def.txt
	mv $(OUTPUT_DIR)/$(DT)_oracles_header.csv $(OUTPUT_DIR)/$(DT)_ac_hi_od_hi_idle_header.csv
	$(VENV_PYTHON) $(ROOT_DIR)/parallelruns/parallel_simulator.py \
		--xp-type=antichain_oracle \
		--build-dir=$(EXPLORER_BUILD) \
		--input-file=$(OUTPUT_DIR)/$(DT)_ac_hi_od_hi_idle_def.txt \
		--output-prefix=$(OUTPUT_DIR)/$(DT)_ac_hi_od_hi_idle_explo

generate-set-statespace-n-tasks: $(VENV)
	$(VENV_PYTHON) $(GENERATOR_EXP) \
	-t modular \
	-o $(OUTPUT_DIR)/$(DT)-statespace-n-tasks.txt \
	-c $(OUTPUT_DIR)/$(DT)-statespace-n-tasks.csv \
	--probability_of_HI 0.5 \
	--minimum_period 5 \
	--utilisation_list 50 \
	--max_period_list 30 \
	--n_tasks_start 3 \
	--n_tasks_stop 11 \
	--n_tasks_step 1 \
	--sets_per_config 20 \
	--seed 1

generate-set-statespace-period-max: $(VENV)
	$(VENV_PYTHON) $(GENERATOR_EXP) \
	-t modular \
	-o $(OUTPUT_DIR)/$(DT)-statespace-period-max.txt \
	-c $(OUTPUT_DIR)/$(DT)-statespace-period-max.csv \
	--probability_of_HI 0.5 \
	--minimum_period 5 \
	--utilisation_list 50 \
	--n_tasks_list 5 \
	--max_period_start 20 \
	--max_period_stop 201 \
	--max_period_step 20 \
	--sets_per_config 20 \
	--seed 2

generate-set-statespace-utilisation: $(VENV)
	$(VENV_PYTHON) $(GENERATOR_EXP) \
	-t modular \
	-o $(OUTPUT_DIR)/$(DT)-statespace-utilisation.txt \
	-c $(OUTPUT_DIR)/$(DT)-statespace-utilisation.csv \
	--probability_of_HI 0.5 \
	--minimum_period 5 \
	--max_period_list 30 \
	--n_tasks_list 5 \
	--utilisation_start 30 \
	--utilisation_stop 101 \
	--utilisation_step 10 \
	--sets_per_config 20 \
	--seed 3


generate-set-statespace-rtss: generate-set-statespace-n-tasks generate-set-statespace-period-max generate-set-statespace-utilisation
