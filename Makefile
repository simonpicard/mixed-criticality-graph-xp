install-py:
	python3.11 -m venv ./src/py/.venv
	./src/py/.venv/bin/pip install --upgrade pip
	./src/py/.venv/bin/pip install -r ./src/py/requirements.txt

make-cpp:
	$(MAKE) -C ./src/cpp/build

install-cpp:
	mkdir -p ./src/cpp/build
	cmake -S ./src/cpp -B ./src/cpp/build
	make-cpp

install-all:
	make install-py
	make install-cpp

xp-statespace-small:
	./src/py/.venv/bin/python src/py/experiment.py -t statespace -o statespace_def.txt -c statespace_header.csv -phi 0.5 -rhi 2.5 -n 2 3 -N 50 -max_t 20
	./src/cpp/build/mcs antichain statespace_def.txt statespace_res.csv
	./src/py/.venv/bin/jupyter notebook src/py/notebooks/plot_statespace.ipynb

all:
	make install-all
	make xp-statespace-small