#pragma once
#include "math_base.h"

namespace mathutils{

/** default relative accuracy of root-finder */
const double ACCURACY_ROOT=1e-6;

/** default relative accuracy of integration */
const double ACCURACY_INTEGR=1e-6;

/** limit on the maximum number of steps in ODE solver */
const int ODE_MAX_NUM_STEP=1e6;

/** test if a number is not infinity or NaN */
bool isFinite(double x);

/** compare two numbers with a relative accuracy eps: 
    \return -1 if x<y, +1 if x>y, or 0 if x and y are approximately equal */
int fcmp(double x, double y, double eps=1e-15);

/** return sign of a number */
inline double sign(double x) { return x>0?1.:x<0?-1.:0; }


/** ensure that the angle lies in [0,2pi) */
double wrapAngle(double x);

/** create a nearly monotonic sequence of angles by adding or subtracting 2Pi
    as many times as needed to bring the current angle to within Pi from the previous one.
    This may be used in a loop over elements of an array, by providing the previous 
    element, already processed by this function, as xprev. 
    Note that this usage scenario is not stable against error accumulation. */
double unwrapAngle(double x, double xprev);


/** find a root of function on the interval [x1,x2].
    function must be finite at the ends of interval and have opposite signs (or be zero),
    otherwise NaN is returned.
    Interval can be (semi-)infinite, in which case an appropriate transformation is applied
    to the variable (but the function still should return finite value for an infinite argument).
    \param[in] rel_toler  determines the accuracy of root location, relative to the range |x2-x1|
*/
double findRoot(const IFunction& F, double x1, double x2, double rel_toler=ACCURACY_ROOT);


/** integrate a (well-behaved) function on a finite interval */
double integrate(const IFunction& F, double x1, double x2, double rel_toler=ACCURACY_INTEGR);

/** integrate a function with a transformation that removes possible singularities
    at the endpoints [x_low,x_upp], and the integral is computed over the interval [x1,x2] 
    such than x_low<=x1<=x2<=x_upp.
*/
double integrateScaled(const IFunction& F, double x1, double x2, 
    double x_low, double x_upp, double rel_toler=ACCURACY_INTEGR);


/** description of function behavior near a given point: the value and two derivatives,
    and the estimates of nearest points where the function takes on 
    strictly positive or negative values, or crosses zero.
    These estimates may be used to safely determine the interval of locating a root: 
    for instance, if one knows that f(x_1)=0 or very nearly zero (to within roundoff errors), 
    f(x_neg) is strictly negative,  and f(x) is positive at some interval between x_1 and x_neg,
    then one needs to find x_pos close to x_1 such that the root is strictly bracketed between 
    x_pos and x_neg (i.e. f(x_pos)>0). This is exactly the task for this little helper class.
*/
class PointNeighborhood {
public:
    double f0, fder, fder2;  ///< the value, first and second derivative at the given point
    PointNeighborhood(const IFunction& fnc, double x0);

    /// return the estimated offset from x0 to the value of x where the function is positive
    double dx_to_positive() const {
        return dx_to_posneg(+1); }
    /// return the estimated offset from x0 to the value of x where the function is negative
    double dx_to_negative() const {
        return dx_to_posneg(-1); }
    /// return the estimated offset from x0 to the nearest root of f(x)=0
    double dx_to_nearest_root() const;
private:
    double delta;   ///< a reasonably small value
    double dx_to_posneg(double sgn) const;
};


/** perform a linear least-square fit (i.e., y=c*x+b);
    store the best-fit slope and intercept of the relation in the corresponding output arguments, 
    and store the rms scatter in the output argument 'rms' if it is not NULL. */
void linearFit(unsigned int N, const double x[], const double y[], 
    double& slope, double& intercept, double* rms=0);

/** perform a linear least-square fit without constant term (i.e., y=c*x);
    return the best-fit slope of the relation, and store the rms scatter 
    in the output argument 'rms' if it is not NULL. */
double linearFitZero(unsigned int N, const double x[], const double y[], double* rms=0);


/** Prototype of a function that is used in integration of ordinary differential equation systems:
    dy/dt = f(t, y), where y is an N-dimensional vector. */
class IOdeSystem {
public:
    IOdeSystem() {};
    virtual ~IOdeSystem() {};
    
    /** compute the r.h.s. of the differential equation: 
        \param[in]  t    is the integration variable (time),
        \param[in]  y    is the vector of values of dependent variables, 
        \param[out] dydt should return the time derivatives of these variables */
    virtual void eval(const double t, const double y[], double* dydt) const=0;
    
    /** return the size of ODE system (number of variables) */
    virtual int size() const=0;
};

/** solve a system of differential equations */
class OdeSolver {
public:
    OdeSolver(const IOdeSystem& F, double abstoler, double reltoler);
    ~OdeSolver();
    int advance(double tstart, double tfinish, double *y);
private:
    void* impl;   ///< implementation details are hidden
};

}  // namespace
