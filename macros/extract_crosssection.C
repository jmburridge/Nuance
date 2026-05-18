// extract_xsec.C
//
// Reads a hadd'd ROOT file and extracts the differential neutrino cross
// section dSigma/dE per flavour as a function of neutrino energy.
//
// Each energy bin:
//   dSigma/dE [bin i] = N_events[bin i] / Phi[bin i]   [cm^2/MeV]
//                     * 1e36                             [pb/MeV]
//
// Trees used:
//   h50  : branch e_neutrino (Float_t)    -- reconstructed neutrino energy
//   h3   : branch p_neutrino[4] (Float_t) -- true 4-momentum; [3] = energy
//          branch neutrino      (Int_t)     -- PDG code
//
// Flux histograms in the same ROOT file (outside the trees):
//   h10007001  PDG  14  (nu_mu)
//   h10007002  PDG -14  (nu_mu_bar)
//   h10007003  PDG  12  (nu_e)
//   h10007004  PDG -12  (nu_e_bar)
//   Units: nu / (50 MeV / cm^2 / POT)  -- one bin = one 50 MeV slice
//
// NOTE ON HADD:
//   Files were combined from many 50-event sub-files so h50 and h3 entry
//   indices do NOT correspond.  We match events by energy value using a
//   map built from h3 (p_neutrino[3] -> PDG code).
//
// PDG codes:  14 = nu_mu   -14 = nu_mu_bar   12 = nu_e   -12 = nu_e_bar
// -----------------------------------------------------------------------

#include <map>
#include <iostream>

// 1 pb = 1e-36 cm^2  =>  to convert cm^2 -> pb multiply by 1e36
static const Float_t CM2_TO_PB = 1; //1.0e36; //levae this be. 
static const Float_t POT = 1;//1.0e20; // protons on target for this run

