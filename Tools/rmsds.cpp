/*
  rmsds.cpp

  Pair-wise RMSD
*/



/*
  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2008, Tod D. Romo
  Department of Biochemistry and Biophysics
  School of Medicine & Dentistry, University of Rochester

  This package (LOOS) is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation under version 3 of the License.

  This package is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#include <loos.hpp>
#include <unistd.h>

using namespace std;
using namespace loos;


namespace opts = loos::OptionsFramework;
namespace po = loos::OptionsFramework::po;

const int matrix_precision = 2;    // Controls precision in output matrix

#if defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <errno.h>
#endif

int verbosity;


// If the estimated cache memory is more than this fraction of physical memory,
// issue a warning to the user to consider turning off the cache
// Note: the total app size may be 20-30% larger than the cache estimate, so
//       take that into consideration when setting the warning threshold

const double cache_memory_fraction_warning = 0.66;



// @cond TOOLS_INTERNAL

string fullHelpMessage(void) {
  string msg =
    "\n"
    "SYNOPSIS\n"
    "\n"
    "\tCalculate a pair-wise RMSD for a trajectory (or two trajectories)\n"
    "DESCRIPTION\n"
    "\n"
    "\tThis tool calculates the pair-wise RMSD between each structure in a trajectory\n"
    "or, alternatively, between each structure in two different trajectories.  In the single\n"
    "trajectory case, the ith structure is aligned with the jth structure and the RMSD calculated.\n"
    "This is stored in a matrix, i.e. R(j, i) = d(S_i, S_j).  The block-structure is indicative\n"
    "of sets of similar conformations.  The presence (or lack thereof) of multiple cross-peaks\n"
    "is diagnostic of the sampling quality of a simulation.\n"
    "\n"
    "\tThe requested subset for each frame is cached in memory for better performance.\n"
    "If the memory used by the cache gets too large, your machine may swap and dramatically slow\n"
    "down.  The tool will try to warn you if this is a possibility.  To use less memory, disable\n"
    "the cache with --cache=0 on the command line.  This will impact performance, but it will\n"
    "likely be a smaller impact than running out of memory.\n"
    "\n"
    "EXAMPLES\n"
    "\n"
    "\trmsds model.pdb simulation.dcd >rmsd.asc\n"
    "This example uses all alpha-carbons and every frame in the trajectory.\n"
    "\n"
    "\trmsds --cache=0 model.pdb simulation.dcd >rmsd.asc\n"
    "This example uses all alpha-carbons and every frame in the trajectory, but the\n"
    "trajectory is not cached in memory.\n"
    "\n"
    "\trmsds inactive.pdb inactive.dcd active.pdb active.dcd >rmsd.asc\n"
    "This example uses all alpha-carbons and compares the \"inactive\" simulation\n"
    "with the \"active\" one.\n"
    "\n"
    "\trmsds --sel1 'resid <= 100 && name == \"CA\"' model.pdb simulation.dcd >rmsds.asc\n"
    "This example calculates the pair-wise RMSD using only the first 100 alpha-carbons\n"
    "\n"
    "\trmsds --sel1 'resid <= 50 && name == \"CA\"' \\\n"
    "\t  --sel2 'resid >=20 && resid <= 69 && name == \"CA\"' \\\n"
    "\t  inactive.pdb inactive.dcd active.pdb active.dcd >rmsd.asc\n"
    "This example compares two trajectories, active and inactive, and uses different selections\n"
    "for both: the first 50 residues from the inactive and residues 20-69 from the active.\n"
    "\n"
    "NOTES\n"
    "\tWhen using two trajectories, the selections must match both in number of atoms selected\n"
    "and in the sequence of atoms (i.e. the first atom in the --sel2 selection is\n" 
    "matched with the first atom in the --sel2 selection.)\n"
    "\n"
    "SEE ALSO\n"
    "\trmsd2ref\n"
    "\n";

  return(msg);
}



class ToolOptions : public opts::OptionsPackage {
public:
  ToolOptions() { }

  void addGeneric(po::options_description& o) {
    o.add_options()
      ("noout,N", po::value<bool>(&noop)->default_value(false), "Do not output the matrix (i.e. only calc pair-wise RMSD stats)")
      ("sel1", po::value<string>(&sel1)->default_value("name == 'CA'"), "Atom selection for first system")
      ("skip1", po::value<uint>(&skip1)->default_value(0), "Skip n-frames of first trajectory")
      ("range1", po::value<string>(&range1), "Matlab-style range of frames to use from first trajectory")
      ("sel2", po::value<string>(&sel2)->default_value("name == 'CA'"), "Atom selection for second system")
      ("skip2", po::value<uint>(&skip2)->default_value(0), "Skip n-frames of second trajectory")
      ("range2", po::value<string>(&range2), "Matlab-style range of frames to use from second trajectory");

  }

  void addHidden(po::options_description& o) {
    o.add_options()
      ("model1", po::value<string>(&model1), "Model-1 Filename")
      ("traj1", po::value<string>(&traj1), "Traj-1 Filename")
      ("model2", po::value<string>(&model2), "Model-2 Filename")
      ("traj2", po::value<string>(&traj2), "Traj-2 Filename");
  }

  void addPositional(po::positional_options_description& pos) {
    pos.add("model1", 1);
    pos.add("traj1", 1);
    pos.add("model2", 1);
    pos.add("traj2", 1);
  }


  bool check(po::variables_map& m) {
    return( ! ( (m.count("model1") && m.count("traj1")) && !(m.count("model2") ^ m.count("traj2"))) );
  }


  string help() const {
    return("model-1 trajectory-1 [model-2 trajectory-2]");
  }


  string print() const {
    ostringstream oss;
    oss << boost::format("noout=%d,sel1='%s',skip1=%d,range1='%s',sel2='%s',skip2=%d,range2='%s',model1='%s',traj1='%s',model2='%s',traj2='%s'")
      % noop
      % sel1
      % skip1
      % range1
      % sel2
      % skip2
      % range2
      % model1
      % traj1
      % model2
      % traj2;

    return(oss.str());
  }


  bool noop;
  uint skip1, skip2;
  string range1, range2;
  string model1, traj1, model2, traj2;
  string sel1, sel2;
};

typedef vector<double>    vDouble;
typedef vector<vDouble>   vMatrix;


// @endcond TOOLS_INTERNAL


vMatrix readCoords(AtomicGroup& model, pTraj& traj) {
  uint l = traj->nframes();
  uint n = model.size();
  vMatrix M = vector< vector<double> >(l, vector<double>(3*n, 0.0));
  
  for (uint j=0; j<l; ++j) {
    traj->readFrame(j);
    traj->updateGroupCoords(model);
    for (uint i=0; i<n; ++i) {
      GCoord c = model[i]->coords();
      M[j][i*3] = c.x();
      M[j][i*3+1] = c.y();
      M[j][i*3+2] = c.z();
    }
  }

  return(M);
}


void centerAtOrigin(vDouble& v) {
  double c[3] = {0.0, 0.0, 0.0};

  for (uint i=0; i<v.size(); i += 3) {
    c[0] += v[i];
    c[1] += v[i+1];
    c[2] += v[i+2];
  }

  for (uint i=0; i<3; ++i)
    c[i] = 3*c[i]/v.size();

  for (uint i=0; i<v.size(); i += 3) {
    v[i] -= c[0];
    v[i+1] -= c[1];
    v[i+2] -= c[2];
  }
}



void centerTrajectory(vMatrix& M) {
  for (uint i=0; i<M.size(); ++i)
    centerAtOrigin(M[i]);
}



double calcRMSD(vDouble& u, vDouble& v) {
  int n = u.size();

  double ssu[3] = {0.0, 0.0, 0.0};
  double ssv[3] = {0.0, 0.0, 0.0};

  for (int j=0; j<n; j += 3) {
    for (uint i=0; i<3; ++i) {
      ssu[i] += u[j+i] * u[j+i];
      ssv[i] += v[j+i] * v[j+i];
    }
  }

  n /= 3;

  double E0 = ssu[0] + ssu[1] + ssu[2] + ssv[0] + ssv[1] + ssv[2];

  // Compute correlation matrix...
  double R[9];

#if defined(__linux__) || defined(__CYGWIN__) || defined(__FreeBSD__)
  char ta = 'N';
  char tb = 'T';
  f77int three = 3;
  double one = 1.0;
  double zero = 0.0;
    
  dgemm_(&ta, &tb, &three, &three, &n, &one, u.data(), &three, v.data(), &three, &zero, R, &three);

#else

  cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, 3, 3, n, 1.0, u.data(), 3, v.data(), 3, 0.0, R, 3);

#endif

  // Now compute the SVD of R...
  char joba='G';
  char jobu = 'U', jobv = 'V';
  int mv = 0;
  f77int m = 3, lda = 3, ldv = 3, lwork=100, info;
  double work[lwork];
  f77int nn = 3;
  double S[3], V[9];
  
  dgesvj_(&joba, &jobu, &jobv, &m, &nn, R, &lda, S, &mv, V, &ldv, work, &lwork, &info);
    
  if (info != 0)
    throw(NumericalError("SVD in AtomicGroup::superposition returned an error", info));

  double ss = S[0] + S[1] + S[2];
  double rmsd = sqrt(abs(E0-2.0*ss)/n);
  return(rmsd);
}



RealMatrix rmsds(vMatrix& M) {
  uint n = M.size();
  RealMatrix R(n, n);

  uint total = floor(n * (n-1) / 2.0);
  PercentProgressWithTime watcher;
  PercentTrigger trigger(0.1);
  ProgressCounter<PercentTrigger, EstimatingCounter> slayer(trigger, EstimatingCounter(total));
  slayer.attach(&watcher);
  slayer.start();

  for (uint j=1; j<n; ++j)
    for (uint i=0; i<j; ++i) {
      R(j, i) = calcRMSD(M[j], M[i]);
      R(i, j) = R(j, i);
      slayer.update();
    }

  slayer.finish();
  return(R);
}



int main(int argc, char *argv[]) {
  string header = invocationHeader(argc, argv);
  
  opts::BasicOptions* bopts = new opts::BasicOptions(fullHelpMessage());
  ToolOptions* topts = new ToolOptions;

  opts::AggregateOptions options;
  options.add(bopts).add(topts);
  if (!options.parse(argc, argv))
    exit(-1);

  verbosity = bopts->verbosity;
  AtomicGroup model = createSystem(topts->model1);
  pTraj traj = createTrajectory(topts->traj1, model);
  AtomicGroup subset = selectAtoms(model, topts->sel1);

  vMatrix T = readCoords(subset, traj);
  centerTrajectory(T);

  RealMatrix M = rmsds(T);

  if (!topts->noop) {
    cout << "# " << header << endl;
    cout << setprecision(matrix_precision) << M;
  }

}


  

