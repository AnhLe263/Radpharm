#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
=========================================================
Author  : Lê Tuấn Anh
Purpose :
    Generalized calculation of enriched Ni(NO3)2 solution
    with arbitrary 64Ni enrichment and isotope composition.

Features:
    - Arbitrary 64Ni enrichment
    - User-defined impurity pattern
    - Self-consistent density solving
    - Elemental composition
    - Automatic isotope vector generation
    - CSV export

Physics/Chemistry Model:
    - Fixed solution volume
    - Enriched nickel nitrate dissolved in water/HNO3
    - Density correlation based on total solute wt%

=========================================================
"""

import numpy as np
import pandas as pd
from scipy.optimize import fsolve

# =========================================================
# USER INPUT
# =========================================================

# ---------------------------------------------------------
# Total solution volume (mL)
# ---------------------------------------------------------

VOLUME_ML = 2.0

# ---------------------------------------------------------
# Total enriched nickel mass (g)
#
# IMPORTANT:
# This is TOTAL nickel mass delivered by supplier.
#
# Example:
# "100 mg enriched 64Ni metal, 95% enrichment"
#
# means:
# total nickel mass = 100 mg
# NOT 100 mg pure 64Ni atoms
# ---------------------------------------------------------

TOTAL_NI_MASS = 0.100

# ---------------------------------------------------------
# 64Ni enrichment fraction
#
# Example:
# 0.95 = 95%
# ---------------------------------------------------------

A64 = 0.95

# ---------------------------------------------------------
# HNO3 concentration list (M)
# ---------------------------------------------------------

HNO3_CONCENTRATIONS = [
    0,
    0.005,
    0.01,
    0.05,
    0.1,
    0.5,
    1.0,
    2.0,
    3.0,
    5.0,
]

# =========================================================
# IMPURITY DISTRIBUTION
#
# Must sum to 1
#
# This defines HOW the remaining impurities are distributed.
# =========================================================

impurity_pattern = {
    "58Ni": 0.6870,
    "60Ni": 0.2640,
    "61Ni": 0.0123,
    "62Ni": 0.0367,
}

# =========================================================
# ISOTOPE MASSES (g/mol)
# =========================================================

isotope_mass = {
    "58Ni": 57.935342,
    "60Ni": 59.930786,
    "61Ni": 60.931056,
    "62Ni": 61.928345,
    "64Ni": 63.927966,
}

# =========================================================
# CONSTANTS
# =========================================================

MW_HNO3 = 63.01
MW_H2O = 18.015

# Effective nitrate salt molar mass:
# Ni + 2*(NO3)
#
# nitrate contribution:
# 2*(14.007 + 3*15.999) = 124.008 g/mol
#
# total salt molar mass depends on Ni isotopes
# =========================================================

NITRATE_GROUP_MASS = 124.008

# =========================================================
# CHECK IMPURITY SUM
# =========================================================

impurity_sum = sum(impurity_pattern.values())

if abs(impurity_sum - 1.0) > 1e-6:
    raise ValueError(
        "Impurity pattern must sum to 1."
    )

# =========================================================
# BUILD ISOTOPE VECTOR
# =========================================================

ni_isotopes = {}

# Main isotope
ni_isotopes["64Ni"] = {
    "fraction": A64,
    "mass": isotope_mass["64Ni"],
}

# Remaining fraction
remaining = 1.0 - A64

for iso, rel_frac in impurity_pattern.items():

    ni_isotopes[iso] = {
        "fraction": remaining * rel_frac,
        "mass": isotope_mass[iso],
    }

# =========================================================
# PRINT ISOTOPE VECTOR
# =========================================================

print("\n==============================")
print("NICKEL ISOTOPE COMPOSITION")
print("==============================")

for iso, data in ni_isotopes.items():
    print(f"{iso:>5s} : {100*data['fraction']:.4f} %")

# =========================================================
# EFFECTIVE MOLAR MASS
# =========================================================

MW_NI_EFFECTIVE = sum(
    iso["fraction"] * iso["mass"]
    for iso in ni_isotopes.values()
)

print("\nEffective Ni molar mass = "
      f"{MW_NI_EFFECTIVE:.6f} g/mol")

# =========================================================
# EFFECTIVE SALT MOLAR MASS
# =========================================================

MW_SALT_EFFECTIVE = (
    MW_NI_EFFECTIVE + NITRATE_GROUP_MASS
)

print("Effective Ni(NO3)2 molar mass = "
      f"{MW_SALT_EFFECTIVE:.6f} g/mol")

# =========================================================
# MASS OF SALT
# =========================================================

mass_salt = (
    TOTAL_NI_MASS
    * MW_SALT_EFFECTIVE
    / MW_NI_EFFECTIVE
)

print(f"\nTotal salt mass = "
      f"{mass_salt:.6f} g")

# =========================================================
# DENSITY FUNCTION
#
# x = total solute wt%
# =========================================================

def density(x):

    return (
        7.760509e-05 * x**2
        + 8.087669e-03 * x
        + 9.986652e-01
    )

# =========================================================
# SOLVER
# =========================================================

def solve_solution(CM_HNO3):

    # -----------------------------------------------------
    # HNO3 mass
    # -----------------------------------------------------

    mass_hno3 = (
        CM_HNO3
        * (VOLUME_ML / 1000.0)
        * MW_HNO3
    )

    # -----------------------------------------------------
    # Total solute mass
    # -----------------------------------------------------

    mass_solute = mass_salt + mass_hno3

    # -----------------------------------------------------
    # Solve self-consistent equation
    #
    # x = wt% solute
    #
    # total_mass = rho * volume
    # -----------------------------------------------------

    def equation(x):

        rho = density(x)

        total_mass = rho * VOLUME_ML

        return (
            x
            - 100.0 * mass_solute / total_mass
        )

    # Initial guess
    x0 = 10.0

    x_solution = fsolve(equation, x0)[0]

    rho_solution = density(x_solution)

    total_mass = rho_solution * VOLUME_ML

    mass_water = total_mass - mass_solute

    # -----------------------------------------------------
    # Weight percentages
    # -----------------------------------------------------

    wt_salt = (
        100.0 * mass_salt / total_mass
    )

    wt_hno3 = (
        100.0 * mass_hno3 / total_mass
    )

    wt_water = (
        100.0 * mass_water / total_mass
    )

    # =====================================================
    # ELEMENTAL COMPOSITION
    # =====================================================

    AW_N = 14.007
    AW_O = 15.999
    AW_H = 1.008

    # -----------------------------------------------------
    # Nickel isotope masses
    # -----------------------------------------------------

    nickel_element_mass = TOTAL_NI_MASS

    # -----------------------------------------------------
    # Nitrogen from nitrate
    # -----------------------------------------------------

    nitrogen_from_salt = (
        mass_salt
        * (2 * AW_N)
        / MW_SALT_EFFECTIVE
    )

    nitrogen_from_hno3 = (
        mass_hno3
        * AW_N
        / MW_HNO3
    )

    total_nitrogen = (
        nitrogen_from_salt
        + nitrogen_from_hno3
    )

    # -----------------------------------------------------
    # Oxygen
    # -----------------------------------------------------

    oxygen_from_salt = (
        mass_salt
        * (6 * AW_O)
        / MW_SALT_EFFECTIVE
    )

    oxygen_from_hno3 = (
        mass_hno3
        * (3 * AW_O)
        / MW_HNO3
    )

    oxygen_from_water = (
        mass_water
        * AW_O
        / MW_H2O
    )

    total_oxygen = (
        oxygen_from_salt
        + oxygen_from_hno3
        + oxygen_from_water
    )

    # -----------------------------------------------------
    # Hydrogen
    # -----------------------------------------------------

    hydrogen_from_hno3 = (
        mass_hno3
        * AW_H
        / MW_HNO3
    )

    hydrogen_from_water = (
        mass_water
        * (2 * AW_H)
        / MW_H2O
    )

    total_hydrogen = (
        hydrogen_from_hno3
        + hydrogen_from_water
    )

    # -----------------------------------------------------
    # wt%
    # -----------------------------------------------------

    wt_Ni = (
        100.0
        * nickel_element_mass
        / total_mass
    )

    wt_N = (
        100.0
        * total_nitrogen
        / total_mass
    )

    wt_O = (
        100.0
        * total_oxygen
        / total_mass
    )

    wt_H = (
        100.0
        * total_hydrogen
        / total_mass
    )

    # =====================================================
    # Isotope-specific wt%
    # =====================================================

    isotope_wt = {}

    for iso, data in ni_isotopes.items():

        iso_mass = (
            TOTAL_NI_MASS
            * data["fraction"]
        )

        isotope_wt[iso] = (
            100.0
            * iso_mass
            / total_mass
        )

    # =====================================================
    # Output dictionary
    # =====================================================

    output = {
        "HNO3_M": CM_HNO3,
        "Density_g_cm3": rho_solution,
        "wt_Ni(NO3)2": wt_salt,
        "wt_HNO3": wt_hno3,
        "wt_H2O": wt_water,
        "wt_Ni_total": wt_Ni,
        "wt_N": wt_N,
        "wt_O": wt_O,
        "wt_H": wt_H,
    }

    # Add isotope wt%
    output.update(isotope_wt)

    return output

# =========================================================
# RUN CALCULATIONS
# =========================================================

results = []

for c in HNO3_CONCENTRATIONS:

    results.append(
        solve_solution(c)
    )

df = pd.DataFrame(results)

# =========================================================
# DISPLAY RESULTS
# =========================================================

pd.set_option("display.max_columns", None)
pd.set_option("display.width", 300)

print("\n==============================")
print("FINAL RESULTS")
print("==============================\n")

print(df.round(7))

# =========================================================
# EXPORT CSV
# =========================================================

csv_name = f"Ni64_HNO3_generalized_results_{A64*100}%.csv"

df.to_csv(csv_name, index=False)

print("\nCSV exported:")
print(csv_name)

# =========================================================
# OPTIONAL: EXPORT EXCEL
# =========================================================

#excel_name = f"Ni64_HNO3_generalized_results_{A64*100}%.xlsx"

#df.to_excel(excel_name, index=False)

#print("\nExcel exported:")
#print(excel_name)

# =========================================================
# END
# =========================================================
