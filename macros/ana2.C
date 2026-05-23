#include <TFile.h>
#include <TTree.h>
#include "TH1F.h"
#include <iostream>
#include <string>
#include <map>

void ana2() {
    Double_t N = 10000000; // Thay bằng tổng số hạt p đã chạy mô phỏng
    Double_t real_rate = 40e-6 / 1.602e-19; // ~ 2.5e14 hạt/giây (for 40 uA)
    Double_t scaleFactor = real_rate / N;
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

    
    hE1->Scale(scaleFactor);
    hE2->Scale(scaleFactor);

    TCanvas* c = new TCanvas("c1", "Energy Analysis", 800, 600);
    gPad->SetLogy(); // Thường phổ năng lượng nên để Log scale ở trục Y

    auto hframe = c->DrawFrame(0,1e7,20,1e14,"; proton energy [MeV]; #proton [s^{-1}]");
    hframe->GetXaxis()->CenterTitle();
    hframe->GetYaxis()->CenterTitle();

    hE1->SetLineColor(kBlue);
    hE2->SetLineColor(kRed);

    hE1->SetFillColorAlpha(kBlue, 0.35); // Tô màu xanh với độ trong suốt 35%
    hE1->SetFillStyle(1001);     
    hE2->SetFillColorAlpha(kRed, 0.35);  // Tô màu đỏ với độ trong suốt 35%
    hE2->SetFillStyle(1001);        // Kiểu tô đặc

    hE1->Draw("HIST same");
    hE2->Draw("HIST same");

    // Thêm chú thích (Legend)
    TLegend *leg = new TLegend(0.15, 0.75, 0.35, 0.88);
    leg->AddEntry(hE1, "After Ti", "f"); // "f" đại diện cho Fill
    leg->AddEntry(hE2, "After Havar", "f");
    leg->SetBorderSize(0);
    leg->Draw();

    // =================================================================
    // PHẦN CODE CẬP NHẬT: CHỈ HIỂN THỊ GIÁ TRỊ MEAN
    // =================================================================
    TLatex text;
    text.SetNDC();          
    text.SetTextSize(0.035); 

    // Hiển thị Mean cho hE1 (Chữ màu Xanh)
    text.SetTextColor(kBlue);
    text.DrawLatex(0.16, 0.63, Form("Mean_{Ti} = %.2f MeV", hE1->GetMean()));

    // Hiển thị Mean cho hE2 (Chữ màu Đỏ)
    text.SetTextColor(kRed);
    text.DrawLatex(0.16, 0.57, Form("Mean_{Havar} = %.2f MeV", hE2->GetMean()));
    // =================================================================

    c->Update();

    // Lấy Mean và MeanError cho hE1
    Double_t mean1      = hE1->GetMean();
    Double_t meanError1 = hE1->GetMeanError();

    // Lấy Mean và MeanError cho hE2
    Double_t mean2      = hE2->GetMean();
    Double_t meanError2 = hE2->GetMeanError();

    // In kết quả ra màn hình theo định dạng Mean +/- MeanError
    std::cout << "\n================ KẾT QUẢ ================" << std::endl;
    std::cout << "Histogram hE1: " << mean1 << " +/- " << meanError1 << std::endl;
    std::cout << "Histogram hE2: " << mean2 << " +/- " << meanError2 << std::endl;
    std::cout << "=========================================\n" << std::endl;

    // 7. Đóng file
    //file->Close();
}