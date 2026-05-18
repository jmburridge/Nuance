//plotting h100 from NUANCE root files. 

#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TLegend.h"
#include "TAxis.h"
#include "TStyle.h"
#include <vector>
#include <iostream>

void plot_histograms_xs(const char* filename = "../output/root/first_test_run/uncombined/events_2.root") {

    // --- Open file and get tree ---
    TFile* f = TFile::Open(filename);
    if (!f || f->IsZombie()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return;
    }

    TTree* h100 = nullptr;
    f->GetObject("h100", h100);
    if (!h100) {
        std::cerr << "Error: Tree 'h100' not found." << std::endl;
        return;
    }

    // --- Declare branch variables ---

    // Electron neutrino (nue) cross sections
    Double_t nue_pfcc;  // nue, pi+ final state, CC
    Double_t nue_pfnc;  // nue, pi+ final state, NC
    Double_t nue_ecc;  // nue, elastic, CC
    Double_t nue_enc;  // nue, elastic, NC
    Double_t nue_ccc;  // nue, coherent, CC
    Double_t nue_cnc;  // nue, coherent, NC
    Double_t nue_nbcc;  // nue, nu-bar, CC
    Double_t nue_nbnc;  // nue, nu-bar, NC
    Double_t nue_pbcc;
    Double_t nue_pbnc;

    // Anti-electron neutrino (anue) cross sections
    Double_t anue_pfc;
    Double_t anue_pfn;
    Double_t anue_ecc;
    Double_t anue_enc;
    Double_t anue_ccc;
    Double_t anue_cnc;
    Double_t anue_nbc;
    Double_t anue_nbn;
    Double_t anue_pbc;
    Double_t anue_pbn;

    // num branches (partially visible — add more as needed)
    Double_t num_pfcc;
    Double_t num_pfnc;

    // --- Set branch addresses ---
  
    h100->SetBranchAddress("nue_pfcc", &nue_pfcc);
    h100->SetBranchAddress("nue_pfnc", &nue_pfnc);
    h100->SetBranchAddress("nue_ecc",  &nue_ecc);
    h100->SetBranchAddress("nue_enc",  &nue_enc);
    h100->SetBranchAddress("nue_ccc",  &nue_ccc);
    h100->SetBranchAddress("nue_cnc",  &nue_cnc);
    h100->SetBranchAddress("nue_nbcc", &nue_nbcc);
    h100->SetBranchAddress("nue_nbnc", &nue_nbnc);
    h100->SetBranchAddress("nue_pbcc", &nue_pbcc);
    h100->SetBranchAddress("nue_pbnc", &nue_pbnc);
    h100->SetBranchAddress("anue_pfc", &anue_pfc);
    h100->SetBranchAddress("anue_pfn", &anue_pfn);
    h100->SetBranchAddress("anue_ecc", &anue_ecc);
    h100->SetBranchAddress("anue_enc", &anue_enc);
    h100->SetBranchAddress("anue_ccc", &anue_ccc);
    h100->SetBranchAddress("anue_cnc", &anue_cnc);
    h100->SetBranchAddress("anue_nbc", &anue_nbc);
    h100->SetBranchAddress("anue_nbn", &anue_nbn);
    h100->SetBranchAddress("anue_pbc", &anue_pbc);
    h100->SetBranchAddress("anue_pbn", &anue_pbn);
    h100->SetBranchAddress("num_pfcc", &num_pfcc);
    h100->SetBranchAddress("num_pfnc", &num_pfnc);

    // --- Fill vectors ---
   
    std::vector<Float_t> vNuePfcc, vNuePfnc, vNueEcc, vNueEnc;
    std::vector<Float_t> vNueCcc,  vNueCnc,  vNueNbcc,vNueNbnc;
    std::vector<Float_t> vNuePbcc, vNuePbnc;
    std::vector<Float_t> vAnuePfc, vAnuePfn, vAnueEcc, vAnueEnc;
    std::vector<Float_t> vAnueCcc, vAnueCnc, vAnueNbc, vAnueNbn;
    std::vector<Float_t> vAnuePbc, vAnuePbn;
    std::vector<Float_t> vNumPfcc, vNumPfnc;

    Long64_t nEntries = h100->GetEntries();
    for (Long64_t i = 0; i < nEntries; i++) {
        h100->GetEntry(i);
      
        vNuePfcc.push_back(nue_pfcc); vNuePfnc.push_back(nue_pfnc);
        vNueEcc .push_back(nue_ecc);  vNueEnc .push_back(nue_enc);
        vNueCcc .push_back(nue_ccc);  vNueCnc .push_back(nue_cnc);
        vNueNbcc.push_back(nue_nbcc); vNueNbnc.push_back(nue_nbnc);
        vNuePbcc.push_back(nue_pbcc); vNuePbnc.push_back(nue_pbnc);
        vAnuePfc.push_back(anue_pfc); vAnuePfn.push_back(anue_pfn);
        vAnueEcc.push_back(anue_ecc); vAnueEnc.push_back(anue_enc);
        vAnueCcc.push_back(anue_ccc); vAnueCnc.push_back(anue_cnc);
        vAnueNbc.push_back(anue_nbc); vAnueNbn.push_back(anue_nbn);
        vAnuePbc.push_back(anue_pbc); vAnuePbn.push_back(anue_pbn);
        vNumPfcc.push_back(num_pfcc); vNumPfnc.push_back(num_pfnc);

        std::cout << "Entry " << i << ": " 
                << ", nue_ecc=" << nue_ecc
                << ", nue_enc=" << nue_enc
                << std::endl;
        
    }

   
   
}
