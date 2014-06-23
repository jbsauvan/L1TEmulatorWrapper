/**
 *  @file  L1TStage2Wrapper.h
 *  @brief  
 *
 *
 *  @author  Jean-Baptiste Sauvan <sauvan@llr.in2p3.fr>
 *
 *  @date    06/23/2014
 *
 *  @internal
 *     Created :  06/23/2014
 * Last update :  06/23/2014 09:45:30 AM
 *          by :  JB Sauvan
 *
 * =====================================================================================
 */



#ifndef L1TSTAGE2WRAPPER_H
#define L1TSTAGE2WRAPPER_H

#include "L1Trigger/L1TCalorimeter/interface/Stage2Layer2ClusterAlgorithm.h"
#include "L1Trigger/L1TCalorimeter/interface/Stage2Layer2EGammaAlgorithm.h"
#include "L1Trigger/L1TCalorimeter/interface/Stage2Layer2TauAlgorithm.h"

#include "CondFormats/L1TObjects/interface/CaloParams.h"

namespace l1twrapper
{

    class L1TStage2Wrapper
    {
        public:
            L1TStage2Wrapper();
            ~L1TStage2Wrapper();
            bool initialize(const std::string& parameterFile);
            bool process(const std::vector<int>& hwEta, const std::vector<int>& hwPhi,
                            const std::vector<int>& hwEtEm, const std::vector<int>& hwEtHad, 
                            const std::vector<int>& hwEtRatio, const std::vector<int>& hwQual
                    );

            const std::vector<l1t::CaloTower>&   towers()      const {return m_towers;}
            const std::vector<l1t::CaloCluster>& egClusters()  const {return m_egClusters;}
            const std::vector<l1t::CaloCluster>& tauClusters() const {return m_tauClusters;}
            const std::vector<l1t::EGamma>&      egammas()     const {return m_egammas;}
            const std::vector<l1t::Tau>&         taus()        const {return m_taus;}

        private:
            bool fillTowers(const std::vector<int>& hwEta, const std::vector<int>& hwPhi,
                            const std::vector<int>& hwEtEm, const std::vector<int>& hwEtHad, 
                            const std::vector<int>& hwEtRatio, const std::vector<int>& hwQual);
            bool fillParameters(const std::string& parameterFile);

            l1t::CaloParams m_params;

            // producers
            l1t::Stage2Layer2ClusterAlgorithm* m_egClusterAlgo;
            l1t::Stage2Layer2EGammaAlgorithm*  m_egAlgo;
            l1t::Stage2Layer2ClusterAlgorithm* m_tauClusterAlgo;
            l1t::Stage2Layer2TauAlgorithm*     m_tauAlgo;

            // produced objects
            std::vector<l1t::CaloTower>   m_towers;
            std::vector<l1t::CaloCluster> m_egClusters;
            std::vector<l1t::CaloCluster> m_tauClusters;
            std::vector<l1t::EGamma>      m_egammas;
            std::vector<l1t::Tau>         m_taus;
    };

}

#endif
