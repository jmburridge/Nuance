//This basic macro will plot the histograms h10007001 to h10007004 from a ROOT file. 
// It will overlay them on the same canvas with different colors and add a legend. 
// The user will be prompted to select the ROOT file containing the histograms. 
// The resulting plot will be saved as "overlaid_histograms.pdf".
// plot_histograms.C
// Usage: root -l plot_histograms.C

void plot_histograms() {

    TFile* f = TFile::Open("/exp/uboone/data/users/jburridg/Nuance/NUANCE/NUANCE_event_files/output/root/first_test_run/uncombined/events_2.root");
    if (!f || f->IsZombie()) {
        std::cerr << "Error: Cannot open file." << std::endl;
        return;
    }

    TCanvas* c = new TCanvas("c", "Overlaid Histograms", 900, 650);
    c->SetLogy();

    // double scalefactor = 5400;//the number of events - and therefore number of targets - in the sample. 
    // This is used to scale the histograms to a flux-like quantity.
    TH1* h1 = dynamic_cast<TH1*>(f->Get("h10007001"));
    TH1* h2 = dynamic_cast<TH1*>(f->Get("h10007002"));
    TH1* h3 = dynamic_cast<TH1*>(f->Get("h10007003"));
    TH1* h4 = dynamic_cast<TH1*>(f->Get("h10007004"));

    //scale histograms to per cm^2 by dividing by 1.168986e6 then divide by bin width (0.05) to get flux per GeV. 
    h1->Scale(20/1.168986e6);
    h2->Scale(20/1.168986e6);
    h3->Scale(20/1.168986e6);
    h4->Scale(20/1.168986e6);

    h1->SetLineColor(kBlue+1);
    h2->SetLineColor(kBlue-7);
    h3->SetLineColor(kRed+1);
    h4->SetLineColor(kRed-7);

    h1->SetLineWidth(2);
    h2->SetLineWidth(2);
    h3->SetLineWidth(2);
    h4->SetLineWidth(2);

    h1->Draw("HIST");
    h1->GetXaxis()->SetRangeUser(0, 5);
    h1->GetXaxis()->SetTitle("Neutrino Energy (GeV)");
    h1->GetYaxis()->SetTitle("Flux [#nu/POT/GeV/cm^2]");
    h1->SetMinimum(0.5e-14);
    h1->SetMaximum(0.5e-08);
    h2->Draw("HIST SAME");
    h3->Draw("HIST SAME");
    h4->Draw("HIST SAME");

    TLegend* leg = new TLegend(0.65, 0.65, 0.88, 0.88);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->AddEntry(h1, "h10007001", "l");
    leg->AddEntry(h2, "h10007002", "l");
    leg->AddEntry(h3, "h10007003", "l");
    leg->AddEntry(h4, "h10007004", "l");
    leg->Draw();
    
    //save each histogram to a root file
    TFile* outFile = new TFile("macro_outputs/overlaid_histograms_percmsq.root", "RECREATE");
    h1->Write("h10007001");
    h2->Write("h10007002");
    h3->Write("h10007003");
    h4->Write("h10007004");
    outFile->Write(); //this is for good measure, the histograms should already be written to the file by the Write() calls above, but this ensures everything is saved properly.
    outFile->Close();

    c->Update();
    c->SaveAs("macro_outputs/overlaid_histograms_percmsq.png");
    std::cout << "Saved to macro_outputs/overlaid_histograms_percmsq.png" << std::endl;


}