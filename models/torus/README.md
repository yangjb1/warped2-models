# Torus Network Simulation Model #

Authors : John Musgrave <br>
High Performance Computing Laboratory <br>
Dept of ECECS, PO Box 210030 <br>
Cincinnati, OH  45221-0030 <br>

## Introduction : ##

A torus is a k-ary network with k^n nodes arranged in a grid with k nodes in each dimension. [1]

## Details : ##

1. A node can send or receive messages based on it's availability.

2. A message has an origin, a destination, and a hop count.

3. The routing is determined by ??? 

4. The initial configuration is ???


Model details adapted from ROSS torus model [2].

## Configuration : ##

User can adjust the following parameters:

    1. Dimension of torus (Default: 5) 

    2. Size of torus grid (Default: 1000)

    3. Order of node connections in torus (Default: 4)


## References : ##

[1] Dally, William James., and Brian Towles. 2004. Principles and practices of interconnection networks. Morgan Kaufmann.

[2] ROSS torus model
