//
//
//

#ifndef __LPMD_API_H__
#define __LPMD_API_H__

/**
 *  \file lpmdapi.h
 *  \brief Include file for the complete API
 *  \author GNM
 */

/**
 *  \mainpage LPMD: Las Palmeras Molecular Dynamics
#include <lpmd/error.h>
#include <lpmd/physunits.h>
#include <lpmd/timer.h>
#include <lpmd/util.h>
// Plugin loading

// Math
#include <lpmd/vector.h>
// Simulation data
#include <lpmd/atompair.h>
#include <lpmd/pairpotential.h>
#include <lpmd/potential.h>
#include <lpmd/potentialarray.h>
// Molecular dynamics

#include <lpmd/temporalproperty.h>

#include <lpmd/plugin.h>
#include <lpmd/pluginmanager.h>

// Math 

#include <lpmd/vector.h>
#include <lpmd/matrix.h>
#include <lpmd/scalartable.h>
#include <lpmd/scalarvalue.h>
#include <lpmd/vectorvalue.h>

// Simulation data 

#include <lpmd/atom.h>
#include <lpmd/cell.h>
#include <lpmd/particles.h>
#include <lpmd/atompair.h>
#include <lpmd/simulationcell.h>

// Potentials

#include <lpmd/potential.h>
#include <lpmd/potentialarray.h>
#include <lpmd/coulomb.h>
#include <lpmd/pairpotential.h>
#include <lpmd/metalpotential.h>

// Integrators

#include <lpmd/integrator.h>
#include <lpmd/onestepintegrator.h>
#include <lpmd/twostepintegrator.h>

// Molecular dynamics 

#include <lpmd/simulation.h>

// Other plugin types (besides Potential and Integrator)

#include <lpmd/cellformat.h>
#include <lpmd/cellgenerator.h>
#include <lpmd/cellmanager.h>
#include <lpmd/cellreader.h>
#include <lpmd/cellwriter.h>
#include <lpmd/instantproperty.h>
#include <lpmd/temporalproperty.h>
#include <lpmd/systemmodifier.h>
#include <lpmd/visualizer.h>

#endif

