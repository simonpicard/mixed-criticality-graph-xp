VENV = ./src/py/.venv

default: all

$(VENV):
	python3.11 -m venv $(VENV)
	$(VENV)/bin/pip install --upgrade pip

install-py: $(VENV)
	$(VENV)/bin/pip install -r ./src/py/requirements.txt

make-cpp:
	$(MAKE) -C ./src/cpp/build -j$(nproc) VERBOSE=1

install-cpp:
	mkdir -p ./src/cpp/build
	cmake -DCMAKE_BUILD_TYPE=Release -S ./src/cpp -B ./src/cpp/build
	make make-cpp

install-all:
	make install-py
	make install-cpp

xp-statespace-small:
	make generate-xp-tasks-small
	./src/cpp/build/evaluation_mcs antichain xp_tasks_small_def.txt xp_tasks_small_sim.csv
	MCS_HEADER_FILE=xp_tasks_small_header.csv; \
	MCS_SIMULATION_FILE=xp_tasks_small_sim.csv; \
	$(VENV)/bin/jupyter notebook src/py/notebooks/plot_statespace.ipynb

all:
	make install-all
	make xp-statespace-small

generate-xp-utilisation:
	$(VENV)/bin/python src/py/experiment.py -t utilisation -o xp_utilisation_def.txt -c xp_utilisation_header.csv -phi 0.5 -rhi 2.5 -ta 3 -u 0.6 -U 1 -us 0.01 -ss 100 -max_t 100 -max_c_lo 40

generate-xp-tasks-small:
	$(VENV)/bin/python src/py/experiment.py -t n_tasks -o xp_tasks_small_def.txt -c xp_tasks_small_header.csv -phi 0.5 -rhi 2.5 -tas 2 3 -s 50 -max_t 20 -max_c_lo 10