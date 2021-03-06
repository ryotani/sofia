// ------------------------------------------------------------
// -----                  R3BSofTwimOnlineSpectra           -----
// -----    Created 29/09/19  by J.L. Rodriguez-Sanchez   -----
// -----           Fill SOFIA online histograms           -----
// ------------------------------------------------------------

/*
 * This task should fill histograms with SOFIA online data
 */

#include "R3BSofTwimOnlineSpectra.h"
#include "R3BEventHeader.h"
#include "R3BSofMwpcHitData.h"
#include "R3BSofTwimCalData.h"
#include "R3BSofTwimHitData.h"
#include "R3BSofTwimMappedData.h"
#include "THttpServer.h"

#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRunOnline.h"
#include "FairRuntimeDb.h"
#include "TCanvas.h"
#include "TFolder.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TVector3.h"

#include "TClonesArray.h"
#include "TLegend.h"
#include "TLegendEntry.h"
#include "TMath.h"
#include "TRandom.h"
#include <array>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

R3BSofTwimOnlineSpectra::R3BSofTwimOnlineSpectra()
    : FairTask("SofTwimOnlineSpectra", 1)
    , fMappedItemsTwim(NULL)
    , fCalItemsTwim(NULL)
    , fHitItemsTwim(NULL)
    , fHitItemsMwpc3(NULL)
    , fNEvents(0)
{
}

R3BSofTwimOnlineSpectra::R3BSofTwimOnlineSpectra(const TString& name, Int_t iVerbose)
    : FairTask(name, iVerbose)
    , fMappedItemsTwim(NULL)
    , fCalItemsTwim(NULL)
    , fHitItemsTwim(NULL)
    , fHitItemsMwpc3(NULL)
    , fNEvents(0)
{
}

R3BSofTwimOnlineSpectra::~R3BSofTwimOnlineSpectra()
{
    LOG(INFO) << "R3BSofTwimOnlineSpectra::Delete instance";
    if (fMappedItemsTwim)
        delete fMappedItemsTwim;
    if (fCalItemsTwim)
        delete fCalItemsTwim;
    if (fHitItemsTwim)
        delete fHitItemsTwim;
    if (fHitItemsMwpc3)
        delete fHitItemsMwpc3;
}

