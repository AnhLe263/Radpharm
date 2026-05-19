#include "TCanvas.h"
#include "TF1.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TMath.h"
#include <vector>
#include <string>

// --- Normalized Bateman Equation Functor (Yielding Percentage of Saturation) ---
// x[0] corresponds to irradiation time t (hours)
// par[0] corresponds to the decay constant lambda (hours^-1)
double BatemanPercentageFunction(double *x, double *par) {
    double t = x[0];
    double lambda = par[0];
    // Returns the percentage: (1 - e^(-lambda * t)) * 100
    return (1.0 - TMath::Exp(-lambda * t)) * 100.0;
}

void PlotGenericBateman2() {
    // --- 1. Define Data Structure for n Isotopes ---
    struct Isotope {
        std::string name;
        double T_half_hours; // Half-life (hours)
        int color;
        int lineStyle;
    };

    // Array holding an arbitrary number of isotopes (R is no longer needed for % saturation)
    std::vector<Isotope> isotopes = {
        {"^{64}Cu", 12.7,       kBlue,   1}, // Primary target product
        {"^{13}N",  9.97 / 60.0, kRed,    2}, // Short-lived impurity (saturates rapidly)
        {"^{61}Cu", 3.33,        kGreen+2,3}, // Intermediate-lived impurity
        {"^{55}Co", 17.5,        kOrange, 4}  // Long-lived impurity
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
    TLegend *leg = new TLegend(0.55, 0.18, 0.88, 0.42); // Moved to bottom-right to avoid curve overlap
    leg->SetBorderSize(1);
    leg->SetTextFont(42);
    leg->SetTextSize(0.032);

    std::vector<TF1*> fList;

    // --- 3. Iterative Calculation for n Isotopes ---
    for (size_t i = 0; i < isotopes.size(); ++i) {
        double lambda = TMath::Log(2) / isotopes[i].T_half_hours;

        std::string fName = "f_" + isotopes[i].name;
        // TF1 configuration: 1 variable (t), 1 parameter (lambda)
        TF1 *f = new TF1(fName.c_str(), BatemanPercentageFunction, 0, 12, 1);
        f->SetParameter(0, lambda);
        
        // Apply aesthetic and graphical properties for each curve
        f->SetLineColor(isotopes[i].color);
        f->SetLineWidth(3);
        f->SetLineStyle(isotopes[i].lineStyle);

        fList.push_back(f);
    }

    // --- 4. Plotting Curves and Managing Layout ---
    for (size_t i = 0; i < fList.size(); ++i) {
        if (i == 0) {
            // Configure global axis properties using the first function plot
            fList[i]->SetTitle("Radioisotope Accumulation Relative to Saturation;Irradiation Time (hours);Saturation Level (%)");
            fList[i]->GetYaxis()->SetRangeUser(0.0, 110.0); // Y-axis scaled from 0% to 110% for clear layout
            fList[i]->GetXaxis()->SetRangeUser(0, 11);
            
            // Format labels and titles to standard publication quality
            fList[i]->GetXaxis()->SetTitleSize(0.045);
            fList[i]->GetXaxis()->SetLabelSize(0.04);
            fList[i]->GetXaxis()->SetTitleOffset(1.1);
            fList[i]->GetYaxis()->SetTitleSize(0.045);
            fList[i]->GetYaxis()->SetLabelSize(0.04);
            fList[i]->GetYaxis()->SetTitleOffset(1.3);
            
            fList[i]->Draw();
        } else {
            // Overlap subsequent curves onto the same canvas
            fList[i]->Draw("same");
        }
        
        // Append entry to the legend with calculated EOB percentage values
        double eob_pct = fList[i]->Eval(t_eob);
        std::string legEntry = isotopes[i].name + " (EOB: " + Form("%.1f", eob_pct) + "%)";
        leg->AddEntry(fList[i], legEntry.c_str(), "l");
    }

    // Draw the completed legend and refresh canvas
    leg->Draw();
    c1->Update();
    c1->Modified();
    
    // Automatically export as high-resolution image file
    c1->SaveAs("Bateman_Saturation_Percentage_Plot.png");
}