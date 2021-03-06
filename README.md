This file is part of https://github.com/fzenke/pub2013ca_inference_net and contains 
code used to simulate the spiking neural networks in

Lütcke, H., Gerhard, F., Zenke, F., Gerstner, W., and Helmchen, F. (2013).
Inference of neuronal network spike dynamics and topology from calcium
imaging data. Front Neural Circuits 7.
http://journal.frontiersin.org/article/10.3389/fncir.2013.00201/abstract

The code for the first part of the paper to simulate  calcium traces and 
inferring APs using the peeling algorithm can be found in the Github repository 
of Henry Lütcke: https://github.com/HelmchenLab/CalciumSim

To run the simulation you need a compiled version of the network simulator
Auryn,  which can be found at http://www.fzenke.net/auryn and
http://github.org/fzenke/auryn. Requires Auryn v0.8

Note that the output files are VERY large!

To run the simulation it will suffice on most Linux systems to call 'make'
from the command line. This will unpack the time series file with the brownian
rate modulation, compile the code and run the simulation. Output files are
written to the subdirectory ./data. 

The simulation generates the following human readable files:


    lgnet.0.e.prate : Firing rate of exc population
    lgnet.0.i.ras : Spikes from inh population
    lgnet.0.e.ras : Spikes from exc population
    lgnet.0.p.ras : Spikes from Poisson population
    lgnet.0.e.mem : First couple of seconds of membrane of neuron 123
    lgnet.0.log : Logfile
    lgnet.0.ee.wmat : Connectivity matrix E->E in MatrixMarket format 
    lgnet.0.ei.wmat : Connectivity matrix E->I in MatrixMarket format 
    lgnet.0.ie.wmat : Connectivity matrix I->E in MatrixMarket format 
    lgnet.0.ii.wmat : Connectivity matrix I->I in MatrixMarket format 

Note that the spikes generated in this step might need further filtering as was
done in Lütcke, H., Gerhard, F., et al. (2013)

Enjoy!

-- F. Zenke, Mon Oct 19 23:47:11 PDT 2015
