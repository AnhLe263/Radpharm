#include <TFile.h>
#include <TTree.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TLegend.h>
#include <vector>
#include <map>

TString GetVolumeName(Int_t i);

void plotEdep() {
    // 1. Mở file
    TFile *file = TFile::Open("Out.root");
    if (!file || file->IsZombie()) return;
    
    TTree *tree = (TTree*)file->Get("Edep");
    if (!tree) return;

    Int_t volumeID;
    Double_t edep, posX, posZ;
    tree->SetBranchAddress("volumeID", &volumeID);
    tree->SetBranchAddress("edep", &edep);
    tree->SetBranchAddress("posX", &posX);
    tree->SetBranchAddress("posZ", &posZ);

    // 2. Khởi tạo Histogram
    // Dùng std::map để quản lý histogram theo ID (1, 3, 4, 5) cho dễ truy xuất
    std::map<Int_t, TH2F*> hMap;
    
    // Histogram tổng
    TH2F *h_all = new TH2F("h_all", "Total Energy Deposition;Z;X", 1000, -1, 5, 1000, -1, 5);
    
    // Các ID mục tiêu
    std::vector<Int_t> ids = {1, 3, 4, 5};
    for(int id : ids) {
        TString name = Form("h_vol%d", id);
        TString title = Form("Volume: %s;Z;X", GetVolumeName(id).Data());
        hMap[id] = new TH2F(name, title, 1000, -1, 5, 1000, -1, 5);
        hMap[id]->SetLineColor(id + 1); 
    }

    // 3. Fill dữ liệu
    Long64_t nEntries = tree->GetEntries();
    for (Long64_t i = 0; i < nEntries; i++) {
        tree->GetEntry(i);
        // Chỉ nhận volumeID 1, 3, 4, 5
        if (hMap.count(volumeID)) {
            h_all->Fill(posZ, posX, edep);       
            hMap[volumeID]->Fill(posZ, posX, edep); 
        }
    }

    // 4. Thống nhất mapping màu cho các volume con
    double maxZ = 0;
    for(auto const& [id, h] : hMap) {
        if(h->GetMaximum() > maxZ) maxZ = h->GetMaximum();
    }
    for(auto const& [id, h] : hMap) {
        h->SetMinimum(0);
        h->SetMaximum(maxZ);
    }

    gStyle->SetOptStat(0);
    gStyle->SetPalette(kBird);

    // --- Canvas 1: Histogram tổng ---
    TCanvas *c1 = new TCanvas("c1", "Canvas 1 - Total", 800, 600);
    h_all->Draw("COLZ");

    // --- Canvas 2: Từng volume riêng biệt ---
    TCanvas *c2 = new TCanvas("c2", "Canvas 2 - Individual", 1200, 800);
    c2->Divide(2, 2); // Chia 4 ô cho 4 volume
    int pad = 1;
    for(int id : ids) {
        c2->cd(pad++);
        hMap[id]->Draw("COLZ");
    }

    // --- Canvas 3: Vẽ chung trên cùng 1 đồ thị ---
    TCanvas *c3 = new TCanvas("c3", "Canvas 3 - Overlaid", 800, 600);
    
    // Vẽ cái đầu tiên (ID 1) để lấy khung
    hMap[1]->SetTitle("Overlaid Volumes (1, 3, 4, 5);Z;X");
    hMap[1]->Draw("COL"); 
    
    // Vẽ các cái còn lại
    for(size_t i = 1; i < ids.size(); i++) {
        hMap[ids[i]]->Draw("COL SAME");
    }
    
    // Hiển thị Palette chung dựa trên thang đo đã thống nhất
    hMap[1]->Draw("COLZ SAME"); 

    // Chú thích
    TLegend *leg = new TLegend(0.15, 0.7, 0.4, 0.88);
    for(int id : ids) {
        leg->AddEntry(hMap[id], GetVolumeName(id), "f");
    }
    leg->Draw();
}

TString GetVolumeName(Int_t ID) {
    if (ID == 0) return "World";
    if (ID == 1) return "Tilayer";
    if (ID == 2) return "Helayer";
    if (ID == 3) return "Havarlayer";
    if (ID == 4) return "Nblayer";
    if (ID == 5) return "Target";
    return "Unknown";
}