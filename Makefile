VENV = ./src/py/.venv-$(shell uname -m)
CPP_BUILD = ./src/cpp/build-$(shell uname -m)

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

install-all:
	make install-py
	make install-cpp

xp-statespace-small:
	$(VENV)/bin/python src/py/experiment.py -t statespace -o statespace_def.txt -c statespace_header.csv -phi 0.5 -rhi 2.5 -n 2 3 -N 50 -max_t 20
	./src/cpp/build/evaluation_mcs antichain statespace_def.txt statespace_res.csv
	$(VENV)/bin/jupyter notebook src/py/notebooks/plot_statespace.ipynb

all:
	make install-all
	make xp-statespace-small