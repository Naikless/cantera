/**
 *   Cantera interface library. This library of functions is designed
 *   to encapsulate Cantera functionality and make it available for
 *   use in languages and applications other than C++. A set of
 *   library functions is provided that are declared "extern C". All
 *   Cantera objects are stored and referenced by integers - no
 *   pointers are passed to or from the calling application.
 */

// turn off warnings under Windows
#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif

// Cantera includes
#include "ChemEquil.h"
#include "KineticsFactory.h"
#include "transport/TransportFactory.h"
#include "ctml.h"
#include "importCTML.h"
#include "converters/ck2ct.h"
#include "Storage.h"
#include "Cabinet.h"
#include "InterfaceKinetics.h"
#include "PureFluidPhase.h"

#include "clib_defs.h"

inline XML_Node* _xml(int i) {
    return Cabinet<XML_Node>::cabinet(false)->item(i);
}


#ifdef INCL_PURE_FLUID
static PureFluid* purefluid(int n) {
    try {
        ThermoPhase* tp = th(n);
        if (tp->eosType() == cPureFluid) {
            return (PureFluid*)tp;
        }
        else {
            throw CanteraError("purefluid","object is not a PureFluid object");
        }
    }
    catch (CanteraError) {
        return 0;
    }
}
#else
static ThermoPhase* purefluid(int n) {
    return th(n);
}
#endif

inline int nThermo() {
    return Storage::storage()->nThermo();
}

 
/**
 * Exported functions.
 */
