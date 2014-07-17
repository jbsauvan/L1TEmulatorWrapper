/**
 *  @file  L1TStage2Wrapper.cpp
 *  @brief  
 *
 *
 *  @author  Jean-Baptiste Sauvan <sauvan@llr.in2p3.fr>
 *
 *  @date    06/23/2014
 *
 *  @internal
 *     Created :  06/23/2014
 * Last update :  06/23/2014 10:02:06 AM
 *          by :  JB Sauvan
 *
 * =====================================================================================
 */

#include <stdlib.h> 
#include <iostream>
#include <fstream>
#include <memory>
//#include "boost/shared_ptr.hpp"

#include <TEnv.h>

#include "L1TStage2Wrapper.h"

#include "L1Trigger/L1TCalorimeter/interface/Stage2Layer2ClusterAlgorithmFirmware.h"
#include "L1Trigger/L1TCalorimeter/interface/Stage2Layer2EGammaAlgorithmFirmware.h"
#include "L1Trigger/L1TCalorimeter/interface/Stage2Layer2TauAlgorithmFirmware.h"

using namespace l1twrapper;
using namespace std;



/*****************************************************************/
L1TStage2Wrapper::L1TStage2Wrapper()
/*****************************************************************/
{
}

/*****************************************************************/
L1TStage2Wrapper::~L1TStage2Wrapper()
/*****************************************************************/
{
    delete m_egClusterAlgo;
    delete m_egAlgo;
    delete m_tauClusterAlgo;
    delete m_tauAlgo;
}


/*****************************************************************/
bool L1TStage2Wrapper::initialize(const string& parameterFile)
/*****************************************************************/
{
    // retrieve parameters
    bool status = fillParameters(parameterFile);
    if(!status) return status;

    // initialize algos
    m_egClusterAlgo  = new l1t::Stage2Layer2ClusterAlgorithmFirmwareImp1(&m_params, l1t::Stage2Layer2ClusterAlgorithmFirmwareImp1::ClusterInput::E);
    m_egAlgo         = new l1t::Stage2Layer2EGammaAlgorithmFirmwareImp1(&m_params);
    m_tauClusterAlgo = new l1t::Stage2Layer2ClusterAlgorithmFirmwareImp1(&m_params, l1t::Stage2Layer2ClusterAlgorithmFirmwareImp1::ClusterInput::EH);
    m_tauAlgo        = new l1t::Stage2Layer2TauAlgorithmFirmwareImp1(&m_params);

    dynamic_cast<l1t::Stage2Layer2ClusterAlgorithmFirmwareImp1*>(m_tauClusterAlgo)->trimCorners(false); 

    return true;
}

/*****************************************************************/
bool L1TStage2Wrapper::process(const vector<int>& hwEta, const vector<int>& hwPhi,
                            const vector<int>& hwEtEm, const vector<int>& hwEtHad, 
                            const vector<int>& hwEtRatio, const vector<int>& hwQual)
/*****************************************************************/
{
    bool status = true;
    status = fillTowers(hwEta, hwPhi, hwEtEm, hwEtHad, hwEtRatio, hwQual);
    if(!status) return status;

    // clear objects
    m_egClusters.clear();
    m_egammas.clear();
    m_tauClusters.clear();
    m_taus.clear();
    // build objects
    m_egClusterAlgo ->processEvent( m_towers, m_egClusters );
    m_egAlgo        ->processEvent( m_egClusters, m_towers, m_egammas );
    m_tauClusterAlgo->processEvent( m_towers, m_tauClusters );
    m_tauAlgo       ->processEvent( m_tauClusters, m_taus );

    return true;
}

/*****************************************************************/
bool L1TStage2Wrapper::fillTowers(const vector<int>& hwEta, const vector<int>& hwPhi,
                            const vector<int>& hwEtEm, const vector<int>& hwEtHad, 
                            const vector<int>& hwEtRatio, const vector<int>& hwQual)
