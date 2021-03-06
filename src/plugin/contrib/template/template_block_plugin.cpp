/******************************************************************************/
/*                                                                            */
/*                             template_block.cpp                             */
/*                                                                            */
/*                        Matching Pursuit Library                            */
/*                                                                            */
/* R�mi Gribonval                                                             */
/* Sacha Krstulovic                                                           */
/* Sylvain Lesage                                             Mon Apr 03 2006 */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Copyright (C) 2005 IRISA                                                  */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or             */
/*  modify it under the terms of the GNU General Public License               */
/*  as published by the Free Software Foundation; either version 2            */
/*  of the License, or (at your option) any later version.                    */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place - Suite 330,                            */
/*  Boston, MA  02111-1307, USA.                                              */
/*                                                                            */
/******************************************************************************/
/*
 * SVN log:
 *
 * $Author: sacha $
 * $Date: 2007-04-24 19:30:55 +0200 (mar., 24 avr. 2007) $
 * $Revision: 1021 $
 *
 */

/***************************************************/
/*                                                 */
/* template_block.cpp: methods for template blocks */
/*                                                 */
/***************************************************/

#include "mptk.h"
#include "mp_system.h"
#include "block_factory.h"
#include "template_atom_plugin.h"
#include "template_block_plugin.h"
#include <sstream>

/* YOUR includes go here. */


/***************************/
/* CONSTRUCTORS/DESTRUCTOR */
/***************************/

/************************/
/* Factory function     */
MP_Block_c* MP_Template_Block_Plugin_c::create( MP_Signal_c *s,
    map<string, string, mp_ltstring> *paramMap )
{
  const char* func = "MP_template_Block_c::init()";
  MP_template_Block_Plugin_c *newBlock = NULL;
  unsigned long int filterLen = 0;
  unsigned long int filterShift =0 ;
  double windowRate =0.0;
  unsigned long int blockOffset = 0;
  unsigned long int yourUnsignedLongInt = 0;
  double yourDouble = 0.0;

  char*  convertEnd;
  /* Instantiate and check */
  newBlock = new MP_template_Block_Plugin_c();
  if ( newBlock == NULL )
    {
      mp_error_msg( func, "Failed to create a new Template block.\n" );
      return( NULL );
    }
    
   /* Analyse the parameter map */

  if (strcmp((*paramMap)["type"].c_str(),"template"))
    {
      mp_error_msg( func, "Parameter map does not define a Template block.\n" );
      return( NULL );
    } 
   
     if ((*paramMap)["windowLen"].size()>0)
    {
      /*Convert window length*/
      filterLen=strtol((*paramMap)["windowLen"].c_str(), &convertEnd, 10);
      if (*convertEnd != '\0')
        {
          mp_error_msg( func, "cannot convert parameter windowLen in unsigned long int.\n" );
          return( NULL );
        }
    }
  else
    {
      mp_error_msg( func, "No parameter windowLen in the parameter map.\n" );
      return( NULL );
    }

  if ((*paramMap)["windowShift"].size()>0)
    {
      /*Convert windowShift*/
      filterShift=strtol((*paramMap)["windowShift"].c_str(), &convertEnd, 10);
      if (*convertEnd != '\0')
        {
          mp_error_msg( func, "cannot convert parameter windowShift in unsigned long int.\n");
          return( NULL );
        }
    }
  else
    {
      if ((*paramMap)["windowRate"].size()>0)
        {
          windowRate =strtod((*paramMap)["windowRate"].c_str(), &convertEnd);
          if (*convertEnd != '\0')
            {
              mp_error_msg( func, "cannot convert parameter windowRate double.\n");
              return( NULL );
            }
          filterShift = (unsigned long int)( (double)(filterLen)*windowRate + 0.5 );
          filterShift = ( filterShift > 1 ? filterShift : 1 ); /* windowShift has to be 1 or more */

        }
      else
        {
          mp_error_msg( func, "No parameter windowShift or windowRate in the parameter map.\n" );
          return( NULL );
        }
    }
 if ((*paramMap)["blockOffset"].size()>0)
    {
      /*Convert blockOffset*/
      blockOffset=strtol((*paramMap)["blockOffset"].c_str(), &convertEnd, 10);
      if (*convertEnd != '\0')
        {
          mp_error_msg( func, "cannot convert parameter windowShift in unsigned long int.\n");
          return( NULL );
        }
    }
    
 if ((*paramMap)["yourUnsignedLongInt"].size()>0)
    {
      /*Convert yourUnsignedLongInt*/
      yourUnsignedLongInt=strtol((*paramMap)["yourUnsignedLongInt"].c_str(), &convertEnd, 10);
      if (*convertEnd != '\0')
        {
          mp_error_msg( func, "cannot convert parameter yourUnsignedLongInt in unsigned long int.\n");
          return( NULL );
        }
    }
     if ((*paramMap)["yourDouble"].size()>0)
    {
      /*Convert windowShift*/
      yourDouble=strtod((*paramMap)["yourDouble"].c_str(), &convertEnd);
      if (*convertEnd != '\0')
        {
          mp_error_msg( func, "cannot convert parameter yourDouble in double.\n");
          return( NULL );
        }
    }
    
  if ( newBlock->init_parameters( filterLen, filterShift,  blockOffset , yourUnsignedLongInt, yourDouble ,.... ) )
    {
      mp_error_msg( func, "Failed to initialize some block parameters in the new Template block.\n" );
      delete( newBlock );
      return( NULL );
    }

  if ( newBlock->init_parameter_map( filterLen, filterShift,  blockOffset , yourUnsignedLongInt, yourDouble ,.... ) )
    {
      mp_error_msg( func, "Failed to initialize block parameter map in the new Template block.\n" );
      delete( newBlock );
      return( NULL );
    }


  /* Set the signal-related parameters */
  if ( newBlock->plug_signal( s ) )
    {
      mp_error_msg( func, "Failed to plug a signal in the new Template block.\n" );
      delete( newBlock );
      return( NULL );
    }

  return ((MP_Block_c*)newBlock);
}

