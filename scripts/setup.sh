# On most systems with Python 3
python3 -m venv .venv 

source .venv/bin/activate

pip install -r pip_requirements.txt

pip freeze

deactivate
