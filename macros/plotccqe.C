// A macro to plot the CCQE events similar to jens thesis  Fig. 6.5. 
// done by using -proc flag from ttree channel 

#include <map>
#include <iostream>

void plotccqe(const char* filename = "/exp/uboone/data/users/jburridg/Nuance/NUANCE/NUANCE_event_files/output/root/PoT_Nuance_event_files/combined/combined_nuance_events_1.root")
{// change this to a -secs file for known pot

    //////////////////////////////////////////////////////////////////////// //
    // SCALE FACTORS 
    // the event numbers puled frim the trees are just rates, not normalised forn the pot. 
    // To get the actual event rates, we need to divide by the pot.
    // Also jens thesis is in terms of /50MeV, so we need to divide by the bin width (50 MeV) to get the rate per MeV.
    //////////////////////////////////////////////////////////////////////// //
    Float_t secs_per_file = 1e10; //need to get this from the -secs file; total -secs exposre is the pot.
    Float_t pot_per_sec = 1.17e6;
    Float_t num_files = 2716; //need to get this from the -secs file; total number of files is the number of files that were hadd'd together.
    Float_t N_target = 1.113e31; //number of target nucleons in the detector, needed to get cross section from rate.
    Float_t scale_factor = pot_per_sec * secs_per_file * num_files; //this is the factor we need to divide the event numbers by to get the actual event rates. 
    std::cout << "Scale factor: " << scale_factor << std::endl;
    // ------------------------------------------------------------------ //
    //  Open file
    // ------------------------------------------------------------------ //
    TFile* f = TFile::Open(filename, "READ");
    if (!f || f->IsZombie()) {
        std::cerr << "ERROR: cannot open " << filename << std::endl;
        return;
    }

     //load data release flux

   //open .dat file and read in per line 
    std::ifstream infile("macro_inputs/pospolarity_fluxes.dat");
    if (!infile.is_open()) {    
        std::cerr << "ERROR: cannot open flux .dat file." << std::endl;
        return;
    }   
    std::vector<double> edges;
    std::vector<double> numu, numubar, nue, nuebar;
    std::string line;
    //no header line in this .dat file, so we can just read in the data directly.
    double lo, hi, a, b, c, d;
    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        ss >> lo >> hi >> a >> b >> c >> d;
        if (edges.empty()) edges.push_back(lo);
        edges.push_back(hi);
        numu.push_back(a);
        numubar.push_back(b);
        nue.push_back(c);
        nuebar.push_back(d);
    }
    infile.close();
    //build histograms from the .dat file
    TH1D* hFlux_numu    = new TH1D("hFlux_numu", "Numu Flux;Neutrino Energy (MeV);Flux [#nu/cm^2/PoT/50 MeV]", numu.size(), &edges[0]);
    TH1D* hFlux_numubar = new TH1D("hFlux_numubar", "Numubar Flux;Neutrino Energy (MeV);Flux [#nu/cm^2/PoT/50 MeV]", numubar.size(), &edges[0]);
    TH1D* hFlux_nue     = new TH1D("hFlux_nue", "Nue Flux;Neutrino Energy (MeV);Flux [#nu/cm^2/PoT/50 MeV]", nue.size(), &edges[0]);
    TH1D* hFlux_nuebar  = new TH1D("hFlux_nuebar", "Nuebar Flux;Neutrino Energy (MeV);Flux [#nu/cm^2/PoT/50 MeV]", nuebar.size(), &edges[0]);
    for (size_t i = 0; i < numu.size(); i++) {
        hFlux_numu   ->SetBinContent(i+1, numu[i]); 
        hFlux_numubar->SetBinContent(i+1, numubar[i]);
        hFlux_nue     ->SetBinContent(i+1, nue[i]);
        hFlux_nuebar  ->SetBinContent(i+1, nuebar[i]);
    }
    //flux histograms are in units of #nu/cm^2/PoT/50 MeV,

    // ------------------------------------------------------------------ //
    //  Step 3 get trees
    // ------------------------------------------------------------------ //
    TTree* h3  = (TTree*)f->Get("h3"); //we just need h3 for this macro. 
    //I've established that p_neutrino[3]=e_neutrino from h50. 

    if (!h3) {
        std::cerr << "ERROR: could not find h3 in file." << std::endl;
        return;
    }

    // ------------------------------------------------------------------ //
    //  Step 4: build a map: true_energy (p_neutrino[3]) -> PDG code
    //            This handles the hadd'd entry-order mismatch between trees.
    // ------------------------------------------------------------------ //
    Float_t p_neutrino[4];
    Int_t    neutrino; //pdg code of the neutrino, needed to identify flavour
    Int_t    channel; //proc code neded to identify (CCQE=1)

    h3->SetBranchAddress("p_neutrino", p_neutrino);
    h3->SetBranchAddress("neutrino",   &neutrino);
    h3->SetBranchAddress("channel",    &channel);

    // ------------------------------------------------------------------ //
    //  Custom variable-width bin edges (GeV)
    // ------------------------------------------------------------------ //
    const Double_t binEdges[] = {
        0.125, 0.180, 0.240, 0.300, 0.350, 0.425, 0.480, 0.540, 0.600,
        0.655, 0.755, 0.785, 0.845, 0.900, 0.955, 1.025, 1.085, 1.145,
        1.200, 1.270, 1.320, 1.380, 1.435, 1.500, 1.560, 1.620, 1.680,
        1.740, 1.800, 1.860, 1.920, 1.980, 2.050, 2.100, 2.166, 2.225,
        2.280, 2.340, 2.400, 2.460, 2.525, 2.575, 2.600, 2.650, 2.700,
        2.750, 2.800, 2.850, 2.900, 2.950, 3.000, 3.050, 3.100, 3.150,
        3.200, 3.250, 3.300, 3.350, 3.400, 3.450, 3.500, 3.550, 3.600,
        3.650, 3.700, 3.750, 3.800
    };
    const Int_t nBins = sizeof(binEdges)/sizeof(binEdges[0]) - 1; // 65 bins

    // ------------------------------------------------------------------ //
    //  Step 6: plot the Numu/Numubar/Nue/Nuebar CCQE rates.
    //           this is for comparison with Fig. 6.5 in Jens thesis. 
    // ------------------------------------------------------------------ //
     
    TH1D* hRate_numu    = new TH1D("hRate_numu",    "CCQE Rate for Numu;Neutrino Energy (GeV);Event Rate [#nu/PoT/GeV]",    nBins, binEdges);
    TH1D* hRate_numubar = new TH1D("hRate_numubar", "CCQE Rate for Numubar;Neutrino Energy (GeV);Event Rate [#nu/PoT/GeV]", nBins, binEdges);
    TH1D* hRate_nue     = new TH1D("hRate_nue",     "CCQE Rate for Nue;Neutrino Energy (GeV);Event Rate [#nu/PoT/GeV]",     nBins, binEdges);
    TH1D* hRate_nuebar  = new TH1D("hRate_nuebar",  "CCQE Rate for Nuebar;Neutrino Energy (GeV);Event Rate [#nu/PoT/GeV]",  nBins, binEdges);


    Int_t   ccqe_events_filled = 0; //counter to keep track of how many ccqe events we fill in total.

    // Loop over the energy->PDG map and fill the appropriate histogram
    for (Long64_t i = 0; i < h3->GetEntries(); i++) {
        h3->GetEntry(i);
        Float_t energy = p_neutrino[3]; //get the neutrino energy from the map
        
        std::cout << " Channel: " << channel << " Neutrino PDG: " << neutrino << " Energy: " << energy << std::endl;
        if (channel == 1) { //only want CCQE events.
            if (neutrino == 14) { //filling Numu
                hRate_numu->Fill(energy/1000); //convert energy to GeV for plotting
            } else if (neutrino == -14) { //filling Numubar
                hRate_numubar->Fill(energy/1000); //convert energy to GeV for plotting      
            } else if (neutrino == 12) { //filling Nue
                hRate_nue->Fill(energy/1000); //convert energy to GeV for plotting
            } else if (neutrino == -12) { //filling Nuebar
                hRate_nuebar->Fill(energy/1000); //convert energy to GeV for plotting
            }
            // add counter for ccqe events filled
            ccqe_events_filled++;
            std::cout << "CCQE event count: " << ccqe_events_filled << std::endl;
        }
    }
    std::cout << "Number of CCQE events filled: " << hRate_numu->GetEntries() + hRate_numubar->GetEntries() + hRate_nue->GetEntries() + hRate_nuebar->GetEntries() << std::endl;

   

    //Divide by bin width and scale factor to get actual event rates per MeV for RATES histograms:
    for (int i = 1; i <= hRate_numu->GetNbinsX(); i++) {
        double binWidth = hRate_numu->GetBinWidth(i);
        hRate_numu   ->SetBinContent(i, hRate_numu->GetBinContent(i) / (binWidth * scale_factor));
        hRate_numubar->SetBinContent(i, hRate_numubar->GetBinContent(i) / (binWidth * scale_factor));
        hRate_nue    ->SetBinContent(i, hRate_nue->GetBinContent(i) / (binWidth * scale_factor));
        hRate_nuebar ->SetBinContent(i, hRate_nuebar->GetBinContent(i) / (binWidth * scale_factor));
    }

    //copy rates histograms to new histograms for cross sections, which we will get by dividing the rates by the flux:
    //rename the axes 

    TH1D* hXsec_numu    = (TH1D*)hRate_numu->Clone("hXsec_numu");
    TH1D* hXsec_numubar = (TH1D*)hRate_numubar->Clone("hXsec_numubar");
    TH1D* hXsec_nue     = (TH1D*)hRate_nue->Clone("hXsec_nue");
    TH1D* hXsec_nuebar  = (TH1D*)hRate_nuebar->Clone("hXsec_nuebar");

    //Divide by flux to get cross secion in units of cm^-2 for XSEC histograms:
    hXsec_numu   ->Divide(hFlux_numu);
    hXsec_numubar->Divide(hFlux_numubar);
    hXsec_nue    ->Divide(hFlux_nue);
    hXsec_nuebar ->Divide(hFlux_nuebar);

    //now scale by /nucleon:
    for (int i = 1; i <= hXsec_numu->GetNbinsX(); i++) {
        hXsec_numu   ->SetBinContent(i, hXsec_numu->GetBinContent(i) / N_target);
        hXsec_numubar->SetBinContent(i, hXsec_numubar->GetBinContent(i) / N_target);
        hXsec_nue    ->SetBinContent(i, hXsec_nue->GetBinContent(i) / N_target);
        hXsec_nuebar ->SetBinContent(i, hXsec_nuebar->GetBinContent(i) / N_target);
    }
    
    //draw canvas 
    TCanvas* c1 = new TCanvas("c1", "Cross Sections", 800, 600);
    //draw histograms with different colors
    hRate_numu   ->SetLineColor(kBlue);
    hRate_numubar->SetLineColor(kRed);
    hRate_nue    ->SetLineColor(kGreen);
    hRate_nuebar ->SetLineColor(kMagenta);
    hRate_numu   ->Draw("HIST");
    hRate_numubar->Draw("HIST SAME");
    hRate_nue    ->Draw("HIST SAME");
    hRate_nuebar ->Draw("HIST SAME");

    c1->BuildLegend();
    c1->SaveAs("macro_outputs/analysis_plots/1/CCQE_rates_plot.png");

    //Draw Xsec canvas
    TCanvas* c2 = new TCanvas("c2", "Cross Sections", 800, 600);
    //draw histograms with different colors
    hXsec_numu   ->SetLineColor(kBlue);
    hXsec_numubar->SetLineColor(kRed);
    hXsec_nue    ->SetLineColor(kGreen);
    hXsec_nuebar ->SetLineColor(kMagenta);
    hXsec_numu   ->Draw("HIST");
    hXsec_numubar->Draw("HIST SAME");
    hXsec_nue    ->Draw("HIST SAME");
    hXsec_nuebar ->Draw("HIST SAME");
    //relabel the axes
    hXsec_numu   ->GetXaxis()->SetTitle("Energy (GeV)");
    hXsec_numu   ->GetYaxis()->SetTitle("Cross Section (cm^{2})");
    //retitle the plot
    hXsec_numu   ->SetTitle("CCQE Cross Sections");
    c2->BuildLegend();
    c2->SaveAs("macro_outputs/analysis_plots/1/CCQE_xsec_plot.png");

    // ------------------------------------------------------------------ //
    //  Step 7:save output histograms to a new ROOT file
    // ------------------------------------------------------------------ //

    TFile* fOut = TFile::Open("macro_outputs/analysis_plots/1/CCQE_rates.root", "RECREATE");
    if (!fOut || fOut->IsZombie()) {
        std::cerr << "ERROR: cannot create output file." << std::endl;
        return;
    }
    hRate_numu   ->Write("hRate_numu");
    hRate_numubar->Write("hRate_numubar");
    hRate_nue    ->Write("hRate_nue");
    hRate_nuebar ->Write("hRate_nuebar");

    fOut->Close();
    f->Close();
    
   // std::cout << "\nOutput written to "CCQE_rates.root" << std::endl;
    std::cout << "Histograms: hRate_numu, hRate_numubar, hRate_nue, hRate_nuebar" << std::endl;
}
