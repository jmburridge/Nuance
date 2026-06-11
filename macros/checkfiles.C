#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TList.h>
#include <TFile.h>
#include <TTree.h>
#include <TKey.h>
#include <iostream>
#include <fstream>

void checkfiles(const char* topDir = "/exp/uboone/data/users/jburridg/Nuance/NUANCE/NUANCE_event_files/output/root/PoT_Nuance_event_files/19th_test_run") {

    int emptyCount   = 0;
    int noTreeCount  = 0;
    int validCount   = 0;
    int corruptCount = 0;
    std::vector<TString> noTreeFiles;

    std::ofstream report("19th_file_report.csv");
    report << "status,filepath,size_bytes,n_trees,tree_names\n";

    std::function<void(const char*)> scanDir = [&](const char* dirPath) {

        TSystemDirectory dir(dirPath, dirPath);
        TList* files = dir.GetListOfFiles();
        if (!files) return;

        TIter next(files);
        TSystemFile* file;

        while ((file = (TSystemFile*)next())) {
            TString name = file->GetName();
            if (name == "." || name == "..") continue;

            TString fullPath = TString(dirPath) + "/" + name;

            if (file->IsDirectory()) {
                scanDir(fullPath.Data());
                continue;
            }

            if (!name.EndsWith(".root")) continue;

            // --- Check 1: file size ---
            FileStat_t stat;
            gSystem->GetPathInfo(fullPath.Data(), stat);
            Long64_t fileSize = stat.fSize;

            if (fileSize == 0) {
                std::cout << "[EMPTY]    " << fullPath << std::endl;
                report << "EMPTY," << fullPath << ",0,0,\n";
                emptyCount++;
                continue;
            }

            // --- Check 2: can it be opened? ---
            TFile* f = TFile::Open(fullPath.Data(), "READ");
            if (!f || f->IsZombie() || f->TestBit(TFile::kRecovered)) {
                std::cout << "[CORRUPT]  " << fullPath << std::endl;
                report << "CORRUPT," << fullPath << "," << fileSize << ",0,\n";
                corruptCount++;
                if (f) f->Close();
                continue;
            }

            // --- Check 3: does it contain any TTrees? ---
            int nTrees = 0;
            TString treeNames = "";
            TList* keys = f->GetListOfKeys();

            if (keys) {
                TIter keyIter(keys);
                TKey* key;
                while ((key = (TKey*)keyIter())) {
                    TString className = key->GetClassName();
                    if (className == "TTree" || className == "TNtuple") {
                        nTrees++;
                        if (treeNames.Length() > 0) treeNames += ";";
                        treeNames += key->GetName();
                    }
                }
            }

            if (nTrees == 0) {
                std::cout << "[NO TREE]  " << fullPath << "  (size: " << fileSize << " bytes)" << std::endl;
                report << "NO_TREE," << fullPath << "," << fileSize << ",0,\n";
                noTreeCount++;
                noTreeFiles.push_back(fullPath);
            } else {
                std::cout << "[OK]       " << fullPath
                          << "  (" << nTrees << " tree(s): " << treeNames << ")" << std::endl;
                report << "OK," << fullPath << "," << fileSize << "," << nTrees << "," << treeNames << "\n";
                validCount++;
            }

            f->Close();
        }
        delete files;
    };

    std::cout << "\n=== Scanning: " << topDir << " ===\n" << std::endl;
    scanDir(topDir);

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Valid files (have trees): " << validCount   << std::endl;
    std::cout << "  Empty files (0 bytes):    " << emptyCount   << std::endl;
    std::cout << "  No TTree found:           " << noTreeCount  << std::endl;
    std::cout << "  Corrupt/unreadable:       " << corruptCount << std::endl;
    std::cout << "  TOTAL scanned:            " << validCount + emptyCount + noTreeCount + corruptCount << std::endl;
    std::cout << "========================================" << std::endl;

    // Print all no-tree filenames clearly at the end
    if (!noTreeFiles.empty()) {
        std::cout << "\n--- Files with NO TTree (" << noTreeCount << ") ---" << std::endl;
        for (const auto& fname : noTreeFiles) {
            std::cout << "  " << fname << std::endl;
        }
        std::cout << "-----------------------------------" << std::endl;

        // Also write them to a plain text list for easy cleanup
        std::ofstream noTreeList("19_no_tree_files.txt");
        for (const auto& fname : noTreeFiles) {
            noTreeList << fname << "\n";
        }
        noTreeList.close();
        std::cout << "  List saved to: 19_no_tree_files.txt" << std::endl;
    }

    std::cout << "  Report saved to: 19th_file_report.csv" << std::endl;
    report.close();
}