#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

void plot_flux_ratiio_dat(const char* datFile = "macro_inputs/pospolarity_fluxes.dat",
                          const char* rootFile = "macro_outputs/overlaid_histograms_percmsq.root")
{
    gROOT->SetBatch(kTRUE);

    // ---- Read .dat file ----
    std::vector<double> edges;
    std::vector<double> numu, numub, nue, nueb;

    std::ifstream infile(datFile);
    if (!infile.is_open()) {
        std::cerr << "Error opening dat file\n";
        return;
    }

    std::string line;
    std::getline(infile, line);

    double lo, hi, a, b, c, d;

    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        ss >> lo >> hi >> a >> b >> c >> d;

        if (edges.empty()) edges.push_back(lo);
        edges.push_back(hi);

        numu.push_back(a);
        numub.push_back(b);
        nue.push_back(c);
        nueb.push_back(d);
    }

    int nbins = numu.size();

    // ---- Build .dat histograms ----
    TH1D *h_dat_numu  = new TH1D("h_dat_numu",  "", nbins, &edges[0]);
    TH1D *h_dat_numub = new TH1D("h_dat_numub", "", nbins, &edges[0]);
    TH1D *h_dat_nue   = new TH1D("h_dat_nue",   "", nbins, &edges[0]);
    TH1D *h_dat_nueb  = new TH1D("h_dat_nueb",  "", nbins, &edges[0]);

    for (int i = 0; i < nbins; i++) {
        h_dat_numu ->SetBinContent(i+1, numu[i]);
        h_dat_numub->SetBinContent(i+1, numub[i]);
        h_dat_nue  ->SetBinContent(i+1, nue[i]);
        h_dat_nueb ->SetBinContent(i+1, nueb[i]);
    }

    // ---- Open ROOT file ----
    TFile *f = TFile::Open(rootFile);
    if (!f || f->IsZombie()) {
        std::cerr << "Error opening ROOT file\n";
        return;
    }

    TH1 *h_numu  = (TH1*)f->Get("h10007001");
    TH1 *h_numub = (TH1*)f->Get("h10007002");
    TH1 *h_nue   = (TH1*)f->Get("h10007003");
    TH1 *h_nueb  = (TH1*)f->Get("h10007004");

    if (!h_numu || !h_numub || !h_nue || !h_nueb) {
        std::cerr << "Missing histograms\n";
        f->ls();
        return;
    }

    // ---- Helper to draw one flavour ----
    auto draw_with_ratio = [](TH1* h_dat, TH1* h_root, const char* name) {

    // ---- Rebin ROOT histogram properly (integrate over bin width) ----
    TH1D *h_root_rebinned = (TH1D*)h_dat->Clone(Form("%s_root_rebinned", name));
    h_root_rebinned->Reset();
    h_root_rebinned->Sumw2();

    for (int i = 1; i <= h_dat->GetNbinsX(); i++) {
        double xlow  = h_dat->GetBinLowEdge(i);
        double xhigh = h_dat->GetBinLowEdge(i+1);

        int bin1 = h_root->FindBin(xlow + 1e-9);
        int bin2 = h_root->FindBin(xhigh - 1e-9);

        double integral = h_root->Integral(bin1, bin2, "width");
        h_root_rebinned->SetBinContent(i, integral / (xhigh - xlow));
    }

    // ---- Normalise if needed (optional but often necessary) ----
    if (h_root_rebinned->Integral() > 0)
        h_root_rebinned->Scale(h_dat->Integral() / h_root_rebinned->Integral());

    // ---- Ratio with proper errors ----
    TH1D *h_ratio = (TH1D*)h_dat->Clone(Form("%s_ratio", name));
    h_ratio->Sumw2();
    h_ratio->Divide(h_root_rebinned);

    // ---- Canvas ----
    TCanvas *c = new TCanvas(name, name, 700, 700);

    TPad *p1 = new TPad(Form("%s_p1", name),"",0,0.3,1,1);
    TPad *p2 = new TPad(Form("%s_p2", name),"",0,0,1,0.3);

    p1->SetBottomMargin(0.02);
    p2->SetTopMargin(0.05);
    p2->SetBottomMargin(0.3);

    p1->Draw();
    p2->Draw();

    // ---- Top pad ----
    p1->cd();

    h_dat->SetLineColor(kRed);
    h_dat->SetLineWidth(2);

    h_root_rebinned->SetLineColor(kBlack);
    h_root_rebinned->SetLineStyle(2);
    h_root_rebinned->SetLineWidth(2);

    h_dat->SetTitle(Form("%s flux;Energy;Flux", name));

    h_dat->Draw("HIST");
    h_root_rebinned->Draw("HIST SAME");

    TLegend *leg = new TLegend(0.65,0.75,0.88,0.88);
    leg->AddEntry(h_dat,  "dat",  "l");
    leg->AddEntry(h_root_rebinned, "root (rebinned)", "l");
    leg->Draw();

    gPad->SetLogy();

    // ---- Bottom pad ----
    p2->cd();

    h_ratio->SetLineColor(kBlue);
    h_ratio->SetLineWidth(2);

    h_ratio->GetYaxis()->SetTitle("dat/root");
    h_ratio->GetYaxis()->SetNdivisions(505);
    h_ratio->GetYaxis()->SetTitleSize(0.08);
    h_ratio->GetYaxis()->SetLabelSize(0.08);

    h_ratio->GetXaxis()->SetTitle("Energy");
    h_ratio->GetXaxis()->SetTitleSize(0.1);
    h_ratio->GetXaxis()->SetLabelSize(0.08);

    h_ratio->SetMinimum(0.5);
    h_ratio->SetMaximum(1.5);

    h_ratio->Draw("E1");  // <-- better than HIST for ratios

    // unity line
    TLine *line = new TLine(h_ratio->GetXaxis()->GetXmin(), 1,
                           h_ratio->GetXaxis()->GetXmax(), 1);
    line->SetLineStyle(2);
    line->Draw();

    c->SaveAs(Form("%s.png", name));
};

    // ---- Produce plots ----
    draw_with_ratio(h_dat_numu,  h_numu,  "numu");
    draw_with_ratio(h_dat_numub, h_numub, "numub");
    draw_with_ratio(h_dat_nue,   h_nue,   "nue");
    draw_with_ratio(h_dat_nueb,  h_nueb,  "nueb");
}