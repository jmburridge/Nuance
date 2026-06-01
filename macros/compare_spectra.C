//a simple macro to compare the fourth (energy)  component of the p_neutrino 
//branch in tree h3 to the e_neutrino branch in h_50. 
//
//a simple macro to compare the fourth (energy) component of the p_neutrino
//branch in tree h3 to the e_neutrino branch in h50.
//
//This is just a simple cross check for sanity.
//
// Usage: root -l compare_spectra.C

void compare_spectra(const char* filename = "/exp/uboone/data/users/jburridg/Nuance/NUANCE/NUANCE_event_files/output/root/first_test_run/combined/NUANCE_events.root")
{
    TFile* f = TFile::Open(filename);
    if (!f || f->IsZombie()) {
        Error("compare_spectra", "Cannot open file: %s", filename);
        return;
    }

    //  Get trees
    TTree* h50 = (TTree*)f->Get("h50");
    TTree* h3  = (TTree*)f->Get("h3");

    if (!h50) { Error("compare_spectra", "Tree 'h50' not found"); return; }
    if (!h3)  { Error("compare_spectra", "Tree 'h3'  not found"); return; }


    int    nbins = 50;
    double xmin  = 0.0;
    double xmax  = 5000.0;

    TH1D* hE_h50 = new TH1D("hE_h50", "e_neutrino (h50 tree)",   nbins, xmin, xmax);
    TH1D* hE_h3  = new TH1D("hE_h3",  "p_neutrino[3] (h3 tree)", nbins, xmin, xmax);

    // Fill h50 histogram
    Float_t e_neutrino = 0.f;
    h50->SetBranchAddress("e_neutrino", &e_neutrino);

    Long64_t nEntries50 = h50->GetEntries();
    for (Long64_t i = 0; i < nEntries50; ++i) {
        h50->GetEntry(i);
        hE_h50->Fill(e_neutrino);
    }

    //Fill h3 histogram
    // p_neutrino is a plain Float_t array: (px, py, pz, E) — energy is index 3
    // If your convention is (E, px, py, pz) change [3] to [0]
    Float_t p_neutrino[4] = {0.f};
    h3->SetBranchAddress("p_neutrino", p_neutrino);

    Long64_t nEntries3 = h3->GetEntries();
    for (Long64_t i = 0; i < nEntries3; ++i) {
        h3->GetEntry(i);
        hE_h3->Fill(p_neutrino[3]);   // index 3 = energy component (px,py,pz,E)
    }

    
    hE_h50->SetLineColor(kBlue);
    hE_h50->SetLineWidth(2);

    hE_h3->SetLineColor(kRed);
    hE_h3->SetLineWidth(2);
    hE_h3->SetLineStyle(2);   // dashed so overlaps are visible

    // Draw
    TCanvas* c = new TCanvas("c_nu_energy", "Neutrino Energy Comparison", 800, 600);
    c->SetLeftMargin(0.12);

    // Draw whichever histogram has the larger maximum first
    TH1D* first  = (hE_h50->GetMaximum() >= hE_h3->GetMaximum()) ? hE_h50 : hE_h3;
    TH1D* second = (first == hE_h50) ? hE_h3 : hE_h50;

    first->GetXaxis()->SetTitle("Energy [MeV]");
    first->GetYaxis()->SetTitle("Events");
    first->SetTitle("Neutrino Energy Spectrum Comparison");

    first ->Draw("HIST");
    second->Draw("HIST SAME");

    TLegend* leg = new TLegend(0.55, 0.70, 0.88, 0.85);
    leg->SetBorderSize(0);
    leg->AddEntry(hE_h50, "e_{#nu} (h50 tree)",       "l");
    leg->AddEntry(hE_h3,  "p_{#nu}[3] (h3 tree)",     "l");
    leg->Draw();

    c->Update();
    c->SaveAs("macro_outputs/e_neutrino_p_neutrino_comparison.png");

    std::cout << "\nEntries read — h50: " << nEntries50
              << "  |  h3: "             << nEntries3  << std::endl;
    std::cout << "Plot saved to macro_outputs/e_neutrino_p_neutrino_comparison.png\n";
}