/* 
* Copyright 2015 Friedemann Zenke
*
* This file is part of lgnet the network simulation used in 
*
* Lütcke, H., Gerhard, F., Zenke, F., Gerstner, W., and Helmchen, F. (2013).
* Inference of neuronal network spike dynamics and topology from calcium
* imaging data. Front Neural Circuits 7.
* http://journal.frontiersin.org/article/10.3389/fncir.2013.00201/abstract
*
* To run the simulation you need a compiled version of the network simulator
* Auryn,  which can be found at http://www.fzenke.net/auryn and
* http://github.org/fzenke/auryn
* 
* lgnet is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* lgnet is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with lgnet.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "auryn.h"

#define NE 20000
#define NI 20000/4

using namespace std;

namespace po = boost::program_options;
namespace mpi = boost::mpi;

int main(int ac,char *av[]) {

	string dir = "./data";
	string infilename = "";
	string strbuf ;
	string msg;

	double w = 0.2;
	double wext = 0.22;
	double gamma = 5;
	double sparseness = 0.1;
	double simtime = 10000.;

	int errcode = 0;

	// BEGIN Global stuff
	mpi::environment env(ac, av);
	mpi::communicator world;
	communicator = &world;

	stringstream oss;
	oss << dir  << "/lgnet." << world.rank() << ".";
	string outputfile = oss.str();

	stringstream logfile;
	logfile << outputfile << "log";
	logger = new Logger(logfile.str(),world.rank());

	sys = new System(&world);
	// END Global stuff
	

    try {

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("load", po::value<string>(), "input weight matrix")
        ;

        po::variables_map vm;        
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);    

        if (vm.count("help")) {
            cout << desc << "\n";
            return 1;
        }

        if (vm.count("load")) {
            cout << "load from matrix " 
                 << vm["load"].as<string>() << ".\n";
			infilename = vm["load"].as<string>();
        } 

    }
    catch(exception& e) {
        cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        cerr << "Exception of unknown type!\n";
    }


	// double primetime = 10;
	logger->msg("Setting up neuron groups ...",PROGRESS,true);
	IFGroup * neurons_e = new IFGroup( NE);
	neurons_e->set_ampa_nmda_ratio(1.0);
	neurons_e->random_nmda(0.1,1);
	IFGroup * neurons_i = new IFGroup( NI);
	neurons_i->set_ampa_nmda_ratio(1.0);
	neurons_i->set_tau_mem(10e-3);

	FileModulatedPoissonGroup * poisson = new FileModulatedPoissonGroup(2000,"./ratemod.dat");
	SparseConnection * con_exte = new SparseConnection( poisson, neurons_e, wext, sparseness, GLUT);
	SparseConnection * con_exti = new SparseConnection( poisson, neurons_i, wext, sparseness, GLUT);

	logger->msg("Setting up connections ...",PROGRESS,true);

	SparseConnection * con_ee;
	SparseConnection * con_ie;
	// SymmetricSTDPConnection * con_ie;
	SparseConnection * con_ei;
	SparseConnection * con_ii;
	// SymmetricSTDPConnection * con_ii;


	if (infilename.empty()) {
		con_ei = new SparseConnection( neurons_e,neurons_i,w,sparseness,GLUT);

		con_ii = new SparseConnection( neurons_i,neurons_i, 0.9*gamma*w,sparseness,GABA);
		con_ie = new SparseConnection( neurons_i,neurons_e, 0.9*gamma*w,sparseness,GABA);
		con_ee = new SparseConnection(  neurons_e,neurons_e,w,sparseness,GLUT);
		// con_ee->random_data(w,w);
	} else {
		stringstream ifss;
		ifss << infilename << "." << world.rank();
		strbuf = ifss.str();
		strbuf += ".ee.wmat";
		con_ee = new SparseConnection(  neurons_e,neurons_e,strbuf.c_str(),GLUT);
		strbuf = ifss.str();
		strbuf += ".ei.wmat";
		con_ei = new SparseConnection(  neurons_e,neurons_i,strbuf.c_str(),GLUT);
		strbuf = ifss.str();
		strbuf += ".ie.wmat";
		con_ie = new SymmetricSTDPConnection(  neurons_i,neurons_e,strbuf.c_str(),GABA);
		strbuf = ifss.str();
		strbuf += ".ii.wmat";
		con_ii = new SymmetricSTDPConnection(  neurons_i,neurons_i,strbuf.c_str(),GABA);
	}

	// if ( !patfilename.empty() ) {
	// 	logger->msg("Loading patterns ...",PROGRESS,true);
	// 	con_ee->load_patterns(patfilename,4*w);
	// }

	msg = "Setting up monitors ...";
	logger->msg(msg,PROGRESS,true);

	stringstream filename;
	filename << outputfile << "e.ras";
	SpikeMonitor * smon_e = new SpikeMonitor( neurons_e, filename.str().c_str() );

	filename.str("");
	filename.clear();
	filename << outputfile << "i.ras";
	SpikeMonitor * smon_i = new SpikeMonitor( neurons_i, filename.str().c_str() );

	filename.str("");
	filename.clear();
	filename << outputfile << "p.ras";
	SpikeMonitor * smon_p = new SpikeMonitor( poisson, filename.str().c_str() );

	filename.str("");
	filename.clear();
	filename << outputfile << "e.prate";
	PopulationRateMonitor * pmon_e = new PopulationRateMonitor( neurons_e, filename.str().c_str(), 1.0 );

	filename.str("");
	filename.clear();
	filename << outputfile << "e.mem";
	VoltageMonitor * vmon = new VoltageMonitor( neurons_e, 123, filename.str().c_str() );

	RateChecker * chk = new RateChecker( neurons_e , -1.0 , 40. , 100e-3);

	for (int j = 0; j<1000 ; j++) {
	  neurons_e->tadd(j,5.);
	}


	logger->msg("Simulating ..." ,PROGRESS,true);
	if (!sys->run(simtime,true)) 
			errcode = 1;

	logger->msg("Saving weight matrix ..." ,PROGRESS,true);
	filename.str("");
	filename.clear();
	filename << outputfile << "ee.wmat";
	con_ee->write_to_file(filename.str().c_str());
	filename.clear();
	filename.str("");
	filename.clear();
	filename << outputfile << "ei.wmat";
	con_ei->write_to_file(filename.str().c_str());
	filename.clear();
	filename.str("");
	filename.clear();
	filename << outputfile << "ie.wmat";
	con_ie->write_to_file(filename.str().c_str());
	filename.clear();
	filename.str("");
	filename.clear();
	filename << outputfile << "ii.wmat";
	con_ii->write_to_file(filename.str().c_str());

	logger->msg("Freeing ..." ,PROGRESS,true);
	delete sys;

	if (errcode)
		env.abort(errcode);

	return errcode;
}
