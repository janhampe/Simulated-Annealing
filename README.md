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

To activate or deactivate error, info and debug messages, comment the "add_compile_definitions" lines in CMakeLists.txt. Everything prints to std::cerr. Error and info are usually left on and debug off. Be warned, debug produces a very large amount of messages.

## Usage
Use neal with

```
./neal -g <genlib file> -v <verilog file> --cf <cost function> --cx <chip width> --cy <chip height> --it <initial temperature> \
--ft <final temperature> --iwx <initial window x> --fwx <final window x> --iwy <initial window y> \
--fwy <final window y> -s <annealing steps> --ws <warmup steps> --ts <tuning steps> --imps <initial moves per step> \
--fmps <final moves per step> --ld <log directory path> --lf <log file prefix> --li <log interval> \
-pi <pins>
```

**genlib file**: Path to genlib file defining the gates that were used to create the verilog file

**verilog file**: Path to verilog file describing the input

**cost function**: Function with which to compute the wire costs. Options are hwpl (half-perimeter wire length), mcl (monotone chain length) and star (star length).

**chip width**: Size of chip in x-dimension

**chip height**: Size of chip in y-dimension

**temperature**: Chance that a new configuration will be accepted, even though it is worse than the current configuration. The maximum value is 1'000'000'000'000, which results in a 100% acceptance rate. A temperature of 0 results in a 0% chance a worse configuration is accepted.

**window**: Maximum distance per dimension a block can be shifted in one move

**annealing steps**: Number of iterations of the annealing algorithm

**warmup**: Because saving the best configuration and logging take time and it is unlikely that the best configuration will be found right at the start, one can decide to decrease the runtime by performing the first annealing steps as warm-up steps. Usually, every time a configuration is found that is better than any found so far, it is explicitly saved to deicated storage. In the beginning this can happen a lot because all configurations found so far were very bad.

**tuning**: During tuning the algorithm continues with the final parameters from the annealing process, but only accepts cost improvements. This option specifies the number of tuning steps.

**moves per step**: How many individual blocks are moved in each annealing step.

**log directory path**: Path to the directory where the log files should be placed. Path must end with "/" and the directory must already exist.

**log file prefix**: Log files are named \<log file prefix>\_\<current step>.pgm

**log interval**: Number of annealing or tuning steps between each log.

**pins**: Experimental option to add input and output pins. See section further down.

Temperature, window sizes and moves per step decrease linearly from initial to final value over the course of the annealing process.


### Usage Example

To place the gates in arbiter.v, which was mapped from the EPFL Combinational Benchmark Suite's best result by size arbiter_size_2024.blif with the provided mcnc_gain.genlib, run

```
./neal -g ../input/mcnc_gain.genlib -v ../input/arbiter.v --cx 250 --cy 180 --it 5000000000 \
 --ft 50 --iwx 30 --fwx 1 --iwy 35 --fwy 1 -s 1000000 --ws 200000 --ts 50000 --imps 50 \
 --fmps 1 --ld "log/" --lf "test" --li 25000
```

### Notes

Neal will maintain a 1 unit separation between each block, as well as between the blocks and edges of the chip.

Neal rounds any block with area less than or equal to 1.0, to 1 and assume a square block with sidelength 1. Blocks with area 0 are not supported.

Cost estimation is done with a half-perimeter wire length function by default. Monotone chain length and star length are also available.

Genlib files must specify each input pin explicitly as seen in mcnc_gain.genlib. Even though allowed by the genlib specification, lorina can't deal with defining multiple pins with "*". The output pin must not be specified.

Lorina's verilog reader does not allow special characters like \[ or \] in the names for inputs, outputs or wires. They do occur in blif files, so be careful and remove them if neccessary. See arbiter.v for reference.

### Pins

There is an experimental option to add the circuits input and output pins, specified in the verilog file, to the chip. Neal will try to evenly space the input pins on the left side and the output pins on the right side. A 1 unit separation is enforced and the process will fail if <chip height> < std::max(2 * inputs, 2 * outputs). Once placed the pins will not be moved, so the final result can be very sensitive to the order in which inputs and outputs are specified in the verilog file. When using the star length cost function it is ensured, that if a net contains an input or output pin, this pin will be the "star". Therefore it is recommended to use the star cost function when enabling the pins option.

## Tests
Neal includes a suite of unit tests. They build into the target "test".

## Citations
For reading verilog and genlib files a modified version of https://github.com/hriener/lorina is used.

Command-line arguments are parsed with https://github.com/jarro2783/cxxopts

The random number generator is based on https://prng.di.unimi.it/xoshiro256plusplus.c

arbiter.v is the result of mapping https://github.com/lsils/benchmarks/blob/master/best_results/size/arbiter_size_2024.blif with https://github.com/berkeley-abc/abc using the map and write_verilog commands. The file was modified as described in Notes.

mcnc_gain.genlib is a slightly modified version of https://github.com/berkeley-abc/mvsis/blob/7e225952c1d32f63905bfd52a0b92dc1e3ab39e2/examples/mcnc_gain.genlib

test.genlib is from https://github.com/nrclark/sis/blob/master/vst2blif/test.genlib
