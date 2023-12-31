install-py:
	python3.11 -m venv ./src/py/.venv
	./src/py/.venv/bin/pip install --upgrade pip
	./src/py/.venv/bin/pip install -r ./src/py/requirements.txt