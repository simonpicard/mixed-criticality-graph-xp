import os
import pathlib
import subprocess
from typing import Any, Dict, Iterable, List, Optional

import pandas as pd
from benchkit.benchmark import Benchmark, CommandAttachment, PostRunHook, PreRunHook
from benchkit.campaign import CampaignCartesianProduct, Constants
from benchkit.commandwrappers import CommandWrapper
from benchkit.dependencies.packages import PackageDependency
from benchkit.platforms import Platform
from benchkit.sharedlibs import SharedLib
from benchkit.utils.dir import gitmainrootdir
from benchkit.utils.types import CpuOrder, PathType


class MCSBench(Benchmark):
    """Benchmark object for Mixed Criticality Systems benchmark."""

    def __init__(
        self,
        timeout_seconds: int,
        command_wrappers: Iterable[CommandWrapper] = (),
        command_attachments: Iterable[CommandAttachment] = (),
        shared_libs: Iterable[SharedLib] = (),
        pre_run_hooks: Iterable[PreRunHook] = (),
        post_run_hooks: Iterable[PostRunHook] = (),
        platform: Platform | None = None,
        build_dir: PathType | None = None,
    ) -> None:
        super().__init__(
            command_wrappers=command_wrappers,
            command_attachments=command_attachments,
            shared_libs=shared_libs,
            pre_run_hooks=pre_run_hooks,
            post_run_hooks=post_run_hooks,
        )
        if platform is not None:
            self.platform = platform  # TODO Warning! overriding upper class platform

        self._timeout_seconds = timeout_seconds
        self._bench_src_path = gitmainrootdir()

        self._inside_docker = (
            ("1" == os.environ["INSIDE_DOCKER"])
            if "INSIDE_DOCKER" in os.environ
            else False
        )
        self._docker_cmd_prefix = (
            [] if self._inside_docker else ["docker", "exec", "mcgraphxp"]
        )

    @property
    def bench_src_path(self) -> pathlib.Path:
        return self._bench_src_path

    @staticmethod
    def get_build_var_names() -> List[str]:
        return []

    @staticmethod
    def get_run_var_names() -> List[str]:
        return [
            "taskset_file",
            "taskset_position",
            "use_idlesim",
            "scheduler",
            "safe_oracles",
            "unsafe_oracles",
            "periodic_tweak",
        ]

    @staticmethod
    def get_tilt_var_names() -> List[str]:
        return []

    def dependencies(self) -> List[PackageDependency]:
        return super().dependencies() + []

    def prebuild_bench(self, **_kwargs) -> None:
        build_dir = self.bench_src_path

        # must_debug = self.must_debug()
        # cmake_build_type = "Debug" if must_debug else "Release"  # TODO use it

        self.platform.comm.shell(
            command=self._docker_cmd_prefix + ["make", "install-explorer"],
            # command="make install-explorer",
            # command=["make", "test"],
            current_dir=str(build_dir),
            output_is_log=True,
            # environment={
            #     "INTERACTIVE": "0",
            # "INSIDE_DOCKER": "1" if self._inside_docker else "0",
            # },
            # shell=True,
        )

    def build_bench(
        self,
        **kwargs,
    ) -> None:
        pass

    def clean_bench(self) -> None:
        pass

    def make_taskset(self, experiment: str) -> None:
        make_target = f"generate-set-{experiment}"
        run_command = self._docker_cmd_prefix + ["make", make_target]
        self.platform.comm.shell(
            command=run_command,
            current_dir=str(self.bench_src_path),
            output_is_log=True,
        )

    def single_run(  # pylint: disable=arguments-differ
        self,
        benchmark_duration_seconds: int,
        taskset_file: PathType,
        taskset_position: int,
        use_idlesim: bool,
        scheduler: str,
        safe_oracles: List[str],
        unsafe_oracles: List[str],
        periodic_tweak: bool = False,
        **kwargs,
    ) -> str:
        environment = self._preload_env(
            benchmark_duration_seconds=benchmark_duration_seconds,
            taskset_file=taskset_file,
            taskset_position=taskset_position,
            use_idlesim=use_idlesim,
            scheduler=scheduler,
            safe_oracles=safe_oracles,
            unsafe_oracles=unsafe_oracles,
            periodic_tweak=periodic_tweak,
            **kwargs,
        )

        if environment is None:
            environment = {}
        environment["INTERACTIVE"] = "0"

        if scheduler not in ["edfvd", "lwlf"]:
            raise ValueError(f"Unknown scheduler: {scheduler}")
        for safe_oracle in safe_oracles:
            if safe_oracle not in ["hi-idle-point"]:
                raise ValueError(f"Unknown safe_oracle: {safe_oracle}")
        for unsafe_oracle in unsafe_oracles:
            if unsafe_oracle not in [
                "negative-laxity",
                "negative-worst-laxity",
                "over-demand",
                "hi-over-demand",
            ]:
                raise ValueError(f"Unknown unsafe_oracle: {unsafe_oracle}")

        cmd_options = [
            "--taskset-file",
            str(taskset_file),
            "--taskset-position",
            str(taskset_position),
            "--scheduler",
            scheduler,
        ]
        if use_idlesim:
            cmd_options.append("--use-idlesim")
        if safe_oracles:
            cmd_options += ["--safe-oracles", ",".join(safe_oracles)]
        if unsafe_oracles:
            cmd_options += ["--unsafe-oracles", ",".join(unsafe_oracles)]
        if periodic_tweak:
            cmd_options.append("--periodic-tweak")

        mcs_build_dir = f"build-{self.platform.architecture}-docker"
        run_command = (
            self._docker_cmd_prefix
            + [f"./mcsexplorer/{mcs_build_dir}/one_system"]
            + cmd_options
        )

        wrapped_run_command, wrapped_environment = self._wrap_command(
            run_command=run_command,
            environment=environment,
            **kwargs,
        )

        try:
            output = self.run_bench_command(
                run_command=run_command,
                wrapped_run_command=wrapped_run_command,
                current_dir=self.bench_src_path,
                environment=environment,
                wrapped_environment=wrapped_environment,
                print_output=False,
                timeout=self._timeout_seconds,
            )
        except subprocess.TimeoutExpired:
            output = "TIMEOUT"

        return output

    def parse_output_to_results(  # pylint: disable=arguments-differ
        self,
        command_output: str,
        run_variables: Dict[str, Any],
        **_kwargs,
    ) -> Dict[str, Any]:
        csv_data_path = "../" + run_variables["taskset_file"].split(".txt")[0] + ".csv"
        taskset_position = run_variables["taskset_position"]

        df = pd.read_csv(csv_data_path)
        df["ts_id"] = df["ts_id"].apply(int)
        df = df.set_index("ts_id")
        generation_data = df.loc[taskset_position].to_dict()
        # max_period = generation_data["max_period"]

        result_dict = generation_data
        result_dict["timeout"] = False

        if command_output.startswith("TIMEOUT"):
            result_dict["timeout"] = True
            return result_dict

        results = command_output.splitlines()[-1]
        results_head = "Results:"
        if not results.startswith(results_head):
            raise ValueError(
                f'Last line of output does not contain results: "{results}"'
            )
        key_values = results[len(results_head) :].strip()
        result_dict |= dict(kv.split("=") for kv in key_values.split(";"))
        return result_dict
