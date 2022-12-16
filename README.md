# fraig
A tool that simplifies circuit

## How to run
* To install
```bash=
# for linux
make linux
make
# for mac
make mac
make
```
* To launch the command line tool
```bash=
./fraig [-File <dofile>]
```
* Command list
```
CIRRead:      read in a circuit and construct the netlist
CIRPrint:     print circuit
CIRGate:      report a gate
CIRWrite:     write the netlist to an ASCII AIG file (.aag)
CIRSWeep:     remove unused gates
CIROPTimize:  perform trivial optimizations
CIRSTRash:    perform structural hash on the circuit netlist
CIRSIMulate:  perform Boolean logic simulation on the circuit
CIRFraig:     perform FRAIG operation on the circuit
```
* To check the usage
```bash=
# when executing fraig
> <command> --help
```