/*********************************************************/
/* Initialization of signal-independent block parameters */
int MP_Template_Block_Plugin_c::init_parameters( const unsigned long int setFilterLen,
    const unsigned long int setFilterShift,
    const unsigned long int setYourUnsignedLongInt,
    const double setYourDouble,
    const unsigned long int setBlockOffset )
{
	 const char* func = "MP_Template_Block_Plugin_c::init_parameters(...)";
	/*Test and initialize your parameters here */

 

  /* Go up the inheritance graph */
  if ( MP_Block_c::init_parameters( setFilterLen, setFilterShift, setFftSize , setBlockOffset ) )
    {
      mp_error_msg( func, "Failed to init the block-level parameters in the new Constant block.\n" );
      return( 1 );
    }

  return( 0 );
}

/*************************************************************/
/* Initialization of signal-independent block parameters map */
int MP_Template_Block_Plugin_c::init_parameter_map( const unsigned long int setFilterLen,
    const unsigned long int setFilterShift,
    const unsigned long int setYourUnsignedLongInt,
    const double setYourDouble,
    const unsigned long int setBlockOffset )
{
const char* func = "MP_Template_Block_Plugin_c::init_parameter_map(...)";

parameterMap = new map< string, string, mp_ltstring>();
   
/*Create a stream for convert number into string */
std::ostringstream oss;

(*parameterMap)["type"] = type_name();

/* put value in the stream */
if (!(oss << setFilterLen)) { 
	  mp_error_msg( func, "Cannot convert windowLen in string for parameterMap.\n" );
      return( 1 );
      }
/* put stream in string */
(*parameterMap)["windowLen"] = oss.str();
/* clear stream */
oss.str("");

if (!(oss << setFilterShift)) { mp_error_msg( func, "Cannot convert windowShift in string for parameterMap.\n" 
                     );
      return( 1 );
      }
(*parameterMap)["windowShift"] = oss.str();
oss.str("");

if (!(oss << setBlockOffset)) { mp_error_msg( func, "Cannot convert blockOffset in string for parameterMap.\n" 
                     );
      return( 1 );
      }
(*parameterMap)["blockOffset"] = oss.str();
oss.str("");

/* Convert your setYourDouble in string */
if (!(oss << setYourDouble)) { mp_error_msg( func, "Cannot convert yourDouble in string for parameterMap.\n" 
                     );
      return( 1 );
      }
(*parameterMap)["yourDouble"] = oss.str();
oss.str("");

/* Convert your setYourUnsignedLongInt in string */
if (!(oss << setYourUnsignedLongInt)) { mp_error_msg( func, "Cannot convert yourUnsignedLongInt in string for parameterMap.\n" 
                     );
      return( 1 );
      }
(*parameterMap)["yourUnsignedLongInt"] = oss.str();
oss.str("");

return (0);
}
/*******************************************************/
/* Initialization of signal-dependent block parameters */
int MP_Template_Block_Plugin_c::plug_signal( MP_Signal_c *setSignal )
{

  const char* func = "MP_Template_Block_Plugin_c::plug_signal( signal )";

  /* Reset any potential previous signal */
  nullify_signal();

  if ( setSignal != NULL )
    {

      /* Go up the inheritance graph */
      if ( MP_Block_c::plug_signal( setSignal ) )
        {
          mp_error_msg( func, "Failed to plug a signal at the block level.\n" );
          nullify_signal();
          return( 1 );
        }

    }

  return( 0 );
}


