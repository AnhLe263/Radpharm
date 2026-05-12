#include <TFile.h>
#include <TTree.h>
#include "TH1F.h"
#include <iostream>
#include <string>
#include <map>

void ana2() {
    // 1. Mở file ROOT
    const char* filename = "Out.root";
    TFile *file = TFile::Open(filename);
    if (!file || file->IsZombie()) {
        std::cerr << "Không thể mở file!" << std::endl;
        return;
    }

    // 2. Lấy Tree
    TTree *tree = (TTree*)file->Get("Energy");
    if (!tree) {
        std::cerr << "Không tìm thấy Tree 'Products'!" << std::endl;
        return;
    }

    // 3. Khai báo biến đọc dữ liệu (chỉ lấy các nhánh cần thiết)
    Int_t volumeID;
    Double_t ene;
    tree->SetBranchAddress("volumeID", &volumeID);
    tree->SetBranchAddress("energy", &ene);

    // 4. Khai báo MAP để thống kê
    // 5. Duyệt Tree và đổ dữ liệu vào map
    Long64_t nEntries = tree->GetEntries();
    std::cout << "Đang xử lý " << nEntries << " entries..." << std::endl;
   
    TH1F *hE1 = new TH1F("Energy1","proton energy",1000,0.,20);
    TH1F *hE2 = new TH1F("Energy2","proton energy",1000,0.,20);
    for (Long64_t i = 0; i < nEntries; i++) {
        tree->GetEntry(i);
        
        // Tăng số đếm cho hạt 'name' tại 'volumeID' tương ứng
        // Nếu key chưa tồn tại, map sẽ tự khởi tạo giá trị là 0 rồi cộng thêm 1
        if (volumeID == 1) hE1->Fill(ene);
        if (volumeID == 3) hE2->Fill(ene);
    }

    
    

    TCanvas* c = new TCanvas("c1", "Energy Analysis", 800, 600);
    gPad->SetLogy(); // Thường phổ năng lượng nên để Log scale ở trục Y

    auto hframe = c->DrawFrame(0,1e0,20,1e7,"; proton energy [MeV]; Frequency");
    hframe->GetXaxis()->CenterTitle();
    hframe->GetYaxis()->CenterTitle();

    hE1->SetLineColor(kBlue);
    hE2->SetLineColor(kRed);

    hE1->SetFillColorAlpha(kBlue, 0.35); // Tô màu xanh với độ trong suốt 35%
    hE1->SetFillStyle(1001);     
    hE2->SetFillColorAlpha(kRed, 0.35);  // Tô màu đỏ với độ trong suốt 35%
    hE2->SetFillStyle(1001);        // Kiểu tô đặc

    hE1->Draw("same");
    hE2->Draw("same");

    // Thêm chú thích (Legend)
    TLegend *leg = new TLegend(0.15, 0.75, 0.35, 0.88);
    leg->AddEntry(hE1, "After Ti (75 um)", "f"); // "f" đại diện cho Fill
    leg->AddEntry(hE2, "After Havar (38 um)", "f");
    leg->SetBorderSize(0);
    leg->Draw();

    c->Update();
    // 7. Đóng file
    //file->Close();
}