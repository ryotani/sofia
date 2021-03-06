// ------------------------------------------------------------
// -----         R3BSofTrimCal2Hit source file         -----
// ------------------------------------------------------------

// ROOT headers
#include "TClonesArray.h"
#include "TMath.h"
#include "TRandom.h"

// Fair headers
#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include <iomanip>

// Trim headers
#include "R3BSofTrimCal2Hit.h"
#include "R3BSofTrimCalData.h"
#include "R3BSofTrimHitPar.h"
// todo : #include "R3BSofSciHitData.h"

// R3BSofTrimCal2Hit: Default Constructor --------------------------
R3BSofTrimCal2Hit::R3BSofTrimCal2Hit()
    : FairTask("R3BSof Trim Hit Calibrator", 1)
    , fNumSections(3)
    , fNumAnodes(6)
    , fTriShape(kTRUE)
    , fOnline(kFALSE)
    , fTrimHitPar(NULL)
    , fTrimCalData(NULL)
    , fTrimHitData(NULL)
// to do : , fSciHitData(NULL)
{
}

// R3BSofTrimCal2HitPar: Standard Constructor --------------------------
R3BSofTrimCal2Hit::R3BSofTrimCal2Hit(const char* name, Int_t iVerbose)
    : FairTask(name, iVerbose)
    , fNumSections(3)
    , fNumAnodes(6)
    , fTriShape(kTRUE)
    , fOnline(kFALSE)
    , fTrimHitPar(NULL)
    , fTrimCalData(NULL)
    , fTrimHitData(NULL)
// to do : , fSciHitData(NULL)
{
}

// Virtual R3BSofTrimCal2Hit: Destructor
R3BSofTrimCal2Hit::~R3BSofTrimCal2Hit()
{
    LOG(INFO) << "R3BSofTrimCal2Hit: Delete instance";
    if (fTrimCalData)
        delete fTrimCalData;
    if (fTrimHitData)
        delete fTrimHitData;
    // to do : if (fSciHitData) delete fSciHitData;
}

void R3BSofTrimCal2Hit::SetParContainers()
{
    FairRuntimeDb* rtdb = FairRuntimeDb::instance();
    if (!rtdb)
    {
        LOG(ERROR) << "FairRuntimeDb not opened!";
    }

    fTrimHitPar = (R3BSofTrimHitPar*)rtdb->getContainer("trimHitPar");
    if (!fTrimHitPar)
    {
        LOG(ERROR) << "R3BSofTrimCal2HitPar::Init() Couldn't get handle on trimHitPar container";
        return;
    }
    else
    {
        if (fTriShape == kTRUE)
        {
            LOG(INFO) << "Triple-MUSIC has triangular shape anodes";
            fTrimHitPar->SetNumAlignGainsPerSection(3);
        }
        else
        {
            LOG(INFO) << "Triple-MUSIC has rectangular shape anodes";
            fTrimHitPar->SetNumAlignGainsPerSection(6);
        }
        LOG(INFO) << "R3BSofTrimCal2Hit:: trimHitPar container open ";
    }
}

// -----   Public method Init   --------------------------------------------
InitStatus R3BSofTrimCal2Hit::Init()
{
    LOG(INFO) << "R3BSofTrimCal2Hit: Init";

    FairRootManager* rootManager = FairRootManager::Instance();
    if (!rootManager)
    {
        return kFATAL;
    }

    // --- ------------------------------- --- //
    // --- INPUT CAL DATA FOR TRIPLE-MUSIC --- //
    // --- ------------------------------- --- //
    fTrimCalData = (TClonesArray*)rootManager->GetObject("TrimCalData");
    if (!fTrimCalData)
    {
        return kFATAL;
    }

    // --- ---------------------- --- //
    // --- INPUT HIT DATA FOR SCI --- //
    // --- ---------------------- --- //
    // to do : fSciHitData = (TClonesArray*)rootManager->GetObject("SciHitData");
    // to do : if (!fSciHitData){
    // to do :   return kFATAL;
    // to do : }

    // --- --------------- --- //
    // --- OUTPUT HIT DATA --- //
    // --- --------------- --- //
    fTrimHitData = new TClonesArray("R3BSofTrimHitData", fNumSections);

    if (!fOnline)
    {
        rootManager->Register("TrimHitData", "Trim Hit", fTrimHitData, kTRUE);
    }
    else
    {
        rootManager->Register("TrimHitData", "Trim Hit", fTrimHitData, kFALSE);
    }
    fTrimHitPar->printParams();

    return kSUCCESS;
}

// -----   Public method ReInit   ----------------------------------------------
InitStatus R3BSofTrimCal2Hit::ReInit()
{
    SetParContainers();
    return kSUCCESS;
}

