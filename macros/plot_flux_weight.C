//lol
void plot_flux_weight() {
    // Open the ROOT file
    TFile *file = TFile::Open("../output/root/second_test_run/combined/NUANCE_events_2.root");
    if (!file || file->IsZombie()) {
        std::cout << "Error opening file!" << std::endl;
        return;
    }

    // Get the tree "h3"
    TTree *tree = (TTree*)file->Get("h3");
    if (!tree) {
        std::cout << "Tree h3 not found!" << std::endl;
        return;
    }

    // Create histogram: 50 bins from 0 to 1
    TH1F *hist = new TH1F("hist", "Flux distribution;Flux;Entries", 50, 0, 0.1);

    // Draw the branch "flux" into the histogram
    tree->Draw("flux>>hist");

    // Draw histogram on canvas
    TCanvas *c1 = new TCanvas("c1", "Flux Plot", 800, 600);
    hist->Draw();

     // Save as PNG
    c1->SaveAs("macro_outputs/flux_weight.png");
}