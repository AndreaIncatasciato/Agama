/** \brief   Action-angle finders using Staeckel potential approximation
    \author  Eugene Vasiliev
    \date    2015
 
Computation of actions and angles for axisymmetric systems, using two related methods:
1) exact expressions for Staeckel potentials  (e.g. for the Perfect Ellipsoid),
2) "Staeckel Fudge" approximation applicable for any axisymmetric potential.
To avoid confusion, the latter is called simply "Fudge" in what follows.

Most sub-steps are shared between the two methods; 
only the computation of integrals of motion, and the auxiliary function that enters 
the expression for canonical momentum, are specific to each method.

The implementation is inspired by the code written by Jason Sanders, but virtually nothing 
of the original code remains.
*/
#pragma once
#include "actions_base.h"
#include "potential_staeckel.h"
#include "math_base.h"

namespace actions{

/// \name ------ Data structures for both Axisymmetric Staeckel and Fudge action-angle finders ------
///@{

/** integration intervals for actions and angles: x=tau+gamma, and tau is lambda or nu 
    (shared between Staeckel and Fudge action finders). */
struct AxisymIntLimits {
    double xlambda_min, xlambda_max, xnu_min, xnu_max;
};

/** Derivatives of integrals of motion over actions (do not depend on angles).
    Note that dE/dJ are the frequencies of oscillations in three directions. */
struct AxisymIntDerivatives {
    double dEdJr, dEdJz, dEdJphi, dI3dJr, dI3dJz, dI3dJphi, dLzdJr, dLzdJz, dLzdJphi;
};

/** Derivatives of generating function over integrals of motion (depend on angles) */
struct AxisymGenFuncDerivatives {
    double dSdE, dSdI3, dSdLz;
};

/** aggregate class that contains the point in prolate spheroidal coordinates,
    integrals of motion, and reference to the potential,
    shared between Axisymmetric Staeckel  and Axisymmetric Fudge action finders;
    only the coordinates and the two classical integrals are in common between them.
    It also implements the IFunction interface, providing the "auxiliary function" 
    that is used in finding the integration limits and computing actions and angles;
    this function F(tau) is related to the canonical momentum p(tau)  as 
    \f$  p(tau)^2 = F(tau) / (2*(tau+alpha)^2*(tau+gamma))  \f$,
    and the actual implementation of this function is specific to each descendant class.
*/
class AxisymFunctionBase: public mathutils::IFunction {
public:
    coord::PosVelProlSph point; ///< position/derivative in prolate spheroidal coordinates
    const double signz;         ///< sign of z coordinate (needed because it is lost in prolate spheroidal coords)
    const double E;             ///< total energy
    const double Lz;            ///< z-component of angular momentum
    AxisymFunctionBase(const coord::PosVelProlSph& _point, double _signz, double _E, double _Lz) :
        point(_point), signz(_signz), E(_E), Lz(_Lz) {};
};


///@}
/// \name ------ SPECIALIZED functions for Staeckel action finder -------
///@{

/** parameters of potential, integrals of motion, and prolate spheroidal coordinates 
    SPECIALIZED for the Axisymmetric Staeckel action finder */
class AxisymFunctionStaeckel: public AxisymFunctionBase {
public:
    const double I3;                        ///< third integral
    const mathutils::IFunction& fncG;       ///< single-variable function of a Staeckel potential
    AxisymFunctionStaeckel(const coord::PosVelProlSph& _point, double _signz, double _E, double _Lz,
        double _I3, const mathutils::IFunction& _fncG) :
        AxisymFunctionBase(_point, _signz, _E, _Lz), I3(_I3), fncG(_fncG) {};

    /** auxiliary function that enters the definition of canonical momentum for 
        for the Staeckel potential: it is the numerator of eq.50 in de Zeeuw(1985);
        the argument tau is replaced by tau+gamma >= 0. */    
    virtual void eval_deriv(const double tauplusgamma, 
        double* value=0, double* deriv=0, double* deriv2=0) const;
    virtual int numDerivs() const { return 2; }
};

/** compute integrals of motion in the Staeckel potential of an oblate perfect ellipsoid, 
    together with the coordinates in its prolate spheroidal coordinate system 
*/
AxisymFunctionStaeckel findIntegralsOfMotionOblatePerfectEllipsoid(
    const potential::StaeckelOblatePerfectEllipsoid& potential, 
    const coord::PosVelCyl& point);

///@}
/// \name -------- SPECIALIZED functions for the Axisymmetric Fudge action finder --------
///@{

/** parameters of potential, integrals of motion, and prolate spheroidal coordinates 
    SPECIALIZED for the Axisymmetric Fudge action finder */
class AxisymFunctionFudge: public AxisymFunctionBase {
public:
    const double Ilambda, Inu;              ///< approximate integrals of motion for two quasi-separable directions
    const potential::BasePotential& poten;  ///< gravitational potential
    AxisymFunctionFudge(const coord::PosVelProlSph& _point, double _signz, double _E, double _Lz,
        double _Ilambda, double _Inu, const potential::BasePotential& _poten) :
        AxisymFunctionBase(_point, _signz, _E, _Lz), Ilambda(_Ilambda), Inu(_Inu), poten(_poten) {};

