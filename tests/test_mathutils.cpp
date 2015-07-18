#include "mathutils.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#if 0
double test1(double x, void* param){
    *(static_cast<int*>(param))+=1;
    return 1/sqrt(1-x*x);
}

double test2(double x, void* param){
    *(static_cast<int*>(param))+=1;
    return pow(1-x*x*x*x,-2./3);
}

double test3(double x, void* param){
    *(static_cast<int*>(param))+=1;
    return mathutils::sign(x-0.3)*pow(fabs(x-0.3), 1./5);
}

double test4(double x, void* param){
    *(static_cast<int*>(param))+=1;
    return x-1+1e-3/sqrt(x);
}

double test5(double x, void* param){
    *(static_cast<int*>(param))+=1;
    return (x-1)-1e5*(x-1.0001)*(x-1.0001)-1e-4;
}

double test6(double x, void* param){
    *(static_cast<int*>(param))+=1;
    return sin(1e4*x);
}

int main()
{
    std::cout << std::setprecision(10);
    bool ok=true;

    // integration routines
    double exact = (M_PI*2/3);
    int nev=0, nev_sc=0;
    double integ = mathutils::integrate(test1, &nev, -1, 1./2);
    double integ_sc = mathutils::integrateScaled(test1, &nev_sc, -1, 1./2, -1, 1);
    std::cout << "Int1: naive="<<integ<<", scaled="<<integ_sc<<", exact="<<exact<<"; neval="<<nev<<",neval_scaled="<<nev_sc<<"\n";
    ok &= fabs(integ-exact)<5e-3 && fabs(integ_sc-exact)<1e-8;

    exact = 2.274454287;
    nev=0; nev_sc=0;
    integ = mathutils::integrate(test2, &nev, -1, 2./3);
    integ_sc = mathutils::integrateScaled(test2, &nev_sc, -1, 2./3, -1, 1);
    std::cout << "Int2: naive="<<integ<<", scaled="<<integ_sc<<", exact="<<exact<<"; neval="<<nev<<",neval_scaled="<<nev_sc<<"\n";
    ok &= fabs(integ-exact)<5e-2 && fabs(integ_sc-exact)<5e-4;

    // root-finding
    exact=0.3;
    nev=0;
    double root = mathutils::findRoot(test3, &nev, 0, 0.8);
    std::cout << "Root1="<<root<<", exact="<<exact<<"; neval="<<nev<<"\n";
    ok &= fabs(root-exact)<1e-6;

    exact=1.000002e-6;
    nev=0;
    root = mathutils::findRootGuess(test4, &nev, 0, 0.8, 0.5, false);
    std::cout << "Root2="<<root<<", exact="<<exact<<"; neval="<<nev<<"\n";
    ok &= fabs(root-exact)<1e-5*exact;

    nev=0;
    double val, der;
    root = mathutils::findPositiveValue(test4, &nev, 5e-6, &val, &der);  // doesn't yet work if started further away..
    std::cout << "positive value at x="<<root<<", value="<<val<<", deriv="<<der<<"; neval="<<nev<<"\n";
    ok &= mathutils::isFinite(root) && root>0 && root<exact && val>0;

    nev=0;
    root = mathutils::findPositiveValue(test5, &nev, 1, &val, &der);  // doesn't yet work if started further away..
    std::cout << "positive value at x=1.0001+"<<(root-1.0001)<<", value="<<val<<", deriv="<<der<<"; neval="<<nev<<"\n";
    ok &= mathutils::isFinite(root) && val>0;

    exact=1.00011;
    nev=0;
    root = mathutils::findRootGuess(test5, &nev, 1.0001, HUGE_VAL, root, false);
    std::cout << "Another root="<<root<<", exact="<<exact<<"; neval="<<nev<<"\n";
    ok &= fabs(root-exact)<1e-5*exact;

    nev=0;
    exact=cos(1e4*0.42)*1e4;
    der = mathutils::deriv(test6, &nev, 0.42, 2e-4, 0);
    std::cout << "deriv="<<der<<", exact="<<exact<<"; neval="<<nev<<"\n";
    ok &= fabs(der-exact)<1e-5*fabs(exact);

    if(ok)
        std::cout << "ALL TESTS PASSED\n";
#else
int main() {
#endif
    return 0;
}