/**************************************************/
/* Nullification of the signal-related parameters */
void MP_Template_Block_Plugin_c::nullify_signal( void )
{

  MP_Block_c::nullify_signal();

}


/********************/
/* NULL constructor */
MP_Template_Block_Plugin_c::MP_Template_Block_Plugin_c( void )
    :MP_Block_c()
{
  mp_debug_msg( MP_DEBUG_CONSTRUCTION, "MP_Template_Block_Plugin_c::MP_Template_Block_Plugin_c()",
                "Constructing a Constant_block...\n" );
  mp_debug_msg( MP_DEBUG_CONSTRUCTION, "MP_Template_Block_Plugin_c::MP_Template_Block_Plugin_c()",
                "Done.\n" );
}


/**************/
/* Destructor */
MP_Template_Block_Plugin_c::~MP_Template_Block_Plugin_c()
{
  mp_debug_msg( MP_DEBUG_DESTRUCTION, "MP_Template_Block_Plugin_c::~MP_Template_Block_Plugin_c()",
                "Deleting Template_block...\n" );
  mp_debug_msg( MP_DEBUG_DESTRUCTION, "MP_Template_Block_Plugin_c::~MP_Template_Block_Plugin_c()",
                "Done.\n" );
}


/***************************/
/* OTHER METHODS           */
/***************************/

/********/
/* Type */
char* MP_Template_Block_Plugin_c::type_name()
{
  return ("template");
}


/********/
/* Readable text dump */
int  MP_Template_Block_Plugin_c::info( FILE* fid )
{

  int nChar = 0;

  nChar += mp_info_msg( fid, "TMPLATE BLOCK", "window of length [%lu], shifted by [%lu] samples,\n",
                        filterLen, filterShift );
  nChar += mp_info_msg( fid, "         O-", "The number of frames for this block is [%lu], "
                        "the search tree has [%lu] levels.\n", numFrames, numLevels );

  return ( nChar );
}


/********************************************/
/* Frame-based update of the inner products */
void MP_Template_Block_Plugin_c::update_frame(unsigned long int frameIdx,
                                       MP_Real_t *maxCorr,
                                       unsigned long int *maxFilterIdx)
{


}

