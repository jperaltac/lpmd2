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
 *
 *  This header aggregates the most commonly used headers in the LPMD
 *  codebase so that applications can include the complete public API with a
 *  single directive.
 */

#include <lpmd/atom.h>
#include <lpmd/atompair.h>
#include <lpmd/cell.h>
#include <lpmd/cellformat.h>
#include <lpmd/cellgenerator.h>
#include <lpmd/cellmanager.h>
#include <lpmd/cellreader.h>
#include <lpmd/cellwriter.h>
#include <lpmd/coulomb.h>
#include <lpmd/error.h>
#include <lpmd/instantproperty.h>
#include <lpmd/integrator.h>
#include <lpmd/matrix.h>
#include <lpmd/metalpotential.h>
#include <lpmd/onestepintegrator.h>
#include <lpmd/pairpotential.h>
#include <lpmd/particles.h>
#include <lpmd/physunits.h>
#include <lpmd/plugin.h>
#include <lpmd/pluginmanager.h>
#include <lpmd/potential.h>
#include <lpmd/potentialarray.h>
#include <lpmd/scalartable.h>
#include <lpmd/scalarvalue.h>
#include <lpmd/simulation.h>
#include <lpmd/simulationcell.h>
#include <lpmd/systemmodifier.h>
#include <lpmd/temporalproperty.h>
#include <lpmd/timer.h>
#include <lpmd/twostepintegrator.h>
#include <lpmd/util.h>
#include <lpmd/vector.h>
#include <lpmd/vectorvalue.h>
#include <lpmd/visualizer.h>

#endif
