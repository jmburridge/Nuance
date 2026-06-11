// plotccqe_internalflux.C
// Plots the numu CCQE cross section using the internal NUANCE flux histogram
// (h1007001) from the same combined ROOT file as the events.
// No data release flux file used.
// Numu only.

#include <iostream>

void plotccqe_numu_secs(const char* filename = "/exp/uboone/data/users/jburridg/Nuance/NUANCE/NUANCE_event_files/output/root/PoT_Nuance_event_files/combined/combined_combined/combined_nuance_events_4.root")
{
    // ------------------------------------------------------------------ //
    //  Scale factor
    //  scale_factor = number of seconds per file
    // ------------------------------------------------------------------ //
    Float_t secs_per_file = 1e10; //only need this for the seconds scaling
    Float_t pot_per_sec   = 1;//1.17e6;       
    Int_t   num_files     = 1; //5102;   // this stays 1 as we divide by the flux which is hadded equal to the number of files 
    //flux is hadded equal to number of files so that solves that. 
    
    Float_t scale_factor  = secs_per_file; // total POT simulated across all combined files
    Float_t N_target      = 1.113e31; // number of target nucleons in detector

    std::cout << "Scale factor (total POT): " << scale_factor << std::endl;

    // ------------------------------------------------------------------ //
    //  Open file
    // ------------------------------------------------------------------ //
    TFile* f = TFile::Open(filename, "READ");
    if (!f || f->IsZombie()) {
        std::cerr << "ERROR: cannot open " << filename << std::endl;
        return;
    }

    // ------------------------------------------------------------------ //
    //  Get internal flux histogram h1007001
    //  This is the numu flux as used by NUANCE internally.
    // ------------------------------------------------------------------ //
    TH1* hFlux_internal = (TH1*)f->Get("h10007003");
    if (!hFlux_internal) {
        std::cerr << "ERROR: cannot find h10007003 in file." << std::endl;
        std::cerr << "Available keys:" << std::endl;
        f->ls();
        return;
    }
    hFlux_internal->SetDirectory(0); // detach from file so it survives after f->Close()

    std::cout << "Flux histogram h10007003 found." << std::endl;
    std::cout << "  Bins: "  << hFlux_internal->GetNbinsX() << std::endl;
    std::cout << "  X min: " << hFlux_internal->GetXaxis()->GetXmin() << std::endl;
    std::cout << "  X max: " << hFlux_internal->GetXaxis()->GetXmax() << std::endl;
    std::cout << "  Peak:  " << hFlux_internal->GetMaximum() << std::endl;

    // ------------------------------------------------------------------ //
    //  Get h3 tree
    // ------------------------------------------------------------------ //
    TTree* h3 = (TTree*)f->Get("h3");
    if (!h3) {
        std::cerr << "ERROR: cannot find h3 in file." << std::endl;
        return;
    }

    Float_t p_neutrino[4];
    Int_t   neutrino;
    Int_t   channel;
    h3->SetBranchAddress("p_neutrino", p_neutrino);
    h3->SetBranchAddress("neutrino",   &neutrino);
    h3->SetBranchAddress("channel",    &channel);

    // ------------------------------------------------------------------ //
    //  Build event histogram with same binning as the internal flux
    //  so the bin-by-bin Divide() is valid.
    // ------------------------------------------------------------------ //
    Int_t    nBins = hFlux_internal->GetNbinsX();
    Double_t xMin  = hFlux_internal->GetXaxis()->GetXmin();
    Double_t xMax  = hFlux_internal->GetXaxis()->GetXmax();

    TH1D* hEvents_numu = new TH1D("hEvents_numu",
        "Numu CCQE Events;Neutrino Energy (GeV);Events",
        nBins, xMin, xMax);

    // ------------------------------------------------------------------ //
    //  Event loop — numu CCQE only
    // ------------------------------------------------------------------ //
    Long64_t nEntries = h3->GetEntries();
    std::cout << "Looping over " << nEntries << " entries..." << std::endl;

    for (Long64_t i = 0; i < nEntries; i++) {
        h3->GetEntry(i);
        if (channel != 1)  continue; // CCQE only
        if (neutrino != 14) continue; // numu only

        Float_t energy = p_neutrino[3]; // energy in MeV
        hEvents_numu->Fill(energy / 1000.0); // convert to GeV
    }

    std::cout << "Numu CCQE events filled: " << hEvents_numu->GetEntries() << std::endl;

    // ------------------------------------------------------------------ //
    //  Normalise events to per POT per 50 MeV
    //  Rate[i] = N[i] * 0.05 / ( scale_factor * binWidth[i] )
    // ------------------------------------------------------------------ //
    const double fiftyMeV = 0.05; // GeV

    TH1D* hRate_numu = (TH1D*)hEvents_numu->Clone("hRate_numu");
    hRate_numu->SetTitle("Numu CCQE Rate;Neutrino Energy (GeV);#nu/POT/50 MeV");

    for (int i = 1; i <= hRate_numu->GetNbinsX(); i++) {
        double width = hRate_numu->GetBinWidth(i);
        double norm  = scale_factor * width / fiftyMeV;
        if (norm > 0)
            hRate_numu->SetBinContent(i, hRate_numu->GetBinContent(i) / norm);
        else
            hRate_numu->SetBinContent(i, 0);
    }

    // ------------------------------------------------------------------ //
    //  Cross section: divide rate by internal flux, then by N_target
    //  Rate [nu/POT/50MeV] / Flux [nu/cm2/POT/50MeV] = cm2
    //  Then / N_target = cm2/nucleon
    // ------------------------------------------------------------------ //
    TH1D* hXsec_numu = (TH1D*)hRate_numu->Clone("hXsec_numu");
    hXsec_numu->SetTitle("Numu CCQE Cross Section (internal flux);Neutrino Energy (GeV);#sigma (cm^{2}/nucleon)");

    hXsec_numu->Divide(hFlux_internal);

    for (int i = 1; i <= hXsec_numu->GetNbinsX(); i++) {
        hXsec_numu->SetBinContent(i, hXsec_numu->GetBinContent(i) / N_target);
    }

    // ------------------------------------------------------------------ //
    //  Draw rate plot
    // ------------------------------------------------------------------ //
    TCanvas* c1 = new TCanvas("c1_rate", "Numu CCQE Rate", 900, 650);
    c1->SetGrid();
    hRate_numu->SetLineColor(kBlue);
    hRate_numu->SetLineWidth(2);
    hRate_numu->Draw("HIST");
    c1->SaveAs("macro_outputs/analysis_plots/plotccqe_numu_secs/numu_rate_internalflux_combined_4.png");

    // ------------------------------------------------------------------ //
    //  Draw flux histogram for inspection
    // ------------------------------------------------------------------ //
    TCanvas* c2 = new TCanvas("c2_flux", "Internal Numu Flux (h10007003)", 900, 650);
    c2->SetGrid();
    hFlux_internal->SetLineColor(kRed);
    hFlux_internal->SetLineWidth(2);
    hFlux_internal->SetTitle("Internal Numu Flux (h10007003);Neutrino Energy (GeV);Flux");
    hFlux_internal->Draw("HIST");
    c2->SaveAs("macro_outputs/analysis_plots/plotccqe_numu_secs/numu_flux_internal_combined_4.png");

    // ------------------------------------------------------------------ //
    //  Draw cross section plot
    // ------------------------------------------------------------------ //
    TCanvas* c3 = new TCanvas("c3_xsec", "Numu CCQE Cross Section", 900, 650);
    c3->SetGrid();
    c3->SetLogy();
    hXsec_numu->SetLineColor(kBlue);
    hXsec_numu->SetLineWidth(2);
    hXsec_numu->Draw("HIST");
    c3->SaveAs("macro_outputs/analysis_plots/plotccqe_numu_secs/numu_xsec_internalflux_combined_4.png");

    // ------------------------------------------------------------------ //
    //  Save to ROOT file
    // ------------------------------------------------------------------ //
    TFile* fOut = TFile::Open("macro_outputs/analysis_plots/plotccqe_numu_secs/numu_internalflux_combined_4.root", "RECREATE");
    if (!fOut || fOut->IsZombie()) {
        std::cerr << "ERROR: cannot create output file." << std::endl;
        return;
    }
    hEvents_numu ->Write();
    hRate_numu   ->Write();
    hXsec_numu   ->Write();
    hFlux_internal->Write("hFlux_internal");
    fOut->Close();

    f->Close();

    std::cout << "Scale factor used: " << scale_factor << std::endl;
    std::cout << "Done. Output written to numu_internalflux_combined_4.root" << std::endl;
}