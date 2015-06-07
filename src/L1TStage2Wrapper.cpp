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
    //m_egClusterAlgo  = new l1t::Stage2Layer2ClusterAlgorithmFirmwareImp1(&m_params, l1t::Stage2Layer2ClusterAlgorithmFirmwareImp1::ClusterInput::EH);
    m_egAlgo         = new l1t::Stage2Layer2EGammaAlgorithmFirmwareImp1(&m_params);
    m_tauClusterAlgo = new l1t::Stage2Layer2ClusterAlgorithmFirmwareImp1(&m_params, l1t::Stage2Layer2ClusterAlgorithmFirmwareImp1::ClusterInput::EH);
    m_tauAlgo        = new l1t::Stage2Layer2TauAlgorithmFirmwareImp1(&m_params);

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
    //m_tauClusterAlgo->processEvent( m_towers, m_tauClusters );
    //m_tauAlgo       ->processEvent( m_tauClusters, m_towers, m_taus );

    return true;
}

/*****************************************************************/
bool L1TStage2Wrapper::process(const vector<l1t::CaloTower>& towers)
/*****************************************************************/
{
    m_towers.clear();
    m_towers = towers;

    // clear objects
    m_egClusters.clear();
    m_egammas.clear();
    m_tauClusters.clear();
    m_taus.clear();
    // build objects
    m_egClusterAlgo ->processEvent( m_towers, m_egClusters );
    m_egAlgo        ->processEvent( m_egClusters, m_towers, m_egammas );
    //m_tauClusterAlgo->processEvent( m_towers, m_tauClusters );
    //m_tauAlgo       ->processEvent( m_tauClusters, m_towers, m_taus );

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
    m_params.setCalibration_version  (params.GetValue("calibration_version"        , 0));

    m_params.setEgLsb                (params.GetValue("egLsb"                      , 0.5));
    m_params.setEgSeedThreshold      (params.GetValue("egSeedThreshold"            , 2.));
    m_params.setEgNeighbourThreshold (params.GetValue("egNeighbourThreshold"       , 1.));
    m_params.setEgHcalThreshold      (params.GetValue("egHcalThreshold"            , 1.));
    m_params.setEgMaxHcalEt          (params.GetValue("egMaxHcalEt"                , 0.));
    m_params.setEgMaxPtHOverE        (params.GetValue("egMaxPtHOverE"          , 128.));
    m_params.setEgPUSType            (params.GetValue("egPUSType"               , "None"));

    stringstream egTrimmingLUTFile;
    egTrimmingLUTFile << getenv("CMSSW_BASE") << "/src/" << params.GetValue("egTrimmingLUTFile", "L1Trigger/L1TCalorimeter/data/egTrimmingLUT.txt");
    std::ifstream egTrimmingLUTStream(egTrimmingLUTFile.str());
    if(!egTrimmingLUTStream.is_open())
    {
        cout<<"[ERROR] Cannot open " << egTrimmingLUTFile.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> egTrimmingLUT( new l1t::LUT(egTrimmingLUTStream) );
    m_params.setEgTrimmingLUT(*egTrimmingLUT);

    stringstream egCompressShapesLUTFile;
    egCompressShapesLUTFile << getenv("CMSSW_BASE") << "/src/" << params.GetValue("egCompressShapesLUTFile", "L1Trigger/L1TCalorimeter/data/egCompressShapesLUT.txt");
    std::ifstream egCompressShapesLUTStream(egCompressShapesLUTFile.str());
    if(!egCompressShapesLUTStream.is_open())
    {
        cout<<"[ERROR] Cannot open " << egCompressShapesLUTFile.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> egCompressShapesLUT( new l1t::LUT(egCompressShapesLUTStream) );
    m_params.setEgCompressShapesLUT(*egCompressShapesLUT);



    //New compression LUTs for calibration

    stringstream egCompressShapesLUTFile_calibr_new;
    egCompressShapesLUTFile_calibr_new << getenv("CMSSW_BASE") << "/src/" << params.GetValue("egCompressShapesLUTFile_calibr_new", "L1Trigger/L1TCalorimeter/data/egCompressShapesLUT_calibr_4bit.txt");
    std::ifstream egCompressShapesLUTStream_calibr_new(egCompressShapesLUTFile_calibr_new.str());
    if(!egCompressShapesLUTStream_calibr_new.is_open())
    {
        cout<<"[ERROR] Cannot open " << egCompressShapesLUTFile_calibr_new.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> egCompressShapesLUT_calibr_new( new l1t::LUT(egCompressShapesLUTStream_calibr_new) );
    m_params.setEgCompressShapesLUT_calibr_new(*egCompressShapesLUT_calibr_new);




    stringstream egCompressELUTFile_calibr_new;
    egCompressELUTFile_calibr_new << getenv("CMSSW_BASE") << "/src/" << params.GetValue("egCompressELUTFile_calibr_new", "L1Trigger/L1TCalorimeter/data/egCompressELUT_4bit.txt");
    std::ifstream egCompressELUTStream_calibr_new(egCompressELUTFile_calibr_new.str());
    if(!egCompressELUTStream_calibr_new.is_open())
    {
        cout<<"[ERROR] Cannot open " << egCompressELUTFile_calibr_new.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> egCompressELUT_calibr_new( new l1t::LUT(egCompressELUTStream_calibr_new) );
    m_params.setEgCompressELUT_calibr_new(*egCompressELUT_calibr_new);



    stringstream egCompressEtaLUTFile_calibr_new;
    egCompressEtaLUTFile_calibr_new << getenv("CMSSW_BASE") << "/src/" << params.GetValue("egCompressEtaLUTFile_calibr_new", "L1Trigger/L1TCalorimeter/data/egCompressEtaLUT_4bit.txt");
    std::ifstream egCompressEtaLUTStream_calibr_new(egCompressEtaLUTFile_calibr_new.str());
    if(!egCompressEtaLUTStream_calibr_new.is_open())
    {
        cout<<"[ERROR] Cannot open " << egCompressEtaLUTFile_calibr_new.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> egCompressEtaLUT_calibr_new( new l1t::LUT(egCompressEtaLUTStream_calibr_new) );
    m_params.setEgCompressEtaLUT_calibr_new(*egCompressEtaLUT_calibr_new);






    stringstream egMaxHOverELUTFile;
    egMaxHOverELUTFile << getenv("CMSSW_BASE") << "/src/" << params.GetValue("egMaxHOverELUTFile", "L1Trigger/L1TCalorimeter/data/egMaxHOverELUT.txt");
    std::ifstream egMaxHOverELUTStream(egMaxHOverELUTFile.str());
    if(!egMaxHOverELUTStream.is_open())
    {
        cout<<"[ERROR] Cannot open " << egMaxHOverELUTFile.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> egMaxHOverELUT( new l1t::LUT(egMaxHOverELUTStream) );
    m_params.setEgMaxHOverELUT(*egMaxHOverELUT);

    stringstream egShapeIdLUTFile;
    egShapeIdLUTFile << getenv("CMSSW_BASE") << "/src/" << params.GetValue("egShapeIdLUTFile", "L1Trigger/L1TCalorimeter/data/egShapeIdLUT.txt");
    std::ifstream egShapeIdLUTStream(egShapeIdLUTFile.str());
    if(!egShapeIdLUTStream.is_open())
    {
        cout<<"[ERROR] Cannot open " << egShapeIdLUTFile.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> egShapeIdLUT( new l1t::LUT(egShapeIdLUTStream) );
    m_params.setEgShapeIdLUT(*egShapeIdLUT);

    stringstream egIsoLUTFile;
    egIsoLUTFile << getenv("CMSSW_BASE") << "/src/" << params.GetValue("egIsoLUTFile", "L1Trigger/L1TCalorimeter/data/egIsoLUT_PU40bx25.txt");
    std::ifstream egIsoLUTStream(egIsoLUTFile.str());
    if(!egIsoLUTStream.is_open())
    {
        cout<<"[ERROR] Cannot open " << egIsoLUTFile.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> egIsoLUT( new l1t::LUT(egIsoLUTStream) );
    m_params.setEgIsolationLUT(*egIsoLUT);

    m_params.setEgIsoAreaNrTowersEta       (params.GetValue("egIsoAreaNrTowersEta"       , 2));
    m_params.setEgIsoAreaNrTowersPhi       (params.GetValue("egIsoAreaNrTowersPhi"       , 4));
    m_params.setEgIsoVetoNrTowersPhi       (params.GetValue("egIsoVetoNrTowersPhi"       , 3));
    //m_params.setEgIsoPUEstTowerGranularity (params.GetValue("egIsoPUEstTowerGranularity" , 1));
    //m_params.setEgIsoMaxEtaAbsForTowerSum  (params.GetValue("egIsoMaxEtaAbsForTowerSum"  , 4));
    //m_params.setEgIsoMaxEtaAbsForIsoSum    (params.GetValue("egIsoMaxEtaAbsForIsoSum"    , 27));
    std::vector<double> egPUSParams(3);
    egPUSParams[0] = params.GetValue("egPUSParams.0" , 1.);
    egPUSParams[1] = params.GetValue("egPUSParams.1" , 4.);
    egPUSParams[2] = params.GetValue("egPUSParams.2" , 27.);
    m_params.setEgPUSParams(egPUSParams);

    stringstream egCalibrationLUTFile;
    egCalibrationLUTFile << getenv("CMSSW_BASE") << "/src/" << params.GetValue("egCalibrationLUTFile", "L1Trigger/L1TCalorimeter/data/egCalibrationLUT.txt");
    std::ifstream egCalibrationLUTStream(egCalibrationLUTFile.str());
    if(!egCalibrationLUTStream.is_open())
    {
        cout<<"[ERROR] Cannot open " << egCalibrationLUTFile.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> egCalibrationLUT( new l1t::LUT(egCalibrationLUTStream) );
    m_params.setEgCalibrationLUT(*egCalibrationLUT);

    //New Calibration LUT

    stringstream egCalibrationLUTFile_new;
    egCalibrationLUTFile_new << getenv("CMSSW_BASE") << "/src/" << params.GetValue("egCalibrationLUTFile", "L1Trigger/L1TCalorimeter/data/egCalibrationLUT_eta4_shape4_E4_correct.txt.txt");
    std::ifstream egCalibrationLUTStream_new(egCalibrationLUTFile_new.str());
    if(!egCalibrationLUTStream_new.is_open())
    {
        cout<<"[ERROR] Cannot open " << egCalibrationLUTFile_new.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> egCalibrationLUT_new( new l1t::LUT(egCalibrationLUTStream_new) );
    m_params.setEgCalibrationLUT_new(*egCalibrationLUT_new);


    // Tau
    m_params.setTauLsb                (params.GetValue("tauLsb"                        , 0.5));
    m_params.setTauSeedThreshold      (params.GetValue("tauSeedThreshold"              , 0.));
    m_params.setTauNeighbourThreshold (params.GetValue("tauNeighbourThreshold"         , 0.));
    m_params.setTauPUSType         (params.GetValue("tauPUSType"                 , "None"));

    m_params.setTauIsoAreaNrTowersEta       (params.GetValue("tauIsoAreaNrTowersEta"       , 2));
    m_params.setTauIsoAreaNrTowersPhi       (params.GetValue("tauIsoAreaNrTowersPhi"       , 4));
    m_params.setTauIsoVetoNrTowersPhi       (params.GetValue("tauIsoVetoNrTowersPhi"       , 2));

    std::vector<double> tauPUSParams(3);
    tauPUSParams[0] = params.GetValue("tauPUSParams.0" , 1.);
    tauPUSParams[1] = params.GetValue("tauPUSParams.1" , 4.);
    tauPUSParams[2] = params.GetValue("tauPUSParams.2" , 27.);
    m_params.setTauPUSParams(tauPUSParams);

    stringstream tauIsoLUTFile;
    tauIsoLUTFile << getenv("CMSSW_BASE") << "/src/" << params.GetValue("tauIsoLUTFile", "L1Trigger/L1TCalorimeter/data/tauIsoLUT.txt");
    std::ifstream tauIsoLUTStream(tauIsoLUTFile.str());
    if(!tauIsoLUTStream.is_open())
    {
        cout<<"[ERROR] Cannot open " << tauIsoLUTFile.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> tauIsoLUT( new l1t::LUT(tauIsoLUTStream) );
    m_params.setTauIsolationLUT(*tauIsoLUT);

    stringstream tauCalibrationLUTFile;
    tauCalibrationLUTFile<< getenv("CMSSW_BASE") << "/src/" << params.GetValue("tauCalibrationLUTFile", "L1Trigger/L1TCalorimeter/data/tauCalibrationLUT.txt");
    std::ifstream tauCalibrationLUTStream(tauCalibrationLUTFile.str());
    if(!tauCalibrationLUTStream.is_open())
    {
        cout<<"[ERROR] Cannot open " << tauCalibrationLUTFile.str()<<"\n";
        return false;
    }
    std::shared_ptr<l1t::LUT> tauCalibrationLUT( new l1t::LUT(tauCalibrationLUTStream) );
    m_params.setTauCalibrationLUT(*tauCalibrationLUT);


    return true;
}
