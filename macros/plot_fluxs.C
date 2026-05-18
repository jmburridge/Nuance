void plot_fluxs() {

  std::ifstream fin("macro_inputs/pospolarity_fluxes.dat");
  if (!fin) {
    std::cout << "Cannot open file\n";
    return;
  }

  double elo, ehi;
  double numu, numub, nue, nueb;

  std::vector<double> edges;
  std::vector<double> v_numu, v_numub, v_nue, v_nueb;

  // skip header
  std::string line;
  std::getline(fin, line);

  while (fin >> elo >> ehi >> numu >> numub >> nue >> nueb) {
    edges.push_back(elo);
    v_numu.push_back(numu);
    v_numub.push_back(numub);
    v_nue.push_back(nue);
    v_nueb.push_back(nueb);
  }

  // add last edge
  edges.push_back(ehi);

  int nbins = v_numu.size();

  auto makeHist = [&](const char* name, const char* title, std::vector<double>& vals, Color_t col) {
    TH1D* h = new TH1D(name, title, nbins, &edges[0]);
    for (int i = 0; i < nbins; i++) {
      h->SetBinContent(i+1, vals[i]);
    }
    h->SetLineColor(col);
    h->SetLineWidth(2);
    return h;
  };

  TH1D* h_numu  = makeHist("h_numu",  "#nu_{#mu} flux;E_{#nu} [GeV];flux", v_numu,  kBlue+1);
  TH1D* h_numub = makeHist("h_numub", "#bar{#nu}_{#mu} flux;E_{#nu} [GeV];flux", v_numub, kRed+1);
  TH1D* h_nue   = makeHist("h_nue",   "#nu_{e} flux;E_{#nu} [GeV];flux", v_nue,   kGreen+2);
  TH1D* h_nueb  = makeHist("h_nueb",  "#bar{#nu}_{e} flux;E_{#nu} [GeV];flux", v_nueb,  kMagenta+1);

  //scale the histograms by 1.168986e6 for detector area
  double scaleFactor = 1.168986e6;
  h_numu->Scale(scaleFactor);
  h_numub->Scale(scaleFactor);
  h_nue->Scale(scaleFactor);
  h_nueb->Scale(scaleFactor);
  // --- canvas 1: numu
  TCanvas* c1 = new TCanvas("c1", "numu flux", 800, 600);
  //c1->SetLogy();
  h_numu->Draw("HIST");

  // --- canvas 2: numub
  TCanvas* c2 = new TCanvas("c2", "numub flux", 800, 600);
  //c2->SetLogy();
  h_numub->Draw("HIST");

  // --- canvas 3: nue
  TCanvas* c3 = new TCanvas("c3", "nue flux", 800, 600);
  //c3->SetLogy();
  h_nue->Draw("HIST");

  // --- canvas 4: nueb
  TCanvas* c4 = new TCanvas("c4", "nueb flux", 800, 600);
  //c4->SetLogy();
  h_nueb->Draw("HIST");

  c1->SaveAs("numu_flux_det.png");
  c2->SaveAs("numub_flux_det.png");
  c3->SaveAs("nue_flux_det.png");
  c4->SaveAs("nueb_flux_det.png");
}