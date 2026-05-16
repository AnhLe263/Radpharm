#include <TFile.h>
#include <TTree.h>
#include "TH1F.h"
#include <iostream>
#include <string>
#include <map>
TString GetVolumeName(Int_t ID);
void ana() {
    // 1. Mở file ROOT
    const char* filename = "Out.root";
    TFile *file = TFile::Open(filename);
    if (!file || file->IsZombie()) {
        std::cerr << "Không thể mở file!" << std::endl;
        return;
    }

    // 2. Lấy Tree
    TTree *tree = (TTree*)file->Get("Products");
    if (!tree) {
        std::cerr << "Không tìm thấy Tree 'Products'!" << std::endl;
        return;
    }

    // 3. Khai báo biến đọc dữ liệu (chỉ lấy các nhánh cần thiết)
    char name[256], target[256], track[256];
    Int_t volumeID;
    Double_t posZ;
    tree->SetBranchAddress("name", name);
    tree->SetBranchAddress("volumeID", &volumeID);
    tree->SetBranchAddress("track", &track);
    tree->SetBranchAddress("target", &target);
    tree->SetBranchAddress("posZ", &posZ);

    // 4. Khai báo MAP để thống kê
    // Cấu trúc: map[volumeID][particle_name] = count
    std::map<int, std::map<std::string, int>> volumeStats;
     std::map<int, std::map<std::string, int>> ReactionStats;

    // 5. Duyệt Tree và đổ dữ liệu vào map
    Long64_t nEntries = tree->GetEntries();
    std::cout << "Đang xử lý " << nEntries << " entries..." << std::endl;
    Double_t Z_vol5_offset = 2.0755;//mm
    TH1F *hz0 = new TH1F("hZCu64","Yield of Cu-64 along the depth of Target chamber",100,0.,3.2);
    for (Long64_t i = 0; i < nEntries; i++) {
        tree->GetEntry(i);
        
        // Tăng số đếm cho hạt 'name' tại 'volumeID' tương ứng
        // Nếu key chưa tồn tại, map sẽ tự khởi tạo giá trị là 0 rồi cộng thêm 1
        std::string sname(name);
        size_t pos = sname.find("[");
        if (pos != std::string::npos) {
            auto tmpname = sname.substr(0, pos);
            if (tmpname == "Cu64") sname = tmpname;
        }
        volumeStats[volumeID][sname]++;
        //std::cout<<volumeID<<std::endl;
        if (volumeID == 5) {
            auto ztemp = posZ - Z_vol5_offset;
            hz0->Fill(ztemp);
        }
        std::string st1(track);
        std::string st2(target);
        std::string react = st1+"("+st2+")"+sname;
        ReactionStats[volumeID][react]++;

    }

    // 6. In kết quả thống kê ra màn hình
    std::cout << "\n--- KẾT QUẢ THỐNG KÊ ---" << std::endl;
    std::cout << "VolumeID | Reaction | Count" << std::endl;
    std::cout << "--------------------------------" << std::endl;
    for (auto const& [volID, reacMap] : ReactionStats) {
        for (auto const& [rName, count] : reacMap) {
            std::printf("ID: %-5s | Reaction: %-10s | Count: %d\n", GetVolumeName(volID).Data(), rName.c_str(), count);
        }
        std::cout << "--------------------------------" << std::endl;
    }
    hz0->Draw();

    
    std::cout << "VolumeID | Particle Name | Count" << std::endl;
    std::cout << "--------------------------------" << std::endl;

    for (auto const& [volID, particleMap] : volumeStats) {
        for (auto const& [pName, count] : particleMap) {
            std::printf("ID: %-5s | Name: %-10s | Count: %d\n", GetVolumeName(volID).Data(), pName.c_str(), count);
        }
        std::cout << "--------------------------------" << std::endl;
    }

    
    // 7. Đóng file
    //file->Close();
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