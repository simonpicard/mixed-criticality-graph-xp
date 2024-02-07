VENV := ./src/py/.venv-$(shell uname -m)
CPP_BUILD := ./src/cpp/build-$(shell uname -m)
DT := $(shell date +%Y%m%d_%H%M%S)

default: all

$(VENV):
	python3 -m venv $(VENV)
	$(VENV)/bin/pip install --upgrade pip

$(CPP_BUILD):
	mkdir -p $(CPP_BUILD)
	cmake -DCMAKE_BUILD_TYPE=Release -S ./src/cpp -B $(CPP_BUILD)

install-py: $(VENV)
	$(VENV)/bin/pip install -r ./src/py/requirements.txt

install-cpp: $(CPP_BUILD)
	$(MAKE) -C $(CPP_BUILD) -j$(nproc) VERBOSE=1

install-all: install-py install-cpp

xp-tasks-small: generate-xp-tasks-small
	$(CPP_BUILD)/evaluation_mcs antichain $(DT)_xp_tasks_small_def.txt $(DT)_xp_tasks_small_sim.csv
	export MCS_HEADER_FILE=$(DT)_xp_tasks_small_header.csv; \
	export MCS_SIMULATION_FILE=$(DT)_xp_tasks_small_sim.csv; \
	$(VENV)/bin/jupyter notebook --port 8888 --ip 0.0.0.0 --no-browser --allow-root --notebook-dir=. --NotebookApp.token='' --NotebookApp.password=''

all: install-all xp-tasks-small

generate-xp-utilisation:
	$(VENV)/bin/python src/py/experiment.py -t utilisation -o $(DT)_xp_utilisation_def.txt -c $(DT)_xp_utilisation_header.csv -phi 0.5 -rhi 3 -ta 3 -u 0.6 -U 0.99 -us 0.01 -ss 100 -max_t 50 -max_c_lo 20

generate-xp-ntasks-ac:
	$(VENV)/bin/python src/py/experiment.py -t n_tasks -o $(DT)_xp_ntasks_ac_def.txt -c $(DT)_xp_ntasks_ac_header.csv -phi 0.5 -rhi 3 -tas 2 3 4 5 -s 1000 -max_t 12 -max_c_lo 3

generate-xp-tasks-small:
	$(VENV)/bin/python src/py/experiment.py -t n_tasks -o $(DT)_xp_tasks_small_def.txt -c $(DT)_xp_tasks_small_header.csv -phi 0.5 -rhi 2.5 -tas 2 3 -s 50 -max_t 20 -max_c_lo 10



xp-ntasks-ac: generate-xp-ntasks-ac
	$(CPP_BUILD)/evaluation_mcs antichain $(DT)_xp_ntasks_ac_def.txt $(DT)_xp_ntasks_ac_sim.csv
	export MCS_HEADER_FILE=$(DT)_xp_ntasks_ac_header.csv; \
	export MCS_SIMULATION_FILE=$(DT)_xp_ntasks_ac_sim.csv; \
	$(VENV)/bin/jupyter notebook --port 8888 --ip 0.0.0.0 --no-browser --allow-root --notebook-dir=. --NotebookApp.token='' --NotebookApp.password=''

xp-utilisation: generate-xp-utilisation
	$(CPP_BUILD)/evaluation_mcs antichain $(DT)_xp_utilisation_def.txt $(DT)_xp_utilisation_sim.csv


notebook:
	$(VENV)/bin/jupyter notebook --port 8888 --ip 0.0.0.0 --no-browser --allow-root --notebook-dir=. --NotebookApp.token='' --NotebookApp.password=''
