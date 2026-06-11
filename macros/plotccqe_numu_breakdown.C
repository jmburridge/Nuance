// plotccqe_stepbystep.C
//
// Plots the numu CCQE rate step by step, producing a plot at each
// normalisation stage so the effect of each step can be inspected.
//
// Stage 0: Raw event counts
// Stage 1: Divided by total POT
// Stage 2: Divided by bin width (gives per GeV)
// Stage 3: Multiplied by 0.05 GeV (renormalised to per 50 MeV)
//
// All histograms use the same variable-width bin edges as the original macro.

#include <iostream>

void plotccqe_numu_breakdown(const char* filename = "/exp/uboone/data/users/jburridg/Nuance/NUANCE/NUANCE_event_files/output/root/PoT_Nuance_event_files/combined/combined_combined/combined_nuance_events_4.root")
{
    // ------------------------------------------------------------------ //
    //  Scale factors
    // ------------------------------------------------------------------ //
    Float_t secs_per_file = 1e10;
    Float_t pot_per_sec   = 1.17e6;         
    Int_t   num_files     = 5102;         // UPDATE: actual number of hadd'd files
    Float_t scale_factor  = pot_per_sec * secs_per_file * num_files;
    const double fiftyMeV = 0.05;      // GeV

    std::cout << "Total POT (scale_factor): " << scale_factor << std::endl;

    // ------------------------------------------------------------------ //
    //  Open file and get tree
    // ------------------------------------------------------------------ //
    TFile* f = TFile::Open(filename, "READ");
    if (!f || f->IsZombie()) {
        std::cerr << "ERROR: cannot open " << filename << std::endl;
        return;
    }

    TTree* h3 = (TTree*)f->Get("h3");
    if (!h3) {
        std::cerr << "ERROR: cannot find h3." << std::endl;
        return;
    }

    Float_t p_neutrino[4];
    Int_t   neutrino;
    Int_t   channel;
    h3->SetBranchAddress("p_neutrino", p_neutrino);
    h3->SetBranchAddress("neutrino",   &neutrino);
    h3->SetBranchAddress("channel",    &channel);

    // ------------------------------------------------------------------ //
    //  Bin edges
    // ------------------------------------------------------------------ //
    const Double_t binEdges[] = {0.00,
        0.125, 0.180, 0.240, 0.300, 0.350, 0.425, 0.480, 0.540, 0.600,
        0.655, 0.755, 0.785, 0.845, 0.900, 0.955, 1.025, 1.085, 1.145,
        1.200, 1.270, 1.320, 1.380, 1.435, 1.500, 1.560, 1.620, 1.680,
        1.740, 1.800, 1.860, 1.920, 1.980, 2.050, 2.100, 2.166, 2.225,
        2.280, 2.340, 2.400, 2.460, 2.525, 2.575, 2.600, 2.650, 2.700,
        2.750, 2.800, 2.850, 2.900, 2.950, 3.000, 3.050, 3.100, 3.150,
        3.200, 3.250, 3.300, 3.350, 3.400, 3.450, 3.500, 3.550, 3.600,
        3.650, 3.700, 3.750, 3.800, 3.850, 3.900, 3.950, 4.000, 4.050,
        4.100, 4.150, 4.200, 4.250, 4.300, 4.350, 4.400, 4.450, 4.500,
        4.550, 4.600, 4.650, 4.700, 4.750, 4.800, 4.850, 4.900, 4.950,
        5.000, 5.050, 5.100, 5.150, 5.200, 5.250, 5.300, 5.350, 5.400,
        5.450, 5.500, 5.550, 5.600, 5.650, 5.700, 5.750, 5.800, 5.850,
        5.900, 5.950, 6.000
    };
    const Int_t nBins = sizeof(binEdges)/sizeof(binEdges[0]) - 1;

    // ------------------------------------------------------------------ //
    //  Stage 0: Raw event counts
    // ------------------------------------------------------------------ //
    TH1D* h0 = new TH1D("h0_raw",
        "Stage 0: Raw CCQE event counts (numu);Neutrino Energy (GeV);Event count",
        nBins, binEdges);

    for (Long64_t i = 0; i < h3->GetEntries(); i++) {
        h3->GetEntry(i);
        if (channel != 1)   continue;
        if (neutrino != 14) continue;
        h0->Fill(p_neutrino[3] / 1000.0); // MeV -> GeV
    }

    std::cout << "Stage 0: " << h0->GetEntries() << " numu CCQE events filled." << std::endl;

    // ------------------------------------------------------------------ //
    //  Stage 1: Divide by total POT
    //  Units: events / POT
    // ------------------------------------------------------------------ //
    TH1D* h1 = (TH1D*)h0->Clone("h1_per_pot");
    h1->SetTitle("Stage 1: Divided by total POT (numu);Neutrino Energy (GeV);Events / POT");

    for (int i = 1; i <= h1->GetNbinsX(); i++)
        h1->SetBinContent(i, h1->GetBinContent(i) / scale_factor);

    // ------------------------------------------------------------------ //
    //  Stage 2: Divide by bin width
    //  Units: events / POT / GeV
    // ------------------------------------------------------------------ //
    TH1D* h2 = (TH1D*)h1->Clone("h2_per_pot_per_gev");
    h2->SetTitle("Stage 2: Divided by bin width (numu);Neutrino Energy (GeV);Events / POT / GeV");

    for (int i = 1; i <= h2->GetNbinsX(); i++) {
        double w = h2->GetBinWidth(i);
        if (w > 0)
            h2->SetBinContent(i, h2->GetBinContent(i) / w);
    }

    // ------------------------------------------------------------------ //
    //  Stage 3: Multiply by 0.05 GeV to renormalise to per 50 MeV
    //  Units: events / POT / 50 MeV   <- this is the final rate
    // ------------------------------------------------------------------ //
    TH1D* h3_rate = (TH1D*)h2->Clone("h3_rate");
    h3_rate->SetTitle("Stage 3: Final rate (numu);Neutrino Energy (GeV);#nu / POT / 50 MeV");

    for (int i = 1; i <= h3_rate->GetNbinsX(); i++)
        h3_rate->SetBinContent(i, h3_rate->GetBinContent(i) * 0.05);

    // ------------------------------------------------------------------ //
    //  Helper lambda for drawing and saving
    // ------------------------------------------------------------------ //
    auto drawAndSave = [](TH1D* h, const char* outpath) {
        TCanvas* c = new TCanvas(outpath, outpath, 900, 650);
        c->SetGrid();
        h->SetLineColor(kBlue);
        h->SetLineWidth(2);
        h->Draw("HIST");
        c->SaveAs(outpath);
        delete c;
    };

    drawAndSave(h0,      "macro_outputs/analysis_plots/plotccqe_numu_breakdown/step0_raw_counts_combined_4.png");
    drawAndSave(h1,      "macro_outputs/analysis_plots/plotccqe_numu_breakdown/step1_per_pot_combined_4.png");
    drawAndSave(h2,      "macro_outputs/analysis_plots/plotccqe_numu_breakdown/step2_per_pot_per_gev_combined_4.png");
    drawAndSave(h3_rate, "macro_outputs/analysis_plots/plotccqe_numu_breakdown/step3_rate_per_50mev_combined_4.png");

    // ------------------------------------------------------------------ //
    //  All four stages on one canvas for comparison
    // ------------------------------------------------------------------ //
    TCanvas* cAll = new TCanvas("cAll", "All normalisation stages", 1200, 900);
    cAll->Divide(2, 2);

    cAll->cd(1); h0->SetLineColor(kBlue);     h0->SetLineWidth(2);      h0->Draw("HIST");
    cAll->cd(2); h1->SetLineColor(kRed);      h1->SetLineWidth(2);      h1->Draw("HIST");
    cAll->cd(3); h2->SetLineColor(kGreen+2);  h2->SetLineWidth(2);      h2->Draw("HIST");
    cAll->cd(4); h3_rate->SetLineColor(kMagenta+1); h3_rate->SetLineWidth(2); h3_rate->Draw("HIST");

    cAll->SaveAs("macro_outputs/analysis_plots/plotccqe_numu_breakdown/all_stages_combined_4.png");

    // ------------------------------------------------------------------ //
    //  Print peak values at each stage for quick sanity check
    // ------------------------------------------------------------------ //
    std::cout << "\n=== Peak values at each stage ===" << std::endl;
    std::cout << "Stage 0 (raw counts):        " << h0->GetMaximum()      << std::endl;
    std::cout << "Stage 1 (/ POT):             " << h1->GetMaximum()      << std::endl;
    std::cout << "Stage 2 (/ POT / GeV):       " << h2->GetMaximum()      << std::endl;
    std::cout << "Stage 3 (/ POT / 50 MeV):   " << h3_rate->GetMaximum() << std::endl;
    std::cout << "Scale factor (total POT):    " << scale_factor           << std::endl;
    std::cout << "Bin 1 width (GeV):           " << h0->GetBinWidth(1)    << std::endl;

    // ------------------------------------------------------------------ //
    //  Save to ROOT file
    // ------------------------------------------------------------------ //
    TFile* fOut = TFile::Open("macro_outputs/analysis_plots/plotccqe_numu_breakdown/stepbystep_numu_combined_4.root", "RECREATE");
    if (!fOut || fOut->IsZombie()) {
        std::cerr << "ERROR: cannot create output file." << std::endl;
        return;
    }
    h0->Write();
    h1->Write();
    h2->Write();
    h3_rate->Write();
    fOut->Close();
    f->Close();

    std::cout << "\nDone." << std::endl;
}