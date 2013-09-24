//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
// $Id: G4C14GEMProbability.cc,v 1.6 2009-09-15 12:54:16 vnivanch Exp $
// GEANT4 tag $Name: not supported by cvs2svn $
//
// Hadronic Process: Nuclear De-excitations
// by V. Lara (Nov 1999)
//


#include "G4C14GEMProbability.hh"

G4C14GEMProbability::G4C14GEMProbability() :
  G4GEMProbability(14,6,0.0) // A,Z,Spin
{

  ExcitEnergies.push_back(6094.2*keV);
  ExcitSpins.push_back(1.0);
  ExcitLifetimes.push_back(7e-15*s);

  ExcitEnergies.push_back(6589.8*keV);
  ExcitSpins.push_back(0.0);
  ExcitLifetimes.push_back(0.4e-12*s);

  ExcitEnergies.push_back(6728.2*keV);
  ExcitSpins.push_back(3.0);
  ExcitLifetimes.push_back(67e-12*s);

  ExcitEnergies.push_back(6902.3*keV);
  ExcitSpins.push_back(0.0);
  ExcitLifetimes.push_back(25e-15*s);

  ExcitEnergies.push_back(7012.0*keV);
  ExcitSpins.push_back(2.0);
  ExcitLifetimes.push_back(9e-15*s);

  ExcitEnergies.push_back(8318.3*keV);
  ExcitSpins.push_back(2.0);
  ExcitLifetimes.push_back(hbar_Planck*std::log(2.0)/(3.4*keV));

  ExcitEnergies.push_back(9799*keV);
  ExcitSpins.push_back(1.0);
  ExcitLifetimes.push_back(hbar_Planck*std::log(2.0)/(45*keV));

  ExcitEnergies.push_back(10437*keV);
  ExcitSpins.push_back(3.0);
  ExcitLifetimes.push_back(hbar_Planck*std::log(2.0)/(16*keV));

  ExcitEnergies.push_back(10509*keV);
  ExcitSpins.push_back(4.0);
  ExcitLifetimes.push_back(hbar_Planck*std::log(2.0)/(26*keV));

  ExcitEnergies.push_back(11306*keV);
  ExcitSpins.push_back(1.0);
  ExcitLifetimes.push_back(hbar_Planck*std::log(2.0)/(46*keV));

  ExcitEnergies.push_back(11397*keV);
  ExcitSpins.push_back(3.0);
  ExcitLifetimes.push_back(hbar_Planck*std::log(2.0)/(22*keV));

  ExcitEnergies.push_back(11667*keV);
  ExcitSpins.push_back(5.0);
  ExcitLifetimes.push_back(hbar_Planck*std::log(2.0)/(20*keV));

  ExcitEnergies.push_back(12860*keV);
  ExcitSpins.push_back(5.0);
  ExcitLifetimes.push_back(hbar_Planck*std::log(2.0)/(30*keV));

  ExcitEnergies.push_back(12964*keV);
  ExcitSpins.push_back(4.0);
  ExcitLifetimes.push_back(hbar_Planck*std::log(2.0)/(30*keV));

  ExcitEnergies.push_back(14667*keV);
  ExcitSpins.push_back(6.0);
  ExcitLifetimes.push_back(hbar_Planck*std::log(2.0)/(57*keV));

  SetExcitationEnergiesPtr(&ExcitEnergies);
  SetExcitationSpinsPtr(&ExcitSpins);
  SetExcitationLifetimesPtr(&ExcitLifetimes);
}


G4C14GEMProbability::G4C14GEMProbability(const G4C14GEMProbability &) : G4GEMProbability()
{
  throw G4HadronicException(__FILE__, __LINE__, "G4C14GEMProbability::copy_constructor meant to not be accessable");
}




const G4C14GEMProbability & G4C14GEMProbability::
operator=(const G4C14GEMProbability &)
{
  throw G4HadronicException(__FILE__, __LINE__, "G4C14GEMProbability::operator= meant to not be accessable");
  return *this;
}


G4bool G4C14GEMProbability::operator==(const G4C14GEMProbability &) const
{
  return false;
}

G4bool G4C14GEMProbability::operator!=(const G4C14GEMProbability &) const
{
  return true;
}


