// ------------------------------------------------------------------
// -----            R3BSofTwimHitPar source file                -----
// -----       Created 24/11/19  by J.L. Rodriguez-Sanchez      -----
// ------------------------------------------------------------------

#include "R3BSofTwimHitPar.h"

#include "FairLogger.h"
#include "FairParamList.h"

#include "TArrayF.h"
#include "TMath.h"
#include "TString.h"

#include <iostream>

// ---- Standard Constructor ---------------------------------------------------
R3BSofTwimHitPar::R3BSofTwimHitPar(const char* name, const char* title, const char* context)
    : FairParGenericSet(name, title, context)
    , fNumParamsZFit(2)
    , fNumAnodes(16)
    , fNumSec(1)
{
    fDetZHitParams = new TArrayF(fNumSec * fNumParamsZFit); // 2 Parameters for Z (Linear fits)
    fIn_use = new TArrayI(fNumAnodes * fNumSec);
    fAnode_pos = new TArrayF(fNumAnodes * fNumSec);
}

// ----  Destructor ------------------------------------------------------------
R3BSofTwimHitPar::~R3BSofTwimHitPar()
{
    clear();
    if (fIn_use)
        delete fIn_use;
    if (fAnode_pos)
        delete fAnode_pos;
    if (fDetZHitParams)
        delete fDetZHitParams;
}

// ----  Method clear ----------------------------------------------------------
void R3BSofTwimHitPar::clear()
{
    status = kFALSE;
    resetInputVersions();
}

// ----  Method putParams ------------------------------------------------------
void R3BSofTwimHitPar::putParams(FairParamList* list)
{
    LOG(INFO) << "R3BSofTwimHitPar::putParams() called";
    if (!list)
    {
        return;
    }

    list->add("twimHitNumSecPar", fNumSec);
    list->add("twimAnodeNumberPar", fNumAnodes);
    Int_t array_anodes = fNumAnodes * fNumSec;
    LOG(INFO) << "Array Size Anodes: " << array_anodes;
    fIn_use->Set(array_anodes);
    list->add("twinInUsePar", *fIn_use);

    fAnode_pos->Set(array_anodes);
    list->add("twimAnodePosPar", *fAnode_pos);

    list->add("twimZHitParamsFitPar", fNumParamsZFit);
    Int_t array_size = fNumParamsZFit;
    LOG(INFO) << "Number of parameters for charge-Z: " << array_size;
    fDetZHitParams->Set(array_size);
    list->add("twimZHitPar", *fDetZHitParams);
}

// ----  Method getParams ------------------------------------------------------
Bool_t R3BSofTwimHitPar::getParams(FairParamList* list)
{
    LOG(INFO) << "R3BSofTwimHitPar::getParams() called";
    if (!list)
    {
        return kFALSE;
    }

    if (!list->fill("twimHitNumSecPar", &fNumSec))
    {
        return kFALSE;
    }

    if (!list->fill("twimAnodeNumberPar", &fNumAnodes))
    {
        return kFALSE;
    }

    Int_t array_anode = fNumAnodes * fNumSec;
    LOG(INFO) << "Array Size: " << array_anode;
    fIn_use->Set(array_anode);
    if (!(list->fill("twimInUsePar", fIn_use)))
    {
        LOG(INFO) << "---Could not initialize twimInUsePar";
        return kFALSE;
    }

    fAnode_pos->Set(array_anode);
    if (!(list->fill("twimAnodePosPar", fAnode_pos)))
    {
        LOG(INFO) << "---Could not initialize twimAnodePosPar";
        return kFALSE;
    }

    if (!list->fill("twimZHitParamsFitPar", &fNumParamsZFit))
    {
        return kFALSE;
    }

    Int_t array_size = fNumParamsZFit * fNumSec;
    LOG(INFO) << "Array Size: " << array_size;
    fDetZHitParams->Set(array_size);

    if (!(list->fill("twimZHitPar", fDetZHitParams)))
    {
        LOG(INFO) << "---Could not initialize twimZHitPar";
        return kFALSE;
    }

    return kTRUE;
}

// ----  Method printParams ----------------------------------------------------
void R3BSofTwimHitPar::printParams()
{
    LOG(INFO) << "R3BSofTwimHitPar: twim detector Parameters";
    LOG(INFO) << "R3BSofTwimHitPar: twim anodes in use: ";

    for (Int_t s = 0; s < fNumSec; s++)
    {
        LOG(INFO) << "Section = " << s + 1;
        for (Int_t j = 0; j < fNumAnodes; j++)
        {
            LOG(INFO) << "Anode " << j + 1 << " in use " << fIn_use->GetAt(s * fNumAnodes + j)
                      << ", Position-Z: " << fAnode_pos->GetAt(j);
        }
    }

    for (Int_t s = 0; s < fNumSec; s++)
    {
        LOG(INFO) << "Section = " << s + 1;
        for (Int_t j = 0; j < fNumParamsZFit; j++)
        {
            LOG(INFO) << "FitParam(" << j << ") = " << fDetZHitParams->GetAt(j + fNumSec * fNumParamsZFit);
        }
    }
}