extern "C" {

    //--------------- Phase ---------------------//

    int DLL_EXPORT phase_nElements(int n) {
        return ph(n)->nElements();
    }

    int DLL_EXPORT phase_nSpecies(int n) {
        return ph(n)->nSpecies();
    }

    doublereal DLL_EXPORT phase_temperature(int n) {
        return ph(n)->temperature();
    }

    int DLL_EXPORT phase_setTemperature(int n, double t) {
        try {
            ph(n)->setTemperature(t);
        }
        catch(CanteraError) {return -1;}
        return 0;
    }

    doublereal DLL_EXPORT phase_density(int n) {
        return ph(n)->density();
    }

    int DLL_EXPORT phase_setDensity(int n, double rho) {
        if (rho < 0.0) return -1;
        ph(n)->setDensity(rho);
        return 0;
    }

    doublereal DLL_EXPORT phase_molarDensity(int n) {
        return ph(n)->molarDensity();
    }

    doublereal DLL_EXPORT phase_meanMolecularWeight(int n) {
        return ph(n)->meanMolecularWeight();
    }

    int DLL_EXPORT phase_elementIndex(int n, char* nm) {
        string elnm = string(nm);
        return ph(n)->elementIndex(elnm);
    }

    int DLL_EXPORT phase_speciesIndex(int n, char* nm) {
        string spnm = string(nm);
        return ph(n)->speciesIndex(spnm);
    }

    int DLL_EXPORT phase_getMoleFractions(int n, int lenx, double* x) {
        ThermoPhase* p = ph(n);
        if (lenx >= p->nSpecies()) {
            p->getMoleFractions(x);
            return 0;
        }
        else
            return -1;
    }

    doublereal DLL_EXPORT phase_moleFraction(int n, int k) {
        ThermoPhase* p = ph(n);
        return p->moleFraction(k);
    }

    int DLL_EXPORT phase_getMassFractions(int n, int leny, double* y) {
        ThermoPhase* p = ph(n);
        if (leny >= p->nSpecies()) {
            p->getMassFractions(y);
            return 0;
        }
        else
            return -1;
    } 

    doublereal DLL_EXPORT phase_massFraction(int n, int k) {
        ThermoPhase* p = ph(n);
        return p->massFraction(k);
    }

    int DLL_EXPORT phase_setMoleFractions(int n, int lenx, double* x, int norm) {
        ThermoPhase* p = ph(n);
        if (lenx >= p->nSpecies()) {
            if (norm) p->setMoleFractions(x);
            else p->setMoleFractions_NoNorm(x);
            return 0;
        }
        else
            return -1;
    }

    int DLL_EXPORT phase_setMoleFractionsByName(int n, char* x) {
        try {
            ThermoPhase* p = ph(n);
            compositionMap xx;
            int nsp = p->nSpecies();
            for (int n = 0; n < nsp; n++) {
                xx[p->speciesName(n)] = -1;
            }
            parseCompString(string(x), xx);
            p->setMoleFractionsByName(xx);
            return 0;
        }
        catch (CanteraError) {return -1;}
        //catch (...) {return ERR;}
    }

    int DLL_EXPORT phase_setMassFractions(int n, int leny, 
        double* y, int norm) {
        ThermoPhase* p = ph(n);
        if (leny >= p->nSpecies()) {
            if (norm) p->setMassFractions(y);
            else p->setMassFractions_NoNorm(y);
            return 0;
        }
        else
            return -10;
    }

    int DLL_EXPORT phase_setMassFractionsByName(int n, char* y) {
        try {
            ThermoPhase* p = ph(n);
            compositionMap yy;
            int nsp = p->nSpecies();
            for (int n = 0; n < nsp; n++) {
                yy[p->speciesName(n)] = -1;
            }
            parseCompString(string(y), yy);
            p->setMassFractionsByName(yy);
            return 0;
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT phase_getAtomicWeights(int n, 
        int lenm, double* atw) {
        ThermoPhase* p = ph(n);
        if (lenm >= p->nElements()) {
            const vector_fp& wt = p->atomicWeights();
            copy(wt.begin(), wt.end(), atw);
            return 0;
        }
        else
            return -10;
    }

    int DLL_EXPORT phase_getMolecularWeights(int n, 
        int lenm, double* mw) {
        ThermoPhase* p = ph(n);
        if (lenm >= p->nSpecies()) {
            const vector_fp& wt = p->molecularWeights();
            copy(wt.begin(), wt.end(), mw);
            return 0;
        }
        else
            return -10;
    }

    int DLL_EXPORT phase_getSpeciesName(int n, int k, int lennm, char* nm) {
        try {
            string spnm = ph(n)->speciesName(k);
            int lout = min(lennm,spnm.size());
            copy(spnm.c_str(), spnm.c_str() + lout, nm);
            nm[lout] = '\0';
            return 0;
        }
        catch (CanteraError) { return -1; }
        //catch (...) {return ERR;}
    }

    int DLL_EXPORT phase_getElementName(int n, int m, int lennm, char* nm) {
        try {
            string elnm = ph(n)->elementName(m);
            int lout = min(lennm,elnm.size());
            copy(elnm.c_str(), elnm.c_str() + lout, nm);
            nm[lout] = '\0';
            return 0;
        }
        catch (CanteraError) { return -1; }
    }


    doublereal DLL_EXPORT phase_nAtoms(int n, int k, int m) {
        try {
            return ph(n)->nAtoms(k,m);
        }
        catch (CanteraError) { return -1; }
    }

    int DLL_EXPORT phase_addElement(int n, char* name, doublereal weight) {
        try {
            ph(n)->addElement(string(name),weight);
            return 0;
        }
        catch (CanteraError) { return -1; }
    }

//     int DLL_EXPORT phase_addSpecies(int n, char* name, int phase,
//         int ncomp, doublereal* comp, int thermoType, int ncoeffs, 
//         double* coeffs, double minTemp, double maxTemp, double refPressure,
//         doublereal charge, doublereal weight) {
//         try {
//             vector_fp cmp(ncomp);
//             copy(comp, comp + ncomp, cmp.begin());
//             vector_fp c(ncoeffs);
//             copy(coeffs, coeffs + ncoeffs, c.begin());
//             ph(n)->addSpecies(string(name), phase, cmp,
//                 thermoType, c, minTemp, maxTemp, refPressure, 
//                 charge, weight);
//             return 0;
//         }
//         catch (CanteraError) { return -1; }
//         catch (...) {return ERR;}
//     }
        


    //-------------- Thermo --------------------//

    //    int DLL_EXPORT newThermo(int eos, int ph, int sptherm) {
    //    return Storage::storage()->addNewThermo(eos, ph, sptherm);
    // }

    int DLL_EXPORT th_thermoIndex(char* id) {
        return thermo_index(id);
    }

//     int DLL_EXPORT newThermo(char* model) {
//         try {
//             string m = string(model);
//             thermo_t* th = newThermoPhase(m);
//             return Storage::storage()->addThermo(th);
//         }
//         catch (CanteraError) { return -1; }
//     }

    int DLL_EXPORT newThermoFromXML(int mxml) {
        try {
            XML_Node* x = _xml(mxml);
            thermo_t* th = newPhase(*x);
            return Storage::storage()->addThermo(th);
        }
        catch (CanteraError) { return -1; }
    }

    //int DLL_EXPORT th_phase(int n) {
    //    return th(n)->phase().index();
    // }

    int DLL_EXPORT th_nSpecies(int n) {
        return th(n)->nSpecies();
    }

    int DLL_EXPORT th_eosType(int n) {
        return th(n)->eosType();
    }

    double DLL_EXPORT th_enthalpy_mole(int n) {
        try {return th(n)->enthalpy_mole();}
        catch (CanteraError) {return DERR;}
    }

    double DLL_EXPORT th_intEnergy_mole(int n) {
        try {return th(n)->intEnergy_mole();}
        catch (CanteraError) {return DERR;}
    }

    double DLL_EXPORT th_entropy_mole(int n) {
        try {return th(n)->entropy_mole();}
        catch (CanteraError) {return DERR;}
    }

    double DLL_EXPORT th_gibbs_mole(int n) {
        try {return th(n)->gibbs_mole();}
        catch (CanteraError) {return DERR;}
    }

    double DLL_EXPORT th_cp_mole(int n) {
        try {return th(n)->cp_mole();}
        catch (CanteraError) {return DERR;}
    }

    double DLL_EXPORT th_cv_mole(int n) {
        try {return th(n)->cv_mole();}
        catch (CanteraError) {return DERR;}
    }

    double DLL_EXPORT th_pressure(int n) {
        try {return th(n)->pressure();}
        catch (CanteraError) {return DERR;}
    }

    double DLL_EXPORT th_enthalpy_mass(int n) {
        try {return th(n)->enthalpy_mass();}
        catch (CanteraError) {return DERR;}
    }

    double DLL_EXPORT th_intEnergy_mass(int n) {
        try {return th(n)->intEnergy_mass();}
        catch (CanteraError) {return DERR;}
    }

    double DLL_EXPORT th_entropy_mass(int n) {
        try {return th(n)->entropy_mass();}
        catch (CanteraError) {return DERR;}
    }

    double DLL_EXPORT th_gibbs_mass(int n) {
        try {return th(n)->gibbs_mass();}
        catch (CanteraError) {return DERR;}
    }

    double DLL_EXPORT th_cp_mass(int n) {
        try {return th(n)->cp_mass();}
        catch (CanteraError) {return DERR;}
    }

    double DLL_EXPORT th_cv_mass(int n) {
        try {return th(n)->cv_mass();}
        catch (CanteraError) {return DERR;}
    }

    int DLL_EXPORT th_chemPotentials(int n, int lenm, double* murt) {
        thermo_t* thrm = th(n);
        int nsp = thrm->nSpecies();
        if (lenm >= nsp) {
            thrm->getChemPotentials(murt);
            return 0;
        }
        else
            return -10;
    }

    int DLL_EXPORT th_elementPotentials(int n, int lenm, double* lambda) {
        thermo_t* thrm = th(n);
        int nel = thrm->nElements();
        if (lenm >= nel) {
            equilibrate(*thrm, "TP");
            thrm->getElementPotentials(lambda);
            return 0;
        }
        else
            return -10;
    }

    int DLL_EXPORT th_setPressure(int n, double p) {
        try {
            if (p < 0.0) throw CanteraError("th_setPressure",
                "pressure cannot be negative");
            th(n)->setPressure(p);
            return 0; 
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT th_set_HP(int n, double* vals) {
        try { 
            if (vals[1] < 0.0) 
                throw CanteraError("th_set_HP",
                    "pressure cannot be negative");
            th(n)->setState_HP(vals[0],vals[1]);
            if (th(n)->temperature() < 0.0) 
                throw CanteraError("th_set_HP",
                    "temperature cannot be negative");
            return 0; 
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT th_set_UV(int n, double* vals) {
        try { 
            if (vals[1] < 0.0) 
                throw CanteraError("th_set_UV",
                    "specific volume cannot be negative");
            th(n)->setState_UV(vals[0],vals[1]);
            if (th(n)->temperature() < 0.0) 
                throw CanteraError("th_set_UV",
                    "temperature cannot be negative");
            return 0; 
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT th_set_SV(int n, double* vals) {
        try { 
            th(n)->setState_SV(vals[0],vals[1]);
            return 0; 
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT th_set_SP(int n, double* vals) {
        try { 
            th(n)->setState_SP(vals[0],vals[1]);
            return 0;
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT th_equil(int n, int XY) {
        try { 
            equilibrate(*th(n), XY); return 0; 
        }
        catch (CanteraError) {return -1;}
    }

    doublereal DLL_EXPORT th_refPressure(int n) {
        return th(n)->refPressure();
    }

    doublereal DLL_EXPORT th_minTemp(int n, int k) {
        return th(n)->minTemp(k);
    }

    doublereal DLL_EXPORT th_maxTemp(int n, int k) {
        return th(n)->maxTemp(k);
    }


    int DLL_EXPORT th_getEnthalpies_RT(int n, int lenm, double* h_rt) {
        try {
            thermo_t* thrm = th(n);
            int nsp = thrm->nSpecies();
            if (lenm >= nsp) {
                thrm->getEnthalpy_RT(h_rt);
                return 0;
            }
            else
                return -10;
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT th_getEntropies_R(int n, int lenm, double* s_r) {
        try {
            thermo_t* thrm = th(n);
            int nsp = thrm->nSpecies();
            if (lenm >= nsp) {
                thrm->getEntropy_R(s_r);
                return 0;
            }
            else
                return -10;
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT th_getCp_R(int n, int lenm, double* cp_r) {
        try {
            thermo_t* thrm = th(n);
            int nsp = thrm->nSpecies();
            if (lenm >= nsp) {
                thrm->getCp_R(cp_r);
                return 0;
            }
            else
                return -10;
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT th_setElectricPotential(int n, double v) {
        th(n)->setElectricPotential(v);
        return 0;
    }

    //-------------- pure fluids ---------------//

    double DLL_EXPORT th_critTemperature(int n) {
        return purefluid(n)->critTemperature();
    }

    double DLL_EXPORT th_critPressure(int n) {
        return purefluid(n)->critPressure();
    }

    double DLL_EXPORT th_critDensity(int n) {
        return purefluid(n)->critDensity();
    }

    double DLL_EXPORT th_vaporFraction(int n) {
        return purefluid(n)->vaporFraction();
    }

    double DLL_EXPORT th_satTemperature(int n, double p) {
        try {
            return purefluid(n)->satTemperature(p);
        }
        catch (CanteraError) { return DERR; }
    }

    double DLL_EXPORT th_satPressure(int n, double t) {
        try {
            return purefluid(n)->satPressure(t);
        }
        catch (CanteraError) { return DERR; }
    }

    int DLL_EXPORT th_setState_Psat(int n, double p, double x) {
        try {
            purefluid(n)->setState_Psat(p, x);
            return 0;
        }
        catch (CanteraError) { return -1; }
    }

    int DLL_EXPORT th_setState_Tsat(int n, double t, double x) {
        try {
            purefluid(n)->setState_Tsat(t, x);
            return 0;
        }
        catch (CanteraError) { return -1; }
    }


    
    //-------------- Kinetics ------------------//

    int DLL_EXPORT newKineticsFromXML(int mxml, int iphase, 
        int neighbor1, int neighbor2, int neighbor3, 
        int neighbor4) {
        try {
            XML_Node* x = _xml(mxml);
            vector<thermo_t*> phases;
            phases.push_back(th(iphase));
            if (neighbor1 >= 0) {
                phases.push_back(th(neighbor1));
                if (neighbor2 >= 0) {
                    phases.push_back(th(neighbor2));
                    if (neighbor3 >= 0) {
                        phases.push_back(th(neighbor3));
                        if (neighbor4 >= 0) {
                            phases.push_back(th(neighbor4));
                        }
                    }
                }
            }
            Kinetics* kin = newKineticsMgr(*x, phases);
            if (kin)
                return Storage::storage()->addKinetics(kin);
            else
                return 0;
        }
        catch (CanteraError) { return -1; }
    }

    int DLL_EXPORT installRxnArrays(int pxml, int ikin, 
        char* default_phase) {
        try {
            XML_Node* p = _xml(pxml);
            kinetics_t* k = kin(ikin);
            string defphase = string(default_phase);
            installReactionArrays(*p, *k, defphase);
            return 0;
        }
        catch (CanteraError) { return -1; }
    }

    //-------------------------------------
    int DLL_EXPORT kin_type(int n) {
        return kin(n)->type();
    }

    int DLL_EXPORT kin_start(int n, int p) {
        return kin(n)->start(p);
    }

    int DLL_EXPORT kin_speciesIndex(int n, const char* nm, const char* ph) {
        return kin(n)->kineticsSpeciesIndex(string(nm), string(ph));
    }

    //---------------------------------------

    int DLL_EXPORT kin_nSpecies(int n) {
        return kin(n)->nTotalSpecies();
    }

    int DLL_EXPORT kin_nReactions(int n) {
        return kin(n)->nReactions();
    }

    double DLL_EXPORT kin_reactantStoichCoeff(int n, int k, int i) {
        return kin(n)->reactantStoichCoeff(k,i);
    }

    double DLL_EXPORT kin_productStoichCoeff(int n, int k, int i) {
        return kin(n)->productStoichCoeff(k,i);
    }

    int DLL_EXPORT kin_reactionType(int n, int i) {
        return kin(n)->reactionType(i);
    }

    int DLL_EXPORT kin_getFwdRatesOfProgress(int n, int len, double* fwdROP) {
        Kinetics* k = kin(n);
        try {
            if (len >= k->nReactions()) {
                k->getFwdRatesOfProgress(fwdROP);
                return 0;
            }
            else 
                return ERR;
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT kin_getRevRatesOfProgress(int n, int len, double* revROP) {
        Kinetics* k = kin(n);
        try {
            if (len >= k->nReactions()) {
                k->getRevRatesOfProgress(revROP);
                return 0;
            }
            else 
                return ERR;
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT kin_isReversible(int n, int i) {
        return (int)kin(n)->isReversible(i);
    }

    int DLL_EXPORT kin_getNetRatesOfProgress(int n, int len, double* netROP) {
        try {
            Kinetics* k = kin(n);
            if (len >= k->nReactions()) {
                k->getNetRatesOfProgress(netROP);
                return 0;
            }
            else 
                return ERR;
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT kin_getFwdRateConstants(int n, int len, double* kfwd) {
        try {
            Kinetics* k = kin(n);
            if (len >= k->nReactions()) {
                k->getFwdRateConstants(kfwd);
                return 0;
            }
            else 
                return ERR;
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT kin_getRevRateConstants(int n, int doIrreversible, int len, double* krev) {
        try {
            Kinetics* k = kin(n);
            bool doirrev = false;
            if (doIrreversible != 0) doirrev = true;
            if (len >= k->nReactions()) {
                k->getRevRateConstants(krev, doirrev);
                return 0;
            }
            else 
                return ERR;
        }
        catch (CanteraError) {return -1;}
    }


    int DLL_EXPORT kin_getActivationEnergies(int n, int len, double* E) {
        try {
            Kinetics* k = kin(n);
            if (len >= k->nReactions()) {
                k->getActivationEnergies(E);
                return 0;
            }
            else 
                return ERR;
        }
        catch (CanteraError) {return -1;}
    }


    int DLL_EXPORT kin_getCreationRates(int n, int len, double* cdot) {
        try {
            Kinetics* k = kin(n);
            if (len >= k->nTotalSpecies()) {
                k->getCreationRates(cdot);
                return 0;
            }
            else 
                return ERR;
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT kin_getDestructionRates(int n, int len, double* ddot) {
        try {
            Kinetics* k = kin(n);
            if (len >= k->nTotalSpecies()) {
                k->getDestructionRates(ddot);
                return 0;
            }
            else 
                return ERR;
        }
        catch (CanteraError) {return -1;}
        //catch (...) {return ERR;}
    }

    int DLL_EXPORT kin_getNetProductionRates(int n, int len, double* wdot) {
        try {
            Kinetics* k = kin(n);
            if (len >= k->nTotalSpecies()) {            
                k->getNetProductionRates(wdot);
                return 0;
            }
            else 
                return ERR;
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT kin_getSourceTerms(int n, int len, double* ydot) {
        try {
            Kinetics* k = kin(n);
            ThermoPhase* p = &k->thermo();
            const vector_fp& mw = p->molecularWeights();
            int nsp = mw.size();
            double rrho = 1.0/p->density();
            if (len >= nsp) {            
                k->getNetProductionRates(ydot);
                multiply_each(ydot, ydot + nsp, mw.begin());
                scale(ydot, ydot + nsp, ydot, rrho);
                return 0;
            }
            else 
                return ERR;
        }
        catch (CanteraError) {return -1;}
    }

    double DLL_EXPORT kin_multiplier(int n, int i) {
        return kin(n)->multiplier(i);
    }

    int DLL_EXPORT kin_phase(int n, int i) {
        return thermo_index(kin(n)->thermo(i).id());
    }

    int DLL_EXPORT kin_getEquilibriumConstants(int n, int len, double* kc) {
        try {
            Kinetics* k = kin(n);
            if (len >= k->nReactions()) {
                k->getEquilibriumConstants(kc);
                return 0;
            }
            else 
                return ERR;
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT kin_getReactionString(int n, int i, int len, char* buf) {
        try {
            Kinetics* k = kin(n);
            string r = k->reactionString(i);
            int lout = min(len,r.size());
            copy(r.c_str(), r.c_str() + lout, buf);
            buf[lout] = '\0';
            return 0;
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT kin_setMultiplier(int n, int i, double v) {
        try {
            if (v >= 0.0) {
                kin(n)->setMultiplier(i,v);
                return 0;
            }
            else return ERR;
        }
        catch (CanteraError) {return -1;}
    }

    int DLL_EXPORT kin_advanceCoverages(int n, double tstep) {
        try {
            Kinetics* k = kin(n);
            if (k->type() == cInterfaceKinetics) {
                ((InterfaceKinetics*)k)->advanceCoverages(tstep);
            }
            else {
                throw CanteraError("kin_advanceCoverages",
                    "wrong kinetics manager type");
            }
            return 0;
        }
        catch (CanteraError) {return -1;}
    }

    //------------------- Transport ---------------------------

    int DLL_EXPORT newTransport(char* model,  
        int ith, int loglevel) {
        string mstr = string(model);
        thermo_t* t = th(ith);
        try {
            Transport* tr = newTransportMgr(mstr,t, loglevel);
            return Storage::storage()->addTransport(tr);
        }
        catch (CanteraError) { return -1; }
    }
    
    double DLL_EXPORT trans_viscosity(int n) {
        try {return trans(n)->viscosity();}
        catch (CanteraError) { return -1.0; }
    }

    double DLL_EXPORT trans_thermalConductivity(int n) {
        try {return trans(n)->thermalConductivity();}
        catch (CanteraError) { return -1.0; }
    }

    int DLL_EXPORT trans_getThermalDiffCoeffs(int n, int ldt, double* dt) {
        try { trans(n)->getThermalDiffCoeffs(dt); return 0; }
        catch (CanteraError) { return -1; }
    }

    int DLL_EXPORT trans_getMixDiffCoeffs(int n, int ld, double* d) {
        try { trans(n)->getMixDiffCoeffs(d); return 0;}
        catch (CanteraError) { return -1; }
    }

    int DLL_EXPORT trans_getBinDiffCoeffs(int n, int ld, double* d) {
        try { trans(n)->getBinaryDiffCoeffs(ld,d); return 0;}
        catch (CanteraError) { return -1; }
    }

    int DLL_EXPORT trans_getMultiDiffCoeffs(int n, int ld, double* d) {
        try { trans(n)->getMultiDiffCoeffs(ld,d); return 0;}
        catch (CanteraError) { return -1; }
    }

    int DLL_EXPORT trans_setParameters(int n, int type, int k, double* d) {
        try { trans(n)->setParameters(type, k, d); return 0;}
        catch (CanteraError) { return -1; }
    }

    int DLL_EXPORT trans_getMolarFluxes(int n, const double* state1,
        const double* state2, double delta, double* fluxes) {
        try { 
            trans(n)->getMolarFluxes(state1, state2, delta, fluxes); 
            return 0;
        }
        catch (CanteraError) { return -1; }
    }

    //-------------------- Functions ---------------------------

    int DLL_EXPORT import_phase(int nth, int nxml, char* id) {
        thermo_t* thrm = th(nth);
        XML_Node* node = _xml(nxml);
        string idstr = string(id);
        try {
            importPhase(*node, thrm);
            return 0;
        }
        catch (CanteraError) { return -1; }
    }

    int DLL_EXPORT import_kinetics(int nxml, char* id, 
        int nphases, integer* ith, int nkin) {
        vector<thermo_t*> phases;
        for (int i = 0; i < nphases; i++) {
            phases.push_back(th(ith[i]));
        }
        XML_Node* node = _xml(nxml);
        Kinetics* k = kin(nkin);
        string idstr = string(id);
        try {
            importKinetics(*node, phases, k);
            return 0;
        }
        catch (CanteraError) { return -1; }
    }


    int DLL_EXPORT phase_report(int nth, 
        int ibuf, char* buf, int show_thermo) {
        try {
            bool stherm = (show_thermo != 0);
            string s = report(*th(nth), stherm);
            if (int(s.size()) > ibuf - 1) {
                return -(static_cast<int>(s.size()) + 1);
            }
            copy(s.begin(), s.end(), buf);
            buf[s.size() - 1] = '\0';
            return 0;
            
        }
        catch (CanteraError) { return -1; }
    }

    int DLL_EXPORT getCanteraError(int buflen, char* buf) {
        string e; 
        e = lastErrorMessage();
        if (buflen > 0) {
            int n = min(static_cast<int>(e.size()), buflen-1);
            copy(e.begin(), e.begin() + n, buf);
            buf[min(n, buflen-1)] = '\0';
        }
        return int(e.size());
    }

    int DLL_EXPORT showCanteraErrors() {
        showErrors();
        return 0;
    }

    int DLL_EXPORT addCanteraDirectory(int buflen, char* buf) {
        addDirectory(string(buf));
        return 0;
    }

    int DLL_EXPORT setLogWriter(void* logger) {
        Logger* logwriter = (Logger*)logger;
        setLogger(logwriter);
    }

    int DLL_EXPORT readlog(int n, char* buf) {
        string s;
        writelog("function readlog is deprecated!");
        //getlog(s);
        int nlog = static_cast<int>(s.size());
        if (n < 0) return nlog; 
        int nn = min(n-1, nlog);
        copy(s.begin(), s.begin() + nn,
            buf);
        buf[min(nlog, n-1)] = '\0';
        //clearlog();
        return 0;

    } 
    int DLL_EXPORT clearStorage() {
        try {
            Storage::storage()->clear();
            return 0;
        }
        catch (CanteraError) {
            return -1;
        }
    }

    int DLL_EXPORT delThermo(int n) {
        try {
            Storage::storage()->deleteThermo(n);
            return 0;
        }
        catch (CanteraError) {
            return -1;
        }
    }

    int DLL_EXPORT delKinetics(int n) {
        Storage::storage()->deleteKinetics(n);
        return 0;
    }

    int DLL_EXPORT delTransport(int n) {
        Storage::storage()->deleteTransport(n);
        return 0;
    }

    int DLL_EXPORT buildSolutionFromXML(char* src, int ixml, char* id, 
        int ith, int ikin) {

        XML_Node* root = 0;
        if (ixml > 0) root = _xml(ixml);

        thermo_t* t = th(ith);
        kinetics_t* k = kin(ikin);

        Kinetics& kin = *k;
        XML_Node *x, *r=0;
        if (root) r = &root->root();
        x = get_XML_Node(string(src), r);
        //x = find_XML(string(src), r, string(id), "", "phase");
        if (!x) return false;
        importPhase(*x, t);
        kin.addPhase(*t);
        kin.init();
        installReactionArrays(*x, kin, x->id());
        t->setState_TP(300.0, OneAtm);
        if (r) { 
            if (&x->root() != &r->root()) delete &x->root();
        }
        else delete &x->root();
        return 0;
    }


//    int DLL_EXPORT ck_to_cti(char* in_file, char* db_file,
//        char* tr_file, char* id_tag) {
//        return pip::convert_ck(in_file, db_file, tr_file, id_tag);
//    }

}
