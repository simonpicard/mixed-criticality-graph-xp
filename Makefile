GENERATOR_SRC = ./tasksetgen
GENERATOR_EXP = $(GENERATOR_SRC)/experiment.py

EXPLORER_SRC = ./mcsexplorer
EXPLORER_BUILD = $(EXPLORER_SRC)/build-$(shell uname -m)
EXPLORER_MAKEFILE = $(EXPLORER_BUILD)/Makefile

VENV = $(GENERATOR_SRC)/.venv-$(shell uname -m)
PYTHON = python3
VENV_PYTHON = $(VENV)/bin/$(PYTHON)

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

xp-statespace-small: generate-xp-tasks-small
	$(EXPLORER_BUILD)/evaluation_mcs antichain xp_tasks_small_def.txt xp_tasks_small_sim.csv
	export MCS_HEADER_FILE=xp_tasks_small_header.csv; \
	export MCS_SIMULATION_FILE=xp_tasks_small_sim.csv; \
	$(VENV)/bin/jupyter notebook --port 8888 --ip 0.0.0.0 --no-browser --allow-root --notebook-dir=. --NotebookApp.token='' --NotebookApp.password=''

all: install-all xp-statespace-small

generate-xp-utilisation: $(VENV)
	$(VENV_PYTHON) $(GENERATOR_EXP) \
		-t utilisation \
		-o xp_utilisation_def.txt \
		-c xp_utilisation_header.csv \
		-phi 0.5 \
		-rhi 2.5 \
		-ta 3 \
		-u 0.6 \
		-U 1 \
		-us 0.01 \
		-ss 100 \
		-max_t 100 \
		-max_c_lo 40

generate-xp-tasks-small: $(VENV)
	$(VENV_PYTHON) $(GENERATOR_EXP) \
		-t n_tasks \
		-o xp_tasks_small_def.txt \
		-c xp_tasks_small_header.csv \
		-phi 0.5 \
		-rhi 2.5 \
		-tas 2 3 \
		-s 50 \
		-max_t 20 \
		-max_c_lo 10
