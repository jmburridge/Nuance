void get_ratio() {

    TFile* flxDatarelease = TFile::Open("macro_outputs/data_release_comparisons/miniboone_datarelease_flux.root");
    TFile* flxNuance      = TFile::Open("macro_outputs/overlaid_histograms_percmsq.root");

    if (!flxDatarelease || flxDatarelease->IsZombie()) { std::cerr << "Error opening datarelease file\n"; return; }
    if (!flxNuance      || flxNuance->IsZombie())      { std::cerr << "Error opening nuance file\n";      return; }

    const char* names[4] = { "h10007001", "h10007002", "h10007003", "h10007004" };

    TFile outFile("macro_outputs/flux_ratio_output.root", "RECREATE");
    TCanvas* c = new TCanvas("c", "Ratios", 800, 700);
    c->Divide(2, 2);

    for (int i = 0; i < 4; i++) {
        TH1D* hDatarelease = (TH1D*)flxDatarelease->Get(names[i]);
        TH1D* hNuance      = (TH1D*)flxNuance->Get(names[i]);

        if (!hDatarelease || !hNuance) { std::cerr << "Missing histogram: " << names[i] << std::endl; continue; }

        
        // Rebin nuance to match datarelease binning
        TH1D* hNuanceRebinned = new TH1D(Form("nuance_rebinned_%s", names[i]), "", 
                                          hDatarelease->GetNbinsX(), 
                                          hDatarelease->GetXaxis()->GetXmin(), 
                                          hDatarelease->GetXaxis()->GetXmax());
        for (int b = 1; b <= hDatarelease->GetNbinsX(); b++) {
            double center = hDatarelease->GetBinCenter(b);
            hNuanceRebinned->SetBinContent(b, hNuance->GetBinContent(hNuance->FindBin(center)));
        }
       

        TH1D* hRatio = (TH1D*)hDatarelease->Clone(Form("ratio_%s", names[i]));
        hRatio->SetTitle(Form("Ratio %s;Energy (GeV);Datarelease / Nuance", names[i]));
        hRatio->Divide(hNuanceRebinned);

        //output value of ratio at 1 GeV (peak of the flux)
        int bin = hRatio->FindBin(1.0);
        std::cout << names[i] << " ratio at 1 GeV: " << hRatio->GetBinContent(bin) << std::endl;

        c->cd(i + 1);
        hRatio->Draw("HIST");
       // hRatio->SetMinimum(0.8e-06);
      //  hRatio->SetMaximum(0.9e-06);
        hRatio->Write();

        delete hNuanceRebinned;
    }

    c->Update();
    c->SaveAs("macro_outputs/ratiocomparison_data_percmsq.png"); //for flux vs nuance use: macro_outputs/second_test_run/flux_ratio_plots.png
    std::cout << "Done.\n";

     outFile.Close();
}