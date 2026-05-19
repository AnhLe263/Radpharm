#include "TCanvas.h"
#include "TF1.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TMath.h"
#include <vector>
#include <string>

// --- Generalized Bateman Equation Functor ---
// x[0] corresponds to irradiation time t (hours)
// par[0] corresponds to the production rate R (particles/hour)
// par[1] corresponds to the decay constant lambda (hours^-1)
double BatemanFunction(double *x, double *par) {
    double t = x[0];
    double R = par[0];
    double lambda = par[1];
    return (R / lambda) * (1.0 - TMath::Exp(-lambda * t));
}

void PlotGenericBateman() {
    // --- 1. Define Data Structure for n Isotopes ---
    struct Isotope {
        std::string name;
        double R_per_sec;    // Production rate (particles/second)
        double T_half_hours; // Half-life (hours)
        int color;
        int lineStyle;
    };

    // Array holding an arbitrary number of isotopes (fully customizable)
    std::vector<Isotope> isotopes = {
        {"^{64}Cu", 40000.0, 12.7,       kBlue,   1}, // Primary target product
        {"^{13}N",  15000.0, 9.97 / 60.0, kRed,    2}, // Short-lived impurity (saturates rapidly)
        {"^{61}Cu",  5000.0, 3.33,        kGreen+2,3}, // Intermediate-lived impurity
        {"^{55}Co",  2000.0, 17.5,        kOrange, 4}  // Long-lived impurity
    };

    double t_eob = 10.0; // End of Beam (EOB) time in hours

    // --- 2. Canvas and Graphics Configuration ---
    gStyle->SetOptStat(0);
    gStyle->SetFrameLineWidth(2);
    
    TCanvas *c1 = new TCanvas("c1", "Generalized Bateman Simulation", 900, 650);
    c1->SetGrid();
    c1->SetLeftMargin(0.14);
    c1->SetBottomMargin(0.12);
    c1->SetRightMargin(0.08);

    // Initialize Legend
    TLegend *leg = new TLegend(0.16, 0.65, 0.55, 0.88);
    leg->SetBorderSize(1);
    leg->SetTextFont(42);
    leg->SetTextSize(0.032);

    double max_Y = 0;
    std::vector<TF1*> fList;

    // --- 3. Iterative Calculation for n Isotopes ---
    for (size_t i = 0; i < isotopes.size(); ++i) {
        // Convert production rate from per-second to per-hour
        double R_hour = isotopes[i].R_per_sec * 3600.0;
        double lambda = TMath::Log(2) / isotopes[i].T_half_hours;

        std::string fName = "f_" + isotopes[i].name;
        TF1 *f = new TF1(fName.c_str(), BatemanFunction, 0, 12, 2);
        f->SetParameters(R_hour, lambda);
        
        // Apply aesthetic and graphical properties for each curve
        f->SetLineColor(isotopes[i].color);
        f->SetLineWidth(3);
        f->SetLineStyle(isotopes[i].lineStyle);

        // Track the global maximum value for automatic Y-axis scaling
        double eob_val = f->Eval(t_eob);
        if (eob_val > max_Y) max_Y = eob_val;
        if (f->GetMaximum(0, 12) > max_Y && f->GetMaximum(0, 12) < 1e15) {
            max_Y = f->GetMaximum(0, 12);
        }

        fList.push_back(f);
    }

    // --- 4. Plotting Curves and Managing Layout ---
    for (size_t i = 0; i < fList.size(); ++i) {
        if (i == 0) {
            // Configure global axis properties using the first function plot
            fList[i]->SetTitle("Generalized Accumulation of Radioisotopes;Irradiation Time (hours);Number of Radioactive Particles");
            fList[i]->GetYaxis()->SetRangeUser(0, max_Y * 1.25); // 25% overhead margin
            fList[i]->GetXaxis()->SetRangeUser(0, 11);
            
            // Format labels and titles to standard publication quality
            fList[i]->GetXaxis()->SetTitleSize(0.045);
            fList[i]->GetXaxis()->SetLabelSize(0.04);
            fList[i]->GetXaxis()->SetTitleOffset(1.1);
            fList[i]->GetYaxis()->SetTitleSize(0.045);
            fList[i]->GetYaxis()->SetLabelSize(0.04);
            fList[i]->GetYaxis()->SetTitleOffset(1.4);
            
            fList[i]->Draw();
        } else {
            // Overlap subsequent curves onto the same canvas
            fList[i]->Draw("same");
        }
        
        // Append entry to the legend with calculated EOB values
        double eob_val = fList[i]->Eval(t_eob);
        std::string legEntry = isotopes[i].name + " (EOB: " + Form("%.2e", eob_val) + ")";
        leg->AddEntry(fList[i], legEntry.c_str(), "l");
    }

    // Draw the completed legend and refresh canvas
    leg->Draw();
    c1->Update();
    c1->Modified();
    
    // Automatically export as high-resolution image file
    c1->SaveAs("Generalized_Bateman_Plot.png");
}