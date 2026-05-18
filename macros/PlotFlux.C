{
  //The flux comes from here:
  //https://www-boone.fnal.gov/for_physicists/data_release/flux/pospolarity_fluxes.dat
  // This macro takes the miniboone data release flux and plots it in the same format 
  // as the flux in the root files, saving theas the same names for use in 
  // other macros easily. 

  TCanvas* c = new TCanvas("c","c",0,0,700,700);
  c->SetLogy();

  ifstream file("macro_inputs/pospolarity_fluxes.dat");
  std::string line;

  Int_t lineCounter = 0;

  TH1D* hNuMu = new TH1D("hNuMu","",200,0,10);
  hNuMu->SetStats(00000);
  TH1D* hNuMuBar = new TH1D("hNuMuBar","",200,0,10);
  TH1D* hNuE = new TH1D("hNuE","",200,0,10);
  TH1D* hNuEBar = new TH1D("hNuEBar","",200,0,10);
  
  while (std::getline(file, line)){
    std::istringstream iss(line);

    std::string elo;
    std::string ehi;
    std::string numu;
    std::string numub;
    std::string nue;
    std::string nueb;
    
    std::getline(iss, elo, ' ');
    std::getline(iss, ehi, ' ');
    std::getline(iss, numu, ' ');
    std::getline(iss, numub, ' ');
    std::getline(iss, nue, ' ');
    std::getline(iss, nueb, ' ');

    hNuMu->SetBinContent(lineCounter+1,std::stod(numu));
    hNuMuBar->SetBinContent(lineCounter+1,std::stod(numub));
    hNuE->SetBinContent(lineCounter+1,std::stod(nue));
    hNuEBar->SetBinContent(lineCounter+1,std::stod(nueb));
    
    // cout << "Row " << lineCounter << ": elo: " << elo
    // 	 << ", ehi: " << ehi
    // 	 << ", numu: " << numu
    // 	 << ", numub: " << numub
    // 	 << ", nue: " << nue
    // 	 << ", nueb: " << nueb
    // 	 << endl;
    lineCounter++;
  }

  hNuMu->GetXaxis()->SetTitle("Neutrino energy / GeV");
  hNuMu->GetYaxis()->SetTitle("Flux / cm^{-2}(PoT)^{-1}(50 MeV)^{-1}");
  hNuMu->SetLineColor(kRed);
  hNuMu->SetLineWidth(4);
  hNuMu->Draw();
  hNuMuBar->SetLineColor(kRed-9);
  hNuMuBar->SetLineWidth(4);
  hNuMuBar->Draw("same");
  hNuE->SetLineColor(kBlue);
  hNuE->SetLineWidth(4);
  hNuE->Draw("same");
  hNuEBar->SetLineColor(kBlue-9);
  hNuEBar->SetLineWidth(4);
  hNuEBar->Draw("same");

  TLegend* leg = new TLegend(0.6,0.6,0.9,0.9);
  leg->SetBorderSize(0);
  leg->AddEntry(hNuMu, "#nu_{#mu}", "l");
  leg->AddEntry(hNuMuBar, "#bar{#nu}_{#mu}", "l");
  leg->AddEntry(hNuE, "#nu_{e}", "l");
  leg->AddEntry(hNuEBar, "#bar{#nu}_{e}", "l");
  leg->Draw();

  TFile* outFile = new TFile("macro_outputs/miniboone_datarelease_flux.root", "RECREATE");

  hNuMu->Write("h10007003");
  hNuMuBar->Write("h10007004");
  hNuE->Write("h10007001");
  hNuEBar->Write("h10007002");

  outFile->Close();
  
  cout << "End." << endl;

}