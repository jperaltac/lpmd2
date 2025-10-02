/*
 *
 *
 *
 */

#ifndef __LPMD_CONFIGURATION_H__
#define __LPMD_CONFIGURATION_H__

#include <lpmd/basiccell.h>
#include <lpmd/basicparticleset.h>
#include <lpmd/cellmanager.h>
#include <lpmd/matrix.h>
#include <lpmd/taghandler.h>

namespace lpmd {
class RuntimeContext;

class Configuration : public TagHandler<Configuration> {
public:
  Configuration() : cellman(0), stresstensor(3, 3), runtime_context_(0) {}
  virtual ~Configuration(){};

  virtual BasicParticleSet& Atoms() = 0;
  virtual const BasicParticleSet& Atoms() const = 0;
  virtual BasicCell& Cell() = 0;
  virtual const BasicCell& Cell() const = 0;

  inline unsigned long int ID() const { return (unsigned long int)(this); }
  void ShowInfo(std::ostream& out);

  void SetRuntimeContext(lpmd::RuntimeContext& context) { runtime_context_ = &context; }

  bool HasRuntimeContext() const { return runtime_context_ != 0; }

  lpmd::RuntimeContext& Context() {
    if (runtime_context_ == 0)
      throw MissingComponent("runtime_context");
    return *runtime_context_;
  }

  const lpmd::RuntimeContext& Context() const {
    if (runtime_context_ == 0)
      throw MissingComponent("runtime_context");
    return *runtime_context_;
  }

  double& Virial() { return virial; }
  const double& Virial() const { return virial; }

  Matrix& StressTensor() { return stresstensor; }
  const Matrix& StressTensor() const { return stresstensor; }

  void SetCellManager(lpmd::CellManager& cm) {
    cellman = &cm;
    cm.UpdateCell(*this);
  }

  lpmd::CellManager& GetCellManager() {
    if (cellman == 0)
      throw MissingComponent("cellmanager");
    return (*cellman);
  }

  NeighborList& Neighbors(long int i, bool full, double rcut) {
    if (cellman == 0)
      throw MissingComponent("cellmanager");
    cellman->BuildNeighborList(*this, i, neighlist, full, rcut);
    return neighlist;
  }

  void Update() {
    if (cellman == 0)
      throw MissingComponent("cellmanager");
    cellman->UpdateCell(*this);
  }

  double MinimumPairDistance() const;

private:
  lpmd::CellManager* cellman;
  NeighborList neighlist;
  double virial;
  Matrix stresstensor;
  lpmd::RuntimeContext* runtime_context_;
};
} // namespace lpmd

#endif