/*****************************************************************/
{
    unsigned int nTowers = hwEta.size();
    if(hwPhi.size()!=nTowers || hwEtEm.size()!=nTowers || hwEtHad.size()!=nTowers || hwEtRatio.size()!=nTowers || hwQual.size()!=nTowers)
    {
        cout<<"[ERROR] Input vectors have different sizes\n";
        return false;
    }
    // clear towers
    m_towers.clear();
    // fill towers from input vectors
    m_towers.reserve(nTowers);
    for(unsigned tt=0; tt<nTowers; tt++)
    {
        l1t::CaloTower tower;
        tower.setHwEta    ( hwEta[tt] );
        tower.setHwPhi    ( hwPhi[tt] );
        tower.setHwEtEm   ( hwEtEm[tt] );
        tower.setHwEtHad  ( hwEtHad[tt] );
        tower.setHwPt     ( hwEtEm[tt] + hwEtHad[tt] );
        tower.setHwEtRatio( hwEtRatio[tt] );
        tower.setHwQual   ( hwQual[tt] );
        m_towers.push_back(tower);
    }
    return true;
}


/*****************************************************************/
bool L1TStage2Wrapper::fillParameters(const string& parameterFile)
/*****************************************************************/
{
    TEnv params;
    int status = params.ReadFile(parameterFile.c_str(),EEnvLevel(0));
    if(status!=0) 
    {
        cout<<"[ERROR] Cannot read parameter file "<<parameterFile<<"\n"; 
        return false;
    }
    // towers
    m_params.setTowerLsbH       (params.GetValue("towerLsbH"        , 0.5));
    m_params.setTowerLsbE       (params.GetValue("towerLsbE"        , 0.5));
    m_params.setTowerLsbSum     (params.GetValue("towerLsbSum"      , 0.5));
    m_params.setTowerNBitsH     (params.GetValue("towerNBitsH"      , 8));
    m_params.setTowerNBitsE     (params.GetValue("towerNBitsE"      , 8));
    m_params.setTowerNBitsSum   (params.GetValue("towerNBitsSum"    , 9));
    m_params.setTowerNBitsRatio (params.GetValue("towerNBitsRatio"  , 3));
    m_params.setTowerEncoding   (params.GetValue("towerEncoding"    , false));

    // EG
    m_params.setEgLsb                (params.GetValue("egLsb"                      , 0.5));
    m_params.setEgSeedThreshold      (params.GetValue("egSeedThreshold"            , 2.));
    m_params.setEgNeighbourThreshold (params.GetValue("egNeighbourThreshold"       , 1.));
    m_params.setEgHcalThreshold      (params.GetValue("egHcalThreshold"            , 1.));
    m_params.setEgMaxHcalEt          (params.GetValue("egMaxHcalEt"                , 0.));
    m_params.setEgEtToRemoveHECut    (params.GetValue("egEtToRemoveHECut"          , 128.));
    m_params.setEgIsoPUSType         (params.GetValue("egIsoPUSType"               , "None"));

    stringstream egMaxHOverELUTFile;
    egMaxHOverELUTFile << getenv("CMSSW_BASE") << "/src/" << params.GetValue("egMaxHOverELUTFile", "L1Trigger/L1TCalorimeter/data/egMaxHOverELUT.txt");
    std::ifstream egMaxHOverELUTStream(egMaxHOverELUTFile.str());
    if(!egMaxHOverELUTStream.is_open())
    {
        cout<<"[ERROR] Cannot open " << egMaxHOverELUTFile.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> egMaxHOverELUT( new l1t::LUT(egMaxHOverELUTStream) );
    m_params.setEgMaxHOverELUT(egMaxHOverELUT);

    stringstream egShapeIdLUTFile;
    egShapeIdLUTFile << getenv("CMSSW_BASE") << "/src/" << params.GetValue("egShapeIdLUTFile", "L1Trigger/L1TCalorimeter/data/egShapeIdLUT.txt");
    std::ifstream egShapeIdLUTStream(egShapeIdLUTFile.str());
    if(!egShapeIdLUTStream.is_open())
    {
        cout<<"[ERROR] Cannot open " << egShapeIdLUTFile.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> egShapeIdLUT( new l1t::LUT(egShapeIdLUTStream) );
    m_params.setEgShapeIdLUT(egShapeIdLUT);

    stringstream egIsoLUTFile;
    egIsoLUTFile << getenv("CMSSW_BASE") << "/src/" << params.GetValue("egIsoLUTFile", "L1Trigger/L1TCalorimeter/data/egIsoLUT_PU40bx25.txt");
    std::ifstream egIsoLUTStream(egIsoLUTFile.str());
    if(!egIsoLUTStream.is_open())
    {
        cout<<"[ERROR] Cannot open " << egIsoLUTFile.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> egIsoLUT( new l1t::LUT(egIsoLUTStream) );
    m_params.setEgIsolationLUT(egIsoLUT);

    m_params.setEgIsoAreaNrTowersEta       (params.GetValue("egIsoAreaNrTowersEta"       , 2));
    m_params.setEgIsoAreaNrTowersPhi       (params.GetValue("egIsoAreaNrTowersPhi"       , 4));
    m_params.setEgIsoVetoNrTowersPhi       (params.GetValue("egIsoVetoNrTowersPhi"       , 3));
    m_params.setEgIsoPUEstTowerGranularity (params.GetValue("egIsoPUEstTowerGranularity" , 1));
    m_params.setEgIsoMaxEtaAbsForTowerSum  (params.GetValue("egIsoMaxEtaAbsForTowerSum"  , 4));
    m_params.setEgIsoMaxEtaAbsForIsoSum    (params.GetValue("egIsoMaxEtaAbsForIsoSum"    , 27));

    stringstream egCalibrationLUTFile;
    egCalibrationLUTFile << getenv("CMSSW_BASE") << "/src/" << params.GetValue("egCalibrationLUTFile", "L1Trigger/L1TCalorimeter/data/egCalibrationLUT.txt");
    std::ifstream egCalibrationLUTStream(egCalibrationLUTFile.str());
    if(!egCalibrationLUTStream.is_open())
    {
        cout<<"[ERROR] Cannot open " << egCalibrationLUTFile.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> egCalibrationLUT( new l1t::LUT(egCalibrationLUTStream) );
    m_params.setEgCalibrationLUT(egCalibrationLUT);

    // Tau
    m_params.setTauLsb                (params.GetValue("tauLsb"                        , 0.5));
    m_params.setTauSeedThreshold      (params.GetValue("tauSeedThreshold"              , 0.));
    m_params.setTauNeighbourThreshold (params.GetValue("tauNeighbourThreshold"         , 0.));
    m_params.setTauIsoPUSType         (params.GetValue("tauIsoPUSType"                 , "None"));

    stringstream tauIsoLUTFile;
    tauIsoLUTFile << getenv("CMSSW_BASE") << "/src/" << params.GetValue("tauIsoLUTFile", "L1Trigger/L1TCalorimeter/data/tauIsoLUT.txt");
    std::ifstream tauIsoLUTStream(tauIsoLUTFile.str());
    if(!tauIsoLUTStream.is_open())
    {
        cout<<"[ERROR] Cannot open " << tauIsoLUTFile.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> tauIsoLUT( new l1t::LUT(tauIsoLUTStream) );
    m_params.setTauIsolationLUT(tauIsoLUT);

    stringstream tauCalibrationLUTFileBarrelA;
    tauCalibrationLUTFileBarrelA << getenv("CMSSW_BASE") << "/src/" << params.GetValue("tauCalibrationLUTFileBarrelA", "L1Trigger/L1TCalorimeter/data/tauCalibrationLUTBarrelA.txt");
    std::ifstream tauCalibrationLUTStreamBarrelA(tauCalibrationLUTFileBarrelA.str());
    if(!tauCalibrationLUTStreamBarrelA.is_open())
    {
        cout<<"[ERROR] Cannot open " << tauCalibrationLUTFileBarrelA.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> tauCalibrationLUTBarrelA( new l1t::LUT(tauCalibrationLUTStreamBarrelA) );
    m_params.setTauCalibrationLUTBarrelA(tauCalibrationLUTBarrelA);

    stringstream tauCalibrationLUTFileBarrelB;
    tauCalibrationLUTFileBarrelB << getenv("CMSSW_BASE") << "/src/" << params.GetValue("tauCalibrationLUTFileBarrelB", "L1Trigger/L1TCalorimeter/data/tauCalibrationLUTBarrelB.txt");
    std::ifstream tauCalibrationLUTStreamBarrelB(tauCalibrationLUTFileBarrelB.str());
    if(!tauCalibrationLUTStreamBarrelB.is_open())
    {
        cout<<"[ERROR] Cannot open " << tauCalibrationLUTFileBarrelB.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> tauCalibrationLUTBarrelB( new l1t::LUT(tauCalibrationLUTStreamBarrelB) );
    m_params.setTauCalibrationLUTBarrelB(tauCalibrationLUTBarrelB);

    stringstream tauCalibrationLUTFileBarrelC;
    tauCalibrationLUTFileBarrelC << getenv("CMSSW_BASE") << "/src/" << params.GetValue("tauCalibrationLUTFileBarrelC", "L1Trigger/L1TCalorimeter/data/tauCalibrationLUTBarrelC.txt");
    std::ifstream tauCalibrationLUTStreamBarrelC(tauCalibrationLUTFileBarrelC.str());
    if(!tauCalibrationLUTStreamBarrelC.is_open())
    {
        cout<<"[ERROR] Cannot open " << tauCalibrationLUTFileBarrelC.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> tauCalibrationLUTBarrelC( new l1t::LUT(tauCalibrationLUTStreamBarrelC) );
    m_params.setTauCalibrationLUTBarrelC(tauCalibrationLUTBarrelC);

    stringstream tauCalibrationLUTFileEndcapsA;
    tauCalibrationLUTFileEndcapsA << getenv("CMSSW_BASE") << "/src/" << params.GetValue("tauCalibrationLUTFileEndcapsA", "L1Trigger/L1TCalorimeter/data/tauCalibrationLUTEndcapsA.txt");
    std::ifstream tauCalibrationLUTStreamEndcapsA(tauCalibrationLUTFileEndcapsA.str());
    if(!tauCalibrationLUTStreamEndcapsA.is_open())
    {
        cout<<"[ERROR] Cannot open " << tauCalibrationLUTFileEndcapsA.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> tauCalibrationLUTEndcapsA( new l1t::LUT(tauCalibrationLUTStreamEndcapsA) );
    m_params.setTauCalibrationLUTEndcapsA(tauCalibrationLUTEndcapsA);

    stringstream tauCalibrationLUTFileEndcapsB;
    tauCalibrationLUTFileEndcapsB << getenv("CMSSW_BASE") << "/src/" << params.GetValue("tauCalibrationLUTFileEndcapsB", "L1Trigger/L1TCalorimeter/data/tauCalibrationLUTEndcapsB.txt");
    std::ifstream tauCalibrationLUTStreamEndcapsB(tauCalibrationLUTFileEndcapsB.str());
    if(!tauCalibrationLUTStreamEndcapsB.is_open())
    {
        cout<<"[ERROR] Cannot open " << tauCalibrationLUTFileEndcapsB.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> tauCalibrationLUTEndcapsB( new l1t::LUT(tauCalibrationLUTStreamEndcapsB) );
    m_params.setTauCalibrationLUTEndcapsB(tauCalibrationLUTEndcapsB);

    stringstream tauCalibrationLUTFileEndcapsC;
    tauCalibrationLUTFileEndcapsC << getenv("CMSSW_BASE") << "/src/" << params.GetValue("tauCalibrationLUTFileEndcapsC", "L1Trigger/L1TCalorimeter/data/tauCalibrationLUTEndcapsC.txt");
    std::ifstream tauCalibrationLUTStreamEndcapsC(tauCalibrationLUTFileEndcapsC.str());
    if(!tauCalibrationLUTStreamEndcapsC.is_open())
    {
        cout<<"[ERROR] Cannot open " << tauCalibrationLUTFileEndcapsC.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> tauCalibrationLUTEndcapsC( new l1t::LUT(tauCalibrationLUTStreamEndcapsC) );
    m_params.setTauCalibrationLUTEndcapsC(tauCalibrationLUTEndcapsC);

    stringstream tauCalibrationLUTFileEta;
    tauCalibrationLUTFileEta << getenv("CMSSW_BASE") << "/src/" << params.GetValue("tauCalibrationLUTFileEta", "L1Trigger/L1TCalorimeter/data/tauCalibrationLUTEta.txt");
    std::ifstream tauCalibrationLUTStreamEta(tauCalibrationLUTFileEta.str());
    if(!tauCalibrationLUTStreamEta.is_open())
    {
        cout<<"[ERROR] Cannot open " << tauCalibrationLUTFileEta.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> tauCalibrationLUTEta( new l1t::LUT(tauCalibrationLUTStreamEta) );
    m_params.setTauCalibrationLUTEta(tauCalibrationLUTEta);

    return true;
}