InitStatus R3BSofTwimOnlineSpectra::Init()
{

    LOG(INFO) << "R3BSofTwimOnlineSpectra::Init ";

    // try to get a handle on the EventHeader. EventHeader may not be
    // present though and hence may be null. Take care when using.

    FairRootManager* mgr = FairRootManager::Instance();
    if (NULL == mgr)
        LOG(FATAL) << "R3BSofTwimOnlineSpectra::Init FairRootManager not found";
    // header = (R3BEventHeader*)mgr->GetObject("R3BEventHeader");

    FairRunOnline* run = FairRunOnline::Instance();
    run->GetHttpServer()->Register("", this);

    // get access to mapped data of the TWIM
    fMappedItemsTwim = (TClonesArray*)mgr->GetObject("TwimMappedData");
    if (!fMappedItemsTwim)
    {
        return kFATAL;
    }

    // get access to cal data of the TWIM
    fCalItemsTwim = (TClonesArray*)mgr->GetObject("TwimCalData");
    if (!fCalItemsTwim)
        LOG(WARNING) << "R3BSofTwimOnlineSpectra: TwimCalData not found";

    // get access to hit data of the TWIM
    fHitItemsTwim = (TClonesArray*)mgr->GetObject("TwimHitData");
    if (!fHitItemsTwim)
        LOG(WARNING) << "R3BSofTwimOnlineSpectra: TwimHitData not found";

    // get access to hit data of the MWPC3
    fHitItemsMwpc3 = (TClonesArray*)mgr->GetObject("Mwpc3HitData");
    if (!fHitItemsMwpc3)
        LOG(WARNING) << "R3BSofTwimOnlineSpectra: Mwpc3HitData not found";

    // Create histograms for detectors
    char Name1[255];
    char Name2[255];

    // TWIM: Map data for E
    for (Int_t i = 0; i < NbSections; i++)
    {
        sprintf(Name1, "Twim_Emap_Sec_%d", i + 1);
        sprintf(Name2, "Section %d", i + 1);
        cTwimMap_E[i] = new TCanvas(Name1, Name2, 10, 10, 800, 700);
        cTwimMap_E[i]->Divide(4, 4);
    }

    for (Int_t i = 0; i < NbSections; i++)
    {
        for (Int_t j = 0; j < NbAnodes; j++)
        {
            sprintf(Name1, "fh1_twim_Emap_sec%d_a%d", i + 1, j + 1);
            sprintf(Name2, "Sec %d:Anode %d", i + 1, j + 1);
            fh1_twimmap_E[i][j] = new TH1F(Name1, Name2, 8192, 0, 8192);
            fh1_twimmap_E[i][j]->GetXaxis()->SetTitle("Raw Energy [channels], if mult==1");
            fh1_twimmap_E[i][j]->GetYaxis()->SetTitle("Counts");
            fh1_twimmap_E[i][j]->GetYaxis()->SetTitleOffset(1.1);
            fh1_twimmap_E[i][j]->GetXaxis()->CenterTitle(true);
            fh1_twimmap_E[i][j]->GetYaxis()->CenterTitle(true);
            fh1_twimmap_E[i][j]->GetXaxis()->SetLabelSize(0.045);
            fh1_twimmap_E[i][j]->GetXaxis()->SetTitleSize(0.045);
            fh1_twimmap_E[i][j]->GetYaxis()->SetLabelSize(0.045);
            fh1_twimmap_E[i][j]->GetYaxis()->SetTitleSize(0.045);
            fh1_twimmap_E[i][j]->SetFillColor(31);
            cTwimMap_E[i]->cd(j + 1);
            fh1_twimmap_E[i][j]->Draw("");
        }
    }

    // TWIM: Map data for E vs DT
    for (Int_t i = 0; i < NbSections; i++)
    {
        sprintf(Name1, "Twim_EvsDT_Sec_%d", i + 1);
        sprintf(Name2, "Section %d", i + 1);
        cTwimMap_EvsDT[i] = new TCanvas(Name1, Name2, 10, 10, 800, 700);
        cTwimMap_EvsDT[i]->Divide(4, 4);
        sprintf(Name1, "Twim_DTvsDT_Sec_%d", i + 1);
        sprintf(Name2, "DTvsDT Section %d", i + 1);
        cTwim_DTvsDT[i] = new TCanvas(Name1, Name2, 10, 10, 800, 700);
        cTwim_DTvsDT[i]->Divide(4, 4);
    }

    char Name3[255];
    char Name4[255];
    cTwimMap_EsumvsDT = new TCanvas("Twim_EsumvsDT", "Twim_EsumvsDT", 10, 10, 800, 700);
    if (NbSections > 1)
        cTwimMap_EsumvsDT->Divide(2, 2);

    for (Int_t i = 0; i < NbSections; i++)
    {
        for (Int_t j = 0; j < NbAnodes; j++)
        {
            sprintf(Name1, "fh1_twim_EvsDT_sec%d_a%d", i + 1, j + 1);
            sprintf(Name2, "Sec %d:Anode %d", i + 1, j + 1);
            fh2_twim_EneRawVsDriftTime[i][j] = new TH2F(Name1, Name2, 500, 0, 8192, 800, 1, 40001);
            fh2_twim_EneRawVsDriftTime[i][j]->GetXaxis()->SetTitle("Energy [channels, 8k ADC resolution], if mult==1");
            fh2_twim_EneRawVsDriftTime[i][j]->GetYaxis()->SetTitle(
                "Drift time [channels, 100ps TDC resolution], if mult==1");
            fh2_twim_EneRawVsDriftTime[i][j]->GetYaxis()->SetTitleOffset(1.1);
            fh2_twim_EneRawVsDriftTime[i][j]->GetXaxis()->CenterTitle(true);
            fh2_twim_EneRawVsDriftTime[i][j]->GetYaxis()->CenterTitle(true);
            fh2_twim_EneRawVsDriftTime[i][j]->GetXaxis()->SetLabelSize(0.045);
            fh2_twim_EneRawVsDriftTime[i][j]->GetXaxis()->SetTitleSize(0.045);
            fh2_twim_EneRawVsDriftTime[i][j]->GetYaxis()->SetLabelSize(0.045);
            fh2_twim_EneRawVsDriftTime[i][j]->GetYaxis()->SetTitleSize(0.045);
            cTwimMap_EvsDT[i]->cd(j + 1);
            fh2_twim_EneRawVsDriftTime[i][j]->Draw("col");
        }

        for (Int_t j = 0; j < NbAnodes - 1; j++)
        {
            sprintf(Name1, "fh2_twim_DTvsDT_sec%d_a%d", i + 1, j);
            sprintf(Name2, "Sec %d: DT_%d vs DT_%d", i + 1, j + 1, j + 2);
            fh2_twim_DTvsDT[i][j] = new TH2F(Name1, Name2, 800, 1, 40001, 800, 1, 40001);
            sprintf(Name1, "Drift time %d [channels, 100ps TDC resolution]", j + 1);
            sprintf(Name2, "Drift time %d [channels, 100ps TDC resolution]", j + 2);
            fh2_twim_DTvsDT[i][j]->GetXaxis()->SetTitle(Name1);
            fh2_twim_DTvsDT[i][j]->GetYaxis()->SetTitle(Name2);
            fh2_twim_DTvsDT[i][j]->GetYaxis()->SetTitleOffset(1.1);
            fh2_twim_DTvsDT[i][j]->GetXaxis()->CenterTitle(true);
            fh2_twim_DTvsDT[i][j]->GetYaxis()->CenterTitle(true);
            fh2_twim_DTvsDT[i][j]->GetXaxis()->SetLabelSize(0.045);
            fh2_twim_DTvsDT[i][j]->GetXaxis()->SetTitleSize(0.045);
            fh2_twim_DTvsDT[i][j]->GetYaxis()->SetLabelSize(0.045);
            fh2_twim_DTvsDT[i][j]->GetYaxis()->SetTitleSize(0.045);
            cTwim_DTvsDT[i]->cd(j + 1);
            fh2_twim_DTvsDT[i][j]->Draw("col");
        }

        sprintf(Name3, "fh2_Twim_EsumvsDT_sec%d", i + 1);
        sprintf(Name4, "Twim: Esum vs DT for sec%d", i + 1);
        fh2_twim_EneRawSumVsDriftTime[i] = new TH2F(Name3, Name4, 1200, 1, 25000, 2192, 0, 8192);
        fh2_twim_EneRawSumVsDriftTime[i]->GetXaxis()->SetTitle("Drift time [channels, 100ps TDC resolution]");
        fh2_twim_EneRawSumVsDriftTime[i]->GetYaxis()->SetTitle("Energy Sum [channels]");
        fh2_twim_EneRawSumVsDriftTime[i]->GetYaxis()->SetTitleOffset(1.1);
        fh2_twim_EneRawSumVsDriftTime[i]->GetXaxis()->CenterTitle(true);
        fh2_twim_EneRawSumVsDriftTime[i]->GetYaxis()->CenterTitle(true);
        fh2_twim_EneRawSumVsDriftTime[i]->GetXaxis()->SetLabelSize(0.045);
        fh2_twim_EneRawSumVsDriftTime[i]->GetXaxis()->SetTitleSize(0.045);
        fh2_twim_EneRawSumVsDriftTime[i]->GetYaxis()->SetLabelSize(0.045);
        fh2_twim_EneRawSumVsDriftTime[i]->GetYaxis()->SetTitleSize(0.045);
        if (NbSections > 1)
            cTwimMap_EsumvsDT->cd(i + 1);
        else
            cTwimMap_EsumvsDT->cd();
        fh2_twim_EneRawSumVsDriftTime[i]->Draw("col");
    }

    // TWIM: Map data for DT
    for (Int_t i = 0; i < NbSections; i++)
    {
        sprintf(Name1, "Twim_DTmap_Sec_%d", i + 1);
        sprintf(Name2, "Section %d", i + 1);
        cTwimMap_DT[i] = new TCanvas(Name1, Name2, 10, 10, 800, 700);
        cTwimMap_DT[i]->Divide(4, 4);
    }

    for (Int_t i = 0; i < NbSections; i++)
    {
        for (Int_t j = 0; j < NbAnodes; j++)
        {
            sprintf(Name1, "fh1_twim_DTmap_sec%d_a%d", i + 1, j + 1);
            sprintf(Name2, "Sec %d:Anode %d", i + 1, j + 1);
            fh1_twimmap_DT[i][j] = new TH1F(Name1, Name2, 6000, 1, 40001);
            fh1_twimmap_DT[i][j]->GetXaxis()->SetTitle("Drift time [channels, 100ps TDC resolution]");
            fh1_twimmap_DT[i][j]->GetYaxis()->SetTitle("Counts");
            fh1_twimmap_DT[i][j]->GetYaxis()->SetTitleOffset(1.1);
            fh1_twimmap_DT[i][j]->GetXaxis()->CenterTitle(true);
            fh1_twimmap_DT[i][j]->GetYaxis()->CenterTitle(true);
            fh1_twimmap_DT[i][j]->GetXaxis()->SetLabelSize(0.045);
            fh1_twimmap_DT[i][j]->GetXaxis()->SetTitleSize(0.045);
            fh1_twimmap_DT[i][j]->GetYaxis()->SetLabelSize(0.045);
            fh1_twimmap_DT[i][j]->GetYaxis()->SetTitleSize(0.045);
            fh1_twimmap_DT[i][j]->SetFillColor(31);
            cTwimMap_DT[i]->cd(j + 1);
            fh1_twimmap_DT[i][j]->Draw("");
        }
    }

    // TWIM: Map data for multiplicities
    TCanvas* cTwim_Mult[NbSections];
    for (Int_t i = 0; i < NbSections; i++)
    {
        sprintf(Name1, "Twim_multiplicity_Sec_%d", i + 1);
        sprintf(Name2, "TWIM: Multiplicity per anode in section %d", i + 1);
        cTwim_Mult[i] = new TCanvas(Name1, Name2, 10, 10, 800, 700);
        sprintf(Name1, "fh1_Twim_mult_sec_%d", i + 1);
        sprintf(Name2, "Twim: Multiplicity section %d", i + 1);
        fh1_Twimmap_mult[i] = new TH1F(Name1, Name2, NbAnodes + NbTref + NbTrig, 0, NbAnodes + NbTref + NbTrig);
        fh1_Twimmap_mult[i]->GetXaxis()->SetTitle("Anode");
        fh1_Twimmap_mult[i]->GetYaxis()->SetTitle("Counts");
        fh1_Twimmap_mult[i]->GetYaxis()->SetTitleOffset(1.1);
        fh1_Twimmap_mult[i]->GetXaxis()->CenterTitle(true);
        fh1_Twimmap_mult[i]->GetYaxis()->CenterTitle(true);
        fh1_Twimmap_mult[i]->GetXaxis()->SetLabelSize(0.045);
        fh1_Twimmap_mult[i]->GetXaxis()->SetTitleSize(0.045);
        fh1_Twimmap_mult[i]->GetYaxis()->SetLabelSize(0.045);
        fh1_Twimmap_mult[i]->GetYaxis()->SetTitleSize(0.045);
        fh1_Twimmap_mult[i]->SetFillColor(31);
        fh1_Twimmap_mult[i]->Draw("");
    }

    // TWIM: Map data for E in the first (anodes from 0 to 7) and final sections (anodes from 8 to 15)
    cTwimMap_ESum = new TCanvas("twim_ESum_1,2", "twim_ESum_1,2", 10, 10, 800, 700);
    cTwimMap_ESum->Divide(1, 2);
    cTwimMap_ESum->cd(1);
    fh1_twim_ESum[0] = new TH1F("fh1_twim_ESum1", "twim:ESum:first", 8192, 0, 8192);
    fh1_twim_ESum[0]->GetXaxis()->SetTitle("Energy [channels]");
    fh1_twim_ESum[0]->GetYaxis()->SetTitle("Counts");
    fh1_twim_ESum[0]->GetYaxis()->SetTitleOffset(1.1);
    fh1_twim_ESum[0]->GetXaxis()->CenterTitle(true);
    fh1_twim_ESum[0]->GetYaxis()->CenterTitle(true);
    fh1_twim_ESum[0]->GetXaxis()->SetLabelSize(0.045);
    fh1_twim_ESum[0]->GetXaxis()->SetTitleSize(0.045);
    fh1_twim_ESum[0]->GetYaxis()->SetLabelSize(0.045);
    fh1_twim_ESum[0]->GetYaxis()->SetTitleSize(0.045);
    fh1_twim_ESum[0]->SetFillColor(31);
    fh1_twim_ESum[0]->Draw("");
    cTwimMap_ESum->cd(2);
    fh1_twim_ESum[1] = new TH1F("fh1_twim_ESum2", "twim:ESum:Second", 8192, 0, 8192);
    fh1_twim_ESum[1]->GetXaxis()->SetTitle("Energy [channels]");
    fh1_twim_ESum[1]->GetYaxis()->SetTitle("Counts");
    fh1_twim_ESum[1]->GetYaxis()->SetTitleOffset(1.1);
    fh1_twim_ESum[1]->GetXaxis()->CenterTitle(true);
    fh1_twim_ESum[1]->GetYaxis()->CenterTitle(true);
    fh1_twim_ESum[1]->GetXaxis()->SetLabelSize(0.045);
    fh1_twim_ESum[1]->GetXaxis()->SetTitleSize(0.045);
    fh1_twim_ESum[1]->GetYaxis()->SetLabelSize(0.045);
    fh1_twim_ESum[1]->GetYaxis()->SetTitleSize(0.045);
    fh1_twim_ESum[1]->SetFillColor(31);
    fh1_twim_ESum[1]->Draw("");

    // TWIM: Map data for Esum
    cTwimMap_ESum1 = new TCanvas("twim_ESum", "twim_ESum", 10, 10, 800, 700);
    fh1_twim_ESum[2] = new TH1F("fh1_twim_ESum", "twim:ESum", 8192, 0, 8192);
    fh1_twim_ESum[2]->GetXaxis()->SetTitle("Energy [channels]");
    fh1_twim_ESum[2]->GetYaxis()->SetTitle("Counts");
    fh1_twim_ESum[2]->GetYaxis()->SetTitleOffset(1.1);
    fh1_twim_ESum[2]->GetXaxis()->CenterTitle(true);
    fh1_twim_ESum[2]->GetYaxis()->CenterTitle(true);
    fh1_twim_ESum[2]->GetXaxis()->SetLabelSize(0.045);
    fh1_twim_ESum[2]->GetXaxis()->SetTitleSize(0.045);
    fh1_twim_ESum[2]->GetYaxis()->SetLabelSize(0.045);
    fh1_twim_ESum[2]->GetYaxis()->SetTitleSize(0.045);
    fh1_twim_ESum[2]->SetFillColor(31);
    fh1_twim_ESum[2]->Draw("");

    // TWIM: Map data for Esum1 vs Esum2
    cTwimMap_ESum2 = new TCanvas("twim_E1vsE2", "twim_E1vsE2", 10, 10, 800, 700);
    fh2_twim_ESum = new TH2F("fh2_twim_ESum", "twim: ESum1 vs Esum2", 2192, 0, 8192, 2192, 0, 8192);
    fh2_twim_ESum->GetXaxis()->SetTitle("Energy1 [channels]");
    fh2_twim_ESum->GetYaxis()->SetTitle("Energy2 [channels]");
    fh2_twim_ESum->GetYaxis()->SetTitleOffset(1.1);
    fh2_twim_ESum->GetXaxis()->CenterTitle(true);
    fh2_twim_ESum->GetYaxis()->CenterTitle(true);
    fh2_twim_ESum->GetXaxis()->SetLabelSize(0.045);
    fh2_twim_ESum->GetXaxis()->SetTitleSize(0.045);
    fh2_twim_ESum->GetYaxis()->SetLabelSize(0.045);
    fh2_twim_ESum->GetYaxis()->SetTitleSize(0.045);
    fh2_twim_ESum->Draw("col");

    // TWIM: Esum versus DT16-DT1
    cTwimMap_ESum_vs_diffDT = new TCanvas("twim_ESum_vs_diffDT", "twim_ESum_vs_diffDT", 10, 10, 800, 700);
    if (NbSections > 1)
        cTwimMap_ESum_vs_diffDT->Divide(2, 2);
    for (Int_t i = 0; i < NbSections; i++)
    {
        sprintf(Name1, "fh2_twim_ESum_vs_diffDT_Sec_%d", i + 1);
        sprintf(Name2, "TWIM: ESum vs diffDT in section %d", i + 1);
        fh2_twim_ESum_vs_diffDT[i] = new TH2F(Name1, Name2, 2000, -2000, 2000, 2192, 0, 8192);
        fh2_twim_ESum_vs_diffDT[i]->GetXaxis()->SetTitle("DT difference [channels]");
        fh2_twim_ESum_vs_diffDT[i]->GetYaxis()->SetTitle("Energy Sum [channels]");
        fh2_twim_ESum_vs_diffDT[i]->GetYaxis()->SetTitleOffset(1.1);
        fh2_twim_ESum_vs_diffDT[i]->GetXaxis()->CenterTitle(true);
        fh2_twim_ESum_vs_diffDT[i]->GetYaxis()->CenterTitle(true);
        fh2_twim_ESum_vs_diffDT[i]->GetXaxis()->SetLabelSize(0.045);
        fh2_twim_ESum_vs_diffDT[i]->GetXaxis()->SetTitleSize(0.045);
        fh2_twim_ESum_vs_diffDT[i]->GetYaxis()->SetLabelSize(0.045);
        fh2_twim_ESum_vs_diffDT[i]->GetYaxis()->SetTitleSize(0.045);
        if (NbSections > 1)
            cTwimMap_ESum_vs_diffDT->cd(i + 1);
        else
            cTwimMap_ESum_vs_diffDT->cd();
        fh2_twim_ESum_vs_diffDT[i]->Draw("col");
    }

    // Twin_MUSIC: Map for delta t between Tref and Trig
    for (Int_t i = 0; i < NbSections; i++)
    {
        sprintf(Name1, "Twim_TrefTrig_Sec_%d", i + 1);
        sprintf(Name2, "Delta T (Tref-Trig) section %d", i + 1);
        cTwimMap_DeltaTrefTrig[i] = new TCanvas(Name1, Name2, 10, 10, 800, 700);
        cTwimMap_DeltaTrefTrig[i]->Divide(1, NbTref);
        for (Int_t j = 0; j < NbTref; j++)
        {
            sprintf(Name1, "fh1_Twim_Sec%d_DeltaTrefTrig%d", i + 1, j + 1);
            sprintf(Name2,
                    "Delta T (Tref%d-Trig%d) in section %d [channels, 100ps TDC resolution] 1ns/bin",
                    j + 1,
                    j + 1,
                    i + 1);
            fh1_twimmap_DeltaTrefTrig[i][j] = new TH1F(Name1, Name2, 8000, -40000, 40000);
            cTwimMap_DeltaTrefTrig[i]->cd(j + 1);
            fh1_twimmap_DeltaTrefTrig[i][j]->Draw("");
        }
    }

    // Cal data, position in mm for each anode
    for (Int_t i = 0; i < NbSections; i++)
    {
        sprintf(Name1, "Twim_Pos-X_Sec_%d", i + 1);
        sprintf(Name2, "Pos-X in mm section %d", i + 1);
        cTwimCal_Pos[i] = new TCanvas(Name1, Name2, 10, 10, 800, 700);
        cTwimCal_Pos[i]->Divide(4, 4);
        for (Int_t j = 0; j < NbAnodes; j++)
        {
            sprintf(Name1, "fh1_Twim_Sec%d_Pos_a%d", i + 1, j + 1);
            sprintf(Name2, "Sec %d, Anode %d", i + 1, j + 1);
            if (i == 0 || i == 3)
                fh1_Twimcal_Pos[i][j] = new TH1F(Name1, Name2, 500, -50., 90.);
            else
                fh1_Twimcal_Pos[i][j] = new TH1F(Name1, Name2, 500, -100., 0.);
            fh1_Twimcal_Pos[i][j]->GetXaxis()->SetTitle("Position-X [mm]");
            fh1_Twimcal_Pos[i][j]->GetYaxis()->SetTitle("Counts");
            fh1_Twimcal_Pos[i][j]->GetYaxis()->SetTitleOffset(1.1);
            fh1_Twimcal_Pos[i][j]->GetXaxis()->CenterTitle(true);
            fh1_Twimcal_Pos[i][j]->GetYaxis()->CenterTitle(true);
            fh1_Twimcal_Pos[i][j]->GetXaxis()->SetLabelSize(0.045);
            fh1_Twimcal_Pos[i][j]->GetXaxis()->SetTitleSize(0.045);
            fh1_Twimcal_Pos[i][j]->GetYaxis()->SetLabelSize(0.045);
            fh1_Twimcal_Pos[i][j]->GetYaxis()->SetTitleSize(0.045);
            fh1_Twimcal_Pos[i][j]->SetFillColor(31);
            fh1_Twimcal_Pos[i][j]->SetLineColor(1);
            cTwimCal_Pos[i]->cd(j + 1);
            fh1_Twimcal_Pos[i][j]->Draw("");
        }
    }

    // Hit data
    TCanvas* cTwim_Z = new TCanvas("Twim_charge_z", "Twim: Charge Z", 10, 10, 800, 700);
    fh1_Twimhit_z = new TH1F("fh1_Twim_charge_z", "Twim: Charge Z", 1200, 6, 38);
    fh1_Twimhit_z->GetXaxis()->SetTitle("Charge (Z)");
    fh1_Twimhit_z->GetYaxis()->SetTitle("Counts");
    fh1_Twimhit_z->GetYaxis()->SetTitleOffset(1.1);
    fh1_Twimhit_z->GetXaxis()->CenterTitle(true);
    fh1_Twimhit_z->GetYaxis()->CenterTitle(true);
    fh1_Twimhit_z->GetXaxis()->SetLabelSize(0.045);
    fh1_Twimhit_z->GetXaxis()->SetTitleSize(0.045);
    fh1_Twimhit_z->GetYaxis()->SetLabelSize(0.045);
    fh1_Twimhit_z->GetYaxis()->SetTitleSize(0.045);
    fh1_Twimhit_z->SetFillColor(29);
    fh1_Twimhit_z->SetLineColor(1);
    fh1_Twimhit_z->Draw("");

    TCanvas* cTwim_theta = new TCanvas("Twim_theta", "Twim: #theta_{XZ}", 10, 10, 800, 700);
    fh1_Twimhit_theta = new TH1F("fh1_Twim_theta", "Twim: #theta_{XZ}", 900, -60, 60);
    fh1_Twimhit_theta->GetXaxis()->SetTitle("#theta_{XZ} [mrad]");
    fh1_Twimhit_theta->GetYaxis()->SetTitle("Counts");
    fh1_Twimhit_theta->GetYaxis()->SetTitleOffset(1.1);
    fh1_Twimhit_theta->GetXaxis()->CenterTitle(true);
    fh1_Twimhit_theta->GetYaxis()->CenterTitle(true);
    fh1_Twimhit_theta->GetXaxis()->SetLabelSize(0.045);
    fh1_Twimhit_theta->GetXaxis()->SetTitleSize(0.045);
    fh1_Twimhit_theta->GetYaxis()->SetLabelSize(0.045);
    fh1_Twimhit_theta->GetYaxis()->SetTitleSize(0.045);
    fh1_Twimhit_theta->SetFillColor(29);
    fh1_Twimhit_theta->SetLineColor(1);
    fh1_Twimhit_theta->Draw("");

    TCanvas* cTwim_zvstheta = new TCanvas("Twim_charge_vs_theta", "Twim: Charge Z vs #theta_{XZ}", 10, 10, 800, 700);
    fh2_Twimhit_zvstheta =
        new TH2F("fh2_Twim_Charge_Z_vs_theta", "Twim: Charge Z vs #theta_{XZ}", 900, -40, 40, 1000, 6, 38);
    fh2_Twimhit_zvstheta->GetXaxis()->SetTitle("#theta_{XZ} [mrad]");
    fh2_Twimhit_zvstheta->GetYaxis()->SetTitle("Charge (Z)");
    fh2_Twimhit_zvstheta->GetYaxis()->SetTitleOffset(1.1);
    fh2_Twimhit_zvstheta->GetXaxis()->CenterTitle(true);
    fh2_Twimhit_zvstheta->GetYaxis()->CenterTitle(true);
    fh2_Twimhit_zvstheta->GetXaxis()->SetLabelSize(0.045);
    fh2_Twimhit_zvstheta->GetXaxis()->SetTitleSize(0.045);
    fh2_Twimhit_zvstheta->GetYaxis()->SetLabelSize(0.045);
    fh2_Twimhit_zvstheta->GetYaxis()->SetTitleSize(0.045);
    fh2_Twimhit_zvstheta->Draw("colz");

    cTwimTheta_vs_mwpc3x = new TCanvas("Twim_theta_vs_mwpc3x", "Twim: Theta vs Mwpc3-X", 10, 10, 800, 700);
    fh2_TwimTheta_vs_mwpc3x =
        new TH2F("fh2_TwiwTheta_vs_mwpc3", "Twim: #theta_{xz} vs MWPC3X", 900, -40, 40, 800, -400, 400);
    fh2_TwimTheta_vs_mwpc3x->GetXaxis()->SetTitle("#theta_{XZ} [mrad]");
    fh2_TwimTheta_vs_mwpc3x->GetYaxis()->SetTitle("(Wixhausen)<---  Mwpc3-X [mm]  ---> (Messel)");
    fh2_TwimTheta_vs_mwpc3x->GetYaxis()->SetTitleOffset(1.1);
    fh2_TwimTheta_vs_mwpc3x->GetXaxis()->CenterTitle(true);
    fh2_TwimTheta_vs_mwpc3x->GetYaxis()->CenterTitle(true);
    fh2_TwimTheta_vs_mwpc3x->GetXaxis()->SetLabelSize(0.045);
    fh2_TwimTheta_vs_mwpc3x->GetXaxis()->SetTitleSize(0.045);
    fh2_TwimTheta_vs_mwpc3x->GetYaxis()->SetLabelSize(0.045);
    fh2_TwimTheta_vs_mwpc3x->GetYaxis()->SetTitleSize(0.045);
    fh2_TwimTheta_vs_mwpc3x->Draw("colz");

    cTwimZ_vs_mwpc3x = new TCanvas("Twim_Z_vs_mwpc3x", "Twim: Z vs Mwpc3-X", 10, 10, 800, 700);
    fh2_TwimZ_vs_mwpc3x = new TH2F("fh2_TwiwZ_vs_mwpc3", "Twim: Z vs Mwpc3-X", 800, -400, 400, 700, 6, 38);
    fh2_TwimZ_vs_mwpc3x->GetXaxis()->SetTitle("(Wixhausen)<---  Mwpc3-X [mm]  ---> (Messel)");
    fh2_TwimZ_vs_mwpc3x->GetYaxis()->SetTitle("Charge Z");
    fh2_TwimZ_vs_mwpc3x->GetYaxis()->SetTitleOffset(1.1);
    fh2_TwimZ_vs_mwpc3x->GetXaxis()->CenterTitle(true);
    fh2_TwimZ_vs_mwpc3x->GetYaxis()->CenterTitle(true);
    fh2_TwimZ_vs_mwpc3x->GetXaxis()->SetLabelSize(0.045);
    fh2_TwimZ_vs_mwpc3x->GetXaxis()->SetTitleSize(0.045);
    fh2_TwimZ_vs_mwpc3x->GetYaxis()->SetLabelSize(0.045);
    fh2_TwimZ_vs_mwpc3x->GetYaxis()->SetTitleSize(0.045);
    fh2_TwimZ_vs_mwpc3x->Draw("colz");

    // MAIN FOLDER-Twim
    TFolder* mainfolTwim = new TFolder("TWIM", "TWIM info");
    for (Int_t i = 0; i < NbSections; i++)
        mainfolTwim->Add(cTwimMap_E[i]);
    for (Int_t i = 0; i < NbSections; i++)
        mainfolTwim->Add(cTwimMap_DT[i]);
    for (Int_t i = 0; i < NbSections; i++)
        mainfolTwim->Add(cTwimMap_DeltaTrefTrig[i]);
    for (Int_t i = 0; i < NbSections; i++)
        mainfolTwim->Add(cTwim_Mult[i]);
    mainfolTwim->Add(cTwimMap_ESum);
    mainfolTwim->Add(cTwimMap_ESum1);
    mainfolTwim->Add(cTwimMap_ESum2);
    mainfolTwim->Add(cTwimMap_ESum_vs_diffDT);
    for (Int_t i = 0; i < NbSections; i++)
    {
        mainfolTwim->Add(cTwimMap_EvsDT[i]);
        mainfolTwim->Add(cTwim_DTvsDT[i]);
    }
    mainfolTwim->Add(cTwimMap_EsumvsDT);
    if (fCalItemsTwim)
    {
        for (Int_t i = 0; i < NbSections; i++)
            mainfolTwim->Add(cTwimCal_Pos[i]);
    }
    if (fHitItemsTwim)
    {
        mainfolTwim->Add(cTwim_Z);
        mainfolTwim->Add(cTwim_theta);
        mainfolTwim->Add(cTwim_zvstheta);
        if (fHitItemsMwpc3)
        {
            mainfolTwim->Add(cTwimTheta_vs_mwpc3x);
            mainfolTwim->Add(cTwimZ_vs_mwpc3x);
        }
    }
    run->AddObject(mainfolTwim);

    // Register command to reset histograms
    run->GetHttpServer()->RegisterCommand("Reset_TWIM_HIST", Form("/Objects/%s/->Reset_Histo()", GetName()));

    return kSUCCESS;
}