// -----   Public method Execution   --------------------------------------------
void R3BSofTrimCal2Hit::Exec(Option_t* option)
{
    // Reset entries in output arrays, local arrays
    Reset();

    // Get the parameters
    if (!fTrimHitPar)
    {
        LOG(ERROR) << "R3BSofTrimCal2Hit: NOT Container Parameter!!";
    }

    // Local variables at Cal Level
    Int_t iSec, iAnode;
    UInt_t mult[fNumSections * fNumAnodes];
    Float_t e[fNumSections * fNumAnodes];
    Double_t dt[fNumSections * fNumAnodes];

    // Local variables at Hit Level
    Int_t nAligned, nRaw;
    if (fTriShape == kTRUE)
        nAligned = fNumAnodes / 2;
    else
        nAligned = fNumAnodes;
    Float_t eal[fNumSections * nAligned];
    Float_t sumRaw, sumBeta, sumDT, sumTheta, zval;

    // Initialization of the local variables
    for (Int_t s = 0; s < fNumSections; s++)
    {
        for (Int_t a = 0; a < fNumAnodes; a++)
        {
            mult[a + s * fNumAnodes] = 0.;
            e[a + s * fNumAnodes] = 0.;
            dt[a + s * fNumAnodes] = -1000000.;
        }
        for (Int_t ch = 0; ch < nAligned; ch++)
            eal[ch + s * nAligned] = 0;
    }

    // TO DO : Get the number of entries of the SciHitData TClonesArray (to get the beam velocity)

    // Get the number of entries of the TrimCalData TClonesArray and loop over it
    Int_t nHitsCalTrim = fTrimCalData->GetEntries();
    if (!nHitsCalTrim)
    {
        return;
    }
    for (Int_t entry = 0; entry < nHitsCalTrim; entry++)
    {
        R3BSofTrimCalData* iCalData = (R3BSofTrimCalData*)fTrimCalData->At(entry);
        iSec = iCalData->GetSecID() - 1;
        iAnode = iCalData->GetAnodeID() - 1;
        mult[iAnode + iSec * fNumAnodes]++;
        e[iAnode + iSec * fNumAnodes] = iCalData->GetEnergyMatch();
        dt[iAnode + iSec * fNumAnodes] = iCalData->GetDriftTimeAligned();
    }

    // --- Fill the HIT level --- //
    for (Int_t s = 0; s < fNumSections; s++)
    {

        // === fEnergyRaw: sum of Aligned Energy ===
        sumRaw = 0.;
        nRaw = 0;
        // if Rectangular shape:
        if (fTriShape == kFALSE)
        {
            for (Int_t a = 0; a < fNumAnodes; a++)
            {
                if (mult[a + s * fNumAnodes] == 1)
                {
                    eal[a + s * fNumAnodes] = e[a + s * fNumAnodes] * fTrimHitPar->GetEnergyAlignGain(s + 1, a);
                    sumRaw += eal[a + s * fNumAnodes];
                    nRaw++;
                }
            } // end of loop over the anodes
        }     // end calculation of fEnergyRaw for rectangular shape anodes
        else
        {
            for (Int_t ch = 0; ch < nAligned; ch++)
            {
                if (mult[2 * ch + s * fNumAnodes] == 1 && mult[2 * ch + 1 + s * fNumAnodes] == 1)
                {
                    eal[ch + s * nAligned] = (e[2 * ch + s * fNumAnodes] + e[2 * ch + 1 + s * fNumAnodes]) *
                                             fTrimHitPar->GetEnergyAlignGain(s + 1, ch);
                    sumRaw += eal[ch + s * nAligned];
                    nRaw++;
                }
            } // end of loop over the anodes
        }     // end of calculation of fEnergyRaw for triangular shape anodes
        if (nRaw > 0)
            sumRaw = sumRaw / (Float_t)nRaw;

        // TO DO : === fEnergyBeta: fEnergyRaw corrected from the beam velocity ===
        sumBeta = sumRaw;

        // TO DO : === fEnergyDT: fEnergyBeta corrected from the X position in the Triple-MUSIC ===
        sumDT = sumBeta;

        // TO DO : === fEnergyTheta: fEnergyDT corrected from the theta angle in the Triple-MUSIC ===
        sumTheta = sumDT;

        // TO DO : === fZ ===
        zval = sumTheta;
        AddHitData(s + 1, sumRaw, sumBeta, sumDT, sumTheta, zval);

    } // end of loop over the sections

    return;
}

// -----   Protected method Finish   --------------------------------------------
void R3BSofTrimCal2Hit::Finish() {}

// -----   Public method Reset   ------------------------------------------------
void R3BSofTrimCal2Hit::Reset()
{
    LOG(DEBUG) << "Clearing TrimHiData Structure";
    if (fTrimHitData)
        fTrimHitData->Clear();
}

// -----   Private method AddCalData  --------------------------------------------
R3BSofTrimHitData* R3BSofTrimCal2Hit::AddHitData(Int_t secID,
                                                 Float_t Eraw,
                                                 Float_t Ebeta,
                                                 Float_t Edt,
                                                 Float_t Etheta,
                                                 Float_t Z)
{
    TClonesArray& clref = *fTrimHitData;
    Int_t size = clref.GetEntriesFast();
    return new (clref[size]) R3BSofTrimHitData(secID, Eraw, Ebeta, Edt, Etheta, Z);
}

ClassImp(R3BSofTrimCal2Hit)
