
#include "CPPFORT/HEPEVT.h"

#include <cstdlib>
#include <iostream>
using namespace std ;

using namespace lcio ;

namespace HEPEVTIMPL{

  void HEPEVT::fromHepEvt(LCEvent * evt){

      float* p = new float[3] ;
      // set event number from stdhep COMMON
      int* IEVENT  = &FTNhep.nevhep ;
      LCEventImpl* evtimpl = reinterpret_cast<LCEventImpl*>( (evt) ) ;
      evtimpl->setEventNumber( *IEVENT ) ;

      // create and add mc particles from stdhep COMMON
      LCCollectionVec* mcVec = new LCCollectionVec( LCIO::MCPARTICLE )  ;

      // create a particle instance for each HEPEVT entry

      int* NMCPART = &FTNhep.nhep;
      for(int j=0;j < *NMCPART;j++)
      {
        MCParticleImpl* mcp = new MCParticleImpl ;
        mcp->setPDG( FTNhep.idhep[j] ) ;
        mcp->setHepEvtStatus( FTNhep.isthep[j] ) ;

        // now get parents if required and set daughter if parent1 is defined
        int* parent1 = &FTNhep.jmohep[j][0] ;
        int* parent2 = &FTNhep.jmohep[j][1] ;

        if( *parent1 > 0 )
        {
          MCParticle* mmcp =
          dynamic_cast<MCParticle*>(mcVec->getElementAt( *parent1 - 1 ) ) ;
          mcp->setParent( mmcp ) ;

          if ( FTNhep.jdahep[*parent1-1][0] > 0 )
          {
              LCCollectionVec* col = mcVec ;
              MCParticleImpl* mcporig = dynamic_cast<MCParticleImpl*>( (*col)[*parent1 - 1] ) ;
              mcporig->addDaughter ( mcp ) ;
          }
        }
        if( *parent2 > 0 )
        {
          for(int jp= *parent1+1; jp <= *parent2; jp++)
          {
            MCParticle* mmcp =
            dynamic_cast<MCParticle*>(mcVec->getElementAt( jp - 1 ) ) ;
            mcp->setSecondParent ( mmcp ) ;

            if ( FTNhep.jdahep[jp-1][0] > 0 )
            {
              LCCollectionVec* col = mcVec ;
              MCParticleImpl* mcporig = dynamic_cast<MCParticleImpl*>( (*col)[jp - 1] ) ;
              mcporig->addDaughter ( mcp ) ;
            }
          }
        }


        // now momentum, vertex, charge
        for(int k=0;k<3;k++)  p[k] = (float)FTNhep.phep[j][k];
        mcp->setMomentum( p );
        mcp->setMass( (float)FTNhep.phep[j][4] ) ;
        mcp->setVertex( FTNhep.vhep[j] ) ;

        // finally store pointer and set charge
        FTNhep1.mcpointerv[j] = reinterpret_cast<PTRTYPE>( (mcp) ) ;
        mcp->setCharge( FTNhep1.mcchargev[j] ) ;

        mcVec->push_back( mcp ) ;
      }

      // add all collection to the event
      evt->addCollection( (LCCollection*) mcVec , "MCParticle" ) ;
      delete p ;
  }  // end of fromHepEvt

/* ============================================================================================================= */

  void HEPEVT::toHepEvt(const LCEvent* evt){

      int* kmax = new int ;
      double* maxxyz = new double;


      // set event number in stdhep COMMON
      FTNhep.nevhep = evt->getEventNumber() ;

      // fill MCParticles into stdhep COMMON
      LCCollection* mcVec = evt->getCollection( LCIO::MCPARTICLE )  ;
      FTNhep.nhep = mcVec->getNumberOfElements() ;
      int* NMCPART = &FTNhep.nhep ;

      for(int j=0;j < *NMCPART;j++)
      {

        //for each MCParticle fill hepevt common line
        const MCParticle* mcp = 
        dynamic_cast<const MCParticle*>( mcVec->getElementAt( j ) ) ;

        FTNhep.idhep[j] = mcp->getPDG() ;
        FTNhep.isthep[j] = mcp->getHepEvtStatus() ;

        // store mother indices
        const MCParticle* mcpp  = mcp->getParent() ;
        try{
          for(int jjm=0;jjm < *NMCPART;jjm++)
          {
            if (mcpp  == dynamic_cast<const MCParticle*>(mcVec->getElementAt( jjm )) ){
              FTNhep.jmohep[j][0] = jjm + 1 ;
              break ;
            }
          }
        }catch(exception& e){
          FTNhep.jmohep[j][0] = 0 ;
          FTNhep.jmohep[j][1] = 0 ;
        }
        if (  FTNhep.jmohep[j][0] > 0 )
        {
          try{
            const MCParticle* mcpsp = mcp->getSecondParent() ;
            for(int jjj=0;jjj < *NMCPART;jjj++)
            {
              if (mcpsp  == dynamic_cast<const MCParticle*>(mcVec->getElementAt( jjj )) ){
                FTNhep.jmohep[j][1] = jjj + 1 ;
                break ;
              }
              break ;
            }
          }catch(exception& e){
            FTNhep.jmohep[j][1] = 0 ;
          }
        }


        // store daugther indices
        FTNhep.jdahep[j][0] = 0 ;
        FTNhep.jdahep[j][1] = 0 ;
        int ndaugthers = mcp->getNumberOfDaughters() ;

        if (ndaugthers > 0)
        {
           const MCParticle* mcpd = mcp->getDaughter( 0 ) ;
           for (int jjj=0; jjj < *NMCPART; jjj++)
           {
             const MCParticle* mcpdtest = dynamic_cast<const MCParticle*>(mcVec->getElementAt( jjj )) ;
             if ( mcpd == mcpdtest ) 
             {
               FTNhep.jdahep[j][0] = jjj + 1 ;
               FTNhep.jdahep[j][1] = FTNhep.jdahep[j][0] + ndaugthers -1 ;
               break ;
             }
           }
        }

        // now momentum, energy, and mass
        for(int k=0;k<3;k++)  FTNhep.phep[j][k] = (double)mcp->getMomentum()[k] ;
        FTNhep.phep[j][3] = (double)mcp->getEnergy() ;
        FTNhep.phep[j][4] = (double)mcp->getMass() ;

        // get vertex and production time
        *kmax = 0 ;
        *maxxyz = 0. ;
        for(int k=0;k<3;k++){
          FTNhep.vhep[j][k] = mcp->getVertex()[k] ;
          if ( fabs( FTNhep.vhep[j][k] ) > *maxxyz ){
            *maxxyz = fabs( FTNhep.vhep[j][k] ) ;
            *kmax = k ;
          }
        }
        if ( mcpp != 0 && *maxxyz > 0. )
        {
          FTNhep.vhep[j][3] = (mcp->getVertex()[*kmax] - mcpp->getVertex()[*kmax]) *
                              (mcpp->getEnergy() / mcpp->getMomentum()[*kmax]) ;
        } 
        else
        {
          FTNhep.vhep[j][3] = 0. ;  // no production time for MCParticle
        }

        // finally store pointer and get charge
        FTNhep1.mcpointerv[j] = reinterpret_cast<PTRTYPE>( (mcp) ) ;
        FTNhep1.mcchargev[j] = mcp->getCharge() ;
      }

      delete kmax ;
      delete maxxyz ;
  } // end of toHepEvt

} //namespace HEPEVTIMPL