/***************************************/
/* Output of the ith atom of the block */
unsigned int  MP_Template_Block_Plugin_c::create_atom( MP_Atom_c **atom,
    const unsigned long int frameIdx,
    const unsigned long int /* filterIdx */, MP_Dict_c* dict )
{

  const char* func = "MP_Template_Block_Plugin_c::create_atom(...)";
  MP_Constant_Atom_Plugin_c *datom;
  int chanIdx;
  unsigned long int pos = frameIdx*filterShift + blockOffset;
  unsigned long int t;
  MP_Real_t* pAmp;
  MP_Real_t ip;

  /* Check the position */
  if ( (pos+filterLen) > s->numSamples )
    {
      mp_error_msg( func, "Trying to create an atom out of the support of the current signal."
                    " Returning a NULL atom.\n" );
      *atom = NULL;
      return( 0 );
    }

  /* Allocate the atom */
  *atom = NULL;
  MP_Atom_c* (*emptyAtomCreator)( MP_Dict_c* dict ) = MP_Atom_Factory_c::get_empty_atom_creator("TemplateAtom");
  if (NULL == emptyAtomCreator)
    {
      mp_error_msg( func, "Template atom is not registred in the atom factory" );
      return( 0 );
    }

  if ( (datom =  (MP_Template_Atom_Plugin_c*)(*emptyAtomCreator)(dict))  == NULL )
    {
      mp_error_msg( "MP_Template_Block_c::create_atom(...)", "Can't create a new Template atom in create_atom()."
                    " Returning NULL as the atom reference.\n" );
      return( 0 );
    }

  if ( datom->alloc_atom_param( s->numChans) )
    {
      mp_error_msg( func, "Failed to allocate some vectors in the new Template atom. Returning a NULL atom.\n" );
      return( 0 );

    }
  /* Set the numSamples */
  datom->numSamples = pos + filterLen;
  datom->totalChanLen = 0;

  /* For each channel: */
  for ( chanIdx=0; chanIdx < s->numChans; chanIdx++ )
    {

    }

  *atom = datom;
  return( 1 );
}


/*********************************************/
/* get Paramater type map defining the block */
void MP_Template_Block_Plugin_c::get_parameters_type_map(map< string, string, mp_ltstring> * parameterMapType){

const char * func = "MP_Template_Block_Plugin_c::get_parameters_type_map( map< string, string, mp_ltstring>* parameterMapType )";

if ((*parameterMapType).empty()) {
(*parameterMapType)["type"] = "string";
(*parameterMapType)["windowLen"] = "ulong";
(*parameterMapType)["windowShift"] = "ulong";
(*parameterMapType)["yourUnsignedLongInt"] =  "ulong";
(*parameterMapType)["yourDouble"] =  "real";
(*parameterMapType)["blockOffset"] = "ulong";

} else  mp_error_msg( func, "Map for parameters type wasn't empty.\n" );

}

/***********************************/
/* get Info map defining the block */
void MP_Template_Block_Plugin_c::get_parameters_info_map(map< string, string, mp_ltstring> * parameterMapInfo ){

const char * func = "MP_Template_Block_Plugin_c::get_parameters_info_map( map< string, string, mp_ltstring>* parameterMapInfo )";

if ((*parameterMapInfo).empty()) {
(*parameterMapInfo)["type"] = "type: the type of blocks";
(*parameterMapInfo)["windowLen"] = "windowLen: the length of the signal window, in number of sample";
(*parameterMapInfo)["windowShift"] = "windowShift: the window shift, in number of samples";
(*parameterMapInfo)["windowLen"] = "windowLen: the length of the signal window, in number of sample";
(*parameterMapInfo)["yourUnsignedLongInt"] = "yourUnsignedLongInt: the descritption...";
(*parameterMapInfo)["yourDouble"] = "yourDouble: the descritption...";
(*parameterMapInfo)["blockOffset"] = "blockOffset: the block offset";

} else  mp_error_msg( func, "Map for parameters info wasn't empty.\n" );

}

/***********************************/
/* get default map defining the block */
void MP_Template_Block_Plugin_c::get_parameters_default_map( map< string, string, mp_ltstring>* parameterMapDefault ){

const char * func = "MP_Template_Block_Plugin_c::get_parameters_default_map( map< string, string, mp_ltstring>* parameterMapDefault )";

if ((*parameterMapDefault).empty()) {
(*parameterMapDefault)["type"] = "template";
(*parameterMapDefault)["windowLen"] = "1024";
(*parameterMapDefault)["windowShift"] = "512";
(*parameterMapDefault)["yourUnsignedLongInt"] = "default value in unsigned long int";
(*parameterMapDefault)["yourDouble"] = "default value in double value";
(*parameterMapDefault)["blockOffset"] = "0"; }

 else  mp_error_msg( func, "Map for parameter default wasn't empty.\n" );

}

DLL_EXPORT void registry(void)
{
  MP_Block_Factory_c::register_new_block("template",&MP_Template_Block_Plugin_c::create, &MP_Template_Block_Plugin_c::get_parameters_type_map, &MP_Template_Block_Plugin_c::get_parameters_info_map, &MP_Template_Block_Plugin_c::get_parameters_default_map );
}
