// Macro to plot raw event counts vs neutrino energy in 50 MeV bins.
// For each input ROOT file it overlays on one canvas:
//   1) total event count (all flavours, all channels)
//   2) numu event count  (neutrino == 14, all channels)
//   3) CCQE numu count   (neutrino == 14 && channel == 1)
// One canvas/plot per file. No PoT normalisation -- raw counts only.
// Finally, a comparison canvas puts all the individual plots side by
// side in a grid of pads so all files can be compared at a glance.

#include <iostream>
#include <vector>
#include <string>

void plotevents()
{
    // ------------------------------------------------------------------ //
    //  Input files
    //  Adjust the base directory / naming if your files live elsewhere.
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
    // Same order as fileNames/fileLabels above.
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

    // Keep pointers to every histogram so we can redraw them all on the
    // comparison canvas after the file loop.
    std::vector<TH1D*> allTotal, allNumu, allNumuCCQE;
    // Same again but normalised by the number of hadd'd files (events/file).
    std::vector<TH1D*> allTotalNorm, allNumuNorm, allNumuCCQENorm;

    // ------------------------------------------------------------------ //
    //  Loop over files
    // ------------------------------------------------------------------ //
    for (size_t iFile = 0; iFile < filePaths.size(); iFile++) {

        const std::string& path  = filePaths[iFile];
        const std::string& label = fileLabels[iFile];

        TFile* f = TFile::Open(path.c_str(), "READ");
        if (!f || f->IsZombie()) {
            std::cerr << "ERROR: cannot open " << path << " -- skipping." << std::endl;
            // keep the vectors aligned with fileLabels/numFiles
            allTotal.push_back(nullptr);
            allNumu.push_back(nullptr);
            allNumuCCQE.push_back(nullptr);
            allTotalNorm.push_back(nullptr);
            allNumuNorm.push_back(nullptr);
            allNumuCCQENorm.push_back(nullptr);
            continue;
        }

        TTree* h3 = (TTree*)f->Get("h3");
        if (!h3) {
            std::cerr << "ERROR: could not find h3 in " << path << " -- skipping." << std::endl;
            allTotal.push_back(nullptr);
            allNumu.push_back(nullptr);
            allNumuCCQE.push_back(nullptr);
            allTotalNorm.push_back(nullptr);
            allNumuNorm.push_back(nullptr);
            allNumuCCQENorm.push_back(nullptr);
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

        // ------------------------------------------------------------------ //
        //  Draw: all three on one canvas per file
        // ------------------------------------------------------------------ //
        TCanvas* c = new TCanvas(Form("c_%s", label.c_str()),
                                 Form("Event Counts file %s", label.c_str()),
                                 800, 600);

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

        // Legend with event counts and number of hadd'd files
        TLegend* leg = new TLegend(0.5, 0.62, 0.88, 0.88);
        leg->SetHeader(Form("N_{files} hadd'd = %d", numFiles[iFile]), "C");
        leg->AddEntry(hTotal,    Form("Total events: %.0f",     hTotal->GetEntries()),    "l");
        leg->AddEntry(hNumu,     Form("#nu_{#mu} events: %.0f", hNumu->GetEntries()),     "l");
        leg->AddEntry(hNumuCCQE, Form("CCQE #nu_{#mu}: %.0f",   hNumuCCQE->GetEntries()), "l");
        leg->Draw();

        c->SaveAs(Form("macro_outputs/analysis_plots/plotevents/event_counts_%s.png",
                       label.c_str()));

        // Detach histograms from the input file so they survive f->Close()
        hTotal   ->SetDirectory(0);
        hNumu    ->SetDirectory(0);
        hNumuCCQE->SetDirectory(0);

        // store for the comparison canvas
        allTotal.push_back(hTotal);
        allNumu.push_back(hNumu);
        allNumuCCQE.push_back(hNumuCCQE);

        // ------------------------------------------------------------------ //
        //  Normalised versions: same plots, divided by the number of
        //  hadd'd NUANCE files -> events per file, comparable across files
        // ------------------------------------------------------------------ //
        TH1D* hTotalNorm    = (TH1D*)hTotal   ->Clone(Form("hTotalNorm_%s",    label.c_str()));
        TH1D* hNumuNorm     = (TH1D*)hNumu    ->Clone(Form("hNumuNorm_%s",     label.c_str()));
        TH1D* hNumuCCQENorm = (TH1D*)hNumuCCQE->Clone(Form("hNumuCCQENorm_%s", label.c_str()));

        double normFactor = 1.0 / numFiles[iFile];
        hTotalNorm   ->Scale(normFactor);
        hNumuNorm    ->Scale(normFactor);
        hNumuCCQENorm->Scale(normFactor);

        hTotalNorm->SetTitle(Form("Event Counts / N_{files} (file %s);Neutrino Energy (GeV);Event count / file", label.c_str()));
        hNumuNorm    ->GetYaxis()->SetTitle("Event count / file count");
        hNumuCCQENorm->GetYaxis()->SetTitle("Event count / file count");

        hTotalNorm   ->SetDirectory(0);
        hNumuNorm    ->SetDirectory(0);
        hNumuCCQENorm->SetDirectory(0);

        TCanvas* cNorm = new TCanvas(Form("cNorm_%s", label.c_str()),
                                     Form("Normalised Event Counts file %s", label.c_str()),
                                     800, 600);

        hTotalNorm   ->SetStats(0);
        hTotalNorm   ->Draw("HIST");
        hNumuNorm    ->Draw("HIST SAME");
        hNumuCCQENorm->Draw("HIST SAME");

        TLegend* legNorm = new TLegend(0.5, 0.62, 0.88, 0.88);
        legNorm->SetHeader(Form("N_{files} hadd'd = %d", numFiles[iFile]), "C");
        legNorm->AddEntry(hTotalNorm,    Form("Total events/file: %.1f",     hTotal->GetEntries()    * normFactor), "l");
        legNorm->AddEntry(hNumuNorm,     Form("#nu_{#mu} events/file: %.1f", hNumu->GetEntries()     * normFactor), "l");
        legNorm->AddEntry(hNumuCCQENorm, Form("CCQE #nu_{#mu}/file: %.1f",   hNumuCCQE->GetEntries() * normFactor), "l");
        legNorm->Draw();

        cNorm->SaveAs(Form("macro_outputs/analysis_plots/plotevents/event_counts_norm_%s.png",
                           label.c_str()));

        // store normalised hists for the comparison canvas
        allTotalNorm.push_back(hTotalNorm);
        allNumuNorm.push_back(hNumuNorm);
        allNumuCCQENorm.push_back(hNumuCCQENorm);

        f->Close();
    }

    // ------------------------------------------------------------------ //
    //  Comparison canvas: all individual plots side by side in a 4x3 grid
    // ------------------------------------------------------------------ //
    TCanvas* cAll = new TCanvas("cAll", "All files comparison", 1600, 1200);
    cAll->Divide(4, 3); // 12 pads for 11 files

    for (size_t iFile = 0; iFile < fileLabels.size(); iFile++) {
        if (!allTotal[iFile]) continue; // file was skipped

        cAll->cd(iFile + 1);

        allTotal[iFile]   ->Draw("HIST");
        allNumu[iFile]    ->Draw("HIST SAME");
        allNumuCCQE[iFile]->Draw("HIST SAME");

        // smaller legend per pad so it stays readable
        TLegend* legPad = new TLegend(0.45, 0.58, 0.88, 0.88);
        legPad->SetTextSize(0.04);
        legPad->SetHeader(Form("N_{files} = %d", numFiles[iFile]), "C");
        legPad->AddEntry(allTotal[iFile],    Form("Total: %.0f",          allTotal[iFile]->GetEntries()),    "l");
        legPad->AddEntry(allNumu[iFile],     Form("#nu_{#mu}: %.0f",      allNumu[iFile]->GetEntries()),     "l");
        legPad->AddEntry(allNumuCCQE[iFile], Form("CCQE #nu_{#mu}: %.0f", allNumuCCQE[iFile]->GetEntries()), "l");
        legPad->Draw();
    }

    cAll->SaveAs("macro_outputs/analysis_plots/plotevents/event_counts_all_files_comparison.png");

    // ------------------------------------------------------------------ //
    //  Normalised comparison canvas: same 4x3 grid, events/file
    // ------------------------------------------------------------------ //
    TCanvas* cAllNorm = new TCanvas("cAllNorm", "All files comparison (normalised)", 1600, 1200);
    cAllNorm->Divide(4, 3); // 12 pads for 11 files

    for (size_t iFile = 0; iFile < fileLabels.size(); iFile++) {
        if (!allTotalNorm[iFile]) continue; // file was skipped

        cAllNorm->cd(iFile + 1);

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

    // ------------------------------------------------------------------ //
    //  Optionally save all histograms to one output ROOT file
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
        }
        fOut->Close();
    }

    std::cout << "Done. Plots saved to macro_outputs/analysis_plots/plotevents/" << std::endl;
}