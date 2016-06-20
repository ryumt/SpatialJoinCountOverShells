An implementation of the spatial join count over shells (SJCS) workload
==================

## Description

This repository contains following things. 
* SJCS with CPU optimized STR-tree 
* SJCS with CPU optimized R-tree [for the baseline]
* uniformly random particle data generator 
* uniformly random halo data generator 
* particle data reader
* halo data reader

#### SJCS 

This SJCS workload counts the particles for each shell of each halo. 
The following SQL-like statement represents an example of this workload. 
```
SELECT SH1.id, SH1.count, SH2.count, SH3.count FROM  
     (SELECT HALO.id, COUNT(*) FROM HALO, PART  
       WHERE distance(HALO.p, PART.p) < THRESHOLD1  
       GROUP BY HALO.id) AS SH1,  
     (SELECT HALO.id, COUNT(*) FROM HALO, PART  
       WHERE distance(HALO.p, PART.p) >= THRESHOLD1  
         AND distance(HALO.p, PART.p) < THRESHOLD2  
       GROUP BY HALO.id) AS SH2,  
     (SELECT HALO.id, COUNT(*) FROM HALO, PART  
       WHERE distance(HALO.p, PART.p) >= THRESHOLD2  
         AND distance(HALO.p, PART.p) < THRESHOLD3  
       GROUP BY HALO.id) AS SH3  
```

Periodic boundary condition is also applied to this workload. 
This assumes that the whole data space consists of 
repetition of the same cells, a cell is a data unit for simulations. 

#### About Data

This repository does not contain real halo/particle data 
because we cannot open data to the public. 
Instead of the real data, 
we put the random data generators. 
They generate id and x, y and z coordinates of the halo/particle data. 
The domain of each coordinate is [0 ~ 1000] 
because the SJCS program handles the cell whose range is [0 ~ 1000] for each dimension. 


## Requirement

* POSIX compatible OS
* Intel CPU that can handle SSE4.2 instructions
* GCC or LLVM compiler

## Usage

### Prepare the data

``` 
## part[0-2].dat will be generated. Particle data need to be divided to pipeline the search in SJCS.
## Each file contains 100000000 particles. This takes rather long time. 
./uniform_decoded_particle_generator.exe -f part0.dat part1.dat part2.dat -n 100000000

## halo.list will be generated. 
## It contains 1000000 halos. 
./uniform_halo_generator.exe -f halo.list -n 10000000
```

### Take a look at the data

```
## To take a look at the generated data, particle/halo data reader can be used. 

./decoded_particle_reader.exe -f part0.dat part1.dat | more
## {coordinates}
#--- files to read ---
#part0.dat
#part1.dat
#{851.985474 123.192955 73.903595}
#{127.631561 894.931763 904.444397}
#{436.493561 242.660706 127.677734}

./halo_reader.exe -f halo.list | more
## id: mass {coordinates}    ...  mass is not used in the SJCS program. 
#--- files to read ---
#halo.list
#0: 827952005120.000000 {418.681091 810.143127 415.767883}
#1: 29357400064.000000 {719.329224 506.941437 444.654205}
#2: 692285014016.000000 {613.010132 267.978607 21.987623}
#3: 792372969472.000000 {90.480103 153.100082 541.549377}
```

### SJCS 

* astr_rcount.exe: STR-tree version
* rt_rcount.exe: R-tree version

#### Non-pipelined SJCS

```
## The options are corresponding to the execution info.
./astr_rcount.exe --thread 20 --next-setup 14 --particle-files part*.dat --halo-file halo.list \
		  --radius 0.001:5:40  --output-file result

#--execution info--
#  particle files (compressed = false): 
#   |- part0.dat
#   |- part1.dat
#   |- part2.dat
#  halo file = halo.list
#  halo filter file = (null)
#  # of threads in thread pool = 20
#  # of index construction threads = 14
#  # of radiuses = 40
#   |- minimum = 0.001000, maximum = 5.000000
#  max # of backlog indexes = 0
#  ! ignored ! # of first backlog indexes = 1
#  ! ignored ! # of first index construction threads = 1
#
#>>>> halo data reading time: 6995 milli seconds
#  # of halos = 10000000
#  -- with no backlog execution
#>>>> particles reading time: 474 milli seconds
#>>>> index construction time: 6683 milli seconds
#  # of particles = 100000000
#>>>> searching time: 21174 milli seconds
#>>>> index destruction time: 9 milli seconds
#>>>> particles reading time: 466 milli seconds
#>>>> index construction time: 6596 milli seconds
#  # of particles = 100000000
#>>>> searching time: 20652 milli seconds
#>>>> index destruction time: 9 milli seconds
#>>>> particles reading time: 465 milli seconds
#>>>> index construction time: 6410 milli seconds
#  # of particles = 100000000
#>>>> searching time: 21143 milli seconds
#>>>> index destruction time: 9 milli seconds
#>>>> total searching time: 84104 milli seconds

## Result writing may takes long time. 
## Output result file is text.
```

#### Pipelined SJCS

```
./astr_rcount.exe --thread 20 --next-setup 3 --particle-files part*.dat --halo-file halo.list \
		  --radius 0.001:5:40  --output-file result \
		  --backlog 2 --first-setup 2:9

#--execution info--
#  particle files (compressed = false): 
#   |- part0.dat
#   |- part1.dat
#   |- part2.dat
#  halo file = halo.list
#  halo filter file = (null)
#  # of threads in thread pool = 20
#  # of index construction threads = 3
#  # of radiuses = 40
#   |- minimum = 0.001000, maximum = 5.000000
#  max # of backlog indexes = 2
#  # of first backlog indexes = 2
#  # of first index construction threads = 9
#>>>> halo data reading time: 6979 milli seconds
#  # of halos = 10000000
#  -- with backlog execution
#index 0
#index 1
#>>>> particles reading time: 452 milli seconds
#>>>> particles reading time: 492 milli seconds
#>>>> index construction time: 10015 milli seconds
#  # of particles = 100000000
#>>>> index getting time: 10481 milli seconds
#>>>> index construction time: 11488 milli seconds
#  # of particles = 100000000
#>>>> searching time: 24713 milli seconds
#index 2
#>>>> index getting time: 0 milli seconds
#>>>> index destruction time: 22 milli seconds
#>>>> particles reading time: 578 milli seconds
#>>>> index construction time: 12885 milli seconds
#  # of particles = 100000000
#>>>> searching time: 20698 milli seconds
#>>>> index getting time: 0 milli seconds
#index 3
#>>>> index destruction time: 16 milli seconds
#>>>> searching time: 22041 milli seconds
#index 4
#>>>> index destruction time: 9 milli seconds
#>>>> total searching time: 77946 milli seconds
```

## Codes
### SJCS main
* src/astr_rcount/
* src/rt_rcount/

### Index 
* src/objects/ArraySTR/
* src/objects/RTree/

### Data generator
* src/uniform_decoded_particle_generator/
* src/uniform_halo_generator/

### Data reader
* src/decoded_particle_reader/
* src/halo_reader/

#### Make etag (from PostgreSQL) for code reading support
./tools/make_etags
