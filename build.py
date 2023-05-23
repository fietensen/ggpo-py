import os, shutil
os.system("cd build/ && cmake --build . --config Debug")

shutil.copyfile("build/Debug/ggpo_py.cp311-win_amd64.pyd", "ggpo_py/ggpo_py.cp311-win_amd64.pyd")