void R3BSofTwimOnlineSpectra::Reset_Histo()
{
    LOG(INFO) << "R3BSofTwimOnlineSpectra::Reset_Histo";
    // Map data
    for (Int_t i = 0; i < NbSections; i++)
    {
        fh1_Twimmap_mult[i]->Reset();
        fh2_twim_EneRawSumVsDriftTime[i]->Reset();
        fh2_twim_ESum_vs_diffDT[i]->Reset();
        for (Int_t j = 0; j < NbAnodes; j++)
        {
            fh1_twimmap_E[i][j]->Reset();
            fh1_twimmap_DT[i][j]->Reset();
            fh2_twim_EneRawVsDriftTime[i][j]->Reset();
        }
        for (Int_t j = 0; j < NbAnodes - 1; j++)
        {
            fh2_twim_DTvsDT[i][j]->Reset();
        }
        for (Int_t j = 0; j < NbTref; j++)
        {
            fh1_twimmap_DeltaTrefTrig[i][j]->Reset();
        }
    }
    fh1_twim_ESum[0]->Reset();
    fh1_twim_ESum[1]->Reset();
    fh1_twim_ESum[2]->Reset();
    fh2_twim_ESum->Reset();

    // Cal data
    if (fCalItemsTwim)
    {
        for (Int_t i = 0; i < NbSections; i++)
            for (Int_t j = 0; j < NbAnodes; j++)
                fh1_Twimcal_Pos[i][j]->Reset();
    }

    // Hit data
    if (fHitItemsTwim)
    {
        fh1_Twimhit_z->Reset();
        fh1_Twimhit_theta->Reset();
        fh2_Twimhit_zvstheta->Reset();
        if (fHitItemsMwpc3)
        {
            fh2_TwimTheta_vs_mwpc3x->Reset();
            fh2_TwimZ_vs_mwpc3x->Reset();
        }
    }
}