    /** Auxiliary function F analogous to that of Staeckel action finder:
        namely, the momentum is given by  p_tau^2 = F(tau) / (2*(tau+alpha)^2*(tau+gamma)),
        where  -gamma<=tau<=-alpha  for the  nu-component of momentum, 
        and   -alpha<=tau<infinity  for the  lambda-component of momentum.
        For numerical convenience, tau is replaced by x=tau+gamma. */
    virtual void eval_deriv(const double tauplusgamma, 
        double* value=0, double* deriv=0, double* deriv2=0) const;
    virtual int numDerivs() const { return 1; }
};

/** compute true (E, Lz) and approximate (Ilambda, Inu) integrals of motion in an arbitrary 
    potential used for the Staeckel Fudge, 
    together with the coordinates in its prolate spheroidal coordinate system 
*/
AxisymFunctionFudge findIntegralsOfMotionAxisymFudge(
    const potential::BasePotential& potential, 
    const coord::PosVelCyl& point, 
    const coord::ProlSph& coordsys);

///@}
/// \name -------- COMMON routines for Staeckel and Fudge action finders --------
///@{
///   In what follows, "data" refers to either "AxisymFunctionStaeckel" or "AxisymFunctionFudge"

/** Compute the intervals of tau for which p^2(tau)>=0, 
    where  -gamma = tau_nu_min <= tau <= tau_nu_max <= -alpha  is the interval for the "nu" branch,
    and  -alpha <= tau_lambda_min <= tau <= tau_lambda_max < infinity  is the interval for "lambda".
    For numerical convenience, we replace tau with  x=tau+gamma. */
AxisymIntLimits findIntegrationLimitsAxisym(const AxisymFunctionBase& fnc);

/** Compute the derivatives of integrals of motion (E, Lz, I3) over actions (Jr, Jz, Jphi),
    using the expressions A4-A9 in Sanders(2012).  These quantities are independent of angles,
    and in particular, the derivatives of energy w.r.t. the three actions are the frequencies. */
AxisymIntDerivatives computeIntDerivatives(const AxisymFunctionBase& fnc, const AxisymIntLimits& lim);

/** Compute the derivatives of generating function S over integrals of motion (E, Lz, I3),
    using the expressions A10-A12 in Sanders(2012).  These quantities do depend on angles. */
AxisymGenFuncDerivatives computeGenFuncDerivatives(const AxisymFunctionBase& fnc, const AxisymIntLimits& lim);

/** Compute actions by integrating the momentum over the range of tau on which it is positive,
    separately for the "nu" and "lambda" branches (equation A1 in Sanders 2012). */
Actions computeActions(const AxisymFunctionBase& fnc, const AxisymIntLimits& lim);

/** Compute angles from the derivatives of integrals of motion and the generating function
    (equation A3 in Sanders 2012). */
Angles computeAngles(const AxisymIntDerivatives& derI, 
    const AxisymGenFuncDerivatives& derS, bool addPiToThetaZ);

/** The sequence of operations needed to compute both actions and angles.
    Note that for a given orbit, only the derivatives of the generating function depend 
    on the angles (assuming that the actions are constant); in principle, this may be used 
    to skip the computation of the derivatives matrix of integrals (not presently implemented). */
ActionAngles computeActionAngles(const AxisymFunctionBase& fnc, const AxisymIntLimits& lim);

///@}
/// \name  ------- The driver routines that combine the above steps -------
///@{

/** Find exact actions in the Staeckel potential of oblate Perfect Ellipsoid */
Actions axisymStaeckelActions(
    const potential::StaeckelOblatePerfectEllipsoid& potential, 
    const coord::PosVelCyl& point);

/** Find exact actions and angles in the Staeckel potential of oblate Perfect Ellipsoid */
ActionAngles axisymStaeckelActionAngles(
    const potential::StaeckelOblatePerfectEllipsoid& potential, 
    const coord::PosVelCyl& point);

/** Find approximate actions in a given axisymmetric potential, using the Staeckel Fudge method;
    the accuracy of the method depends on the "interfocal distance" parameter, 
    if it is not provided, an estimate will be made based on the potential derivatives
    averaged over the area in the meridional plane that the orbit probably covers. */
Actions axisymFudgeActions(
    const potential::BasePotential& potential, 
    const coord::PosVelCyl& point,
    double interfocalDistance=0);

/** Find approximate actions and angles in a given axisymmetric potential, 
    using the Staeckel Fudge method */
ActionAngles axisymFudgeActionAngles(
    const potential::BasePotential& potential, 
    const coord::PosVelCyl& point, 
    double interfocalDistance=0);

///@}
/// \name  ------- Routines for estimating the interfocal distance for the Fudge approximation -------
///@{

/** Estimate the orbit extent in R and z directions; on success, return true 
    and fill the output arguments with the coordinates of closest and farthest corner point */
bool estimateOrbitExtent(
    const potential::BasePotential& potential, const coord::PosVelCyl& point,
    double& R1, double& R2, double& z1, double& z2);

/** Estimate the interfocal distance using the potential derivatives (equation 9 in Sanders 2012)
    averaged over the given box in the meridional plane  */
double estimateInterfocalDistanceBox(
    const potential::BasePotential& potential, 
    double R1, double R2, double z1, double z2);

/** Estimate the orbit extent and then estimate the best-fit interfocal distance over this region */
double estimateInterfocalDistance(
    const potential::BasePotential& potential, const coord::PosVelCyl& point);

///@}
}  // namespace actions
