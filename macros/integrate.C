//a macro to integrate a histogarm. 
// integrate_histogram.C
// Basic ROOT macro: open a file, read a tree, fill a histogram, integrate it

void integrate() {

    //1. Open the ROOT file 
    TFile *f = TFile::Open("/exp/uboone/data/users/jburridg/Nuance/NUANCE/NUANCE_event_files/output/root/first_test_run/combined/NUANCE_events.root");
    if (!f || f->IsZombie()) {
        std::cerr << "Error: cannot open file!" << std::endl;
        return;
    }

    //2. Get the tree
    TTree *tree = (TTree*)f->Get("h50");   // <-- change to your tree name
    if (!tree) {
        std::cerr << "Error: tree not found!" << std::endl;
        return;
    }

    //2. Or get the histogram
    TH1F *flux = (TH1F*)f->Get("h10007004");  // <-- change to your histogram name
        if (!flux) {
            std::cerr << "Error: histogram not found!" << std::endl;
            return;
        }

    //3. Create a histogram and fill it from the tree
    // Syntax: tree->Draw("branch >> histName(nBins, xMin, xMax)")
    TH1F *h = new TH1F("h", "My Histogram;X;Counts", 100, 0.0, 5500.0);
    tree->Draw("e_neutrino >> h");   // <-- change "myBranch" to your branch name

    //4. Integrate (sum all bin contents)
    // Integral()        — bins 1..N only (excludes under/overflow)
    // Integral(0, N+1)  — includes underflow (bin 0) and overflow (bin N+1)

    //double total = flux->Integral();   // excludes under/overflow
    double total = flux->Integral(0, flux->GetNbinsX() + 1);  // include overflow

    std::cout << "Total integral (sum of bin contents): " << total << std::endl;


    f->Close();  