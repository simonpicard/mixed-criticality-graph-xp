#!/usr/bin/env python3
"""
Usage:
    parallel_simulator.py --build-dir=<build_directory> --input-file=<input_file> --output-prefix=<output_prefix>

Options:
    -h --help                           Show this screen.
    --build-dir=<build_directory>       The build directory of the explorer.
    --input-file=<input_file>           The input file (ending with _oracles_def.txt).
    --output-prefix=<output_prefix>     The prefix for the output file.
"""

from docopt import docopt
import subprocess
from multiprocessing import cpu_count


def read_tasksets_count(file_path):
    with open(file_path, 'r') as file:
        first_line = file.readline().strip()
    return int(first_line)


def create_processes(build_dir, input_file, output_prefix, i, start_taskset, num_tasksets):
    processes = []
    output_file = f"{output_prefix}_{i}.csv"
    command = f"{build_dir}/evaluation_mcs oracle {input_file} {output_file} {start_taskset} {num_tasksets}"
    processes.append(subprocess.Popen(command, shell=True))
    print(command)
    return processes


def main(args):
    build_dir = args['--build-dir']
    input_file = args['--input-file']
    output_prefix = args['--output-prefix']

    total_tasksets = read_tasksets_count(input_file)
    cpus = cpu_count()

    tasksets_per_cpu = total_tasksets // cpus
    remaining_tasksets = total_tasksets % cpus

    start_taskset = 0
    processes = []
    i = 0

    for cpu in range(cpus):
        num_tasksets = tasksets_per_cpu + (1 if cpu < remaining_tasksets else 0)
        if num_tasksets > 0:
            processes.extend(create_processes(build_dir, input_file, output_prefix, i, start_taskset, num_tasksets))
            i += 1
            start_taskset += num_tasksets

    for process in processes:
        process.wait()
    print("Parallel simulation completed.")


if __name__ == '__main__':
    args = docopt(__doc__)
    main(args)