void R3BSofTwimOnlineSpectra::Exec(Option_t* option)
{
    FairRootManager* mgr = FairRootManager::Instance();
    if (NULL == mgr)
        LOG(FATAL) << "R3BSofTwimOnlineSpectra::Exec FairRootManager not found";

    // Fill mapped data
    if (fMappedItemsTwim && fMappedItemsTwim->GetEntriesFast() > 0)
    {
        Double_t Eraw[NbSections][NbAnodes];
        Double_t e1 = 0., e2 = 0.;
        Double_t Traw[NbSections][NbAnodes + NbTref + NbTrig];
        Double_t n1 = 0., n2 = 0.;
        UInt_t mult[NbSections][NbAnodes + NbTref + NbTrig];
        for (Int_t j = 0; j < NbSections; j++)
            for (Int_t i = 0; i < NbAnodes; i++)
                Eraw[j][i] = 0.; // mult=1 !!!
        for (Int_t j = 0; j < NbSections; j++)
            for (Int_t i = 0; i < NbAnodes + NbTref + NbTrig; i++)
            {
                Traw[j][i] = 0.; // mult=1 !!!
                mult[j][i] = 0;
            }
        Int_t nHits = fMappedItemsTwim->GetEntriesFast();
        // std::cout << "Event:\n";
        for (Int_t ihit = 0; ihit < nHits; ihit++)
        {
            R3BSofTwimMappedData* hit = (R3BSofTwimMappedData*)fMappedItemsTwim->At(ihit);
            if (!hit)
                continue;
            fh1_Twimmap_mult[hit->GetSecID()]->Fill(hit->GetAnodeID());
            mult[hit->GetSecID()][hit->GetAnodeID()]++;
            if (Traw[hit->GetSecID()][hit->GetAnodeID()] == 0)
            {
                if (hit->GetEnergy() < 8192 && hit->GetEnergy() > 0 && Eraw[hit->GetSecID()][hit->GetAnodeID()] == 0)
                    Eraw[hit->GetSecID()][hit->GetAnodeID()] = hit->GetEnergy(); // mult=1 !!!
                Traw[hit->GetSecID()][hit->GetAnodeID()] = hit->GetTime();       // mult=1 !!!
            }
        }
        Int_t idTref;
        for (Int_t j = 0; j < NbSections; j++)
        {
            for (Int_t i = 0; i < NbTref; i++)
            {
                if (mult[j][16 + i] == 1 && mult[j][18 + i] == 1)
                {
                    fh1_twimmap_DeltaTrefTrig[j][i]->Fill(Traw[j][16 + i] - Traw[j][18 + i]);
                }
            }
            for (Int_t i = 0; i < NbAnodes; i++)
            {
                // Tref = 16 for ch 0 to 7 and Tref = 17 for ch 8 to 15
                idTref = NbAnodes + i / 8;
                if ((mult[j][i] == 1) && (mult[j][idTref] == 1))
                {
                    fh1_twimmap_E[j][i]->Fill(Eraw[j][i]);
                    fh1_twimmap_DT[j][i]->Fill(Traw[j][i] - Traw[j][idTref]);
                    fh2_twim_EneRawVsDriftTime[j][i]->Fill(Eraw[j][i], Traw[j][i] - Traw[j][idTref]);

                    if (i < NbAnodes / 2)
                    {
                        e1 = e1 + Eraw[j][i];
                        n1++;
                    }
                    else if (i >= NbAnodes / 2 && i < NbAnodes)
                    {
                        e2 = e2 + Eraw[j][i];
                        n2++;
                    }
                }
            }
            for (Int_t i = 0; i < NbAnodes - 1; i++)
            {
                idTref = NbAnodes + i / 8;
                if ((mult[j][i] == 1) && (mult[j][i + 1] == 1) && (mult[j][idTref] == 1))
                {
                    fh2_twim_DTvsDT[j][i]->Fill(Traw[j][i] - Traw[j][idTref], Traw[j][i + 1] - Traw[j][idTref]);
                }
            }
            if ((mult[j][15] == 1) && (mult[j][0] == 1) && (mult[j][16] == 1) && (mult[j][17] == 1))
            {
                fh2_twim_ESum_vs_diffDT[j]->Fill((Traw[j][15] - Traw[j][17]) - (Traw[j][0] - Traw[j][16]),
                                                 (e1 + e2) / (n1 + n2));
            }
            if ((mult[j][5] == 1) && (mult[j][16] == 1))
            {
                fh2_twim_EneRawSumVsDriftTime[j]->Fill(Traw[j][5] - Traw[j][16], (e1 + e2) / (n1 + n2));
            }
        }
        fh1_twim_ESum[0]->Fill(e1 / n1);
        fh1_twim_ESum[1]->Fill(e2 / n2);
        fh1_twim_ESum[2]->Fill((e1 + e2) / (n1 + n2));
        fh2_twim_ESum->Fill(e1 / n1, e2 / n2);
    }

    // Fill cal data
    if (fCalItemsTwim && fCalItemsTwim->GetEntriesFast() > 0)
    {
        Int_t nHits = fCalItemsTwim->GetEntriesFast();
        for (Int_t ihit = 0; ihit < nHits; ihit++)
        {
            R3BSofTwimCalData* hit = (R3BSofTwimCalData*)fCalItemsTwim->At(ihit);
            if (!hit)
                continue;
            fh1_Twimcal_Pos[hit->GetSecID()][hit->GetAnodeID()]->Fill(hit->GetDTime());
        }
    }

    Double_t mwpc3x = -500;
    // Fill MWPC3 Hit data
    if (fHitItemsMwpc3 && fHitItemsMwpc3->GetEntriesFast() > 0)
    {
        Int_t nHits = fHitItemsMwpc3->GetEntriesFast();
        for (Int_t ihit = 0; ihit < nHits; ihit++)
        {
            R3BSofMwpcHitData* hit = (R3BSofMwpcHitData*)fHitItemsMwpc3->At(ihit);
            if (!hit)
                continue;
            mwpc3x = hit->GetX();
        }
    }

    // Fill hit data
    if (fHitItemsTwim && fHitItemsTwim->GetEntriesFast() > 0)
    {
        Int_t nHits = fHitItemsTwim->GetEntriesFast();
        for (Int_t ihit = 0; ihit < nHits; ihit++)
        {
            R3BSofTwimHitData* hit = (R3BSofTwimHitData*)fHitItemsTwim->At(ihit);
            if (!hit)
                continue;
            fh1_Twimhit_z->Fill(hit->GetZcharge());
            fh1_Twimhit_theta->Fill(hit->GetTheta() * 1000.);
            fh2_Twimhit_zvstheta->Fill(hit->GetTheta() * 1000., hit->GetZcharge());
            if (mwpc3x > -500)
            {
                fh2_TwimTheta_vs_mwpc3x->Fill(hit->GetTheta() * 1000., mwpc3x);
                fh2_TwimZ_vs_mwpc3x->Fill(mwpc3x, hit->GetZcharge());
            }
        }
    }

    fNEvents += 1;
}

