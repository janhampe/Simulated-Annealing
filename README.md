# Neal
Neal is a simple global placer using a simulated annealing.
Currently there is no way to write the final placement to a file, but it does produce pgm files and prints out the cost.

## Build
Requires libfmt (https://github.com/fmtlib/fmt) to be installed as it is a dependency of lorina.

```
mkdir build
cd build 
cmake ..
make
./neal
```

## Usage
Use neal with

```
./neal -g <genlib file> -v <verilog file> --cx <chip width> --cy <chip height> --it <initial temperature> \
--ft <final temperature> --iwx <initial window x> --fwx <final window x> --iwy <initial window y> \
--fwy <final window y> -s <annealing steps> --ws <warmup steps> --ts <tuning steps> --imps <initial moves per step> \
--fmps <final moves per step> --ld <log directory path> --lf <log file prefix> --li <log interval>
```

genlib file: Path to genlib file defining the gates that were used to create the verilog file

verilog file: Path to verilog file describing the input

chip width: Size of chip in x-dimension

chip height: Size of chip in y-dimension

temperature: Chance that a new configuration will be accepted, even though it is worse than the current configuration. The maximum value is 1'000'000'000'000, which results in a 100% acceptance rate. A temperature of 0 results in a 0% chance a worse configuration is accepted.

window: Maximum distance per dimension a block can be shifted in one move

annealing steps: Number of iterations of the annealing algorithm

warmup: Because saving the best configuration and logging take time and it is unlikely that the best configuration will be found right at the start, one can decide to decrease the runtime by performing the first annealing steps as warm-up steps. Usually, every time a configuration is found that is better than any found so far, it is explicitly saved to deicated storage. In the beginning this can happen a lot because all configurations found so far were very bad.

tuning: During tuning the algorithm continues with the final parameters from the annealing process, but only accepts cost improvements.

moves per step: How many individual blocks are moved in each annealing step.

log directory path: Path to the directory where the log files should be placed. Path must end with "/" and the directory must already exist.

log file prefix: Log files are named \<log file prefix>\_\<current step>.pgm

log interval: Number of annealing or tuning steps between each log.

Temperature, window sizes and moves per step decrease linearly from initial to final value over the course of the annealing process.

Neal will maintain a 1 unit separation between each block, as well as between the blocks and edges of the chip.

### Usage Example

To place the gates in arbiter.v, which was mapped from the EPFL Combinational Benchmark Suite's best result by size arbiter_size_2024.blif with the provided mcnc_gain.genlib, run

```
./neal -g ../input/mcnc_gain.genlib -v ../input/arbiter.v --cx 250 --cy 180 --it 5000000000 \
 --ft 50 --iwx 30 --fwx 1 --iwy 35 --fwy 1 -s 1000000 --ws 200000 --ts 50000 --imps 50 \
 --fmps 1 --ld "log2/" --lf "test" --li 25000
```

### Notes

## Tests
Neal includes a suite of unit tests. They build into the target "test".

## Citations
arbiter.v is the result of mapping https://github.com/lsils/benchmarks/blob/master/best_results/size/arbiter_size_2024.blif with https://github.com/berkeley-abc/abc using the map and write_verilog commands.

mcnc_gain.genlib is a slightly modified version of https://github.com/berkeley-abc/mvsis/blob/7e225952c1d32f63905bfd52a0b92dc1e3ab39e2/examples/mcnc_gain.genlib

For reading verilog and genlib files a modified version of https://github.com/hriener/lorina is used.

Command-line arguments are parsed with https://github.com/jarro2783/cxxopts

The random number generator is based on https://prng.di.unimi.it/xoshiro256plusplus.c
