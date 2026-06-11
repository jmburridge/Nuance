// Macro to plot raw event counts vs neutrino energy in 50 MeV bins.
// For each input ROOT file it produces:
//   A) raw counts:     total / numu / CCQE numu overlaid   (one canvas per file)
//   B) normalised:     same, divided by N hadd'd files     (one canvas per file, common y-range)
//   C) ratio:          N(CCQE numu) / N(numu) vs energy    (one canvas per file, common y-range)
// plus a 4x3 comparison grid for each of the three sets.
// All plots have y-axis gridlines.
//
// NOTE: the normalised and ratio canvases are drawn AFTER the file loop,
// because the common y-axis range can only be set once all files are read.

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

void plotevents()
{
    // ------------------------------------------------------------------ //
    //  Input files
    // ------------------------------------------------------------------ //
    const std::string baseDir =
        "/exp/uboone/data/users/jburridg/Nuance/NUANCE/NUANCE_event_files/output/root/PoT_Nuance_event_files/combined/";

    // Full filenames, written out explicitly since the naming isn't uniform.
    std::vector<std::string> fileNames = {
        "NUANCE_events_6.root",
        "NUANCE_events_7.root",
        "NUANCE_events_11.root",
        "NUANCE_events_12.root",
        "NUANCE_events_13.root",
        "NUANCE_events_15.root",
        "NUANCE_events_16.root",
        "NUANCE_events_17.root",
        "NUANCE_events_18.root",
        "NUANCE_events_19.root",
        "combined_combined/combined_nuance_events_4.root"
    };

    // Short labels used for histogram names, canvas names, and output PNGs.
    std::vector<std::string> fileLabels = {
        "6", "7", "11", "12", "13", "15", "16", "17", "18", "19", "combined_4"
    };

    // Number of NUANCE files hadd'd into each ROOT file (shown in the legend).
    std::vector<int> numFiles = {
        313, 680, 1039, 305, 369, 470, 295, 693, 432, 506, 5102
    };

    std::vector<std::string> filePaths;
    for (const auto& name : fileNames) {
        filePaths.push_back(baseDir + name);
    }

    // ------------------------------------------------------------------ //
    //  Binning: uniform 50 MeV (0.05 GeV) bins from 0 to 6 GeV -> 120 bins
    // ------------------------------------------------------------------ //
    const Int_t    nBins = 120;
    const Double_t eMin  = 0.0;   // GeV
    const Double_t eMax  = 6.0;   // GeV

    // Storage for all histograms (index-aligned with fileLabels/numFiles).
    std::vector<TH1D*> allTotal,     allNumu,     allNumuCCQE;
    std::vector<TH1D*> allTotalNorm, allNumuNorm, allNumuCCQENorm;
    std::vector<TH1D*> allRatio; // N(numu) / N(CCQE numu)

    // ------------------------------------------------------------------ //
    //  Loop over files: fill histograms, draw the RAW per-file canvases
    // ------------------------------------------------------------------ //
    for (size_t iFile = 0; iFile < filePaths.size(); iFile++) {

        const std::string& path  = filePaths[iFile];
        const std::string& label = fileLabels[iFile];

        TFile* f = TFile::Open(path.c_str(), "READ");
        if (!f || f->IsZombie()) {
            std::cerr << "ERROR: cannot open " << path << " -- skipping." << std::endl;
            allTotal.push_back(nullptr);     allNumu.push_back(nullptr);     allNumuCCQE.push_back(nullptr);
            allTotalNorm.push_back(nullptr); allNumuNorm.push_back(nullptr); allNumuCCQENorm.push_back(nullptr);
            allRatio.push_back(nullptr);
            continue;
        }

        TTree* h3 = (TTree*)f->Get("h3");
        if (!h3) {
            std::cerr << "ERROR: could not find h3 in " << path << " -- skipping." << std::endl;
            allTotal.push_back(nullptr);     allNumu.push_back(nullptr);     allNumuCCQE.push_back(nullptr);
            allTotalNorm.push_back(nullptr); allNumuNorm.push_back(nullptr); allNumuCCQENorm.push_back(nullptr);
            allRatio.push_back(nullptr);
            f->Close();
            continue;
        }

        // Branches
        Float_t p_neutrino[4];
        Int_t   neutrino; // PDG code
        Int_t   channel;  // proc code (CCQE = 1)

        h3->SetBranchAddress("p_neutrino", p_neutrino);
        h3->SetBranchAddress("neutrino",   &neutrino);
        h3->SetBranchAddress("channel",    &channel);

        // Histograms (unique names per file so ROOT doesn't complain)
        TH1D* hTotal = new TH1D(Form("hTotal_%s", label.c_str()),
            Form("Event Counts (file %s);Neutrino Energy (GeV);Event count", label.c_str()),
            nBins, eMin, eMax);
        TH1D* hNumu = new TH1D(Form("hNumu_%s", label.c_str()),
            "Numu events;Neutrino Energy (GeV);Event count",
            nBins, eMin, eMax);
        TH1D* hNumuCCQE = new TH1D(Form("hNumuCCQE_%s", label.c_str()),
            "CCQE Numu events;Neutrino Energy (GeV);Event count",
            nBins, eMin, eMax);

        // Loop over entries
        Long64_t nEntries = h3->GetEntries();
        for (Long64_t i = 0; i < nEntries; i++) {
            h3->GetEntry(i);
            Float_t energyGeV = p_neutrino[3] / 1000.0; // MeV -> GeV

            hTotal->Fill(energyGeV);                       // everything
            if (neutrino == 14) {
                hNumu->Fill(energyGeV);                    // all numu
                if (channel == 1) hNumuCCQE->Fill(energyGeV); // CCQE numu
            }
        }

        std::cout << "File " << label
                  << ": total = "      << hTotal->GetEntries()
                  << ", numu = "       << hNumu->GetEntries()
                  << ", CCQE numu = "  << hNumuCCQE->GetEntries()
                  << std::endl;

        // Detach histograms from the input file so they survive f->Close()
        hTotal   ->SetDirectory(0);
        hNumu    ->SetDirectory(0);
        hNumuCCQE->SetDirectory(0);

        // -------------------------------------------------------------- //
        //  A) RAW per-file canvas (y-range free, gridlines on)
        // -------------------------------------------------------------- //
        TCanvas* c = new TCanvas(Form("c_%s", label.c_str()),
                                 Form("Event Counts file %s", label.c_str()),
                                 800, 600);
        c->SetGridy(); // y-axis gridlines

        hTotal   ->SetLineColor(kBlack);
        hNumu    ->SetLineColor(kBlue);
        hNumuCCQE->SetLineColor(kRed);

        hTotal   ->SetLineWidth(2);
        hNumu    ->SetLineWidth(2);
        hNumuCCQE->SetLineWidth(2);

        hTotal   ->SetStats(0);
        hTotal   ->Draw("HIST");
        hNumu    ->Draw("HIST SAME");
        hNumuCCQE->Draw("HIST SAME");

        TLegend* leg = new TLegend(0.5, 0.62, 0.88, 0.88);
        leg->SetHeader(Form("N_{files} hadd'd = %d", numFiles[iFile]), "C");
        leg->AddEntry(hTotal,    Form("Total events: %.0f",     hTotal->GetEntries()),    "l");
        leg->AddEntry(hNumu,     Form("#nu_{#mu} events: %.0f", hNumu->GetEntries()),     "l");
        leg->AddEntry(hNumuCCQE, Form("CCQE #nu_{#mu}: %.0f",   hNumuCCQE->GetEntries()), "l");
        leg->Draw();

        c->SaveAs(Form("macro_outputs/analysis_plots/plotevents/event_counts_%s.png",
                       label.c_str()));

        // -------------------------------------------------------------- //
        //  Build normalised clones (events / file) -- drawn after the loop
        // -------------------------------------------------------------- //
        TH1D* hTotalNorm    = (TH1D*)hTotal   ->Clone(Form("hTotalNorm_%s",    label.c_str()));
        TH1D* hNumuNorm     = (TH1D*)hNumu    ->Clone(Form("hNumuNorm_%s",     label.c_str()));
        TH1D* hNumuCCQENorm = (TH1D*)hNumuCCQE->Clone(Form("hNumuCCQENorm_%s", label.c_str()));

        double normFactor = 1.0 / numFiles[iFile];
        hTotalNorm   ->Scale(normFactor);
        hNumuNorm    ->Scale(normFactor);
        hNumuCCQENorm->Scale(normFactor);

        hTotalNorm->SetTitle(Form("Event Counts / N_{files} (file %s);Neutrino Energy (GeV);Event count / file", label.c_str()));
        hNumuNorm    ->GetYaxis()->SetTitle("Event count / file");
        hNumuCCQENorm->GetYaxis()->SetTitle("Event count / file");

        hTotalNorm   ->SetDirectory(0);
        hNumuNorm    ->SetDirectory(0);
        hNumuCCQENorm->SetDirectory(0);

        // -------------------------------------------------------------- //
        //  Build ratio: N(CCQE numu) / N(numu) -- drawn after the loop
        //  (bins where numu = 0 are set to 0 by TH1::Divide)
        // -------------------------------------------------------------- //
        TH1D* hRatio = (TH1D*)hNumuCCQE->Clone(Form("hRatio_%s", label.c_str()));
        hRatio->Divide(hNumu);
        hRatio->SetTitle(Form("CCQE #nu_{#mu} / #nu_{#mu} ratio (file %s);Neutrino Energy (GeV);N(CCQE #nu_{#mu}) / N(#nu_{#mu})", label.c_str()));
        hRatio->SetLineColor(kBlack);
        hRatio->SetLineWidth(2);
        hRatio->SetDirectory(0);

        // store everything
        allTotal.push_back(hTotal);
        allNumu.push_back(hNumu);
        allNumuCCQE.push_back(hNumuCCQE);
        allTotalNorm.push_back(hTotalNorm);
        allNumuNorm.push_back(hNumuNorm);
        allNumuCCQENorm.push_back(hNumuCCQENorm);
        allRatio.push_back(hRatio);

        f->Close();
    }

    // ------------------------------------------------------------------ //
    //  Common y-axis ranges across files
    //  (computed now that every file has been read)
    // ------------------------------------------------------------------ //
    double maxNorm  = 0.0;
    double maxRatio = 0.0;
    for (size_t iFile = 0; iFile < fileLabels.size(); iFile++) {
        if (allTotalNorm[iFile])
            maxNorm  = std::max(maxNorm,  allTotalNorm[iFile]->GetMaximum());
        if (allRatio[iFile])
            maxRatio = std::max(maxRatio, allRatio[iFile]->GetMaximum());
    }
    const double yMaxNorm  = 1.1 * maxNorm;  // 10% headroom
    const double yMaxRatio = 1.1 * maxRatio;

    // ------------------------------------------------------------------ //
    //  B) NORMALISED per-file canvases (common y-range, gridlines on)
    // ------------------------------------------------------------------ //
    for (size_t iFile = 0; iFile < fileLabels.size(); iFile++) {
        if (!allTotalNorm[iFile]) continue;

        const std::string& label = fileLabels[iFile];
        double normFactor = 1.0 / numFiles[iFile];

        TCanvas* cNorm = new TCanvas(Form("cNorm_%s", label.c_str()),
                                     Form("Normalised Event Counts file %s", label.c_str()),
                                     800, 600);
        cNorm->SetGridy();

        allTotalNorm[iFile]->SetStats(0);
        allTotalNorm[iFile]->SetMinimum(0.0);
        allTotalNorm[iFile]->SetMaximum(yMaxNorm); // same range for every file

        allTotalNorm[iFile]   ->Draw("HIST");
        allNumuNorm[iFile]    ->Draw("HIST SAME");
        allNumuCCQENorm[iFile]->Draw("HIST SAME");

        TLegend* legNorm = new TLegend(0.5, 0.62, 0.88, 0.88);
        legNorm->SetHeader(Form("N_{files} hadd'd = %d", numFiles[iFile]), "C");
        legNorm->AddEntry(allTotalNorm[iFile],    Form("Total events/file: %.1f",     allTotal[iFile]->GetEntries()    * normFactor), "l");
        legNorm->AddEntry(allNumuNorm[iFile],     Form("#nu_{#mu} events/file: %.1f", allNumu[iFile]->GetEntries()     * normFactor), "l");
        legNorm->AddEntry(allNumuCCQENorm[iFile], Form("CCQE #nu_{#mu}/file: %.1f",   allNumuCCQE[iFile]->GetEntries() * normFactor), "l");
        legNorm->Draw();

        cNorm->SaveAs(Form("macro_outputs/analysis_plots/plotevents/event_counts_norm_%s.png",
                           label.c_str()));
    }

    // ------------------------------------------------------------------ //
    //  C) RATIO per-file canvases (common y-range, gridlines on)
    // ------------------------------------------------------------------ //
    for (size_t iFile = 0; iFile < fileLabels.size(); iFile++) {
        if (!allRatio[iFile]) continue;

        const std::string& label = fileLabels[iFile];

        TCanvas* cRatio = new TCanvas(Form("cRatio_%s", label.c_str()),
                                      Form("Numu/CCQE ratio file %s", label.c_str()),
                                      800, 600);
        cRatio->SetGridy();

        allRatio[iFile]->SetStats(0);
        allRatio[iFile]->SetMinimum(0.0);
        allRatio[iFile]->SetMaximum(yMaxRatio); // same range for every file
        allRatio[iFile]->Draw("HIST");

        TLegend* legRatio = new TLegend(0.5, 0.74, 0.88, 0.88);
        legRatio->SetHeader(Form("N_{files} hadd'd = %d", numFiles[iFile]), "C");
        legRatio->AddEntry(allRatio[iFile],
            Form("Overall ratio: %.2f",
                 allNumu[iFile]->GetEntries() > 0
                     ? allNumuCCQE[iFile]->GetEntries() / allNumu[iFile]->GetEntries()
                     : 0.0),
            "l");
        legRatio->Draw();

        cRatio->SaveAs(Form("macro_outputs/analysis_plots/plotevents/event_counts_ratio_%s.png",
                            label.c_str()));
    }

    // ------------------------------------------------------------------ //
    //  Comparison grids (4x3): raw, normalised, ratio
    // ------------------------------------------------------------------ //

    // --- raw ---
    TCanvas* cAll = new TCanvas("cAll", "All files comparison", 1600, 1200);
    cAll->Divide(4, 3);
    for (size_t iFile = 0; iFile < fileLabels.size(); iFile++) {
        if (!allTotal[iFile]) continue;
        cAll->cd(iFile + 1);
        gPad->SetGridy();

        allTotal[iFile]   ->Draw("HIST");
        allNumu[iFile]    ->Draw("HIST SAME");
        allNumuCCQE[iFile]->Draw("HIST SAME");

        TLegend* legPad = new TLegend(0.45, 0.58, 0.88, 0.88);
        legPad->SetTextSize(0.04);
        legPad->SetHeader(Form("N_{files} = %d", numFiles[iFile]), "C");
        legPad->AddEntry(allTotal[iFile],    Form("Total: %.0f",          allTotal[iFile]->GetEntries()),    "l");
        legPad->AddEntry(allNumu[iFile],     Form("#nu_{#mu}: %.0f",      allNumu[iFile]->GetEntries()),     "l");
        legPad->AddEntry(allNumuCCQE[iFile], Form("CCQE #nu_{#mu}: %.0f", allNumuCCQE[iFile]->GetEntries()), "l");
        legPad->Draw();
    }
    cAll->SaveAs("macro_outputs/analysis_plots/plotevents/event_counts_all_files_comparison.png");

    // --- normalised (common y-range) ---
    TCanvas* cAllNorm = new TCanvas("cAllNorm", "All files comparison (normalised)", 1600, 1200);
    cAllNorm->Divide(4, 3);
    for (size_t iFile = 0; iFile < fileLabels.size(); iFile++) {
        if (!allTotalNorm[iFile]) continue;
        cAllNorm->cd(iFile + 1);
        gPad->SetGridy();

        // range already fixed via SetMinimum/SetMaximum above
        allTotalNorm[iFile]   ->Draw("HIST");
        allNumuNorm[iFile]    ->Draw("HIST SAME");
        allNumuCCQENorm[iFile]->Draw("HIST SAME");

        double normFactor = 1.0 / numFiles[iFile];

        TLegend* legPadN = new TLegend(0.45, 0.58, 0.88, 0.88);
        legPadN->SetTextSize(0.04);
        legPadN->SetHeader(Form("N_{files} = %d", numFiles[iFile]), "C");
        legPadN->AddEntry(allTotalNorm[iFile],    Form("Total/file: %.1f",          allTotal[iFile]->GetEntries()    * normFactor), "l");
        legPadN->AddEntry(allNumuNorm[iFile],     Form("#nu_{#mu}/file: %.1f",      allNumu[iFile]->GetEntries()     * normFactor), "l");
        legPadN->AddEntry(allNumuCCQENorm[iFile], Form("CCQE #nu_{#mu}/file: %.1f", allNumuCCQE[iFile]->GetEntries() * normFactor), "l");
        legPadN->Draw();
    }
    cAllNorm->SaveAs("macro_outputs/analysis_plots/plotevents/event_counts_all_files_comparison_norm.png");

    // --- ratio (common y-range) ---
    TCanvas* cAllRatio = new TCanvas("cAllRatio", "All files comparison (CCQE/numu ratio)", 1600, 1200);
    cAllRatio->Divide(4, 3);
    for (size_t iFile = 0; iFile < fileLabels.size(); iFile++) {
        if (!allRatio[iFile]) continue;
        cAllRatio->cd(iFile + 1);
        gPad->SetGridy();

        allRatio[iFile]->Draw("HIST");

        
    }
    cAllRatio->SaveAs("macro_outputs/analysis_plots/plotevents/event_counts_all_files_comparison_ratio.png");

    // ------------------------------------------------------------------ //
    //  Save all histograms to one output ROOT file
    // ------------------------------------------------------------------ //
    TFile* fOut = TFile::Open(
        "macro_outputs/analysis_plots/plotevents/event_counts_all_files.root",
        "RECREATE");
    if (fOut && !fOut->IsZombie()) {
        for (size_t iFile = 0; iFile < fileLabels.size(); iFile++) {
            if (allTotal[iFile])        allTotal[iFile]->Write();
            if (allNumu[iFile])         allNumu[iFile]->Write();
            if (allNumuCCQE[iFile])     allNumuCCQE[iFile]->Write();
            if (allTotalNorm[iFile])    allTotalNorm[iFile]->Write();
            if (allNumuNorm[iFile])     allNumuNorm[iFile]->Write();
            if (allNumuCCQENorm[iFile]) allNumuCCQENorm[iFile]->Write();
            if (allRatio[iFile])        allRatio[iFile]->Write();
        }
        fOut->Close();
    }

    std::cout << "Done. Plots saved to macro_outputs/analysis_plots/plotevents/" << std::endl;
}