void R3BSofTwimOnlineSpectra::FinishEvent()
{
    if (fMappedItemsTwim)
    {
        fMappedItemsTwim->Clear();
    }
    if (fCalItemsTwim)
    {
        fCalItemsTwim->Clear();
    }
    if (fHitItemsTwim)
    {
        fHitItemsTwim->Clear();
    }
    if (fHitItemsMwpc3)
    {
        fHitItemsMwpc3->Clear();
    }
}

void R3BSofTwimOnlineSpectra::FinishTask()
{
    if (fMappedItemsTwim)
    {
        for (Int_t i = 0; i < NbSections; i++)
        {
            fh1_Twimmap_mult[i]->Write();
            cTwimMap_E[i]->Write();
            cTwimMap_DT[i]->Write();
            cTwimMap_EvsDT[i]->Write();
            cTwim_DTvsDT[i]->Write();
            cTwimMap_DeltaTrefTrig[i]->Write();
        }
        cTwimMap_EsumvsDT->Write();
        cTwimMap_ESum_vs_diffDT->Write();
        fh1_twim_ESum[0]->Write();
        fh1_twim_ESum[1]->Write();
        fh1_twim_ESum[2]->Write();
        fh2_twim_ESum->Write();
    }
    if (fCalItemsTwim)
    {
        for (Int_t i = 0; i < NbSections; i++)
            cTwimCal_Pos[i]->Write();
    }
    if (fHitItemsTwim)
    {
        fh1_Twimhit_z->Write();
        fh1_Twimhit_theta->Write();
        fh2_Twimhit_zvstheta->Write();
        if (fHitItemsMwpc3)
        {
            cTwimTheta_vs_mwpc3x->Write();
            cTwimZ_vs_mwpc3x->Write();
        }
    }
}

ClassImp(R3BSofTwimOnlineSpectra)