void extract_crosssection(const char* filename = "../output/root/first_test_run/combined/NUANCE_events.root")
{
    // ------------------------------------------------------------------ //
    //  Open file
    // ------------------------------------------------------------------ //
    TFile* f = TFile::Open(filename, "READ");
    if (!f || f->IsZombie()) {
        std::cerr << "ERROR: cannot open " << filename << std::endl;
        return;
    }

    // ------------------------------------------------------------------ //
    //  Step 1 -- load the flux histograms
    //            These define the energy binning we will use throughout.
    // ------------------------------------------------------------------ //
    TH1* hFlux_numu    = (TH1*)f->Get("h10007001");  // convert from GeV to MeV
    TH1* hFlux_numubar = (TH1*)f->Get("h10007002");
    TH1* hFlux_nue     = (TH1*)f->Get("h10007003");
    TH1* hFlux_nuebar  = (TH1*)f->Get("h10007004");
    //now scaled them all to MeV
    hFlux_numu   ->Scale(1000.0*POT); // 1 GeV = 1000 MeV
    hFlux_numubar->Scale(1000.0*POT);
    hFlux_nue    ->Scale(1000.0*POT);
    hFlux_nuebar ->Scale(1000.0*POT);

    if (!hFlux_numu || !hFlux_numubar || !hFlux_nue || !hFlux_nuebar) {
        std::cerr << "ERROR: could not find one or more flux histograms." << std::endl;
        return;
    }

    // Grab the axis from the nu_mu flux histogram to use as our standard
    // binning. All four flux histograms should share the same binning.
    TAxis* axis    = hFlux_numu->GetXaxis();
    int    nBins   = axis->GetNbins();
    double eMin    = axis->GetXmin()*1000.0; // convert from GeV to MeV
    double eMax    = axis->GetXmax()*1000.0; // convert from GeV to MeV

    std::cout << "Flux histogram binning: "
              << nBins << " bins,  " << eMin << " to " << eMax << " MeV" << std::endl;

    // ------------------------------------------------------------------ //
    //  Step 2 -- create event-count histograms with the SAME binning
    //            as the flux histograms (one per flavour)
    // ------------------------------------------------------------------ //
    TH1D* hEvt_numu    = new TH1D("hEvt_numu",    "Events nu_mu",    nBins, eMin, 3000.0);
    TH1D* hEvt_numubar = new TH1D("hEvt_numubar", "Events nu_mu_bar",nBins, eMin, 3000.0);
    TH1D* hEvt_nue     = new TH1D("hEvt_nue",     "Events nu_e",     nBins, eMin, 3000.0);
    TH1D* hEvt_nuebar  = new TH1D("hEvt_nuebar",  "Events nu_e_bar", nBins, eMin, 3000.0);

    // Prevent ROOT from deleting these when the file is closed
    hEvt_numu   ->SetDirectory(0);
    hEvt_numubar->SetDirectory(0);
    hEvt_nue    ->SetDirectory(0);
    hEvt_nuebar ->SetDirectory(0);

    // ------------------------------------------------------------------ //
    //  Step 3 -- get trees
    // ------------------------------------------------------------------ //
    TTree* h3  = (TTree*)f->Get("h3");
    TTree* h50 = (TTree*)f->Get("h50");

    if (!h3 || !h50) {
        std::cerr << "ERROR: could not find h3 or h50 in file." << std::endl;
        return;
    }

    // ------------------------------------------------------------------ //
    //  Step 4 -- build a map: true_energy (p_neutrino[3]) -> PDG code
    //            This handles the hadd'd entry-order mismatch between trees.
    // ------------------------------------------------------------------ //
    Float_t p_neutrino[4];
    Int_t    pdg_code;

    h3->SetBranchAddress("p_neutrino", p_neutrino);
    h3->SetBranchAddress("neutrino",   &pdg_code);

    std::map<Float_t, Int_t> energyToPDG;

    Long64_t nH3 = h3->GetEntries();
    for (Long64_t i = 0; i < nH3; i++) {
        h3->GetEntry(i);
        energyToPDG[ p_neutrino[3] ] = pdg_code;
    }

    std::cout << "h3 entries read      : " << nH3 << std::endl;
    std::cout << "Unique energies mapped: " << energyToPDG.size() << std::endl;

    // ------------------------------------------------------------------ //
    //  Step 5 -- loop over h50, find each event's flavour, fill the
    //            corresponding event-count histogram at that energy
    // ------------------------------------------------------------------ //
    Float_t e_neutrino;
    h50->SetBranchAddress("e_neutrino", &e_neutrino);

    int count_unknown = 0;

    Long64_t nH50 = h50->GetEntries();
    for (Long64_t i = 0; i < nH50; i++) {
        h50->GetEntry(i);

        auto it = energyToPDG.find(e_neutrino);
        if (it == energyToPDG.end()) {
            count_unknown++;
            continue;
        }

        int pdg = it->second;
        if      (pdg ==  14) hEvt_numu   ->Fill(e_neutrino);
        else if (pdg == -14) hEvt_numubar->Fill(e_neutrino);
        else if (pdg ==  12) hEvt_nue    ->Fill(e_neutrino);
        else if (pdg == -12) hEvt_nuebar ->Fill(e_neutrino);
        else                 count_unknown++;
    }

    std::cout << "h50 entries read: " << nH50
              << "  unmatched: " << count_unknown << std::endl;

    // ------------------------------------------------------------------ //
    //  Step 6 -- divide event histogram by flux histogram bin by bin
    //            to get dSigma/dE in pb/MeV
    //
    //  TH1::Divide(other) does bin-by-bin:  result[i] = N[i] / Phi[i]
    //  Then multiply the whole histogram by CM2_TO_PB to convert to pb/MeV.
    //
    //  We clone the event histograms first so the raw counts are preserved.
    // ------------------------------------------------------------------ //
    TH1D* hXsec_numu    = (TH1D*)hEvt_numu   ->Clone("hXsec_numu");
    TH1D* hXsec_numubar = (TH1D*)hEvt_numubar->Clone("hXsec_numubar");
    TH1D* hXsec_nue     = (TH1D*)hEvt_nue    ->Clone("hXsec_nue");
    TH1D* hXsec_nuebar  = (TH1D*)hEvt_nuebar ->Clone("hXsec_nuebar");

    // bin-by-bin division by flux
    hXsec_numu   ->Divide(hFlux_numu);
    hXsec_numubar->Divide(hFlux_numubar);
    hXsec_nue    ->Divide(hFlux_nue);
    hXsec_nuebar ->Divide(hFlux_nuebar);

    // convert cm^2/MeV -> pb/MeV
    hXsec_numu   ->Scale(CM2_TO_PB);
    hXsec_numubar->Scale(CM2_TO_PB);
    hXsec_nue    ->Scale(CM2_TO_PB);
    hXsec_nuebar ->Scale(CM2_TO_PB);

    // set axis labels
    const char* xLabel = "Neutrino Energy [MeV]";
    const char* yLabel = "d#sigma/dE [pb/MeV]";
    hXsec_numu   ->GetXaxis()->SetTitle(xLabel);  hXsec_numu   ->GetYaxis()->SetTitle(yLabel);
    hXsec_numubar->GetXaxis()->SetTitle(xLabel);  hXsec_numubar->GetYaxis()->SetTitle(yLabel);
    hXsec_nue    ->GetXaxis()->SetTitle(xLabel);  hXsec_nue    ->GetYaxis()->SetTitle(yLabel);
    hXsec_nuebar ->GetXaxis()->SetTitle(xLabel);  hXsec_nuebar ->GetYaxis()->SetTitle(yLabel);

    //draw canvas and make log scale
    TCanvas* c = new TCanvas("c", "Cross Sections", 800, 600);
    c->SetLogy();
    hXsec_numu   ->SetLineColor(kBlue);
    hXsec_numubar->SetLineColor(kRed);
    hXsec_nue    ->SetLineColor(kGreen+2);
    hXsec_nuebar ->SetLineColor(kMagenta);
    hXsec_numu   ->Draw("HIST");
    hXsec_numubar->Draw("HIST SAME");
    hXsec_nue    ->Draw("HIST SAME");
    hXsec_nuebar ->Draw("HIST SAME");
    c->BuildLegend();
    c->SaveAs("macro_outputs/xsec_plot.png");

    // ------------------------------------------------------------------ //
    //  Step 7 -- save output histograms to a new ROOT file
    // ------------------------------------------------------------------ //
    TFile* fOut = TFile::Open("macro_outputs/xsec_output.root", "RECREATE");

    // raw event counts (useful for debugging)
    hEvt_numu   ->Write("hEvt_numu");
    hEvt_numubar->Write("hEvt_numubar");
    hEvt_nue    ->Write("hEvt_nue");
    hEvt_nuebar ->Write("hEvt_nuebar");

    // cross section distributions
    hXsec_numu   ->Write("hXsec_numu");
    hXsec_numubar->Write("hXsec_numubar");
    hXsec_nue    ->Write("hXsec_nue");
    hXsec_nuebar ->Write("hXsec_nuebar");

    fOut->Close();
    f->Close();

    std::cout << "\nOutput written to xsec_output.root" << std::endl;
    std::cout << "Histograms: hXsec_numu, hXsec_numubar, hXsec_nue, hXsec_nuebar  [pb/MeV]" << std::endl;
}